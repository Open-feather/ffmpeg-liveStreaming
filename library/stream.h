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

struct lsInput
{
	char *name;
	/** Frame rate */
	AVRational fr;
	/** Time base */
	AVFormatContext	*ic;
	AVCodecContext *dec_ctx;
	AVStream *st;
	AVFilterContext *in_filter;
};
struct liveStream
{
	struct lsInput *inputs;
	int nb_input;
	AVCodecContext  *pCodecCtx;
	AVCodec * pCodec;
	AVFrame *InFrame;
	AVFrame *OutFrame;
	AVFrame *frame;
	AVFormatContext *oc;
	AVRational video_avg_frame_rate;
	AVCodecContext *dec_ctx;
	AVFilterGraph *filter_graph;
	AVFilterContext *out_filter;
	long long cur_pts;
	long long dts;
	long long sync_out_pts;

};

#define STREAM_HEIGHT 480
#define STREAM_WIDTH 640
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P
#endif
