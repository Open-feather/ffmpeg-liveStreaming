//#define IMPORT_DLL "libstream_64
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace steam_video
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
	public static int Main(string[] args)
        {
			int ret = 0;
			IntPtr cameraPtr;
			cameraPtr =initWebCapture("rtmp://192.168.1.100/mytv");
			if(cameraPtr==IntPtr.Zero)
			{
				Console.WriteLine("Please verify your camera is on\n");
				return -1;
			}
			pause_stream(cameraPtr,100);
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
