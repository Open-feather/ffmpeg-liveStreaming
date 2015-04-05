using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace helloworld
{
    class Program
    {
		[DllImport("libstream_64")]
		public static extern IntPtr init_capture([MarshalAs(UnmanagedType.LPStr)]string m);
		[DllImport("libstream_64")]
		public static extern int pause_stream(IntPtr pausePtr,long duration);
		[DllImport("libstream_64")]
		public static extern int start_capture(IntPtr starPtr);
		[DllImport("libstream_64")]
		public static extern int stop_capture(IntPtr stopPtr);
		[DllImport("libstream_64")]
		public static extern int set_image(IntPtr cameraPtr,[MarshalAs(UnmanagedType.LPStr)]string file_path,int xpos,int ypos,int height,int width);
		public static int Main(string[] args)
        {
			int xpos = 200;
			int ypos = 200;
			int height = 200;
			int width = 200;
			int ret = 0;
			IntPtr cameraPtr;
			cameraPtr=init_capture("rtmp://192.168.1.114/myapp/mystream");
			if(cameraPtr==IntPtr.Zero)
			{
				Console.WriteLine("Please verify your camera is on\n");
				return -1;
			}
			set_image(cameraPtr,"test.jpg",xpos,ypos,height,width);
			ret = start_capture(cameraPtr);
			if(ret<0)
			{
				Console.WriteLine("Error while capturing video");
			}
			stop_capture(cameraPtr);
			return 0;
		}
	}
}
