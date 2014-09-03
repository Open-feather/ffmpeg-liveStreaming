#include "stream.h"
#include "inputs.h"

void dinit_filters(struct liveStream *ctx)
{
	avfilter_graph_free(&ctx->filter_graph);
	sem_close(&ctx->filter_lock);
}

int configure_input_filter(struct liveStream *ctx, long in_id, AVFilterInOut *in)
{
	AVFilter *buffer = avfilter_get_by_name("buffer");
	struct lsInput *input = NULL;
	AVStream *st;
	AVCodecContext *dec_ctx;
	AVRational tb;
	AVBPrint args;
	int ret = 0;
	char name[128];

	input = get_input_by_id(ctx->inputs,in_id);
	if(!input)
	{
		av_log(NULL,AV_LOG_ERROR,"Invalid input id for inputs list\n");
		return -1;
	}
	st = input->st;
	dec_ctx = input->dec_ctx;
	tb = st->time_base;

	av_bprint_init(&args, 0, 1);
	/* buffer video source: the decoded frames from the decoder will be inserted here. */
	av_bprintf(&args,"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
			tb.num, tb.den,
			dec_ctx->sample_aspect_ratio.num,
			dec_ctx->sample_aspect_ratio.den);
	snprintf(name, sizeof(name), "Input %ld", in_id);
	ret = avfilter_graph_create_filter(&input->in_filter, buffer, name, args.str, NULL, ctx->filter_graph);
	if(ret < 0)
		return ret;

	if ((ret = avfilter_link(input->in_filter, 0, in->filter_ctx, in->pad_idx)) < 0)
		return ret;


	return ret;
}
int configure_output_filter(struct liveStream *ctx, AVFilterInOut *out)
{
	int ret = 0;
	AVFilterContext *filter;
	int pad_idx = out->pad_idx;
	AVFilterContext *last_filter = out->filter_ctx;

	ret = avfilter_graph_create_filter(&ctx->out_filter,avfilter_get_by_name("buffersink"),"out", NULL, NULL, ctx->filter_graph);
	if(ret < 0)
		return ret;
	ret = avfilter_graph_create_filter(&filter,avfilter_get_by_name("format"),"format", "yuv420p", NULL, ctx->filter_graph);
	if(ret < 0)
		return ret;

	 if ((ret = avfilter_link(last_filter, pad_idx, filter, 0)) < 0)
		return ret;

	last_filter = filter;
	pad_idx = 0;
	if((ret = avfilter_link(last_filter, pad_idx, ctx->out_filter, 0)) < 0)
		return ret;

	return ret;

}

int take_filter_lock(sem_t *sem)
{
	struct timespec ts;
	int ret;
	if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
	{
		av_log(NULL,AV_LOG_ERROR,"clock_gettime failed\n");
	}
	/* lock time set to 5 second */
	ts.tv_sec += 5;

	while ((ret = sem_timedwait(sem, &ts)) == -1)
	{
		if (errno == EINTR)
			continue;       /* Restart if interrupted by handler */
		else
		{
			av_log(NULL,AV_LOG_ERROR,"locks messed up\n");
			break;
		}
	}
	return ret;

}
int give_filter_lock(sem_t *sem)
{
	return sem_post(sem);
}
int configure_filter(struct liveStream *ctx)
{
	int ret = 0;
	int i = 0;
	/** Input Id */
	long in_id;
	AVFilterInOut *outputs;
	AVFilterInOut *inputs;
	AVFilterInOut *cur;

	take_filter_lock(&ctx->filter_lock);
	avfilter_graph_free(&ctx->filter_graph);
	ctx->filter_graph = avfilter_graph_alloc();
	if (NULL == ctx->filter_graph)
		return -1;

	ret = avfilter_graph_parse2(ctx->filter_graph, ctx->graph_desc.str, &inputs, &outputs);
	if(ret < 0)
		return ret;

	for (i = 0,cur = inputs; cur; cur = cur->next,i++)
	{
		in_id = strtol(cur->name,NULL,0);
		if(in_id < 0 || in_id >= ctx->nb_input)
		{
			/** Invalid index of video provided */
			ret = -1;
			break;
		}
		configure_input_filter(ctx, in_id, cur);
	}
	avfilter_inout_free(&inputs);
	ret = configure_output_filter(ctx,outputs);
	if(ret < 0)
	{
		printf("unable to configure output filter\n");
		return ret;
	}
	avfilter_inout_free(&outputs);
	if ((ret = avfilter_graph_config(ctx->filter_graph, NULL)) < 0)
		return ret;
	give_filter_lock(&ctx->filter_lock);

	return 0;
}
int init_filters(struct liveStream *ctx)
{
	int ret = 0;

	if (sem_init(&ctx->filter_lock, 0, 1) == -1)
	{
		av_log(NULL,AV_LOG_ERROR,"Unable to init filter locks\n");
	}
	av_bprint_init(&ctx->graph_desc, 0, 1);
	av_bprintf(&ctx->graph_desc, "[0]format=yuv420p,scale=%d:%d",STREAM_WIDTH,STREAM_HEIGHT);

	ret = configure_filter(ctx);
	if(ret <  0)
	{
		printf("unable to configure filter\n");
	}

	return 0;
}
