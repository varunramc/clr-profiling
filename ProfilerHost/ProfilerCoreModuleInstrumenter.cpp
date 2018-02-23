#include "stdafx.h"
#include "ProfilerCoreModuleInstrumenter.h"
#include "MscorlibModuleInstrumenter.h"

#include <pshpack1.h>
#include "ILRewriter.h"
#include "Globals.h"
#include "DebugILGenerator.h"

struct SetCallbackIL
{
    byte loadStaticField;
    mdFieldDef valueField;
    byte storeStaticField;
    mdFieldDef targetField;

    SetCallbackIL(mdFieldDef valueField, mdFieldDef targetField)
    {
        this->loadStaticField = CEE_LDSFLD;
        this->valueField = valueField;
        this->storeStaticField = CEE_STSFLD;
        this->targetField = targetField;
    }
};

struct InitializeMscorlibBridgeMethodIL
{
    byte header;
    SetCallbackIL setCtxCaptureCallback;
    SetCallbackIL setCtxResumeCallback;
    byte ret;

    InitializeMscorlibBridgeMethodIL(SetCallbackIL setCtxCaptureCallback, SetCallbackIL setCtxResumeCallback)
        : setCtxCaptureCallback(setCtxCaptureCallback), setCtxResumeCallback(setCtxResumeCallback)
    {
        byte ctorSize = sizeof(InitializeMscorlibBridgeMethodIL) - 1;
        this->header = CorILMethod_TinyFormat | (ctorSize << (CorILMethod_FormatShift - 1));

        this->ret = CEE_RET;
    }
};
#include <poppack.h>

ProfilerCoreModuleInstrumenter::ProfilerCoreModuleInstrumenter(
    MetadataRegistry *metadataRegistry,
    const CComPtr<ICorProfilerInfo5> &corInfos,
    ModuleID moduleId) : ModuleInstrumenter(metadataRegistry, corInfos, moduleId)
{
}

ProfilerCoreModuleInstrumenter::~ProfilerCoreModuleInstrumenter()
{
}

void ProfilerCoreModuleInstrumenter::GetBridgeReferences()
{
    ComOp(this->metaDataImport->FindTypeDefByName(L"ProfilerCore.ProfilingHook", NULL, &this->profilingHookType));

    ASSEMBLYMETADATA assemblyMetadata = { 0 };
    mdAssemblyRef mscorlibAssemblyRef;
    ComOp(this->metaDataAsmEmit->DefineAssemblyRef(
        nullptr,
        0,
        L"mscorlib",
        &assemblyMetadata,
        nullptr,
        0,
        0,
        &mscorlibAssemblyRef));
    ComOp(this->metaDataEmit->DefineTypeRefByName(
        mscorlibAssemblyRef, MSCORLIB_BRIDGE_TYPE_NAME, &this->mscorlibBridgeType));

    mdTypeRef actionType;
    ComOp(this->metaDataEmit->DefineTypeRefByName(
        mscorlibAssemblyRef, L"System.Action", &actionType));

    byte callbackSig[] = { IMAGE_CEE_CS_CALLCONV_FIELD, ELEMENT_TYPE_CLASS, 0, 0, 0, 0 };
    ULONG callbackSigLen = 2 + CorSigCompressToken(actionType, callbackSig + 2);

    ComOp(this->metaDataEmit->DefineMemberRef(
        this->mscorlibBridgeType, MSCORLIB_CTX_CAPTURE_DELEGATE_NAME, callbackSig, callbackSigLen, &this->ctxCaptureCallback));

    ComOp(this->metaDataEmit->DefineMemberRef(
        this->mscorlibBridgeType, MSCORLIB_CTX_RESUME_DELEGATE_NAME, callbackSig, callbackSigLen, &this->ctxResumeCallback));
}

void ProfilerCoreModuleInstrumenter::DefineBridgeInitializeMethod()
{
    mdFieldDef onExecCtxCaptured, onExecCtxResumed;
    ComOp(this->metaDataImport->FindField(this->profilingHookType, L"onExecCtxCaptured", nullptr, 0, &onExecCtxCaptured));
    ComOp(this->metaDataImport->FindField(this->profilingHookType, L"onExecCtxResumed", nullptr, 0, &onExecCtxResumed));

    byte signature[] = { IMAGE_CEE_CS_CALLCONV_DEFAULT, 0, ELEMENT_TYPE_VOID };
    ComOp(this->metaDataEmit->DefineMethod(
        this->profilingHookType,
        L"InitializeMscorlibBridge",
        mdStatic | mdPrivate,
        signature,
        sizeof(signature),
        0,
        miIL | miManaged,
        &this->initializeMscorlibBridgeMethod));

    InitializeMscorlibBridgeMethodIL initBridgeIL {
        SetCallbackIL {onExecCtxCaptured, this->ctxCaptureCallback },
        SetCallbackIL {onExecCtxResumed, this->ctxResumeCallback }
    };

    LPBYTE methodIL = static_cast<LPBYTE>(this->methodAlloc->Alloc(sizeof(initBridgeIL)));
    memcpy(methodIL, &initBridgeIL, sizeof(initBridgeIL));
    ComOp(this->corInfos->SetILFunctionBody(this->moduleId, this->initializeMscorlibBridgeMethod, methodIL));
}

void ProfilerCoreModuleInstrumenter::InjectCtorBridgeInitCall() const
{
    mdMethodDef profilingHookCtor;
    ComOp(this->metaDataImport->FindMethod(
        this->profilingHookType,
        L".cctor",
        nullptr,
        0,
        &profilingHookCtor));

    InjectEnterExitHooks(
        this->corInfos,
        nullptr,
        this->moduleId,
        profilingHookCtor,
        mdTokenNil,
        this->initializeMscorlibBridgeMethod,
        nullptr);
}

void ProfilerCoreModuleInstrumenter::Instrument()
{
    Logger->info("Instrumenting ProfilingHook with mscorlib bridge init method");

    this->GetBridgeReferences();
    this->DefineBridgeInitializeMethod();
    this->InjectCtorBridgeInitCall();
}
