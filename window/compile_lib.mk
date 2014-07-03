SRC= ..\library\decoder.c ..\library\stream.c
FFMPEG_LIB= avcodec.lib avformat.lib avdevice.lib avfilter.lib avutil.lib swresample.lib postproc.lib swscale.lib
FFMPEG_VERSION_32=ffmpeg-20140609-git-6d40849-win32-dev
FFMPEG_VERSION_64=ffmpeg-20140702-git-42a92a2-win64-dev
INCLUDE_64= /I3rdparty\$(FFMPEG_VERSION_64)\include
INCLUDE_32= /I3rdparty\$(FFMPEG_VERSION_32)\include
INCLUDE= /I..\library  /I.
INCLUDE= $(INCLUDE) /I"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include"
OUTPUT_32=libstream_32.dll
OUTPUT_64=libstream_64.dll

all: libstream_32 libstream_64
	
libstream_64:
	$(CC) $(INCLUDE_64) $(INCLUDE) $(SRC) /link /DLL /OUT:$(OUTPUT_64) /LIBPATH:3rdparty\$(FFMPEG_VERSION_64)\lib $(FFMPEG_LIB)
	
libstream_32: 
	$(CC) $(INCLUDE) $(INCLUDE_32) $(SRC) /link /DLL  /MACHINE:X64  /OUT:$(OUTPUT_32) /LIBPATH:3rdparty\$(FFMPEG_VERSION_32)\lib $(FFMPEG_LIB)


