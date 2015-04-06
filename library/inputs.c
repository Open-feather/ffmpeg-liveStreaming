#include "platform.h"
#include "decoder.h"
#include "inputs.h"
#include "pthread.h"
#include "libavutil/intreadwrite.h"

int read_frame(struct lsInput *input,  AVPacket *pkt)
{
	int ret = 0;
	int index = 0;
	int16_t size = 0;
	if(input->ic)
	{
		ret = av_read_frame(input->ic, pkt);
		
	}
	else if(input->pb)
	{

		ret = avio_read(input->pb,(unsigned char*)&size, 2);
		size = AV_RB16(&size);
		if(size > input->in_buf_size)
		{
			input->in_buf_size *= 2;
        		input->in_buffer = av_malloc(input->in_buf_size);
		}

		pkt->stream_index  = 0; 
		pkt->size = size;
		pkt->data = input->in_buffer;
		while(size) {
			ret = avio_read(input->pb, pkt->data + index, size);
			if(ret <= 0){
				break;
			}
			size -= ret;
			index += ret;
		}
	}
	return ret;
}
static void *input_thread(void *arg)
{
	struct lsInput *input = arg;
	int ret = 0;
	while(1)
	{
		AVPacket pkt = {0};
		av_init_packet(&pkt);
		ret = read_frame(input, &pkt);
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
					"Unable to send packet to main thread: %d\n",
					ret);

			}
			
			av_free_packet(&pkt);
			av_thread_message_queue_set_err_recv(input->in_thread_queue, ret);
			break;
		}
	}
	return NULL;
}

int configure_input(struct liveStream *ctx, const char *name, struct inputCfg *cfg)
{
	int ret = 0;
	int i = 0;
	char *fmt = NULL;
	AVFormatContext *ic = NULL;

	/** Input index */
	struct lsInput *input;
	struct lsInput *prev_input;
	AVCodecContext *dec_ctx = NULL;
	AVStream *st;
#define CHECK_END if (ret < 0)goto end
	if(IN_WEBCAM == cfg->type )
	{
		fmt = CAM_DRIVER;
	}
	else if ( IN_STREAM == cfg->type)
	{
		fmt = NULL;
	}


	for(i = 0,input = ctx->inputs;i < ctx->nb_input;i++)
	{
		prev_input = input;
		input = input->next;
	}
	input = av_mallocz(sizeof(*ctx->inputs));
	if (!input)
	{
		dinit_decoder(&ic,dec_ctx);
		return -1;
	}
	if(!ctx->inputs)
		ctx->inputs = input;
	else
		prev_input->next = input;

	if ( 1 == cfg->need_decoder)
	{
		ret = init_decoder(&ic,name,fmt);
		CHECK_END;
		ret = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
		st = ic->streams[ret];
		dec_ctx = st->codec;
		input->InFrame  = av_frame_alloc();
		input->need_decoder = 1;
	}
	else
	{
		input->in_buf_size = 1024 * 1024;
		input->in_buffer = av_malloc(input->in_buf_size);
		input->need_decoder = 0;
		/* XXX need to free somewhere */
		ret = avio_open2(&input->pb, name, AVIO_FLAG_READ ,&input->cb, NULL);
		CHECK_END;
	}


	input->next = NULL;
	input->id = ctx->nb_input;
	ctx->nb_input++;
	input->ic = ic;
	input->dec_ctx = dec_ctx;
	input->st = st;
	input->eof_reached = 0;
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

end:
	return ret;
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
		if (in->need_decoder)
		{
			dinit_decoder(&in->ic,in->dec_ctx);
			av_frame_free(&in->InFrame);
		}
		else
		{
			av_free(in->in_buffer);
			avio_close(in->pb);
		}
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
