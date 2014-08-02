
/*

	Play web cam using ffmpeg

*/

#include "stream.h"
#include "decoder.h"
#define STREAM_HEIGHT 480
#define STREAM_WIDTH 640
#define STREAM_PIX_FMT    AV_PIX_FMT_YUV420P

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
	AVRational tb = ctx->ic->streams[0]->time_base;

	AVFilter *buffersrc = avfilter_get_by_name("buffer");
	AVFilter *buffersink = avfilter_get_by_name("buffersink");
	AVFilterInOut *outputs = avfilter_inout_alloc();
	AVFilterInOut *inputs = avfilter_inout_alloc();
	AVBufferSinkParams *buffersink_params;
	enum AVPixelFormat pix_fmts[] =
	{ AV_PIX_FMT_GRAY8, AV_PIX_FMT_NONE };


	len += snprintf(fd_args, sizeof(fd_args), "format=yuv420p,scale=%d:%d",STREAM_WIDTH,STREAM_HEIGHT);

	ctx->filter_graph = avfilter_graph_alloc();
	/* buffer video source: the decoded frames from the decoder will be inserted here. */
	snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			ctx->dec_ctx->width, ctx->dec_ctx->height, ctx->dec_ctx->pix_fmt,
			tb.num, tb.den,
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

void init_ffmpeg(void)
{
	// register all common ffmpeg things
	av_register_all();
	// register all the codec
	avcodec_register_all();
	// register all the devices
	avdevice_register_all();
	// register all filters
	avfilter_register_all();
}


/* Add an output stream. */
static AVStream *add_webcam_stream(struct webPlay *web_ctx, AVCodec **codec, enum AVCodecID codec_id)
{
        AVCodecContext *c = NULL;
        AVStream *st = NULL;
	AVFormatContext *oc = web_ctx->oc;

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

        c->codec_id = codec_id;

        /* Resolution must be a multiple of two. */
        c->width = STREAM_WIDTH;
        c->height = STREAM_HEIGHT;
        /* timebase: This is the fundamental unit of time (in seconds) in terms
         * of which frame timestamps are represented. For fixed-fps content,
         * timebase should be 1/framerate and timestamp increments should be
         * identical to 1.
	 * Timebase = 1/frame rate therfore den = num
         */
	st->avg_frame_rate.den = web_ctx->video_avg_frame_rate.den;
	st->avg_frame_rate.num = web_ctx->video_avg_frame_rate.num;
        st->time_base.den = web_ctx->video_avg_frame_rate.num;
        st->time_base.num = web_ctx->video_avg_frame_rate.den;
        c->time_base.den = web_ctx->video_avg_frame_rate.num;
        c->time_base.num = web_ctx->video_avg_frame_rate.den;
        c->gop_size = web_ctx->video_avg_frame_rate.num; /* emit one intra frame every twelve frames at most */
        c->pix_fmt = STREAM_PIX_FMT;
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO)
        {
            /* Needed to avoid using macroblocks in which some coeffs overflow.
             * This does not happen with normal video, it just happens here as
             * the motion of the chroma plane does not match the luma plane. */
            c->mb_decision = 2;
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
	unsigned int i;

	if(!loc)
		return;

        fmt = loc->oformat;

        av_write_trailer(loc);
        /* Close each codec. */
        for (i = 0; i < loc->nb_streams; i++)
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

static int init_encoder(struct webPlay *stWebPlay, const char* oname)
{
	int ret = 0;
	char arr_string[128] = "";
	AVOutputFormat *fmt;
	AVCodec *video_codec;
	AVStream *vst = NULL;
	AVFormatContext *loc;
	AVDictionary *options;


	/* allocate the output media context */
	avformat_alloc_output_context2(&stWebPlay->oc, NULL, NULL, oname);
	if (!stWebPlay->oc)
	{
		av_log(NULL, AV_LOG_ERROR, "Could not deduce output format\n");
		ret = -1;
		goto end;
	}
	//save output context in local context
	loc = stWebPlay->oc;
	
	// set wrap around option in hls
	options = NULL;
	av_dict_set(&options, "hls_wrap", "5", 0);
	av_dict_set(&options, "hls_time", "2", 0);
	ret = av_opt_set_dict2(loc->priv_data, &options, AV_OPT_SEARCH_CHILDREN);
	if(ret < 0)
	{
		av_log(NULL, AV_LOG_WARNING, "unable to wrap hls segment\n");
	}
 
	fmt = loc->oformat;
	if (fmt->video_codec != AV_CODEC_ID_NONE)
	{
		vst = add_webcam_stream(stWebPlay, &video_codec, fmt->video_codec);
	}
	if (!vst)
	{
		ret = -1;
		goto end;
	}

	if(vst)
	{
		/* open the codec */
		ret = avcodec_open2(vst->codec, video_codec, NULL);
		if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "Could not open video codec: %s\n",
					av_make_error_string(arr_string, 128, ret));
			ret = -1;
			goto end;
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
			ret = -1;
			goto end;
		}
	}
	av_dump_format(loc, 0, "output", 1);
	/* Write the stream header, if any. */
	ret = avformat_write_header(loc, NULL);
	if (ret < 0)
	{
		av_log(NULL, AV_LOG_ERROR, "Error occurred when writing header: %s\n",
				av_make_error_string(arr_string, 128, ret));
		ret = -1;
		goto end;
	}

