#include <iostream>
#include "lib_wrapper.h"
#include <pthread.h>

using namespace std;

void *ask4image(void*arg)
{
	int xpos = 200;
	int ypos = 200;
	int height = 200;
	int width = 200;
	set_image(arg,"test.jpg",xpos,ypos,height,width);
	return NULL;
}
int main(int argc, char*argv[])
{
	int ret = 0;
	pthread_t tid;

	void *ctx = initWebCapture("out.m3u8");
	if(!ctx)
	{
		cout<<"Please verify your camera is On"<<endl;
		return -1;
	}

	/** create a thread which will set image */
	pthread_create(&tid,NULL,ask4image,ctx);

	ret = start_capture(ctx);
	if(ret < 0)
	{
		cout<<"error while capturing video"<<endl;
	}

	stop_capture(ctx);

}
