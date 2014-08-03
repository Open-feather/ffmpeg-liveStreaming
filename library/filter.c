#include "stream.h"
#include "libavutil/bprint.h"

void dinit_filters(struct liveStream *ctx)
{
	avfilter_graph_free(&ctx->filter_graph);
}

int configure_input_filter(struct liveStream *ctx, long in_idx, AVFilterInOut *in)
{
	AVFilter *buffer = avfilter_get_by_name("buffer");
	struct lsInput *input = &ctx->inputs[in_idx];
	AVStream *st = input->st;
	AVCodecContext *dec_ctx = input->dec_ctx;
	AVRational tb = st->time_base;
	AVBPrint args;
	int ret = 0;
	char name[128];

	av_bprint_init(&args, 0, 1);
	/* buffer video source: the decoded frames from the decoder will be inserted here. */
	av_bprintf(&args,"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
			tb.num, tb.den,
			dec_ctx->sample_aspect_ratio.num,
			dec_ctx->sample_aspect_ratio.den);
	snprintf(name, sizeof(name), "Input %ld", in_idx);
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
static int configure_filter(struct liveStream *ctx, char *graph_desc)
{
	int ret = 0;
	int i = 0;
	long in_idx;
	AVFilterInOut *outputs;
	AVFilterInOut *inputs;
	AVFilterInOut *cur;

	avfilter_graph_free(&ctx->filter_graph);
	ctx->filter_graph = avfilter_graph_alloc();
	if (NULL == ctx->filter_graph)
		return -1;

	ret = avfilter_graph_parse2(ctx->filter_graph, graph_desc, &inputs, &outputs);
	if(ret < 0)
		return ret;

	for (i = 0,cur = inputs; cur; cur = cur->next,i++)
	{
		in_idx = strtol(inputs->name,NULL,0);
		if(in_idx < 0 || in_idx >= ctx->nb_input)
		{
			/** Invalid index of video provided */
			ret = -1;
			break;
		}
		configure_input_filter(ctx, in_idx, cur);
	}
	ret = configure_output_filter(ctx,outputs);
	if(ret < 0)
	{
		printf("unable to configure output filter\n");
		return ret;
	}

	return 0;
}
int init_filters(struct liveStream *ctx)
{
	char fd_args[512];
	int len = 0;
	int ret = 0;

	//len += snprintf(fd_args, sizeof(fd_args), "[0]format=yuv420p,scale=%d:%d; [1] scale=%d:%d",STREAM_WIDTH,STREAM_HEIGHT,100,100);
	len += snprintf(fd_args, sizeof(fd_args), "[0]format=yuv420p,scale=%d:%d",STREAM_WIDTH,STREAM_HEIGHT);

	ret = configure_filter(ctx,fd_args);
	if(ret <  0)
	{
		printf("unable to configure filter\n");
	}
	if ((ret = avfilter_graph_config(ctx->filter_graph, NULL)) < 0)
		return ret;

	return 0;
}
