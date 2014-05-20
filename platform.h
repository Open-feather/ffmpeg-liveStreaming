#ifdef _MSC_VER
#define inline __inline
#define snprintf(str,size,format,...) _snprintf_s(str,size-1,_TRUNCATE,format,__VA_ARGS__)
#endif
