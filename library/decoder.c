#include "stream.h"
#include "inputs.h"

void dinit_decoder(AVFormatContext **pFormatCtx,AVCodecContext *dec_ctx)
{
	if (dec_ctx)
                avcodec_close(dec_ctx);

	avformat_close_input(pFormatCtx);
}
int init_decoder(AVFormatContext **pFormatCtx,const char *fname,char *fmt)
{
	AVInputFormat *inputFormat;
	AVDictionary *options;
	AVCodec *dec = NULL;
	AVCodecContext *dec_ctx = NULL;
	AVStream *st;
	int ret = 0;

	/** If format is provided then use it
	 * neccassary in case of webcam.
	*/
	inputFormat = av_find_input_format(fmt);

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
	ret = avformat_open_input(pFormatCtx, fname, inputFormat, &options);
	if(ret < 0)
	{
		goto end;
		ret = -1;
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
	dec_ctx = st->codec;
	dec = avcodec_find_decoder(dec_ctx->codec_id);

	ret = avcodec_open2(dec_ctx, dec,NULL);
	if (ret < 0)
	{
		fprintf(stderr, "Could not find video stream in camera\n");
		goto end;
	}
end:
	if(ret < 0)	
	{
		dinit_decoder(pFormatCtx,dec_ctx);
		avformat_close_input(pFormatCtx);
	}
	return ret;

}


int init_wo_decoder(AVFormatContext **pFormatCtx, const char *fname, char *fmt)
{
	AVInputFormat *inputFormat;
	AVDictionary *options;
	AVCodec *dec = NULL;
	AVCodecContext *dec_ctx = NULL;
	AVStream *st;
	int ret = 0;

	/** If format is provided then use it
	 * neccassary in case of webcam.
	*/
	inputFormat = av_find_input_format(fmt);

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
	ret = avformat_open_input(pFormatCtx, fname, inputFormat, &options);
	if(ret < 0)
	{
		goto end;
		ret = -1;
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
	dec_ctx = st->codec;
	dec = avcodec_find_decoder(dec_ctx->codec_id);

	ret = avcodec_open2(dec_ctx, dec,NULL);
	if (ret < 0)
	{
		fprintf(stderr, "Could not find video stream in camera\n");
		goto end;
	}
end:
	if(ret < 0)
	{
		dinit_decoder(pFormatCtx,dec_ctx);
		avformat_close_input(pFormatCtx);
	}
	return ret;

}
