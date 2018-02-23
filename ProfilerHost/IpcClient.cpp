#include "stdafx.h"
#include "IpcClient.h"
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include "ProfilingConfig.h"
#include "StringUtils.h"

namespace ipc = profilerHostIpc;
using grpc::ClientContext;

std::unique_ptr<ClientContext> IpcClient::GetClientContext() const
{
    auto context = std::make_unique<ClientContext>();
    context->AddMetadata("pid", this->processId);
    return context;
}

IpcClient::IpcClient()
{
    auto channel = CreateChannel("localhost:7777", grpc::InsecureChannelCredentials());
    this->stub = ProfilerHostIpc::NewStub(channel);
    this->processId = std::to_string(GetCurrentProcessId());
}

void IpcClient::GetProfilingConfig() const
{
    ipc::Void request;
    ipc::ProfilingConfigMsg response;

    this->stub->GetProfilingConfig(this->GetClientContext().get(), request, &response);

    auto& config = ProfilingConfig::Get();
    config.SetNamespaceToProfile(ToWstring(response.namespacetoprofile()));
    config.SetProfilerCoreDllPath(ToWstring(response.profilercoredllpath()));
}

void IpcClient::RegisterMethod(const MethodMetadata& method) const
{
    ipc::MethodMetadataMsg request;
    ipc::Void response;

    request.set_assemblyname(ToString(method.assemblyName));
    request.set_typename_(ToString(method.typeName));
    request.set_signature(ToString(method.signature));
    request.set_token(method.token);

    this->stub->RegisterMethod(this->GetClientContext().get(), request, &response);
}
