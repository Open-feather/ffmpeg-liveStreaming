SRC=..\test\test.cpp
INCLUDE=/I..\library /I"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include" /I3rdparty\Pre-built.2\include
PTHREAD_LIB= libpthreadGC2.a

TEST_PROGRAM=test_64 multi_thread_test_64 h264bitstream_64 rtmpSave_64
TEST_PROGRAM_SHARP=stream_video_64.exe overly_image_64.exe duplicate_stream_64.exe overly_stream_64.exe

all:$(TEST_PROGRAM) $(TEST_PROGRAM_SHARP)
	@move *_64.exe dist_x64 > NUL


test.obj:..\test\test.cpp
	@$(CXX) /nologo /EHsc /c $(INCLUDE) $**

multi_thread_test.obj:..\test\multi_thread_test.cpp
	@$(CXX) /nologo /EHsc /c $(INCLUDE) $**

h264bitstream.obj:..\test\h264bitstream.cpp
	@$(CXX) /nologo /EHsc /c $(INCLUDE) $**

rtmp_save.obj: ..\test\rtmp_save.cpp
	@$(CXX) /nologo /EHsc /c $(INCLUDE) $**

test_64: test.obj
	@$(CXX) /nologo  /Fe$@ /EHsc $(INCLUDE) $** libstream_64.lib

multi_thread_test_64: multi_thread_test.obj
	@$(CXX) /nologo /Fe$@ /EHsc $(INCLUDE) $** /link /LIBPATH:3rdparty\Pre-built.2\lib\x64 libstream_64.lib  $(PTHREAD_LIB)

h264bitstream_64:h264bitstream.obj
	@$(CXX) /nologo /Fe$@ /EHsc $(INCLUDE) $** /link /LIBPATH:3rdparty\Pre-built.2\lib\x64 libstream_64.lib  $(PTHREAD_LIB)

rtmpSave_64:rtmp_save.obj
	@$(CXX) /nologo /Fe$@ /EHsc $(INCLUDE) $** /link /LIBPATH:3rdparty\Pre-built.2\lib\x64 libstream_64.lib  $(PTHREAD_LIB)


# following files for c-sharp program...
stream_video_64.exe : stream_video.cs
	@csc /nologo /out:$@ $**
overly_image_64.exe : overly_image.cs
	@csc  /nologo /out:$@ $**
duplicate_stream_64.exe : duplicate_stream.cs
	@csc /nologo /out:$@ $**
overly_stream_64.exe : overly_stream.cs
	@csc /nologo /out:$@ $**

clean:
	@del *.obj $(TEST_PROGRAM) $(TEST_PROGRAM_SHARP) 2> nul
