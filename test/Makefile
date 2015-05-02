##########################################################################
#
# To run Maefile in Windows cygwin
# export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/

CXXFLAGS=-g -I../library/
LDLIBS += -pthread
SRC= test.cpp

#OBJS=$(addsuffix .o,$(EXAMPLES))

EXAMPLES= test multi_thread_test h264bitstream rtmp_save
SYS := $(shell $(CXX) -dumpmachine)

ifneq (, $(findstring linux, $(SYS)))
LDFLAGS=-L../library
LIBRARY=-lstream
endif

ifneq (, $(findstring mingw, $(SYS)))
LIBRARY=../library/libstream.dll
endif

ifneq (, $(findstring cygwin, $(SYS)))
LIB=libstream.dll
CXXFLAGS += -g -DWINDOWS -static-libgcc -static-libstdc++ -I/usr/local/include/
CFLAGS += -g -DWINDOWS -static-libgcc -static-libstdc++ -I/usr/local/include/
CXX=x86_64-w64-mingw32-g++
CC=x86_64-w64-mingw32-gcc
#LIBS= -L/usr/lib -L./lib -lavfilter -lswscale -lavformat -lavcodec -lavutil -lpostproc -lws2_32
LDFLAGS=-L../library
LIBRARY=-lstream
endif


LDLIBS += $(LIBRARY)
all:$(EXAMPLES)

clean:
	rm -rf $(EXAMPLES) test.ts *.o
