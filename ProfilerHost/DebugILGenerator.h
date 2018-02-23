#pragma once

#include "ILRewriter.h"
#include <pshpack1.h>
struct PrintDebugStringIL
{
    byte loadString;
    mdString debugStr;
    byte call;
    mdMethodDef consoleWrite;

    PrintDebugStringIL(mdString debugStr, mdMethodDef consoleWrite)
    {
        this->loadString = CEE_LDSTR;
        this->debugStr = debugStr;
        this->call = CEE_CALL;
        this->consoleWrite = consoleWrite;
    }

    PrintDebugStringIL()
    {}
};
#include <poppack.h>

PrintDebugStringIL GetPrintDebugStringIL(
    const std::wstring &debugString,
    const std::wstring &assemblyName,
    CComPtr<IMetaDataEmit> metaDataEmit,
    CComPtr<IMetaDataImport2> metaDataImport,
    CComPtr<IMetaDataAssemblyEmit> metaDataAsmEmit);
