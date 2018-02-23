namespace ProfilerCore.Ipc
{
    using System;
    using System.ServiceModel;

    internal static class ProfilerIpc
    {
        private const string ServiceBase = "net.pipe://localhost";
        private const string ServiceName = "ProfilerService";

        public static IProfilerServer CreateServer(SessionRegistry sessionRegistry)
        {
            ProfilerService server = new ProfilerService(sessionRegistry);
            ServiceHost serviceHost = new ServiceHost(server, new Uri(ProfilerIpc.ServiceBase));
            serviceHost.AddServiceEndpoint(
                typeof(IProfilerService),
                new NetNamedPipeBinding(),
                ProfilerIpc.ServiceName);
            serviceHost.Open();

            return server;
        }

        public static IProfilerClient CreateClient()
        {
            return new ProfilerIpcClient(ProfilerIpc.ServiceBase + '/' + ProfilerIpc.ServiceName);
        }
    }
}