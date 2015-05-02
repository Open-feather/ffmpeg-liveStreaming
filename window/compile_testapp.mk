SRC=..\test\test.cpp
INCLUDE=/I..\library /I"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include" /I3rdparty\Pre-built.2\include
PTHREAD_LIB= libpthreadGC2.a

TEST_PROGRAM=test.exe multi_thread_test.exe h264bitstream.exe
TEST_PROGRAM_SHARP=stream_video.exe overly_image.exe duplicate_stream.exe overly_stream.exe

all:$(TEST_PROGRAM) $(TEST_PROGRAM_SHARP)

test.obj:..\test\test.cpp
	$(CXX) /EHsc /c $(INCLUDE) $**

multi_thread_test.obj:..\test\multi_thread_test.cpp
	$(CXX) /EHsc /c $(INCLUDE) $**

h264bitstream.obj:..\test\h264bitstream.cpp
	$(CXX) /EHsc /c $(INCLUDE) $**

test.exe: test.obj
	$(CXX) /EHsc $(INCLUDE) test.obj libstream_64.lib

multi_thread_test.exe : multi_thread_test.obj
	$(CXX) /EHsc $(INCLUDE) multi_thread_test.obj /link /LIBPATH:3rdparty\Pre-built.2\lib\x64 libstream_64.lib  $(PTHREAD_LIB)

h264bitstream.exe:h264bitstream.obj
	$(CXX) /EHsc $(INCLUDE) h264bitstream.obj /link /LIBPATH:3rdparty\Pre-built.2\lib\x64 libstream_64.lib  $(PTHREAD_LIB)

# following files for c-sharp program...
stream_video.exe : stream_video.cs
	csc stream_video.cs
overly_image.exe : overly_image.cs
	csc overly_image.cs
duplicate_stream.exe : duplicate_stream.cs
	csc duplicate_stream.cs
overly_stream.exe : overly_stream.cs
	csc overly_stream.cs
clean:
	@del *.obj $(TEST_PROGRAM) $(TEST_PROGRAM_SHARP) 2> nul
