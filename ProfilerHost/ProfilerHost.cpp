// ProfilerHost.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ProfilerHost.h"


// This is an example of an exported variable
PROFILERHOST_API int nProfilerHost=0;

// This is an example of an exported function.
PROFILERHOST_API int fnProfilerHost(void)
{
    return 42;
}

// This is the constructor of a class that has been exported.
// see ProfilerHost.h for the class definition
CProfilerHost::CProfilerHost()
{
    return;
}
