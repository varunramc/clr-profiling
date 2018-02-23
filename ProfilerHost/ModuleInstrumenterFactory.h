#pragma once
#include "ModuleInstrumenter.h"

class ModuleInstrumenterFactory
{
public:
    static std::unique_ptr<ModuleInstrumenter> Create(
        MetadataRegistry *metadataRegistry,
        const CComPtr<ICorProfilerInfo5> &corInfos,
        ModuleID moduleId);
};

