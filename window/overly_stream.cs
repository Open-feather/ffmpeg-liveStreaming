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
		public static extern IntPtr initWebCapture([MarshalAs(UnmanagedType.LPStr)]string m);
		[DllImport("libstream_64")]
		public static extern int pause_stream(IntPtr pausePtr, long duration);
		[DllImport("libstream_64")]
		public static extern int start_capture(IntPtr starPtr);
		[DllImport("libstream_64")]
		public static extern int stop_capture(IntPtr stopPtr);
		[DllImport("libstream_64")]
		public static extern int duplicate_overlayed_stream(IntPtr cameraPtr,int xpos,int ypos,int height,int width);
		public static void call_duplicate_overlayed_stream(IntPtr cameraPtr,int xpos,int ypos,int height,int width)
		{
			duplicate_overlayed_stream(cameraPtr,xpos,ypos,height,width);
		}
		public static int Main(string[] args)
        {
			int xpos = 200;
			int ypos = 200;
			int height = 200;
			int width = 200;
			int ret = 0;
			IntPtr cameraPtr;
			cameraPtr =	initWebCapture("rtmp://192.168.1.100/mytv");
			if(cameraPtr==IntPtr.Zero)
			{
				Console.WriteLine("Please verify your camera is on\n");
				return -1;
			}
			call_duplicate_overlayed_stream(cameraPtr,xpos,ypos,height,width);
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
