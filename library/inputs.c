#include "platform.h"
#include "decoder.h"
#include "inputs.h"
#include "pthread.h"
static void *input_thread(void *arg)
{
	struct lsInput *input = arg;
	int ret = 0;
	while(1)
	{
		AVPacket pkt;
		ret = av_read_frame(input->ic, &pkt);
		if (ret == AVERROR(EAGAIN))
		{
			av_usleep(10000);
			continue;
		}
		if (ret < 0)
		{
			av_thread_message_queue_set_err_recv(input->in_thread_queue, ret);
			break;
		}
		av_dup_packet(&pkt);
		ret = av_thread_message_queue_send(input->in_thread_queue, &pkt, 0);
		if (ret < 0)
		{
			if (ret != AVERROR_EOF)
			{
				av_log(input->ic, AV_LOG_ERROR,
					"Unable to send packet to main thread: %s\n",
					av_err2str(ret));

			}
			av_free_packet(&pkt);
			av_thread_message_queue_set_err_recv(input->in_thread_queue, ret);
			break;
		}
	}
	return NULL;
}
int configure_input(struct liveStream *ctx, char *name,enum InputType type)
{
	int ret = 0;
	int i = 0;
	char *fmt = NULL;
	AVFormatContext *ic;

	/** Input index */
	struct lsInput *input;
	struct lsInput *prev_input;
	AVCodecContext *dec_ctx;
	AVStream *st;

	if(IN_WEBCAM == type )
	{
		fmt = CAM_DRIVER;
	}
	ret = init_decoder(&ic,name,fmt);
	if(ret < 0)
	{
		return -1;
	}
#if 0
	ret = av_find_best_stream(ic,AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	st = ic->streams[ret];
	dec_ctx = st->codec;

	for(i = 0,input = ctx->inputs;i < ctx->nb_input;i++)
	{
		prev_input = input;
		input = input->next;
	}
	input= malloc(sizeof(*ctx->inputs));
	if (!input)
	{
		dinit_decoder(&ic,dec_ctx);
		return -1;
	}
	if(!ctx->inputs)
		ctx->inputs = input;
	else
		prev_input->next = input;
	input->next = NULL;
	input->id = ctx->nb_input;
	ctx->nb_input++;
	input->ic = ic;
	input->dec_ctx = dec_ctx;
	input->st = st;
	input->eof_reached = 0;
	input->InFrame  = av_frame_alloc();
	ret = av_thread_message_queue_alloc(&input->in_thread_queue,8, sizeof(AVPacket));
	if(ret < 0)
	{
		dinit_decoder(&ic,dec_ctx);
		free(input);
		if(prev_input)
			prev_input->next = NULL;
		return ret;
	}
	ret = pthread_create(&input->thread, NULL, input_thread, input);
	if(ret)
	{
		dinit_decoder(&ic,dec_ctx);
		free(input);
		if(prev_input)
			prev_input->next = NULL;
		return -1;
	}
#endif
	return 0;
}

void dinit_inputs(struct lsInput **input,int *nb)
{
	struct lsInput *in = *input;
	struct lsInput *sin = *input;
	if (NULL == *input)
		return;
	while(in)
	{
		AVPacket pkt;
		if (in->in_thread_queue)
		{
			av_thread_message_queue_set_err_send(in->in_thread_queue, AVERROR_EOF);
			while (av_thread_message_queue_recv(in->in_thread_queue, &pkt, 0) >= 0)
				av_free_packet(&pkt);
			pthread_join(in->thread, NULL);
			av_thread_message_queue_free(&in->in_thread_queue);
		}
		dinit_decoder(&in->ic,in->dec_ctx);
		av_frame_free(&in->InFrame);
		sin = in->next;
		free(in);
		in = NULL;
		in = sin;
	}
	*input = NULL;
}
struct lsInput *get_input_by_id(struct lsInput *input,int id)
{
	for(;input;input = input->next)
		if(input->id == id)
			return input;
	return NULL;
}
