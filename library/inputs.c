#include "decoder.h"
#include "inputs.h"

int configure_input(struct liveStream *ctx, char *name,enum InputType type)
{
	int ret = 0;
	char *fmt;
	AVFormatContext *ic;
	/** Input index */
	int in_idx;
	struct lsInput *input;
	AVCodecContext *dec_ctx;
	AVStream *st;

	if(IN_WEBCAM == type )
	{
		if (name == NULL)
			name = CAM_DEVICE_NAME;
		fmt = CAM_DRIVER;

	}
	ret = init_decoder(&ic,name,fmt);
	if(ret < 0)
	{
		return -1;
	}

	ret = av_find_best_stream(ic,AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	st = ic->streams[ret];
	dec_ctx = st->codec;

	in_idx = ctx->nb_input;

	input = realloc(ctx->inputs,(in_idx + 1) * sizeof(*ctx->inputs));
	if (!input)
	{
		dinit_decoder(&ic,dec_ctx);
		return -1;
	}
	ctx->nb_input++;
	ctx->inputs = input;
	input = &ctx->inputs[in_idx];
	input->ic = ic;
	input->dec_ctx = dec_ctx;
	input->st = st;

	return 0;
}
void dinit_inputs(struct lsInput **in,int *nb)
{
	while(*nb)
	{
		dinit_decoder(&(*in)->ic,(*in)->dec_ctx);
		(*nb)--;
	}
	*in = NULL;
}
