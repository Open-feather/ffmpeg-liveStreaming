#include "stream.h"

int init_decoder_webcam(AVFormatContext **pFormatCtx, AVCodecContext **dec_ctx)
{
	AVInputFormat *inputFormat;
	AVDictionary *options;
	AVStream *st;
	AVCodec *dec = NULL;
	int ret = 0;

	// get the camera input format from 
	inputFormat = av_find_input_format(CAM_DRIVER);

	// allocate contex
	*pFormatCtx = avformat_alloc_context();
	if(*pFormatCtx == NULL)
	{
		fprintf(stderr,"could not allocate avformat\n");
		goto end;
		ret = -1;
	}

	// set avdict option
	options = NULL;
	// set the frame rate
	av_dict_set(&options, "framerate", "15", 0); 
	// open the camera to get the data
	ret = avformat_open_input(pFormatCtx, CAM_DEVICE_NAME, inputFormat, &options);
	if(ret < 0)
	{
		fprintf(stderr,"Error in avformat open input ret : %d\n", ret);
		return -1;
	}

	// get the camera stream information
	if(avformat_find_stream_info(*pFormatCtx,NULL) < 0)   
	{
		fprintf(stderr,"Error in finding stream infon");
		return -13;
	}
	ret = av_find_best_stream(*pFormatCtx,AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0); 
	if (ret < 0)
	{
		fprintf(stderr, "Could not find video stream in camera\n");
		return ret;
	}
	st = (*pFormatCtx)->streams[ret];
	*dec_ctx = st->codec;
	dec = avcodec_find_decoder((*dec_ctx)->codec_id);

	ret = avcodec_open2((*dec_ctx), dec,NULL);
	if (ret < 0)
	{
		fprintf(stderr, "Could not find video stream in camera\n");
		return ret;
	}
end:
	if(ret < 0)	
		avformat_close_input(pFormatCtx);
	return ret;

}

