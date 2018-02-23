#include "stdafx.h"
#include "MetadataRegistry.h"

MetadataRegistry::MetadataRegistry()
    : tokenCounter(0)
{
}

MetadataRegistry::~MetadataRegistry()
{
}

MethodMetadata MetadataRegistry::RegisterMethod(
    std::wstring assemblyName,
    std::wstring typeName,
    std::wstring signature)
{
    MethodMetadata metadata;
    metadata.token = InterlockedIncrement64((volatile LONG64*)(&this->tokenCounter));
    metadata.assemblyName = assemblyName;
    metadata.typeName = typeName;
    metadata.signature = signature;

    this->methodMetadatas.push_back(metadata);
    if (this->methodRegistrationCallback)
    {
        this->methodRegistrationCallback(metadata);
    }

    return metadata;
}

void MetadataRegistry::SetMethodRegistrationCallback(std::function<void(const MethodMetadata&)> callback)
{
    this->methodRegistrationCallback = callback;
}
