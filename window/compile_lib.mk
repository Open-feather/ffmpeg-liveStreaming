SRC= ..\library\decoder.c ..\library\stream.c
FFMPEG_LIB= avcodec.lib avformat.lib avdevice.lib avfilter.lib avutil.lib swresample.lib postproc.lib swscale.lib
FFMPEG_VERSION_32=ffmpeg-20140609-git-6d40849-win32-dev
FFMPEG_VERSION_64=ffmpeg-20140619-git-ab12373-win64-dev
INCLUDE= /I..\library /I3party\$(FFMPEG_VERSION_32)\include /I.
INCLUDE= $(INCLUDE) /I"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include"
OUTPUT_32=libstream_32.dll
OUTPUT_64=libstream_64.dll

all: libstream_32 libstream_64
	
libstream_64:
	$(CC) $(INCLUDE) $(SRC) /link /DLL /OUT:$(OUTPUT_64) /LIBPATH:3party\$(FFMPEG_VERSION_64)\lib $(FFMPEG_LIB)
	
libstream_32: 
	$(CC) $(INCLUDE) $(SRC) /link /DLL  /MACHINE:X64  /OUT:$(OUTPUT_32) /LIBPATH:3party\$(FFMPEG_VERSION_32)\lib $(FFMPEG_LIB)


