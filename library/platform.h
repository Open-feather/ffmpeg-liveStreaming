#ifdef _MSC_VER
#define inline __inline
#define snprintf(str,size,format,...) _snprintf_s(str,size-1,_TRUNCATE,format,__VA_ARGS__)
#define CAM_DRIVER "vfwcap"
#define CAM_DEVICE_NAME "0"
#else
#define CAM_DRIVER "v4l2"
#define CAM_DEVICE_NAME "/dev/video0"
#endif


