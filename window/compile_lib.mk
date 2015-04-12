SRC= ..\library\stream.c ..\library\decoder.c ..\library\filter.c ..\library\inputs.c ..\library\platform.c
FFMPEG_LIB= avcodec.lib avformat.lib avdevice.lib avfilter.lib avutil.lib swresample.lib postproc.lib swscale.lib
DSHOW_LIB= ole32.lib psapi.lib strmiids.lib uuid.lib
PTHREAD_LIB=libpthreadGC2.a
FFMPEG_VERSION_32=ffmpeg-2.5.2-win32
FFMPEG_VERSION_64=ffmpeg-2.5.2-win64
INCLUDE_64= /I3rdparty\$(FFMPEG_VERSION_64)-dev\include /I3rdparty\Pre-built.2\include
INCLUDE_32= /I3rdparty\$(FFMPEG_VERSION_32)-dev\include /I3rdparty\Pre-built.2\include
INCLUDE= /I..\library  /I.
INCLUDE= $(INCLUDE) /I"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include"

#Adding Windows include
INCLUDE= $(INCLUDE) /I"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include"
OUTPUT_32=libstream_32.dll
OUTPUT_64=libstream_64.dll

#set LIBPATH for ffmpeg and pthread
LIB_DIR_64=/LIBPATH:3rdparty\$(FFMPEG_VERSION_64)-dev\lib
LIB_DIR_64=$(LIB_DIR_64) /LIBPATH:3rdparty\Pre-built.2\lib\x64

LIB_DIR_32=/LIBPATH:3rdparty\$(FFMPEG_VERSION_32)-dev\lib
LIB_DIR_32=$(LIB_DIR_32) /LIBPATH:3rdparty\Pre-built.2\lib\x86


DEPEND_64=3rdparty\$(FFMPEG_VERSION_64)-shared\bin\*.dll
DEPEND2_64=3rdParty\Pre-built.2\dll\x64\pthreadGC2.dll
DEPEND_32=3rdparty\$(FFMPEG_VERSION_32)-shared\bin\*.dll
DEPEND2_32=3rdParty\Pre-built.2\dll\x64\pthreadGC2.dll

all: libstream_32 $(OUTPUT_64)

$(OUTPUT_64):
	$(CC) $(INCLUDE_64) $(INCLUDE) $(SRC) /link /DLL /OUT:$(OUTPUT_64) $(LIB_DIR_64) $(FFMPEG_LIB) $(DSHOW_LIB) $(PTHREAD_LIB)

$(OUTPUT_32):
	$(CC) $(INCLUDE) $(INCLUDE_32) $(SRC) /link /DLL /OUT:$(OUTPUT_32) $(LIB_DIR_32) $(FFMPEG_LIB) $(DSHOW_LIB) $(PTHREAD_LIB)

dist_64:$(OUTPUT_64)
	move $(OUTPUT_64) dist_x64 1> nul 2> nul
	copy $(DEPEND_64) dist_x64 1> nul 2> nul
	copy $(DEPEND2_64) dist_x64 1> nul 2> nul

dist_32:$(OUTPUT_32)
	move $(OUTPUT_32) dist_x86 1> nul 2> nul
	copy $(DEPEND_32) dist_x86 1> nul 2> nul
	copy $(DEPEND2_32) dist_x86 1> nul 2> nul

clean:
	@del *.obj *.exp *.lib *.dll 1> nul 2> nul