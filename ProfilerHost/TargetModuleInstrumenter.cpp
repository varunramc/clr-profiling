#include "stdafx.h"
#include "TargetModuleInstrumenter.h"
#include "ProfilingConfig.h"
#include "Globals.h"
#include "StringUtils.h"
#include "ILRewriter.h"

#include <pshpack1.h>
struct HookBridgeCtorIL
{
    byte header;
    byte call1;
    mdMethodDef getAppDomain;
    byte call2;
    mdMethodDef injectProfilerCore;
    byte ret;

    HookBridgeCtorIL(mdMethodDef getAppDomainDef, mdMethodDef injectProfilerCoreDef)
    {
        byte ctorSize = sizeof(HookBridgeCtorIL) - 1;
        this->header = CorILMethod_TinyFormat | (ctorSize << (CorILMethod_FormatShift - 1));

        this->call1 = this->call2 = CEE_CALL;
        this->getAppDomain = getAppDomainDef;
        this->injectProfilerCore = injectProfilerCoreDef;
        this->ret = CEE_RET;
    }
};

struct MethodHookIL
{
    byte header;
    byte loadArg0;
    byte call;
    mdMethodDef hook;
    byte ret;

    MethodHookIL(mdMethodDef hookDef)
    {
        byte ctorSize = sizeof(MethodHookIL) - 1;
        this->header = CorILMethod_TinyFormat | (ctorSize << (CorILMethod_FormatShift - 1));

        this->loadArg0 = CEE_LDARG_0;
        this->call = CEE_CALL;
        this->hook = hookDef;
        this->ret = CEE_RET;
    }
};
#include <poppack.h>

byte methodHookSig[] = { IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_U8 };

TargetModuleInstrumenter::TargetModuleInstrumenter(
    MetadataRegistry *metadataRegistry,
    const CComPtr<ICorProfilerInfo5> &corInfos,
    ModuleID moduleId) : ModuleInstrumenter(metadataRegistry, corInfos, moduleId)
{
}


TargetModuleInstrumenter::~TargetModuleInstrumenter()
{
}

std::list<ModuleInstrumenter::TypeInfo> TargetModuleInstrumenter::GetModuleTypesToInstrument(
    const std::wstring& namespaceFilter) const
{
    std::list<TypeInfo> moduleTypes = this->GetModuleTypes();

    moduleTypes.remove_if([&](TypeInfo typeInfo) {
        return !this->IsTypeInNamespace(
            typeInfo.name.c_str(),
            namespaceFilter,
            typeInfo.id,
            typeInfo.typeDefFlags); });

    return moduleTypes;
}

void TargetModuleInstrumenter::AddProfilerCoreReferences()
{
    ASSEMBLYMETADATA assemblyMetadata = { 0 };
    mdAssemblyRef mscorlibAssemblyRef, profilerCoreAssemblyRef;

    ComOp(this->metaDataAsmEmit->DefineAssemblyRef(
        nullptr,
        0,
        L"mscorlib",
        &assemblyMetadata,
        nullptr,
        0,
        0,
        &mscorlibAssemblyRef));

    ComOp(this->metaDataAsmEmit->DefineAssemblyRef(
        nullptr,
        0,
        L"ProfilerCore",
        &assemblyMetadata,
        nullptr,
        0,
        0,
        &profilerCoreAssemblyRef));

    ComOp(this->metaDataEmit->DefineModuleRef(L"ProfilerHost", &this->profilerHostModuleRef));

    mdTypeRef appDomainTypeRef, profilingHookTypeRef;
    ComOp(this->metaDataEmit->DefineTypeRefByName(mscorlibAssemblyRef, L"System.Object", &this->objectTypeRef));
    ComOp(this->metaDataEmit->DefineTypeRefByName(mscorlibAssemblyRef, L"System.AppDomain", &appDomainTypeRef));
    ComOp(this->metaDataEmit->DefineTypeRefByName(
        profilerCoreAssemblyRef, L"ProfilerCore.ProfilingHook", &profilingHookTypeRef));

    byte getAppDomainSig[] = { IMAGE_CEE_CS_CALLCONV_DEFAULT, 0, ELEMENT_TYPE_CLASS, 0, 0, 0, 0, 0 };
    ULONG tokenSize = CorSigCompressToken(appDomainTypeRef, getAppDomainSig + 3);
    ComOp(this->metaDataEmit->DefineMemberRef(
        appDomainTypeRef, L"get_CurrentDomain", getAppDomainSig, 3 + tokenSize, &this->getAppDomainMethodRef));

    ComOp(this->metaDataEmit->DefineMemberRef(
        profilingHookTypeRef, L"OnMethodEnter", methodHookSig, sizeof(methodHookSig), &this->onMethodEnterRef));

    ComOp(this->metaDataEmit->DefineMemberRef(
        profilingHookTypeRef, L"OnMethodExit", methodHookSig, sizeof(methodHookSig), &this->onMethodExitRef));
}

