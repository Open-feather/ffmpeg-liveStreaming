#ifndef _PLAY_WEBCAM
#define _PLAY_WEBCAM

#include "platform.h"
#ifdef __cplusplus
extern "C" {
#endif
#define __STDC_CONSTANT_MACROS
#include <libavformat/avformat.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avstring.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
}
#endif


struct webPlay{

	AVFormatContext	*ic;
	AVCodecContext  *pCodecCtx;
	AVCodec * pCodec;
	AVFrame *frame;
	AVFormatContext *oc;
	AVStream *input_stream;
	AVCodecContext *dec_ctx;
	AVFilterGraph *filter_graph;
	AVFilterContext *buffersink;
	AVFilterContext *buffersrc;
	long long cur_pts;

};


#ifdef __cplusplus
extern "C" {
#endif
int cap_stream(const char*path);
#ifdef __cplusplus
}
#endif
#endif




