#include "stdafx.h"
#include "ModuleInstrumenterFactory.h"
#include "TargetModuleInstrumenter.h"
#include "MscorlibModuleInstrumenter.h"
#include "ProfilerCoreModuleInstrumenter.h"

std::unique_ptr<ModuleInstrumenter> ModuleInstrumenterFactory::Create(
    MetadataRegistry *metadataRegistry,
    const CComPtr<ICorProfilerInfo5> &corInfos,
    ModuleID moduleId)
{
    LPCBYTE baseLoadAddress;
    wchar_t moduleFileName[512];
    wchar_t assemblyName[256];
    AssemblyID assemblyId;
    ULONG strLength;

    ComOp(
        corInfos->GetModuleInfo(
            moduleId,
            &baseLoadAddress,
            sizeof(moduleFileName) / sizeof(moduleFileName[0]),
            &strLength,
            moduleFileName,
            &assemblyId));

    ComOp(
        corInfos->GetAssemblyInfo(
            assemblyId,
            sizeof(assemblyName) / sizeof(assemblyName[0]),
            &strLength,
            assemblyName,
            nullptr,
            nullptr));

    if (!wcscmp(assemblyName, L"mscorlib"))
    {
        return std::unique_ptr<ModuleInstrumenter>(
            std::make_unique<MscorlibModuleInstrumenter>(metadataRegistry, corInfos, moduleId));
    }

    if (!wcscmp(assemblyName, L"ProfilerCore"))
    {
        return std::unique_ptr<ModuleInstrumenter>(
            std::make_unique<ProfilerCoreModuleInstrumenter>(metadataRegistry, corInfos, moduleId));
    }

    return std::unique_ptr<ModuleInstrumenter>(
        std::make_unique<TargetModuleInstrumenter>(metadataRegistry, corInfos, moduleId));
}
