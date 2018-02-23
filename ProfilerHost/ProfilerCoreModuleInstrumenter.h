#pragma once
#include "ModuleInstrumenter.h"

class ProfilerCoreModuleInstrumenter : public ModuleInstrumenter
{
    mdTypeDef profilingHookType;
    mdTypeRef mscorlibBridgeType;
    mdMemberRef ctxCaptureCallback, ctxResumeCallback;
    mdMethodDef initializeMscorlibBridgeMethod;

public:
    ProfilerCoreModuleInstrumenter(
        MetadataRegistry *metadataRegistry,
        const CComPtr<ICorProfilerInfo5> &corInfos,
        ModuleID moduleId);
    ~ProfilerCoreModuleInstrumenter();
    void GetBridgeReferences();
    void DefineBridgeInitializeMethod();
    void InjectCtorBridgeInitCall() const;

    void Instrument() override;
};

