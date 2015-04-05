#include <iostream>
#include "lib_wrapper.h"
using namespace std;
int main(int argc, char*argv[])
{
	int ret = 0;

	if(argc < 3 ) {
		cout<<"Usage "<<argv[0]<<" "<<"<input> <output>\n";
		cout<<"Usage "<<argv[0]<<" "<<"\"tcp://127.0.0.1:5099?listen\" \"rtsp://localhost:6565/live.sdp\"\n";
		return -1;
	}

//	void *ctx = init_bitstream("tcp://127.0.0.1:5099?listen", "rtsp://localhost:6565/live.sdp");
	void *ctx = init_bitstream(argv[1], argv[2]);

	if(!ctx)
	{
		cout<<"Please verify you of checked transmit"<<endl;
		return -1;
	}

	ret = start_bitstream(ctx);
	if(ret < 0)
	{
		cout<<"error while capturing video"<<endl;
	}
	stop_bitstream(ctx);


}
