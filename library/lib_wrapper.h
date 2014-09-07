#ifndef _LIB_WRAPPER_H
#define _LIB_WRAPPER_H
#include "constant.h"

#ifdef __cplusplus
extern "C" {
#endif

int start_capture(void *ctx);
void *init_capture(const char*path);
void stop_capture(void *ctx);

int set_image(void *actx,const char*path, int xpos, int ypos, int height, int width);
int duplicate_stream(void *actx,enum DuplicateFormat format);
int duplicate_overlayed_stream(void *actx,int xpos, int ypos, int height, int width);
int pause_stream(void *actx, long long duration);

#ifdef __cplusplus
}
#endif

#endif
