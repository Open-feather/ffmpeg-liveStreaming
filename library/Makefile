CFLAGS= -Wall -ggdb3 -std=gnu99
LDFLAGS= -shared -pthread
FFMPEG_LIBS=libavfilter
SRC=decoder.c stream.c

#default library name
LIB=libstream.so
SYS := $(shell gcc -dumpmachine)
ifneq (, $(findstring linux, $(SYS)))
FFMPEG_LIBS= libavdevice libavformat libavfilter libavcodec libswresample libswscale libavutil
LIBS := $(shell pkg-config --libs $(FFMPEG_LIBS)) $(LIBS)
LIB=libstream.so
endif

ifneq (, $(findstring mingw, $(SYS)))
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
	$(CC) -o $@ $(CFLAGS) $(OBJ) -Wl,--whole-archive $(LDFLAGS) $(LIBS) -Wl,--no-whole-archive 
clean:
	rm -rf $(OBJ) $(LIB)


