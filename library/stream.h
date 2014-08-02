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
	AVFrame *InFrame;
	AVFrame *OutFrame;
	AVFrame *frame;
	AVFormatContext *oc;
	AVRational video_avg_frame_rate;
	AVCodecContext *dec_ctx;
	AVFilterGraph *filter_graph;
	AVFilterContext *buffersink;
	AVFilterContext *buffersrc;
	long long cur_pts;
	long long dts;
	long long sync_out_pts;

};

#endif




