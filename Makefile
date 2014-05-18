SRC=play-webcam-enc.c

CFLAGS+=$(shell pkg-config --cflags libavdevice)
CFLAGS+=$(shell pkg-config --cflags libavfilter)
CFLAGS+=-g -std=gnu99
LDFLAGS=$(shell pkg-config --libs libavdevice)
all:$(SRC)
	gcc $^ $(CFLAGS) $(LDFLAGS) -o webcam_stream
