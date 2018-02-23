#pragma once
#include "stdafx.h"
#include <functional>

typedef ULONG64 ProfilerToken;

struct MethodMetadata
{
    ProfilerToken token;
    std::wstring assemblyName;
    std::wstring typeName;
    std::wstring signature;
};

class MetadataRegistry
{
    volatile ProfilerToken tokenCounter;
    std::list<MethodMetadata> methodMetadatas;
    std::function<void(const MethodMetadata&)> methodRegistrationCallback;

public:
    MetadataRegistry();
    ~MetadataRegistry();

    MethodMetadata RegisterMethod(
        std::wstring assemblyName,
        std::wstring typeName,
        std::wstring signature);
    void SetMethodRegistrationCallback(std::function<void(const MethodMetadata&)> callback);
};
