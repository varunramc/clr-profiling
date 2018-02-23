#pragma once
#include "MetadataRegistry.h"

class ModuleInstrumenter
{
protected:
    struct TypeInfo
    {
        mdTypeDef id;
        std::wstring name;
        DWORD typeDefFlags;
    };

    struct MethodInfo
    {
        mdMethodDef id;
        std::wstring name;
        ULONG parameterCount;
    };

    MetadataRegistry *metadataRegistry;
    CComPtr<ICorProfilerInfo5> corInfos;
    CComPtr<IMetaDataEmit> metaDataEmit;
    CComPtr<IMetaDataImport2> metaDataImport;
    CComPtr<IMetaDataAssemblyEmit> metaDataAsmEmit;
    CComPtr<IMethodMalloc> methodAlloc;
    ModuleID moduleId;
    std::wstring moduleName;
    std::wstring assemblyName;
    std::wstring moduleFileName;

    std::list<TypeInfo> GetModuleTypes() const;
    std::list<mdMethodDef> GetTypeMethods(mdTypeDef typeDef) const;
    std::list<MethodInfo> GetMethodsByName(mdTypeDef typeDef, const std::wstring &name) const;
    std::wstring GetMethodDescription(mdMethodDef methodDef) const;
    bool IsTypeInNamespace(const wchar_t* typeName, const std::wstring& namespaceFilter, mdTypeDef typeDef, DWORD typeDefFlags) const;
    static bool IsNestedType(DWORD typeDefFlags);
    bool IsNestedTypeInNamespace(mdTypeDef typeDef, const std::wstring &namespaceFilter) const;

    void ReadModuleInfo();

public:
    ModuleInstrumenter(
        MetadataRegistry *metadataRegistry,
        const CComPtr<ICorProfilerInfo5> &corInfos,
        ModuleID moduleId);
    virtual ~ModuleInstrumenter();

    virtual void Instrument() = 0;
};

