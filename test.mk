CXXFLAGS=-I./library/
LDFLAGS= -ldl
SRC= test.cpp
debug:$(SRC)
	$(CXX) -g $(SRC)  $(CXXFLAGS) ./library/libstream.so 

all:test.cpp
	$(CXX) -g $(SRC)   $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -rf a.out test.ts

