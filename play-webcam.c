
/*

	Play web cam using ffmpeg

*/

#ifdef __cplusplus
extern "C" {
#endif
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


/*------------------------------------------------------------

	H264 Encoder 

------------------------------------------------------------*/
#define STREAM_HEIGHT 480
#define STREAM_WIDTH 640
#define STREAM_FRAME_RATE 30
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P
//#define STREAM_PIX_FMT    AV_PIX_FMT_YUYV422

struct webPlay{

	AVFormatContext	*pFormatCtx;
	AVCodecContext  *pCodecCtx;
	AVCodec * pCodec;
	AVInputFormat *inputFormat;
	AVDictionary *options;
	AVFrame *frame;
	AVFormatContext *oc;
	AVStream *input_stream;
	AVCodecContext *dec_ctx;
	AVFilterGraph *filter_graph;
        AVFilterContext *buffersink;
        AVFilterContext *buffersrc;

};

static int video_stream_idx = -1;
static uint8_t *video_dst_data[4] = {NULL};
static int video_dst_linesize[4];
static int video_dst_bufsize;
static void dinit_filters(struct webPlay *ctx)
{
        avfilter_graph_free(&ctx->filter_graph);
}
static int init_filters(struct webPlay *ctx)
{
        char args[512];
        char fd_args[512];

        int ret = 0;
        int len = 0;

        AVFilter *buffersrc = avfilter_get_by_name("buffer");
        AVFilter *buffersink = avfilter_get_by_name("buffersink");
        AVFilterInOut *outputs = avfilter_inout_alloc();
        AVFilterInOut *inputs = avfilter_inout_alloc();
        AVBufferSinkParams *buffersink_params;
        enum AVPixelFormat pix_fmts[] =
        { AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE };


        len += snprintf(fd_args, sizeof(fd_args), "format=yuv420p,scale=640:480");

        ctx->filter_graph = avfilter_graph_alloc();
        /* buffer video source: the decoded frames from the decoder will be inserted here. */
        snprintf(args, sizeof(args),
                        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                        ctx->dec_ctx->width, ctx->dec_ctx->height, ctx->dec_ctx->pix_fmt,
                        ctx->dec_ctx->time_base.num, ctx->dec_ctx->time_base.den,
                        ctx->dec_ctx->sample_aspect_ratio.num,
                        ctx->dec_ctx->sample_aspect_ratio.den);

        ret = avfilter_graph_create_filter(&ctx->buffersrc, buffersrc, "in", args,
        NULL, ctx->filter_graph);
        if (ret < 0)
        {
                av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
                return ret;
        }
        /* buffer video sink: to terminate the filter chain. */
        buffersink_params = av_buffersink_params_alloc();
        buffersink_params->pixel_fmts = pix_fmts;
        ret = avfilter_graph_create_filter(&ctx->buffersink, buffersink, "out",
        NULL, buffersink_params, ctx->filter_graph);
        av_free(buffersink_params);
        if (ret < 0)
        {
                av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
                return ret;
        }

        /* Endpoints for the filter graph. */
        outputs->name = av_strdup("in");
        outputs->filter_ctx = ctx->buffersrc;
        outputs->pad_idx = 0;
        outputs->next = NULL;

        inputs->name = av_strdup("out");
        inputs->filter_ctx = ctx->buffersink;
        inputs->pad_idx = 0;
        inputs->next = NULL;

        if ((ret = avfilter_graph_parse_ptr(ctx->filter_graph,fd_args,
                        &inputs, &outputs, NULL)) < 0)
                return ret;

        if ((ret = avfilter_graph_config(ctx->filter_graph, NULL)) < 0)
                return ret;
        return 0;
}

/* Add an output stream. */
static AVStream *
add_stream(AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id)
{
        AVCodecContext *c = NULL;
        AVStream *st = NULL;

        /* find the encoder */
        *codec = avcodec_find_encoder(codec_id);
        if (!(*codec))
        {
                av_log(NULL, AV_LOG_ERROR, "Could not find encoder for '%s'\n",
                                avcodec_get_name(codec_id));
                return NULL;
        }

        st = avformat_new_stream(oc, *codec);
        if (!st)
        {
                av_log(NULL, AV_LOG_ERROR, "Could not allocate stream\n");
                return NULL;
        }
        st->id = oc->nb_streams - 1;
        c = st->codec;

        switch ((*codec)->type)
        {
        case AVMEDIA_TYPE_AUDIO:
                c->sample_fmt =
                                (*codec)->sample_fmts ?
                                                (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;

                c->bit_rate = 128000;
                c->sample_rate = 48000;
                c->channels = 2;
                break;

        case AVMEDIA_TYPE_VIDEO:
                c->codec_id = codec_id;

                c->bit_rate = 400000;
                /* Resolution must be a multiple of two. */
                c->width = STREAM_WIDTH;
                c->height = STREAM_HEIGHT;
                /* timebase: This is the fundamental unit of time (in seconds) in terms
                 * of which frame timestamps are represented. For fixed-fps content,
                 * timebase should be 1/framerate and timestamp increments should be
                 * identical to 1. */
                c->time_base.den = STREAM_FRAME_RATE;
                c->time_base.num = 1;
                c->gop_size = 12; /* emit one intra frame every twelve frames at most */
                c->pix_fmt = STREAM_PIX_FMT;
                if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
                {
                        /* Needed to avoid using macroblocks in which some coeffs overflow.
                         * This does not happen with normal video, it just happens here as
                         * the motion of the chroma plane does not match the luma plane. */
                        c->mb_decision = 2;
                }
                break;

        default:
                break;
        }

        /* Some formats want stream headers to be separate. */
        if (oc->oformat->flags & AVFMT_GLOBALHEADER)
                c->flags |= CODEC_FLAG_GLOBAL_HEADER;

        return st;
}
static void dinit_encoder(AVFormatContext **oc)
{
        AVOutputFormat *fmt;
        AVStream *st;
        AVFormatContext *loc = *oc;
        fmt = loc->oformat;

        av_write_trailer(loc);
        /* Close each codec. */
        for (unsigned int i = 0; i < loc->nb_streams; i++)
        {
                st = loc->streams[i];
                avcodec_close(st->codec);
        }

        if (!(fmt->flags & AVFMT_NOFILE))
        {
                /* Close the output file. */
                avio_close(loc->pb);
        }

        avformat_free_context(loc);
}

static int init_encoder(AVFormatContext **oc, const char* oname)
{
        int ret = 0;
        char arr_string[128] = "";
        AVOutputFormat *fmt;
        AVCodec *audio_codec, *video_codec;
        AVStream *vst = NULL;
        AVStream *ast = NULL;


        /* allocate the output media context */
        avformat_alloc_output_context2(oc, NULL, NULL, oname);
        if (!*oc)
        {
                av_log(NULL, AV_LOG_ERROR, "Could not deduce output format\n");
                return -1;
        }
        AVFormatContext *loc = *oc;
        fmt = loc->oformat;
        if (fmt->video_codec != AV_CODEC_ID_NONE)
        {
                vst = add_stream(loc, &video_codec, fmt->video_codec);
        }

        if (fmt->audio_codec != AV_CODEC_ID_NONE)
        {
                ast = add_stream(loc, &audio_codec, fmt->audio_codec);
        }
        if (!ast && !vst)
        {
                return -1;
        }

	if(vst)
	{
		/* open the codec */
		ret = avcodec_open2(vst->codec, video_codec, NULL);
		if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "Could not open video codec: %s\n",
					av_make_error_string(arr_string, 128, ret));
			return -1;
		}
	}
	if(ast)
	{
		ast->codec->channel_layout = 3;
		/* open the audio codec */
		ret = avcodec_open2(ast->codec, audio_codec, NULL);
		if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "Could not open video codec: %s\n",
					av_make_error_string(arr_string, 128, ret));
			return -1;
		}
	}
        /* open the output file, if needed */
        if (!(fmt->flags & AVFMT_NOFILE))
        {
                ret = avio_open(&loc->pb, oname, AVIO_FLAG_WRITE);
                if (ret < 0)
                {
                        av_log(NULL, AV_LOG_ERROR, "Could not open '%s': %s\n", oname,
                                        av_make_error_string(arr_string, 128, ret));
                        //xxx decide kya karna hai
                        return -1;
                }
        }
	av_dump_format(loc, 0, "output", 1);
        /* Write the stream header, if any. */
        ret = avformat_write_header(loc, NULL);
        if (ret < 0)
        {
                av_log(NULL, AV_LOG_ERROR, "Error occurred when writing header: %s\n",
                                av_make_error_string(arr_string, 128, ret));
                dinit_encoder(oc);
        }

        return ret;
}

