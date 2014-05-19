SRC=play-webcam.c decoder.c

CFLAGS+=$(shell pkg-config --cflags libavdevice)
CFLAGS+=$(shell pkg-config --cflags libavfilter)
CFLAGS+=-g -std=gnu99 -Wall
LDFLAGS=$(shell pkg-config --libs libavdevice)
all:$(SRC)
	gcc $^ $(CFLAGS) $(LDFLAGS) -o webcam_stream

clean:
	rm -rf webcam_stream