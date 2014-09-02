
CXXFLAGS=-g -I../library/
LDFLAGS= -pthread
SRC= test.cpp

#OBJS=$(addsuffix .o,$(EXAMPLES))

EXAMPLES= test multi_thread_test
SYS := $(shell $(CXX) -dumpmachine)

ifneq (, $(findstring linux, $(SYS)))
LIBRARY=../library/libstream.so
endif

ifneq (, $(findstring mingw, $(SYS)))
LIBRARY=../library/libstream.dll
endif
LDFLAGS += $(LIBRARY)
all:$(EXAMPLES)

clean:
	rm -rf $(EXAMPLES) test.ts *.o
