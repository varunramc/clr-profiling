#include "stdafx.h"
#include "ModuleInstrumenter.h"
#include "Globals.h"
#include <regex>

ModuleInstrumenter::ModuleInstrumenter(
    MetadataRegistry *metadataRegistry,
    const CComPtr<ICorProfilerInfo5>& corInfos,
    ModuleID moduleId)
    : metadataRegistry(metadataRegistry), corInfos(corInfos), moduleId(moduleId)
{
    ComOp(corInfos->GetILFunctionBodyAllocator(moduleId, &this->methodAlloc));
    ComOp(corInfos->GetModuleMetaData(
        moduleId,
        ofRead | ofWrite,
        IID_IMetaDataEmit,
        reinterpret_cast<IUnknown**>(&this->metaDataEmit)));

    ComOp(this->metaDataEmit->QueryInterface(
        IID_IMetaDataAssemblyEmit,
        reinterpret_cast<void**>(&this->metaDataAsmEmit)));
    ComOp(this->metaDataEmit->QueryInterface(
        IID_IMetaDataImport2,
        reinterpret_cast<void**>(&this->metaDataImport)));

    this->ReadModuleInfo();
}

ModuleInstrumenter::~ModuleInstrumenter()
{
}

std::list<ModuleInstrumenter::TypeInfo> ModuleInstrumenter::GetModuleTypes() const
{
    HCORENUM hCorEnum = nullptr;
    mdTypeDef typeDefs[164];
    ULONG enumCount;
    std::list<TypeInfo> moduleTypes;

    while (
        ComOp(this->metaDataImport->EnumTypeDefs(
            &hCorEnum,
            typeDefs,
            sizeof(typeDefs) / sizeof(typeDefs[0]),
            &enumCount)) == S_OK)
    {
        for (ULONG i = 0; i < enumCount; i++)
        {
            wchar_t typeName[1024];
            ULONG typeNameLength = sizeof(typeName) / sizeof(typeName[0]);
            DWORD typeDefFlags;

            if (ComOp(this->metaDataImport->GetTypeDefProps(
                typeDefs[i],
                typeName,
                typeNameLength,
                &typeNameLength,
                &typeDefFlags,
                nullptr)) == S_OK)
            {
                TypeInfo info;
                info.id = typeDefs[i];
                info.name = typeName;
                info.typeDefFlags = typeDefFlags;
                moduleTypes.push_back(info);
            }
            else
            {
                Logger->error("Failed to parse name from typeDef {}", typeDefs[i]);
            }
        }
    }

    this->metaDataImport->CloseEnum(hCorEnum);

    return moduleTypes;
}

bool ModuleInstrumenter::IsTypeInNamespace(
    const wchar_t* typeName, const std::wstring& namespaceFilter, mdTypeDef typeDef, DWORD typeDefFlags) const
{
    return
        !std::wcsncmp(typeName, namespaceFilter.c_str(), namespaceFilter.length())
        || (this->IsNestedType(typeDefFlags) && this->IsNestedTypeInNamespace(typeDef, namespaceFilter));
}

bool ModuleInstrumenter::IsNestedType(DWORD typeDefFlags)
{
    return
        typeDefFlags & tdNestedPublic
        || typeDefFlags & tdNestedAssembly
        || typeDefFlags & tdNestedFamANDAssem
        || typeDefFlags & tdNestedFamORAssem
        || typeDefFlags & tdNestedFamily
        || typeDefFlags & tdNestedPrivate;
}

bool ModuleInstrumenter::IsNestedTypeInNamespace(mdTypeDef typeDef, const std::wstring& namespaceFilter) const
{
    while (true)
    {
        mdTypeDef enclosingClassTypeDef;
        if (FAILED(this->metaDataImport->GetNestedClassProps(typeDef, &enclosingClassTypeDef)))
        {
            return false;
        }

        wchar_t typeName[1024];
        ULONG typeNameLength = sizeof(typeName) / sizeof(typeName[0]);
        DWORD typeDefFlags;

        if (FAILED(ComOp(this->metaDataImport->GetTypeDefProps(
            enclosingClassTypeDef,
            typeName,
            typeNameLength,
            &typeNameLength,
            &typeDefFlags,
            nullptr))))
        {
            return false;
        }

        if (!std::wcsncmp(typeName, namespaceFilter.c_str(), namespaceFilter.length()))
        {
            return true;
        }

        if (!this->IsNestedType(typeDefFlags))
        {
            return false;
        }

        typeDef = enclosingClassTypeDef;
    }
}

