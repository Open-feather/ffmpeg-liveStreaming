@ECHO off


if [%1] == [clean] goto clean

SET VCVAR_SHELL="C:\PROGRA~2\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"

@echo Compiling 64 bit library
call %VCVAR_SHELL% amd64
mkdir dist_x64 > nul 2> nul
nmake /NOLOGO -f compile_lib.mk dist_64

@echo Compiling 32 bit library
call %VCVAR_SHELL% x86
mkdir dist_x86 > nul 2> nul
nmake /NOLOGO -f compile_lib.mk dist_32

goto :eof

:clean
del *.obj