##########################################################################
#
# To run Maefile in Windows cygwin
# export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/



CFLAGS= -Wall -std=gnu99
FFMPEG_LIBS=libavfilter
SRC=decoder.c stream.c filter.c inputs.c platform.c

#default library name
LIB=libstream.so
SYS := $(shell $(CC) -dumpmachine)
ifneq (, $(findstring linux, $(SYS)))
FFMPEG_LIBS= libavdevice libavformat libavfilter libavcodec libswresample libswscale libavutil
LIBS= -shared -pthread
#LIBS += -Wl,--whole-archive
LIBS += $(shell pkg-config --libs $(FFMPEG_LIBS))
#LIBS += -Wl,--no-whole-archive
CFLAGS += $(shell pkg-config --cflags $(FFMPEG_LIBS))
LIB=libstream.so
endif

ifneq (, $(findstring mingw, $(SYS)))
FFMPEG_LIBS= libavdevice libavformat libavfilter libavcodec libswresample libswscale libavutil
#DSHOW LIBS dependency
LIBS+= -lpsapi -lole32 -lstrmiids -luuid
LIBS += -shared -pthread
LIBS += $(shell pkg-config --libs $(FFMPEG_LIBS))
CFLAGS += $(shell pkg-config --cflags $(FFMPEG_LIBS))
CFLAGS += -DMINGW
LIB=libstream.dll
endif

ifneq (, $(findstring cygwin, $(SYS)))
LIB=libstream.dll
CXXFLAGS += -DWINDOWS -static-libgcc -static-libstdc++ -I/usr/local/include/
CFLAGS += -DWINDOWS -static-libgcc -static-libstdc++ -I/usr/local/include/
CXX=x86_64-w64-mingw32-g++
CC=x86_64-w64-mingw32-gcc
#LIBS= -L/usr/lib -L./lib -lavfilter -lswscale -lavformat -lavcodec -lavutil -lpostproc -lws2_32
LIBS += -shared -pthread

LIBS+=`pkg-config --libs libavformat`
LIBS+=`pkg-config --libs libavdevice`
LIBS+=`pkg-config --libs libavcodec`

endif

OBJ=$(SRC:.c=.o)
debug: CFLAGS += -Wall -ggdb3
debug: $(LIB)
all: $(LIB)

$(LIB): $(OBJ)
	$(CC) -o $@ $(CFLAGS) $(ECFLAGS) $(OBJ) $(LIBS)
clean:
	rm -rf $(OBJ) $(LIB)


