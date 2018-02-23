// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ClassFactory.h"
#include <iostream>

const IID IID_IUnknown = { 0x00000000, 0x0000, 0x0000,{ 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };

const IID IID_IClassFactory = { 0x00000001, 0x0000, 0x0000,{ 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };

extern "C" HRESULT STDMETHODCALLTYPE DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    // {af2ec6c0-ce72-4d0b-9891-6b4e416813f5}
    const GUID CLSID_CorProfiler = {
        0xaf2ec6c0,
        0xce72,
        0x4d0b,
        { 0x98, 0x91, 0x6b, 0x4e, 0x41, 0x68, 0x13, 0xf5 }
    };

    if (ppv == nullptr || rclsid != CLSID_CorProfiler)
    {
        return E_FAIL;
    }

    auto factory = new ClassFactory();
    if (factory == nullptr)
    {
        return E_FAIL;
    }

    return factory->QueryInterface(riid, ppv);
}

extern "C" HRESULT STDMETHODCALLTYPE DllCanUnloadNow()
{
    return S_OK;
}

BOOL STDMETHODCALLTYPE DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        std::wcout << L">>> Profiler host attached to process" << std::endl;
        break;

    case DLL_THREAD_ATTACH:
        std::wcout << L">>> Profiler host attached to thread" << std::endl;
        break;

    case DLL_THREAD_DETACH:
        std::wcout << L">>> Profiler host unloaded from thread" << std::endl;
        break;

    case DLL_PROCESS_DETACH:
        std::wcout << L">>> Profiler host unloaded from process" << std::endl;
        break;
    }

    return true;
}
