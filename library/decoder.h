#ifndef __DECODER_H
#define __DECODER_H

#include <libavformat/avformat.h>
int init_decoder(AVFormatContext **pFormatCtx,const char *fname,char *fmt);
void dinit_decoder(AVFormatContext **pFormatCtx,AVCodecContext *dec_ctx);
int init_wo_decoder(AVFormatContext **pFormatCtx, const char *fname, char *fmt);

#endif
