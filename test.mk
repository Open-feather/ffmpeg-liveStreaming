CXXFLAGS=-I./library/
LDFLAGS= -ldl
SRC= test.cpp

SYS := $(shell $(CXX) -dumpmachine)

ifneq (, $(findstring linux, $(SYS)))
LIBRARY=./library/libstream.so
endif

ifneq (, $(findstring mingw, $(SYS)))
LIBRARY=./library/libstream.dll
endif

debug:$(SRC)
	$(CXX) -g $(SRC)  $(CXXFLAGS) $(LIBRARY)

all:test.cpp
	$(CXX) -g $(SRC)   $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -rf a.out test.ts