/**
 * xxx we do not support B frame videos
 */
static int write_video_frame(AVFormatContext *oc, AVStream *st, AVFrame *frame)
{
        int ret = 0; 
        AVCodecContext *c = st->codec;
        int got_packet = 0; 
        char arr_string[128];
        AVPacket pkt; 
        av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0; 
        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, frame, &got_packet);
        if (ret < 0) 
        {    
                av_log(NULL, AV_LOG_ERROR, "Error encoding video frame: %s\n",
                                av_make_error_string(arr_string, 128, ret));
                return -1;
        }    
        /* If size is zero, it means the image was buffered. */
        if (!ret && got_packet && pkt.size)
        {    
                pkt.stream_index = st->index;

                /* Write the compressed frame to the media file. */
                ret = av_interleaved_write_frame(oc, &pkt);
        }    
        else 
        {    
                ret = 0; 
        }    
        if (ret != 0)
        {    
                av_log(NULL, AV_LOG_ERROR, "Error while writing video frame: %s\n",
                                av_make_error_string(arr_string, 128, ret));
                return -1;
        }    
        return -1;
}

int h264_encoder_init( struct webPlay* stWebPlay)
{

	int codecId = AV_CODEC_ID_H264;
	int ret = 0;
	//avcodec_init();
	//avcodec_register_all();
	stWebPlay->pCodec = avcodec_find_encoder(CODEC_ID_H264);
	if(!stWebPlay->pCodec)
	{
		fprintf(stderr,"Error : in finding encoder for H264\n");
		return -1;
	}	
	

	stWebPlay->pCodecCtx = avcodec_alloc_context3(stWebPlay->pCodec);	
	if(!stWebPlay->pCodecCtx)
	{
		fprintf(stderr,"Error : in codec context allocation\n");
		return -1;
	}
	
	stWebPlay->pCodecCtx->bit_rate  = 400000;       // bitrate 400kbps
	stWebPlay->pCodecCtx->width     = 640;          // video width
	stWebPlay->pCodecCtx->height    = 480;          // video height
	stWebPlay->pCodecCtx->time_base = (AVRational){1,25};  // framerate
	stWebPlay->pCodecCtx->gop_size  = 10;           // Intra frame
	stWebPlay->pCodecCtx->max_b_frames = 1;	
	stWebPlay->pCodecCtx->pix_fmt      = AV_PIX_FMT_YUV420P; 
	
	// only for h264
	//av_opt_set(stWebPlay->pCodecCtx->priv_data, "preset", "slow", 0);

	if(avcodec_open2(stWebPlay->pCodecCtx, stWebPlay->pCodec, NULL) < 0)
	{
		fprintf(stderr,"Error in open codec \n");
		return -1;
	}
	
	stWebPlay->frame = avcodec_alloc_frame();	
	if(!stWebPlay->frame)
	{
		fprintf(stderr,"Error in avcodec alloc frame\n");
		return -1;
	}

	stWebPlay->frame->format  = stWebPlay->pCodecCtx->pix_fmt;
	stWebPlay->frame->width   = stWebPlay->pCodecCtx->width;
	stWebPlay->frame-> height = stWebPlay->pCodecCtx->height;

	ret = av_image_alloc( stWebPlay->frame->data, stWebPlay->frame->linesize, stWebPlay->pCodecCtx->width, stWebPlay->pCodecCtx->height,stWebPlay->pCodecCtx->pix_fmt, 32);
        if (ret < 0) 
	{
		fprintf(stderr,"Error in image allocation\n");
		return -1;
        }
	printf("Encoder init complete\n");
	return 0;
}


