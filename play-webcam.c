
/*

	Play web cam using ffmpeg

*/


#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>


#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcodec/avfft.h>

int main()
{
	
	int ret = 0, i = 0, numBytes = 0;
	char *buffer;

	/* register all the codecs */
	av_register_all();
        avcodec_register_all();
	avdevice_register_all();
	AVFormatContext	*pFormatCtx;
	AVInputFormat *inputFormat = av_find_input_format("v4l2");
	AVCodecContext  *pCodecCtx;
	AVCodec * pCodec;

	FILE* tempFile;

	tempFile = fopen("cameradump.yuv", "w");
	if(tempFile == NULL)
	{
		printf("ERROR in opening output file\n");
		return -1;
	}	

	AVDictionary *options = NULL;
	pFormatCtx = avformat_alloc_context();
	if(pFormatCtx == NULL)
	{
		printf("pFormatCtx : NULL\n");
		return -1;
	}
	av_dict_set(&options, "framerate", "25", 0); 

	//ret = avformat_open_input(&pFormatCtx, "/home/tushar/iwiilbe.mp4", NULL,NULL); //inputFormat, &options);
	ret = avformat_open_input(&pFormatCtx, "/dev/video0", inputFormat, &options);
	{
		printf("ret : %d\n", ret);
	}

	if(av_find_stream_info(pFormatCtx) < 0)   
        {
                printf("Error in finding stream infon");
                return -13;
        }

	printf("Stream info is as follows :\n");
	av_dump_format(pFormatCtx, 0, "/dev/video0", 0);	

	int videoStream = -1;
        for( i=0; i < pFormatCtx->nb_streams; i++)
        {
                if(pFormatCtx->streams[i]->codec->coder_type==AVMEDIA_TYPE_VIDEO)
                {
                        videoStream = i;
                        break;
                }
        }

	if(videoStream == -1) 
	{
		printf("Video stream not found\n");
		return -14;
	}
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	
	printf("pCodecCtx->width,pCodecCtx->height : %d : %d\n", pCodecCtx->width, pCodecCtx->height);
	numBytes = (pCodecCtx->width * pCodecCtx->height * 3/2);
		
        pCodec =avcodec_find_decoder(pCodecCtx->codec_id);
        if(pCodec==NULL) 
	{
		printf("Error in decoder find\n");
		return -15; //codec not found
	}

        if(avcodec_open2(pCodecCtx,pCodec,NULL) < 0) 
	{
		printf("Error in avcodec open\n");
		return -16;
	}

	int res;
        int frameFinished;
        AVPacket packet;
        while(res = av_read_frame(pFormatCtx,&packet)>=0)
        {

		printf("PTS : %lld\n", packet.pts);
		if(packet.stream_index == videoStream)
		{
			printf("DATA SIZE : %lld\n", packet.size);
			fwrite(packet.data, 1, packet.size, tempFile);
		}
			
	}
	
	printf("Yahooooooooooooooooooooooooo  ENd of code here \n");
}
