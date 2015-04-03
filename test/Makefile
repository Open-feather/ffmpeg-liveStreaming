
CXXFLAGS=-g -I../library/
LDLIBS += -pthread
SRC= test.cpp

#OBJS=$(addsuffix .o,$(EXAMPLES))

EXAMPLES= test multi_thread_test h264bitstream
SYS := $(shell $(CXX) -dumpmachine)

ifneq (, $(findstring linux, $(SYS)))
LDFLAGS=-L../library
LIBRARY=-lstream
endif

ifneq (, $(findstring mingw, $(SYS)))
LIBRARY=../library/libstream.dll
endif
LDLIBS += $(LIBRARY)
all:$(EXAMPLES)

clean:
	rm -rf $(EXAMPLES) test.ts *.o
