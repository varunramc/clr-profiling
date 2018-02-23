namespace ProfilerCore.Ipc
{
    using System.Collections.Concurrent;
    using System.ServiceModel;
    using System.Threading;

    [ServiceBehavior(InstanceContextMode = InstanceContextMode.Single)]
    internal sealed class ProfilerService : IProfilerService, IProfilerServer
    {
        private readonly SessionRegistry sessionRegistry;
        private readonly ConcurrentDictionary<EndpointAddress, int> clientEndpointToPid;

        public ProfilerService(SessionRegistry sessionRegistry)
        {
            this.sessionRegistry = sessionRegistry;
            this.clientEndpointToPid = new ConcurrentDictionary<EndpointAddress, int>();
        }

        public void RegisterClient(int pid)
        {
            this.clientEndpointToPid[OperationContext.Current.Channel.RemoteAddress] = pid;
            this.sessionRegistry[pid].ProfilerServiceCallback =
                OperationContext.Current.GetCallbackChannel<IProfilerServiceCallback>();
        }

        public void OnMethodEnter(ulong methodToken, int threadId)
        {
            this.GetSession().OnMethodEnter(methodToken, threadId);
        }

        public void OnMethodExit(ulong methodToken, int threadId)
        {
            this.GetSession().OnMethodExit(methodToken, threadId);
        }

        private ProfilingSession GetSession()
        {
            int pid = this.clientEndpointToPid[OperationContext.Current.Channel.RemoteAddress];
            return this.sessionRegistry[pid];
        }
    }
}
