SRC=..\test.cpp
INCLUDE=/I..\library /I"C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\include"

all:
	cl /EHsc $(INCLUDE) $(SRC) libstream_64.lib

clean:
	del *.obj test.exe