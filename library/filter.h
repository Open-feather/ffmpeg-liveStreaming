#ifndef _FILTER_H
#define _FILTER_H

int init_filters(struct liveStream *ctx);
int configure_filter(struct liveStream *ctx);

void dinit_filters(struct liveStream *ctx);
#endif