std::list<mdMethodDef> ModuleInstrumenter::GetTypeMethods(mdTypeDef typeDef) const
{
    HCORENUM hCorEnum = nullptr;
    std::list<mdMethodDef> methods;
    mdMethodDef enumeratedMethods[64];
    ULONG enumCount;

    while (
        ComOp(this->metaDataImport->EnumMethods(
            &hCorEnum,
            typeDef,
            enumeratedMethods,
            sizeof(enumeratedMethods) / sizeof(enumeratedMethods[0]),
            &enumCount)) == S_OK)
    {
        methods.insert(methods.end(), std::begin(enumeratedMethods), std::begin(enumeratedMethods) + enumCount);
    }

    this->metaDataImport->CloseEnum(hCorEnum);

    return methods;
}

ULONG GetParameterCountFromMethodSig(PCCOR_SIGNATURE signature)
{
    ULONG index = 1;

    if (signature[0] & IMAGE_CEE_CS_CALLCONV_GENERIC)
    {
        ULONG typeParameterCount;
        index += CorSigUncompressData(signature + index, &typeParameterCount);
    }

    ULONG paramCount;
    CorSigUncompressData(signature + index, &paramCount);
    return paramCount;
}

std::list<ModuleInstrumenter::MethodInfo> ModuleInstrumenter::GetMethodsByName(
    mdTypeDef typeDef, const std::wstring& name) const
{
    HCORENUM hCorEnum = nullptr;
    std::list<MethodInfo> methods;
    mdMethodDef enumeratedMethods[64];
    ULONG enumCount;

    while (
        ComOp(this->metaDataImport->EnumMethodsWithName(
            &hCorEnum,
            typeDef,
            name.c_str(),
            enumeratedMethods,
            sizeof(enumeratedMethods) / sizeof(enumeratedMethods[0]),
            &enumCount)) == S_OK)
    {
        for (int i=0 ; i < enumCount; i++)
        {
            wchar_t methodName[1024];
            ULONG methodNameLength;
            mdTypeDef implementingType;
            DWORD methodAttributes;
            PCCOR_SIGNATURE sigBlob;
            ULONG sigLen;

            this->metaDataImport->GetMethodProps(
                enumeratedMethods[i],
                &implementingType,
                methodName,
                1024,
                &methodNameLength,
                &methodAttributes,
                &sigBlob,
                &sigLen,
                nullptr,
                nullptr);

            MethodInfo methodInfo;
            methodInfo.id = enumeratedMethods[i];
            methodInfo.name = methodName;
            methodInfo.parameterCount = GetParameterCountFromMethodSig(sigBlob);

            methods.push_back(methodInfo);
        }
    }

    this->metaDataImport->CloseEnum(hCorEnum);

    return methods;
}

std::wstring ModuleInstrumenter::GetMethodDescription(mdMethodDef methodDef) const
{
    wchar_t methodName[256];
    ULONG strLength;

    ComOp(
        this->metaDataImport->GetMethodProps(
            methodDef,
            nullptr,
            methodName,
            sizeof(methodName) / sizeof(methodName[0]),
            &strLength,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr));

    return std::wstring(methodName);
}

void ModuleInstrumenter::ReadModuleInfo()
{
    LPCBYTE baseLoadAddress;
    wchar_t moduleFileName[512];
    wchar_t assemblyName[256];
    AssemblyID assemblyId;
    ULONG strLength;

    ComOp(
        this->corInfos->GetModuleInfo(
            this->moduleId,
            &baseLoadAddress,
            sizeof(moduleFileName) / sizeof(moduleFileName[0]),
            &strLength,
            moduleFileName,
            &assemblyId));

    ComOp(
        this->corInfos->GetAssemblyInfo(
            assemblyId,
            sizeof(assemblyName) / sizeof(assemblyName[0]),
            &strLength,
            assemblyName,
            nullptr,
            nullptr));

    this->moduleFileName.assign(moduleFileName);
    this->assemblyName.assign(assemblyName);
    this->moduleName.assign(this->assemblyName + L":" + std::to_wstring(this->moduleId));
}
