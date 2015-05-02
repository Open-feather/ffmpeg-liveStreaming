#include <iostream>
#include "lib_wrapper.h"
using namespace std;
int main(int argc, char*argv[])
{
	int xpos = 200;
	int ypos = 200;
	int height = 200;
	int width = 200;
	int ret = 0;
	void *ctx = NULL;

	if(argc < 2)
	{
		cout<<"Usage "<<argv[0]<<" rtmp://ipaddress/App"<<endl;
		return -1;
	}

	ctx = initRtmpCapture(argv[1], "save.flv");
	if(!ctx)
	{
		cout<<"Please verify your camera is On"<<endl;
		return -1;
	}

	ret = start_capture(ctx);
	if(ret < 0)
	{
		cout<<"error while capturing video"<<endl;
	}
	stop_capture(ctx);


}
