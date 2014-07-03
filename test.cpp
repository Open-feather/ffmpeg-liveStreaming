#include <iostream>
#include "lib_wrapper.h"
using namespace std;
int main(int argc, char*argv[])
{
	void *ctx = init_capture("test.m3u8");
	if(!ctx)
		cout<<"Please verify your camera is On"<<endl;
	start_capture(ctx);
	
	stop_capture(ctx);

}
