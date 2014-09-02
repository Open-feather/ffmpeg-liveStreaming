SRC= ..\library\stream.c ..\library\decoder.c ..\library\filter.c ..\library\inputs.c ..\library\platform.c
FFMPEG_LIB= avcodec.lib avformat.lib avdevice.lib avfilter.lib avutil.lib swresample.lib postproc.lib swscale.lib
DSHOW_LIB= ole32.lib psapi.lib strmiids.lib uuid.lib
PTHREAD_LIB=libpthreadGC2.a
FFMPEG_VERSION_32=ffmpeg-20140822-git-1b5ec6a-win32-dev
FFMPEG_VERSION_64=ffmpeg-20140822-git-1b5ec6a-win64-dev
INCLUDE_64= /I3rdparty\$(FFMPEG_VERSION_64)\include /I3rdparty\Pre-built.2\include
INCLUDE_32= /I3rdparty\$(FFMPEG_VERSION_32)\include /I3rdparty\Pre-built.2\include
INCLUDE= /I..\library  /I.
INCLUDE= $(INCLUDE) /I"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include" 
#Adding Windows include
INCLUDE= $(INCLUDE) /I"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include"
OUTPUT_32=libstream_32.dll
OUTPUT_64=libstream_64.dll

#set LIBPATH for ffmpeg and pthread
LIB_DIR_64=/LIBPATH:3rdparty\$(FFMPEG_VERSION_64)\lib
LIB_DIR_64=$(LIB_DIR_64) /LIBPATH:3rdparty\Pre-built.2\lib\x64
LIB_DIR_32=/LIBPATH:3rdparty\$(FFMPEG_VERSION_32)\lib
LIB_DIR_32=$(LIB_DIR_32) /LIBPATH:3rdparty\Pre-built.2\lib\x86

all: libstream_32 libstream_64
	
libstream_64:
	$(CC) $(INCLUDE_64) $(INCLUDE) $(SRC) /link /DLL /OUT:$(OUTPUT_64) $(LIB_DIR_64) $(FFMPEG_LIB) $(DSHOW_LIB) $(PTHREAD_LIB)
	
libstream_32: 
	$(CC) $(INCLUDE) $(INCLUDE_32) $(SRC) /link /DLL  /MACHINE:X64  /OUT:$(OUTPUT_32) $(LIB_DIR_64) $(FFMPEG_LIB)

clean:
	del *.obj *.exp *.lib


