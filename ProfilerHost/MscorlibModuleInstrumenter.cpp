#include "stdafx.h"
#include "MscorlibModuleInstrumenter.h"
#include "Globals.h"
#include "ILRewriter.h"
#include "DebugILGenerator.h"

#include <pshpack1.h>
struct ActionProxyMethodIL
{
    byte header;
    byte loadStaticField;
    mdFieldDef actionField;
    byte call;
    mdMethodDef actionInvokeMethod;
    byte ret;

    ActionProxyMethodIL(mdFieldDef actionFieldDef, mdMethodDef actionInvokeDef)
    {
        byte ctorSize = sizeof(ActionProxyMethodIL) - 1;
        this->header = CorILMethod_TinyFormat | (ctorSize << (CorILMethod_FormatShift - 1));

        this->loadStaticField = CEE_LDSFLD;
        this->actionField = actionFieldDef;
        this->call = CEE_CALLVIRT;
        this->actionInvokeMethod = actionInvokeDef;
        this->ret = CEE_RET;
    }
};
#include <poppack.h>

byte voidParamlessStaticSig[] = { IMAGE_CEE_CS_CALLCONV_DEFAULT, 0, ELEMENT_TYPE_VOID };
byte voidParamlessInstanceSig[] = { IMAGE_CEE_CS_CALLCONV_HASTHIS, 0, ELEMENT_TYPE_VOID };

MscorlibModuleInstrumenter::MscorlibModuleInstrumenter(
    MetadataRegistry *metadataRegistry,
    const CComPtr<ICorProfilerInfo5> &corInfos,
    ModuleID moduleId) : ModuleInstrumenter(metadataRegistry, corInfos, moduleId)
{
}

MscorlibModuleInstrumenter::~MscorlibModuleInstrumenter()
{
}

mdMethodDef MscorlibModuleInstrumenter::DefineCallback(
    const std::wstring& fieldName,
    const std::wstring& methodName) const
{
    mdTypeDef actionType;
    ComOp(this->metaDataImport->FindTypeDefByName(L"System.Action", NULL, &actionType));

    // Define the callback fields
    byte actionFieldSig[] = { IMAGE_CEE_CS_CALLCONV_FIELD, ELEMENT_TYPE_CLASS, 0, 0, 0, 0 };
    ULONG actionFieldSigLen = 2 + CorSigCompressToken(actionType, actionFieldSig + 2);
    mdFieldDef fieldDef;
    ComOp(this->metaDataEmit->DefineField(
        this->bridgeTypeDef,
        fieldName.c_str(),
        fdPublic | fdStatic,
        actionFieldSig,
        actionFieldSigLen,
        ELEMENT_TYPE_END,
        nullptr,
        0,
        &fieldDef));

    // Define the callback methods
    mdMethodDef actionInvokeDef;
    ComOp(this->metaDataImport->FindMethod(
        actionType,
        L"Invoke",
        voidParamlessInstanceSig,
        sizeof(voidParamlessInstanceSig),
        &actionInvokeDef));

    mdMethodDef hookDef;
    ComOp(this->metaDataEmit->DefineMethod(
        this->bridgeTypeDef,
        methodName.c_str(),
        mdStatic | mdPublic | mdHideBySig,
        voidParamlessStaticSig,
        sizeof(voidParamlessStaticSig),
        0,
        miIL | miManaged,
        &hookDef));

    ActionProxyMethodIL callbackIl{ fieldDef, actionInvokeDef };
    LPBYTE methodIL = static_cast<LPBYTE>(this->methodAlloc->Alloc(sizeof(callbackIl)));
    memcpy(methodIL, &callbackIl, sizeof(callbackIl));
    ComOp(this->corInfos->SetILFunctionBody(this->moduleId, hookDef, methodIL));

    return hookDef;
}

void MscorlibModuleInstrumenter::AddMscorlibBridgeType()
{
    ComOp(this->metaDataImport->FindTypeDefByName(L"System.Object", NULL, &this->objectTypeDef));

    ComOp(this->metaDataEmit->DefineTypeDef(
        MSCORLIB_BRIDGE_TYPE_NAME, tdClass | tdAbstract | tdSealed | tdPublic, this->objectTypeDef, nullptr, &this->bridgeTypeDef));

    this->onExecCtxCaptureHookDef = this->DefineCallback(MSCORLIB_CTX_CAPTURE_DELEGATE_NAME, L"OnExecutionContextCaptured");
    this->onExecCtxResumeHookDef = this->DefineCallback(MSCORLIB_CTX_RESUME_DELEGATE_NAME, L"OnExecutionContextResumed");
}

bool MscorlibModuleInstrumenter::AddMethodHook(
    std::wstring typeName, const std::wstring& methodName, int requiredParamCount, mdMethodDef hook)
{
    mdTypeDef typeDef;
    ComOp(this->metaDataImport->FindTypeDefByName(typeName.c_str(), NULL, &typeDef));

    std::list<MethodInfo> methods = this->GetMethodsByName(typeDef, methodName);
    std::list<MethodInfo>::const_iterator captureMethod = std::find_if(
        methods.begin(), methods.end(), [=](const MethodInfo& mi) { return mi.parameterCount == requiredParamCount; });

    if (captureMethod == methods.end())
    {
        return false;
    }

    return InjectEnterExitHooks(
        this->corInfos,
        nullptr,
        this->moduleId,
        captureMethod->id,
        hook,
        mdTokenNil,
        nullptr) == S_OK;
}

void MscorlibModuleInstrumenter::InstrumentContextFlowMethods()
{
    Logger->info("> Instrumenting ExecutionContext.Capture()");

    if (!this->AddMethodHook(
        L"System.Threading.ExecutionContext",
        L"Capture",
        2,
        this->onExecCtxCaptureHookDef))
    {
        throw std::runtime_error("Failed to find ExecutionContext.Capture method");
    }

    Logger->info("> Instrumenting ExecutionContext.RunInternal()");

    if (!this->AddMethodHook(
        L"System.Threading.ExecutionContext",
        L"RunInternal",
        4,
        this->onExecCtxResumeHookDef))
    {
        throw std::runtime_error("Failed to find ExecutionContext.RunInternal method");
    }
}

void MscorlibModuleInstrumenter::Instrument()
{
    Logger->info("Instrumenting mscorlib types");

    this->AddMscorlibBridgeType();
    this->InstrumentContextFlowMethods();
}
