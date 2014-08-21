CFLAGS= -Wall -ggdb3 -std=gnu99
FFMPEG_LIBS=libavfilter
SRC=decoder.c stream.c filter.c inputs.c

#default library name
LIB=libstream.so
SYS := $(shell $(CC) -dumpmachine)
ifneq (, $(findstring linux, $(SYS)))
FFMPEG_LIBS= libavdevice libavformat libavfilter libavcodec libswresample libswscale libavutil
LIBS= -shared -pthread
LIBS += -Wl,--whole-archive
LIBS += $(shell pkg-config --libs $(FFMPEG_LIBS))
LIBS += -Wl,--no-whole-archive
CFLAGS += $(shell pkg-config --cflags $(FFMPEG_LIBS))
LIB=libstream.so
endif

ifneq (, $(findstring mingw, $(SYS)))
FFMPEG_LIBS= libavdevice libavformat libavfilter libavcodec libswresample libswscale libavutil
LIBS += -shared -pthread
LIBS += $(shell pkg-config --libs $(FFMPEG_LIBS))
CFLAGS += $(shell pkg-config --cflags $(FFMPEG_LIBS))
CFLAGS += -DMINGW
LIB=libstream.dll
endif

ifneq (, $(findstring cygwin, $(SYS)))
LIB=libstream.dll
CXXFLAGS += -DWINDOWS -static-libgcc -static-libstdc++ -I/usr/local/include/
CXX=x86_64-w64-mingw32-g++
LIBS= -L/usr/lib -L./lib -lavfilter swresample-0.dll -lswscale -lavformat -lavcodec -lavutil -lpostproc -lws2_32 

endif

OBJ=$(SRC:.c=.o)

all: $(LIB)

$(LIB): $(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $(LIBS)
clean:
	rm -rf $(OBJ) $(LIB)


