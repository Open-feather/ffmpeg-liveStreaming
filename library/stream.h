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
#include <libavcodec/avcodec.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avstring.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libavutil/audio_fifo.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
#include <libavutil/threadmessage.h>
#include <libavutil/time.h>
#include <libavutil/bprint.h>
#ifdef __cplusplus
}
#endif

#include <pthread.h>
#include <semaphore.h>
struct lsInput
{
	int id;
	/** Frame rate */
	AVRational fr;
	/** Time base */
	AVFormatContext	*ic;
	AVCodecContext *dec_ctx;
	AVStream *st;
	AVFilterContext *in_filter;
	pthread_t thread;
	AVThreadMessageQueue *in_thread_queue;
	AVFrame *InFrame;
	int eof_reached;
	struct lsInput *next;
};
struct liveStream
{
	struct lsInput *inputs;
	int nb_input;
	AVCodecContext  *pCodecCtx;
	AVCodec * pCodec;
	AVFrame *OutFrame;
	AVFrame *frame;
	AVFormatContext *oc;
	AVRational video_avg_frame_rate;
	AVCodecContext *dec_ctx;
	AVFilterGraph *filter_graph;
	AVBPrint graph_desc;
	AVFilterContext *out_filter;
	long long cur_pts;
	long long dts;
	long long sync_out_pts;
	sem_t filter_lock;
	struct timespec lock_time;
};

#define STREAM_HEIGHT 480
#define STREAM_WIDTH 640
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P
#endif
