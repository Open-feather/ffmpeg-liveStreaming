using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace duplicate_stream
{
	public enum DuplicateFormat {SIDE_BY_SIDE, TOP_N_BOTTOM};
    class Program
    {
		[DllImport("libstream_64")]
		public static extern IntPtr init_capture([MarshalAs(UnmanagedType.LPStr)]string m);
		[DllImport("libstream_64")]
		public static extern int start_capture(IntPtr starPtr);
		[DllImport("libstream_64")]
		public static extern int stop_capture(IntPtr stopPtr);
		[DllImport("libstream_64")]
		public static extern int duplicate_stream(IntPtr cameraPtr, DuplicateFormat TOP_N_BOTTOM);
		public static void call_duplicate_stream(IntPtr cameraPtr, DuplicateFormat TOP_N_BOTTOM)
		{
			duplicate_stream(cameraPtr,TOP_N_BOTTOM);
		}
        public static int Main(string[] args)
        {
			int ret = 0;
			IntPtr cameraPtr;
			DuplicateFormat enumFormat = DuplicateFormat.TOP_N_BOTTOM;
			cameraPtr =	init_capture("rtmp://192.168.1.114/myapp/mystream");
			if(cameraPtr==IntPtr.Zero)
			{
				Console.WriteLine("Please verify your camera is on\n");
				return -1;
			}
			call_duplicate_stream(cameraPtr,enumFormat);
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
