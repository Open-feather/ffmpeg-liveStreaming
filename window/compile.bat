REM Iincluded
set src= ..\library\decoder.c ..\library\stream.c
set ffmpeg_lib= avcodec.lib avformat.lib avdevice.lib avfilter.lib avutil.lib swresample.lib postproc.lib swscale.lib
set include_flag= /I..\library /I3party\ffmpeg-20140608-git-d4be3a8-win32-dev\include /I.
REM compile library
cl %include_flag% %src% /link /DLL /OUT:libstream.dll /LIBPATH:3party\ffmpeg-20140608-git-d4be3a8-win32-dev\lib %ffmpeg_lib%

