#pragma once
#include "ModuleInstrumenter.h"

#define MSCORLIB_BRIDGE_TYPE_NAME           L"MscorlibBridgeType"
#define MSCORLIB_CTX_CAPTURE_DELEGATE_NAME  L"executionContextCapturedCallback"
#define MSCORLIB_CTX_RESUME_DELEGATE_NAME   L"executionContextResumedCallback"

class MscorlibModuleInstrumenter : public ModuleInstrumenter
{
    mdTypeDef objectTypeDef, bridgeTypeDef;
    mdMethodDef onExecCtxCaptureHookDef;
    mdMethodDef onExecCtxResumeHookDef;

    void AddMscorlibBridgeType();
    bool AddMethodHook(std::wstring typeName, const std::wstring& methodName, int requiredParamCount, mdMethodDef hook);
    void InstrumentContextFlowMethods();
    mdMethodDef DefineCallback(const std::wstring& fieldName, const std::wstring& methodName) const;

public:
    MscorlibModuleInstrumenter(
        MetadataRegistry *metadataRegistry,
        const CComPtr<ICorProfilerInfo5> &corInfos,
        ModuleID moduleId);
    ~MscorlibModuleInstrumenter();

    void Instrument() override;
};

