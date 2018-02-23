#pragma once
#include "ModuleInstrumenter.h"
class MetadataRegistry;

class TargetModuleInstrumenter : public ModuleInstrumenter
{
    mdTypeRef objectTypeRef;
    mdModuleRef profilerHostModuleRef;
    mdMethodDef getAppDomainMethodRef;
    mdMethodDef onMethodEnterRef, onMethodExitRef;  // External
    mdMethodDef methodEnterHookDef, methodExitHookDef;  // Internal ones to inject

    std::list<TypeInfo> GetModuleTypesToInstrument(const std::wstring &namespaceFilter) const;
    void AddProfilerCoreReferences();
    void AddHookBridgeType();
    void InstrumentMethods();

public:
    TargetModuleInstrumenter(
        MetadataRegistry *metadataRegistry,
        const CComPtr<ICorProfilerInfo5> &corInfos,
        ModuleID moduleId);
    ~TargetModuleInstrumenter();

    void Instrument() override;
};

