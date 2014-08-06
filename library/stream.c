
/*

	Live Streaming using ffmpeg

*/

#include "stream.h"
#include "decoder.h"
#include "filter.h"
#include "inputs.h"
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
static AVStream *add_webcam_stream(struct liveStream *ctx, AVCodec **codec, enum AVCodecID codec_id)
{
        AVCodecContext *c = NULL;
        AVStream *st = NULL;
	AVFormatContext *oc = ctx->oc;

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
	st->avg_frame_rate.den = ctx->video_avg_frame_rate.den;
	st->avg_frame_rate.num = ctx->video_avg_frame_rate.num;
        st->time_base.den = ctx->video_avg_frame_rate.num;
        st->time_base.num = ctx->video_avg_frame_rate.den;
        c->time_base.den = ctx->video_avg_frame_rate.num;
        c->time_base.num = ctx->video_avg_frame_rate.den;
        c->gop_size = ctx->video_avg_frame_rate.num; /* emit one intra frame every twelve frames at most */
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

static int init_encoder(struct liveStream *ctx, const char* oname)
{
	int ret = 0;
	char arr_string[128] = "";
	AVOutputFormat *fmt;
	AVCodec *video_codec;
	AVStream *vst = NULL;
	AVFormatContext *loc;
	AVDictionary *options;


	/* allocate the output media context */
	avformat_alloc_output_context2(&ctx->oc, NULL, NULL, oname);
	if (!ctx->oc)
	{
		av_log(NULL, AV_LOG_ERROR, "Could not deduce output format\n");
		ret = -1;
		goto end;
	}
	//save output context in local context
	loc = ctx->oc;

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
		vst = add_webcam_stream(ctx, &video_codec, fmt->video_codec);
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
		dinit_encoder(&ctx->oc);
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

EXPORT void stop_capture(void *actx)
{
	struct liveStream *ctx = (struct liveStream *)actx;
	if (ctx)
	{
		dinit_filters(ctx);
		dinit_inputs(&ctx->inputs,&ctx->nb_input);
		dinit_encoder(&ctx->oc);
		av_frame_free(&ctx->OutFrame);
		free(ctx);
	}
}

EXPORT void *init_capture(const char*path)
{

	int ret = 0;
	// structure with ffmpeg variables
	struct liveStream *ctx;
	AVStream *stream = NULL;

	// allocation of Live Stream structure
	ctx = malloc(sizeof(struct liveStream));
	if(ctx == NULL)
	{
		fprintf(stderr,"Error in web play struct alloc\n");
		ret =-1;
		goto end;
	}
	memset(ctx, 0, sizeof(*ctx));

	init_ffmpeg();
	ret = configure_input(ctx,CAM_DEVICE_NAME,IN_WEBCAM);
	if(ret <0)
	{
		ret =-1;
		goto end;
	}
	stream = ctx->inputs[0].st;
	/** Initalize framerate coming from webcam */
	ctx->video_avg_frame_rate.num = stream->avg_frame_rate.num;
	ctx->video_avg_frame_rate.den = stream->avg_frame_rate.den;

	init_filters(ctx);

	ret = init_encoder(ctx, path);
	if(ret < 0)
	{
		printf("Error in encoder init\n");
		ret =-1;
		goto end;
	}

	ctx->OutFrame         = av_frame_alloc();
end:
	if(ret < 0)
	{
		stop_capture((void*)ctx);
		return NULL;
	}
	return ctx;
}

int get_input_packet(struct lsInput *input,AVPacket *pkt)
{
	return av_thread_message_queue_recv(input->in_thread_queue,pkt,AV_THREAD_MESSAGE_NONBLOCK);
}
static int output_packet(struct lsInput *input,const AVPacket *pkt)
{
	AVPacket avpkt;
	int got_output = 0;
	int ret = 0;
	if (pkt == NULL)
	{
		av_init_packet(&avpkt);
		avpkt.data = NULL;
		avpkt.size = 0;
	}
	else
	{
		avpkt = *pkt;
	}
	ret = avcodec_decode_video2(input->dec_ctx,input->InFrame, &got_output,&avpkt);
	if (!got_output || ret < 0)
	{
		av_buffersrc_add_ref(input->in_filter, NULL, 0);
	}
	if(ret < 0)
		av_log(NULL,AV_LOG_ERROR,"unable to decode video\n");

	return ret;
}
struct lsInput* get_best_input(struct liveStream *ctx)
{
	struct lsInput* input = NULL;
	struct lsInput* best_input = NULL;
	int nb_requests = 0;
	int nb_requests_max = 0;
	int ret = 0;

	ret = avfilter_graph_request_oldest(ctx->filter_graph);
	if(ret >= 0)
		av_log(NULL,AV_LOG_ERROR,"possible loss of data\n");

	for(input = ctx->inputs;input;input = input->next)
	{
		if(input->eof_reached)
			continue;
		nb_requests = av_buffersrc_get_nb_failed_requests(input->in_filter);
		if (nb_requests >  nb_requests_max)
		{
			nb_requests_max = nb_requests;
			best_input = input;
		}
	}
//	if(!best_input)
//	{
//		for (i = 0; i < ctx->graph->nb_outputs; i++)
//	}
	return best_input;
}
EXPORT int start_capture(void *actx)
{
	struct liveStream *ctx = (struct liveStream *)actx;
	int got_frame;
	int ret;
	AVPacket packet;
	AVFormatContext *ic;
	long long start_time;
	struct lsInput* input = NULL;

	if(!ctx)
	{
		ret = -1;
		goto end;
	}


	while(1)
	{
		input = get_best_input(ctx);
		if(!input)
		{
			continue;
		}
		ic = input->ic;
		if (ic->start_time != AV_NOPTS_VALUE)
			start_time = ic->start_time;

		AVCodecContext *dec_ctx = input->dec_ctx;
		ret = get_input_packet(input,&packet);
		if (ret == AVERROR(EAGAIN))
		{
			continue;
		}
		else if (ret == AVERROR_EOF)
		{
			output_packet(input,NULL);
			input->eof_reached = 1;
			continue;
		}
		if(ret < 0)
		{
			av_log(NULL,AV_LOG_ERROR,"No Input packet %x\n",ret);
			break;
		}
		if(input->id != 1)
		{
			if (packet.pts != AV_NOPTS_VALUE)
				packet.pts -= av_rescale_q(start_time, AV_TIME_BASE_Q, ic->streams[0]->time_base);

			if (packet.dts != AV_NOPTS_VALUE)
				packet.dts -= av_rescale_q(start_time, AV_TIME_BASE_Q, ic->streams[0]->time_base);
		}

                //packet.dts = av_rescale_q(stWebPlay->dts, AV_TIME_BASE_Q, ic->streams[0]->time_base);
		ret = avcodec_decode_video2(dec_ctx, input->InFrame, &got_frame, &packet);
		if (ret < 0)
		{
			av_log(NULL, AV_LOG_ERROR, "Error decoding video\n");
			goto end;
		}
		if(!got_frame)
			continue;


		input->InFrame->pts = av_frame_get_best_effort_timestamp(input->InFrame);

		if (av_buffersrc_add_frame_flags(input->in_filter, input->InFrame, AV_BUFFERSRC_FLAG_PUSH) < 0)
		{
			av_log(NULL, AV_LOG_ERROR,
					"Error while feeding the filtergraph\n");
		}

		/* pull filtered frames from the filtergraph */
		while (1)
		{
			int i = 0;
			int nb_frames = 1;
			ret = av_buffersink_get_frame_flags(ctx->out_filter, ctx->OutFrame,AV_BUFFERSINK_FLAG_NO_REQUEST);
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
			if (ctx->OutFrame->pts != AV_NOPTS_VALUE)
			{
				ctx->OutFrame->pts = av_rescale_q(ctx->OutFrame->pts, ctx->out_filter->inputs[0]->time_base, ctx->oc->streams[0]->codec->time_base);
			}
			nb_frames += ctx->OutFrame->pts - ctx->sync_out_pts;
			/** drop all frames if extra are provided */
			if(nb_frames < 0)
				nb_frames = 1;
			/** Some time insane gap is seen,remove that in ffmpeg itself */
			if(nb_frames > 15)
				nb_frames = 1;
			for( i = 0;i < nb_frames;i++)
			{
				ctx->OutFrame->pts = ctx->sync_out_pts;
				write_video_frame(ctx->oc,ctx->oc->streams[0],ctx->OutFrame);
				ctx->sync_out_pts++;
			}
			av_frame_unref(ctx->OutFrame);
		}
	}
	av_frame_unref(input->InFrame);
	av_free_packet(&packet);
end:
	return ret;
}

EXPORT int set_image(void *actx,char*path, int xpos,int ypos,int height, int width)
{
	int ret = 0;
	char*str = NULL;
	struct liveStream *ctx = (struct liveStream *)actx;
	ret = configure_input(ctx,path,IN_IMAGE);
	if(ret < 0)
	{
		av_log(NULL,AV_LOG_ERROR,"unable to configure input\n");
		return ret;
	}

	str = malloc(ctx->graph_desc.len +1);
	if(!str)
	{
		av_log(NULL,AV_LOG_ERROR,"No memory Available\n");
		return -1;
	}
	strcpy(str,ctx->graph_desc.str);
	av_bprintf(&ctx->graph_desc, "[web];[1]format=yuv420p,scale=%d:%d[onit];[web][onit]overlay",height,width);
	ret = configure_filter(ctx);
	if(ret < 0)
	{
		av_log(NULL,AV_LOG_ERROR,"Unable to configure Filter\n");
		return -1;
	}
	return ret;
}
