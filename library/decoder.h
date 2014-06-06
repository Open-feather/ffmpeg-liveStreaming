#ifndef __DECODER_H
#define __DECODER_H



void dinit_decoder_webcam(AVFormatContext **pFormatCtx,AVCodecContext *dec_ctx);
int init_decoder_webcam(AVFormatContext **pFormatCtx,AVCodecContext **dec_ctx);
#endif
