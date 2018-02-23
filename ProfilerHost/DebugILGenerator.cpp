#include "stdafx.h"
#include "DebugILGenerator.h"

mdMethodDef GetConsoleWriteDef(CComPtr<IMetaDataImport2> metaDataImport)
{
    mdTypeDef consoleDef;
    ComOp(metaDataImport->FindTypeDefByName(L"System.Console", NULL, &consoleDef));

    byte writeLineSig[] = { IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_STRING };
    mdMethodDef consoleWriteDef;
    ComOp(metaDataImport->FindMethod(
        consoleDef, L"WriteLine", writeLineSig, sizeof(writeLineSig), &consoleWriteDef));

    return consoleWriteDef;
}

mdMethodDef GetConsoleWriteRef(
    CComPtr<IMetaDataAssemblyEmit> metaDataAsmEmit,
    CComPtr<IMetaDataEmit> metaDataEmit,
    CComPtr<IMetaDataImport2> metaDataImport)
{
    ASSEMBLYMETADATA assemblyMetadata = { 0 };
    mdAssemblyRef mscorlibAssemblyRef;

    ComOp(metaDataAsmEmit->DefineAssemblyRef(
        nullptr,
        0,
        L"mscorlib",
        &assemblyMetadata,
        nullptr,
        0,
        0,
        &mscorlibAssemblyRef));

    mdTypeDef consoleRef;
    ComOp(metaDataEmit->DefineTypeRefByName(mscorlibAssemblyRef, L"System.Console", &consoleRef));

    byte writeLineSig[] = { IMAGE_CEE_CS_CALLCONV_DEFAULT, 1, ELEMENT_TYPE_VOID, ELEMENT_TYPE_STRING };
    mdMethodDef consoleWriteRef;
    ComOp(metaDataEmit->DefineMemberRef(
        consoleRef, L"WriteLine", writeLineSig, sizeof(writeLineSig), &consoleWriteRef));

    return consoleWriteRef;
}

PrintDebugStringIL GetPrintDebugStringIL(
    const std::wstring &debugString,
    const std::wstring &assemblyName,
    CComPtr<IMetaDataEmit> metaDataEmit,
    CComPtr<IMetaDataImport2> metaDataImport,
    CComPtr<IMetaDataAssemblyEmit> metaDataAsmEmit)
{
    mdString debugStringDef;
    metaDataEmit->DefineUserString(debugString.c_str(), debugString.length(), &debugStringDef);

    mdToken consoleWriteDef =
        !wcscmp(assemblyName.c_str(), L"mscorlib")
        ? GetConsoleWriteDef(metaDataImport)
        : GetConsoleWriteRef(metaDataAsmEmit, metaDataEmit, metaDataImport);

    return PrintDebugStringIL{ debugStringDef, consoleWriteDef };
}
