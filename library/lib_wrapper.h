#ifndef _LIB_WRAPPER_H
#define _LIB_WRAPPER_H
#ifdef __cplusplus
extern "C" {
#endif
enum DuplicateFormat
{
	SIDE_BY_SIDE,
	TOP_N_BOTTOM
};
int start_capture(void *ctx);
void *init_capture(const char*path);
void stop_capture(void *ctx);

int set_image(void *actx,const char*path, int xpos, int ypos, int height, int width);
int duplicate_stream(void *ctx,enum DuplicateFormat format);
int duplicate_overlayed_stream(void *ctx,int xpos, int ypos, int height, int width);

#ifdef __cplusplus
}
#endif

#endif
