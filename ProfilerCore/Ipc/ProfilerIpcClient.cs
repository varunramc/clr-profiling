namespace ProfilerCore.Ipc
{
    using System;
    using System.Diagnostics;
    using System.Linq;
    using System.ServiceModel;

    internal sealed class ProfilerIpcClient : IProfilerServiceCallback, IProfilerClient
    {
        private readonly IProfilerService service;

        public ProfilerIpcClient(string endpointAddress)
        {
            DuplexChannelFactory<IProfilerService> factory = new DuplexChannelFactory<IProfilerService>(
                new InstanceContext(this),
                new NetNamedPipeBinding(),
                new EndpointAddress(endpointAddress));

            this.service = factory.CreateChannel();
            this.service.RegisterClient(Process.GetCurrentProcess().Id);
        }

        public void OnMethodEnter(ulong methodToken, int threadId)
        {
            this.service.OnMethodEnter(methodToken, threadId);
        }

        public void OnMethodExit(ulong methodToken, int threadId)
        {
            this.service.OnMethodExit(methodToken, threadId);
        }

        public void StopNotifications()
        {
            throw new NotImplementedException();
        }
    }
}