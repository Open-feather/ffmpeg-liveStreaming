SRC=..\test\test.cpp
INCLUDE=/I..\library /I"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include" /I3rdparty\Pre-built.2\include
PTHREAD_LIB= libpthreadGC2.a

TEST_PROGRAM=test.exe multi_thread_test.exe

test.obj:..\test\test.cpp
	$(CXX) /EHsc /c $(INCLUDE) $**

multi_thread_test.obj:..\test\multi_thread_test.cpp
	$(CXX) /EHsc /c $(INCLUDE) $**

test.exe: test.obj
	$(CXX) /EHsc $(INCLUDE) test.obj libstream_64.lib

multi_thread_test.exe:multi_thread_test.obj
	$(CXX) /EHsc $(INCLUDE) multi_thread_test.obj /link /LIBPATH:3rdparty\Pre-built.2\lib\x64 libstream_64.lib  $(PTHREAD_LIB)

all:$(TEST_PROGRAM)



clean:
	del *.obj *.exe