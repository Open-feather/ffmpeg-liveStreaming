#include <iostream>
#include "stream.h"
using namespace std;
int main(int argc, char*argv[])
{
	void *ctx = init_capture("test.m3u8");
	if(!ctx)
		cout<<"unable to initialize capture"<<endl;
	start_capture(ctx);
	
	stop_capture(ctx);

}
