#define COBJMACROS
#include "platform.h"

#if defined (__MINGW32__) || defined (_MSC_VER)
#include <direct.h>
#include <Windows.h>
#include <winnt.h>
#include <winbase.h>
#include <dshow.h>
#endif
#include <pthread.h>
#include <string.h>
void get_devicename(char *str,int index)
{

#if defined (__MINGW32__) || defined (_MSC_VER)
	int ret;
	char *name = malloc(MAX_LEN);
	ICreateDevEnum *devenum = NULL;
	IEnumMoniker *classenum = NULL;
	IMoniker *m = NULL;
	int i = 0;
	*name = '\0';

	/** Initialize COM  */
	CoInitialize(NULL);

	ret = CoCreateInstance(&CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			&IID_ICreateDevEnum, (void **) &devenum);
	if (ret != S_OK)
	{
		printf("Could not enumerate system devices.\n");
		goto end;
	}
	ret = ICreateDevEnum_CreateClassEnumerator(devenum,
			&CLSID_VideoInputDeviceCategory, (IEnumMoniker **) &classenum, 0);
	if (ret != S_OK)
	{
		printf("Could not enumerate video devices.\n");
		goto end;
	}

	for (i = 0;i <= index && IEnumMoniker_Next(classenum, 1, &m, NULL) == S_OK;i++)
	{
		IPropertyBag *bag = NULL;
		VARIANT var;

		ret = IMoniker_BindToStorage(m, 0, 0, &IID_IPropertyBag, (void *) &bag);
		if (ret != S_OK)
		{
			printf("IMoniker_BindToStorage failed\n");
			goto fail;
		}

		var.vt = VT_BSTR;
		ret = IPropertyBag_Read(bag, L"FriendlyName", &var, NULL);
		if (ret != S_OK)
		{
			printf("IPropertyBag_Read failed\n");
			goto fail;
		}
		WideCharToMultiByte(CP_UTF8, 0, var.bstrVal, -1, name, MAX_LEN, 0, 0);
		snprintf(str,MAX_LEN,"video=%s",name);
fail:
		free(name);

		if (bag)
			IPropertyBag_Release(bag);
		IMoniker_Release(m);

	}
end:
	if(classenum)
		IEnumMoniker_Release(classenum);
	if (devenum)
		ICreateDevEnum_Release(devenum);

	CoUninitialize();

#else
	strcpy(str, LINUX_CAM_DEVICE_NAME);
#endif
	return;
}
#ifdef _MSC_VER
LARGE_INTEGER getFILETIMEoffset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return (t);
}
int clock_gettime(int X, struct timespec *tv)
{
    LARGE_INTEGER           t;
    FILETIME            f;
    double                  microseconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToMicroseconds;
    static int              initialized = 0;
    static BOOL             usePerformanceCounter = 0;

    if (!initialized) {
        LARGE_INTEGER performanceFrequency;
        initialized = 1;
        usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
        if (usePerformanceCounter) {
            QueryPerformanceCounter(&offset);
            frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
        } else {
            offset = getFILETIMEoffset();
            frequencyToMicroseconds = 10.;
        }
    }
    if (usePerformanceCounter) QueryPerformanceCounter(&t);
    else {
        GetSystemTimeAsFileTime(&f);
        t.QuadPart = f.dwHighDateTime;
        t.QuadPart <<= 32;
        t.QuadPart |= f.dwLowDateTime;
    }

    t.QuadPart -= offset.QuadPart;
    microseconds = (double)t.QuadPart / frequencyToMicroseconds;
    t.QuadPart = microseconds;
    tv->tv_sec = t.QuadPart / 1000000;
    tv->tv_nsec = t.QuadPart % 1000000;
    return (0);
}
#endif