/*----------------------------------------------------------
	
	Main
	
------------------------------------------------------*/

int main()
{
	
	int ret = 0, i = 0, numBytes = 0;
	char *buffer;
	FILE* tempFile;
	int vid_size = 0;
	AVStream *st;
	AVCodecContext *dec_ctx = NULL;
	AVCodec *dec = NULL;
	AVPacket packet;

	// structure with ffmpeg variables
	struct webPlay *stWebPlay;

	// allocation of webplay structure
	stWebPlay = malloc(sizeof(struct webPlay));
	if(stWebPlay == NULL)
	{
		fprintf(stderr,"Error in web play struct alloc\n");
		return -1;
	}

	// register all the codec
	av_register_all();
	// register all the codec
        avcodec_register_all();
	// register all the devices
	avdevice_register_all();
	// register all filters
	avfilter_register_all();

	// get the camera input format form v4l2
	stWebPlay->inputFormat = av_find_input_format("v4l2");

	// Temp file to dump the video output of camera
	tempFile = fopen("cameradump.yuv", "w");
	if(tempFile == NULL)
	{
		fprintf(stderr,"ERROR in opening output file\n");
		return -1;
	}	
        // set avdict option
	stWebPlay->options = NULL;
	// allocate contex
	stWebPlay->pFormatCtx = avformat_alloc_context();
	if(stWebPlay->pFormatCtx == NULL)
	{
		fprintf(stderr,"could not allocate avformat\n");
		return -1;
	}

	// set the frame rate
	av_dict_set(&stWebPlay->options, "framerate", "5", 0); 
	// open the camera to get the data
	ret = avformat_open_input(&stWebPlay->pFormatCtx, "/dev/video0", stWebPlay->inputFormat, &stWebPlay->options);
	if(ret == -1)
	{
		fprintf(stderr,"Error in avformat open input ret : %d\n", ret);
		return -1;
	}

	// get the camera stream information
	if(avformat_find_stream_info(stWebPlay->pFormatCtx,NULL) < 0)   
        {
                fprintf(stderr,"Error in finding stream infon");
                return -13;
        }
	ret = av_find_best_stream(stWebPlay->pFormatCtx,AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0); 
	if (ret < 0)
	{
		fprintf(stderr, "Could not find video stream in camera\n");
		return ret;
	}
	st = stWebPlay->pFormatCtx->streams[ret];
	dec_ctx = st->codec;
	dec = avcodec_find_decoder(dec_ctx->codec_id);
	ret = avcodec_open2(dec_ctx, dec,NULL);
	if (ret < 0)
	{
		fprintf(stderr, "Could not find video stream in camera\n");
		return ret;
	}
	stWebPlay->dec_ctx = dec_ctx;
	init_filters(stWebPlay);
	/* allocate image where the decoded image will be put */
	ret = av_image_alloc(video_dst_data, video_dst_linesize,
		dec_ctx->width, dec_ctx->height,
		dec_ctx->pix_fmt, 1); 
	if (ret < 0)
	{
		fprintf(stderr, "Could not allocate raw video buffer\n");
		return ret;
        }   
        video_dst_bufsize = ret;

	// dump the camera stream information
	av_dump_format(stWebPlay->pFormatCtx, 0, "/dev/video0", 0);	

	init_encoder(&stWebPlay->oc,"some.h264");
	ret = h264_encoder_init(stWebPlay);
	if(ret < 0)
	{
		printf("Error in encoder init\n");
		return -1;
	}
#if 0
	int res;
        int frameFinished;
	
	stWebPlay->frame->data = malloc(vid_size);
	if(stWebPlay->frame->data == NULL)
	{
		printf("Error in data allocation\n");
		return -1;
	}
#endif
	long long size = 0;
	AVFrame *InFrame = av_frame_alloc();
	AVFrame *OutFrame = av_frame_alloc();
	int got_frame;
        while(ret = av_read_frame(stWebPlay->pFormatCtx,&packet)>=0)
	{
		ret = avcodec_decode_video2(dec_ctx, InFrame, &got_frame,
				&packet);
		if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "Error decoding video\n");
			return ret;
		}
		if (av_buffersrc_add_frame_flags(stWebPlay->buffersrc, InFrame,
					AV_BUFFERSRC_FLAG_KEEP_REF) < 0)
		{
			av_log(NULL, AV_LOG_ERROR,
					"Error while feeding the filtergraph\n");
		}

		/* pull filtered frames from the filtergraph */
		while (1)
		{

			ret = av_buffersink_get_frame(stWebPlay->buffersink, OutFrame);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			{
				ret = 0;
				break;
			}
			if (ret < 0)
			{
				av_log(NULL, AV_LOG_ERROR, "nothing in buffer sink\n");
				ret = -1;
				break;
			}
			write_video_frame(stWebPlay->oc,stWebPlay->oc->streams[0],OutFrame);

			//	fwrite(packet.data, 1,packet.size, tempFile);

			//	memcpy(stWebPlay->frame->data, packet.data, vid_size);
			stWebPlay->frame->pts = packet.pts;
			//		printf("Encode video here to h264\n");
			if(size > (1024 * 1024 * 1024))
				break;
			size += packet.size;
			printf("getting frames\n");
			//h264_encoder(stWebPlay);
		}

		//break;
			
	}
	fclose(tempFile);
}