void TargetModuleInstrumenter::AddHookBridgeType()
{
    mdTypeDef bridgeType;
    std::wstringstream bridgeTypeName;
    bridgeTypeName << L"ProfilerHookBridge" << GetTickCount();
    ComOp(this->metaDataEmit->DefineTypeDef(
        bridgeTypeName.str().c_str(), tdClass | tdAbstract | tdSealed, this->objectTypeRef, nullptr, &bridgeType));

    mdMethodDef injectProfilerCoreMethod;
    byte injectProfilerCoreSig[] = { IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_OBJECT };
    ComOp(this->metaDataEmit->DefineMethod(
        bridgeType,
        L"InjectProfilerCore",
        mdStatic | mdPinvokeImpl,
        injectProfilerCoreSig,
        sizeof(injectProfilerCoreSig),
        0,
        0,
        &injectProfilerCoreMethod));

    // P/Invoke setup
    byte iunknownType = NATIVE_TYPE_IUNKNOWN;
    mdParamDef appDomainParamDef;
    ComOp(this->metaDataEmit->DefinePinvokeMap(injectProfilerCoreMethod, 0, L"InjectProfilerCore", this->profilerHostModuleRef));
    ComOp(this->metaDataEmit->DefineParam(
        injectProfilerCoreMethod, 1, L"appDomain", pdIn | pdHasFieldMarshal, 0, nullptr, 0, &appDomainParamDef));
    ComOp(this->metaDataEmit->SetFieldMarshal(appDomainParamDef, &iunknownType, sizeof(iunknownType)));

    // Static ctor setup
    byte bridgeCtorSig[] = { IMAGE_CEE_CS_CALLCONV_DEFAULT, 0, ELEMENT_TYPE_VOID };
    HookBridgeCtorIL ctorIL{ this->getAppDomainMethodRef, injectProfilerCoreMethod };
    mdMethodDef bridgeCtor;
    ComOp(this->metaDataEmit->DefineMethod(
        bridgeType,
        L".cctor",
        mdStatic | mdPublic | mdSpecialName | mdRTSpecialName | mdHideBySig,
        bridgeCtorSig,
        sizeof(bridgeCtorSig),
        0,
        miIL | miManaged | miPreserveSig,
        &bridgeCtor));
    LPBYTE methodIL = static_cast<LPBYTE>(this->methodAlloc->Alloc(sizeof(ctorIL)));
    memcpy(methodIL, &ctorIL, sizeof(ctorIL));
    ComOp(this->corInfos->SetILFunctionBody(this->moduleId, bridgeCtor, methodIL));

    // OnMethodEnter hook
    MethodHookIL entryHookIL{ this->onMethodEnterRef };
    ComOp(this->metaDataEmit->DefineMethod(
        bridgeType,
        L"OnMethodEnter",
        mdStatic | mdPublic | mdHideBySig,
        methodHookSig,
        sizeof(methodHookSig),
        0,
        miIL | miManaged,
        &this->methodEnterHookDef));
    methodIL = static_cast<LPBYTE>(this->methodAlloc->Alloc(sizeof(entryHookIL)));
    memcpy(methodIL, &entryHookIL, sizeof(entryHookIL));
    ComOp(this->corInfos->SetILFunctionBody(this->moduleId, this->methodEnterHookDef, methodIL));

    // OnMethodExit hook
    MethodHookIL exitHookIL{ this->onMethodExitRef };
    ComOp(this->metaDataEmit->DefineMethod(
        bridgeType,
        L"OnMethodExit",
        mdStatic | mdPublic | mdHideBySig,
        methodHookSig,
        sizeof(methodHookSig),
        0,
        miIL | miManaged,
        &this->methodExitHookDef));
    methodIL = static_cast<LPBYTE>(this->methodAlloc->Alloc(sizeof(exitHookIL)));
    memcpy(methodIL, &exitHookIL, sizeof(exitHookIL));
    ComOp(this->corInfos->SetILFunctionBody(this->moduleId, this->methodExitHookDef, methodIL));
}

void TargetModuleInstrumenter::InstrumentMethods()
{
    auto moduleTypes = this->GetModuleTypesToInstrument(ProfilingConfig::Get().GetNamespaceToProfile());
    if (!moduleTypes.size())
    {
        Logger->info("No types in {} found for profiling", ToString(this->moduleName));
        return;
    }

    Logger->info("Instrumenting {} type(s) in module {}", moduleTypes.size(), ToString(this->moduleName));

    for (auto type : moduleTypes)
    {
        std::list<mdMethodDef> typeMethodDefs = this->GetTypeMethods(type.id);

        for (auto methodDef : typeMethodDefs)
        {
            auto methodDesc = this->GetMethodDescription(methodDef);
            auto methodMetadata =
                this->metadataRegistry->RegisterMethod(
                    this->moduleName,
                    type.name,
                    methodDesc);
            Logger->info("> Instrumenting {} with token {}", ToString(methodDesc), methodMetadata.token);

            InjectEnterExitHooks(
                this->corInfos,
                nullptr,
                this->moduleId,
                methodDef,
                this->methodEnterHookDef,
                this->methodExitHookDef,
                &methodMetadata.token);
        }
    }
}

void TargetModuleInstrumenter::Instrument()
{
    this->AddProfilerCoreReferences();
    this->AddHookBridgeType();
    this->InstrumentMethods();
}
