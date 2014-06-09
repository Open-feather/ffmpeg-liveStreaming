SRC= ..\library\decoder.c ..\library\stream.c
FFMPEG_LIB= avcodec.lib avformat.lib avdevice.lib avfilter.lib avutil.lib swresample.lib postproc.lib swscale.lib
FFMPEG_VERSION=ffmpeg-20140609-git-6d40849-win32-dev
INCLUDE= /I..\library /I3party\$(FFMPEG_VERSION)\include /I.
INCLUDE= $(INCLUDE) /I"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include"
OUTPUT=libstream_32.dll
all:
	$(CC) $(INCLUDE) $(SRC) /link /DLL /OUT:$(OUTPUT) /LIBPATH:3party\$(FFMPEG_VERSION)\lib $(FFMPEG_LIB)

