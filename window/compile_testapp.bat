@echo off
IF [%1] == [clean] goto clean

SET VCVAR_SHELL="C:\PROGRA~2\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"

@echo Compiling 64 bit test Application
call %VCVAR_SHELL% amd64
mkdir dist_x64 > nul 2> nul
nmake /nologo -f compile_testapp.mk all

REM @echo Compiling 32 bit library
REM call %VCVAR_SHELL% x86
REM mkdir dist_x86 > nul 2> nul
REM nmake -f compile_lib.mk dist_32

goto :eof

:clean
nmake /NOLOGO -f compile_testapp.mk clean