end:
        if(ret < 0)  
		dinit_encoder(&stWebPlay->oc);
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

	frame->pict_type = AV_PICTURE_TYPE_NONE;
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
        return 0;
}

EXPORT void stop_capture(void *ctx)
{
	struct webPlay *stWebPlay = (struct webPlay *)ctx;
	if (stWebPlay)
	{
		dinit_filters(stWebPlay);
		dinit_decoder_webcam(&stWebPlay->ic, stWebPlay->dec_ctx);
		dinit_encoder(&stWebPlay->oc);
		av_frame_free(&stWebPlay->OutFrame);
		av_frame_free(&stWebPlay->InFrame);
		free(stWebPlay);
	}
}

EXPORT void *init_capture(const char*path)
{

	int ret = 0;
	// structure with ffmpeg variables
	struct webPlay *stWebPlay;
	AVStream *stream = NULL;

	// allocation of webplay structure
	stWebPlay = malloc(sizeof(struct webPlay));
	if(stWebPlay == NULL)
	{
		fprintf(stderr,"Error in web play struct alloc\n");
		ret =-1;
		goto end;
	}
	memset(stWebPlay, 0, sizeof(*stWebPlay));

	init_ffmpeg();
	ret = init_decoder_webcam(&stWebPlay->ic, &stWebPlay->dec_ctx, &stream);
	if(ret <0)
	{
		ret =-1;
		goto end;
	}
	/** Initalize framerate coming from webcam */
	stWebPlay->video_avg_frame_rate.num = stream->avg_frame_rate.num;
	stWebPlay->video_avg_frame_rate.den = stream->avg_frame_rate.den;

	init_filters(stWebPlay);

	ret = init_encoder(stWebPlay,path);
	if(ret < 0)
	{
		printf("Error in encoder init\n");
		ret =-1;
		goto end;
	}

	stWebPlay->InFrame          = av_frame_alloc();
	stWebPlay->OutFrame         = av_frame_alloc();
end:
	if(ret < 0)
	{
		stop_capture((void*)stWebPlay);
		return NULL;
	}
	return stWebPlay;
}


EXPORT int start_capture(void *ctx)
{
	struct webPlay *stWebPlay = (struct webPlay *)ctx;
	int got_frame;
	int ret;
	AVPacket packet;
	AVFormatContext *ic;
	long long start_time;

	if(!stWebPlay)
	{
		ret = -1;
		goto end;
	}
	ic = stWebPlay->ic;

	if (ic->start_time != AV_NOPTS_VALUE)
		start_time = ic->start_time;

	while( (ret = av_read_frame(stWebPlay->ic,&packet) ) >= 0)
	{
		AVCodecContext *dec_ctx = stWebPlay->ic->streams[0]->codec;
		if (packet.pts != AV_NOPTS_VALUE)
			packet.pts -= av_rescale_q(start_time, AV_TIME_BASE_Q, ic->streams[0]->time_base);

		if (packet.dts != AV_NOPTS_VALUE)
			packet.dts -= av_rescale_q(start_time, AV_TIME_BASE_Q, ic->streams[0]->time_base);


                //packet.dts = av_rescale_q(stWebPlay->dts, AV_TIME_BASE_Q, ic->streams[0]->time_base);
		ret = avcodec_decode_video2(dec_ctx, stWebPlay->InFrame, &got_frame, &packet);
		if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "Error decoding video\n");
			goto end;
		}
		if(!got_frame)
			continue;
		stWebPlay->InFrame->pts = av_frame_get_best_effort_timestamp(stWebPlay->InFrame);
		if (av_buffersrc_add_frame_flags(stWebPlay->buffersrc, stWebPlay->InFrame, AV_BUFFERSRC_FLAG_PUSH) < 0)
		{
			av_log(NULL, AV_LOG_ERROR,
					"Error while feeding the filtergraph\n");
		}

		/* pull filtered frames from the filtergraph */
		while (1)
		{
			int i = 0;
			int nb_frames = 1;
			ret = av_buffersink_get_frame_flags(stWebPlay->buffersink, stWebPlay->OutFrame,AV_BUFFERSINK_FLAG_NO_REQUEST);
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
			if (stWebPlay->OutFrame->pts != AV_NOPTS_VALUE)
			{
				stWebPlay->OutFrame->pts = av_rescale_q(stWebPlay->OutFrame->pts, stWebPlay->buffersink->inputs[0]->time_base, stWebPlay->oc->streams[0]->codec->time_base);
			}
			nb_frames += stWebPlay->OutFrame->pts - stWebPlay->sync_out_pts;
			/** drop all frames if extra are provided */
			if(nb_frames < 0)
				nb_frames = 1;
			/** Some time insane gap is seen,remove that in ffmpeg itself */
			if(nb_frames > 15)
				nb_frames = 1;
			for( i = 0;i < nb_frames;i++)
			{
				stWebPlay->OutFrame->pts = stWebPlay->sync_out_pts;
				write_video_frame(stWebPlay->oc,stWebPlay->oc->streams[0],stWebPlay->OutFrame);
				stWebPlay->sync_out_pts++;
			}
			//h264_encoder(stWebPlay);
			av_frame_unref(stWebPlay->OutFrame);
		}
		av_frame_unref(stWebPlay->InFrame);
		av_free_packet(&packet);
	}
end:    if(ret <0)
		stop_capture(stWebPlay);
	return ret;
}



