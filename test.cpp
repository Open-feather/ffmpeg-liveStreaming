#include <iostream>
#include "lib_wrapper.h"
using namespace std;
int main(int argc, char*argv[])
{
	int xpos = 10;
	int ypos = 10;
	int height = 200;
	int width = 200;
	int ret = 0;

	void *ctx = init_capture("out.m3u8");
	if(!ctx)
	{
		cout<<"Please verify your camera is On"<<endl;
		return -1;
	}

	set_image(ctx,"test.jpg",xpos,ypos,height,width);

	ret = start_capture(ctx);
	if(ret < 0)
	{
		cout<<"error while capturing video"<<endl;
	}
	
	stop_capture(ctx);

}
