namespace ProfilerCore.HostIpc
{
    using System;
    using System.Collections.Generic;
    using System.Linq;
    using System.Reflection;
    using System.Threading.Tasks;
    using Grpc.Core;
    using NLog;
    using ProfilerHostIpc;
    using Void = ProfilerHostIpc.Void;

    internal sealed class HostIpcService : ProfilerHostIpc.ProfilerHostIpcBase
    {
        private static readonly Logger Logger = LogManager.GetCurrentClassLogger();

        private readonly SessionRegistry sessionRegistry;

        public HostIpcService(SessionRegistry sessionRegistry)
        {
            this.sessionRegistry = sessionRegistry;
        }

        public override Task<ProfilingConfigMsg> GetProfilingConfig(Void request, ServerCallContext context)
        {
            this.GetSessionForRequest(context); // To initialize session

            return Task.FromResult(
                new ProfilingConfigMsg
                {
                    NamespaceToProfile = "ProfilingTarget.",
                    ProfilerCoreDllPath = Assembly.GetExecutingAssembly().Location,
                    ShouldProfile = true
                });
        }

        public override Task<Void> RegisterMethod(MethodMetadataMsg request, ServerCallContext context)
        {
            this.GetSessionForRequest(context).RegisterMethod(
                new MethodMetadata
                {
                    Token = request.Token,
                    AssemblyName = request.AssemblyName,
                    TypeName = request.TypeName,
                    Signature = request.Signature
                });

            return Task.FromResult(new Void());
        }

        private ProfilingSession GetSessionForRequest(ServerCallContext context)
        {
            int pid = int.Parse(context.RequestHeaders.First(e => e.Key == "pid").Value);
            return this.sessionRegistry[pid];
        }
    }
}
