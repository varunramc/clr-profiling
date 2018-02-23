#pragma once

#include "../IpcShared/HostIpc.grpc.pb.h"
#include "MetadataRegistry.h"

using profilerHostIpc::ProfilerHostIpc;

class IpcClient
{
    std::string processId;
    std::unique_ptr<ProfilerHostIpc::Stub> stub;

    std::unique_ptr<grpc::ClientContext> GetClientContext() const;

public:
    IpcClient();

    void GetProfilingConfig() const;
    void RegisterMethod(const MethodMetadata& method) const;
};
