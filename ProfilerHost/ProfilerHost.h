// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PROFILERHOST_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PROFILERHOST_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef PROFILERHOST_EXPORTS
#define PROFILERHOST_API __declspec(dllexport)
#else
#define PROFILERHOST_API __declspec(dllimport)
#endif

// This class is exported from the ProfilerHost.dll
class PROFILERHOST_API CProfilerHost {
public:
	CProfilerHost(void);
	// TODO: add your methods here.
};

extern PROFILERHOST_API int nProfilerHost;

PROFILERHOST_API int fnProfilerHost(void);
