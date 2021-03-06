#ifndef _PLATFORM_H
#define PLATFORM_H

#define MAX_LEN 128

#if defined (__MINGW32__) || defined (_MSC_VER)
#define CAM_DRIVER "dshow"
#endif

#ifdef _MSC_VER
#define inline __inline
#define snprintf(str,size,format,...) _snprintf_s(str,size-1,_TRUNCATE,format,__VA_ARGS__)
#define OF_ATTR __stdcall
#define EXPORT __declspec(dllexport)
#include <direct.h>
#define GetCurrentDir _getcwd

/** Define Pos clock */

#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3
#define CLOCK_MONOTONIC_RAW 4
#define CLOCK_REALTIME_COARSE 5
#define CLOCK_MONOTONIC_COARSE 6
#define CLOCK_BOOTTIME 7
#define CLOCK_REALTIME_ALARM 8
#define CLOCK_BOOTTIME_ALARM 9

#elif defined (MINGW) || defined (__MINGW32__) || defined (_MSC_VER)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#define CAM_DRIVER "v4l2"
#define LINUX_CAM_DEVICE_NAME "/dev/video"
#endif

void get_devicename(char *str,int index);
#endif
