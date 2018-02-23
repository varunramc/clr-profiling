#include "stdafx.h"
#include <fstream>
#include "Globals.h"
#include "ProfilingConfig.h"
#include "StringUtils.h"
#include "ErrorUtils.h"

struct __declspec(uuid("16F3E434-F45C-429A-A105-A8B4EE51A964")) IProfilerAssemblyResolver : IUnknown
{
    virtual HRESULT __stdcall ConfigureResolver() = 0;
};

LPSAFEARRAY GetFileBytes(std::wstring path)
{
    std::ifstream file{ path, std::ios::binary | std::ios::ate };

    if (!file.is_open())
    {
        Logger->error("File not found to read bytes from: {}", ToString(path));
        return nullptr;
    }

    auto fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), fileSize);

    file.close();

    SAFEARRAYBOUND safeArrayBound = { 0 };
    safeArrayBound.lLbound = 0;
    safeArrayBound.cElements = fileSize;

    LPBYTE fileBytes;
    LPSAFEARRAY fileBytesSafeArray = SafeArrayCreate(VT_UI1, 1, &safeArrayBound);
    ComOp(SafeArrayAccessData(fileBytesSafeArray, reinterpret_cast<void**>(&fileBytes)));
    memcpy(fileBytes, buffer.data(), fileSize);
    SafeArrayUnaccessData(fileBytesSafeArray);

    return fileBytesSafeArray;
}

extern "C" HRESULT STDAPICALLTYPE InjectProfilerCore(IUnknown *unknown)
{
    try
    {
        CComPtr<mscorlib::_AppDomain> appDomain;
        ComOp(unknown->QueryInterface(__uuidof(mscorlib::_AppDomain), reinterpret_cast<void**>(&appDomain)));

        LPSAFEARRAY dllBytes = GetFileBytes(ProfilingConfig::Get().GetProfilerCoreDllPath());
        CComPtr<mscorlib::_Assembly> assembly;
        HRESULT hr = appDomain->Load_3(dllBytes, &assembly);
        SafeArrayDestroy(dllBytes);
        if (FAILED(hr))
        {
            Logger->error("Failed to load core profiler DLL into AppDomain");
            return hr;
        }

        CComVariant comVariant;
        ComOp(assembly->CreateInstance(_bstr_t(L"ProfilerCore.ProfilerAssemblyResolver"), &comVariant));

        CComPtr<IProfilerAssemblyResolver> assemblyResolver;
        ComOp(comVariant.punkVal->QueryInterface(
            __uuidof(IProfilerAssemblyResolver), reinterpret_cast<void**>(&assemblyResolver)));

        return assemblyResolver->ConfigureResolver();
    }
    catch (_com_error &err)
    {
        LogComError(err, "injecting profiler core assembly");
    }

    return S_OK;
}
