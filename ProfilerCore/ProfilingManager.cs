namespace ProfilerCore
{
    using Grpc.Core;
    using ProfilerCore.HostIpc;
    using ProfilerCore.Ipc;

    public sealed class ProfilingManager
    {
        private readonly SessionRegistry sessionRegistry;
        private readonly Server hostIpcServer;
        private readonly HostIpcService hostIpcService;
        private readonly IProfilerServer profilerServer;

        public ProfilingManager()
        {
            this.sessionRegistry = new SessionRegistry();
            this.hostIpcService = new HostIpcService(this.sessionRegistry);
            this.hostIpcServer = new Server
            {
                Services = { ProfilerHostIpc.ProfilerHostIpc.BindService(this.hostIpcService) },
                Ports = { new ServerPort("localhost", 7777, ServerCredentials.Insecure) }
            };
            this.profilerServer = ProfilerIpc.CreateServer(this.sessionRegistry);
        }

        public void Start()
        {
            this.hostIpcServer.Start();
        }

        public void Stop()
        {
            this.hostIpcServer.ShutdownAsync().Wait();

        }
    }
}
