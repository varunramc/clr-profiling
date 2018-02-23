#pragma once

#ifndef  _MT
#   error "Multithreading is required"
#endif

#ifndef _UNICODE
#   define_UNICODE
#endif

#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <time.h>
#include <locale.h>
#include <malloc.h>
#include <exception>
#include <vector>
#include <list>
#include <string>
#include <set>
#include <map>
#include <stack>
#include <limits>
#include <memory>
#include <sstream>
#include <algorithm>

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <strsafe.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <ole2.h>
#include <olectl.h>
#include <objbase.h>
#include <objidl.h>
#include <shlobj.h>
#include <oleauto.h>

#include <comdef.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <atlbase.h>
#include <atlstr.h>

#include <corerror.h>
#include <cor.h>
#include <corhdr.h>
#include <corprof.h>
#include <corhlpr.h>
#include <corsym.h>
#include <mscoree.h>
#import  <mscorlib.tlb> named_guids raw_interfaces_only \
    rename("ReportEvent","netReportEvent") exclude("IObjectHandle","_Stream")

#include "spdlog/spdlog.h"

#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"oleaut32.lib")
#pragma comment(lib,"uuid.lib")

#if _WIN64
#   if _DEBUG
#       pragma comment(lib, "libprotobufd.lib")
#   else
#       pragma comment(lib, "libprotobuf.lib")
#   endif
#endif

#pragma comment(lib, "gpr.lib")
#pragma comment(lib, "grpc.lib")
#pragma comment(lib, "grpc++.lib")
#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "ssleay32.lib")
#pragma comment(lib, "libeay32.lib")

inline HRESULT ComOp(HRESULT hr)
{
    if (FAILED(hr))
    {
        IErrorInfo *piErrInfo;
        HRESULT hr2 = GetErrorInfo(0, &piErrInfo);

        if (hr2 != S_OK)
        {
            piErrInfo = nullptr;
        }

        _com_raise_error(hr, piErrInfo);
    }

    return hr;
}
