namespace ProfilerCore
{
    using System;
    using ProfilerCore.Ipc;

    internal sealed class ProfilingSession
    {
        private readonly MetadataRegistry metadataRegistry;
        public IProfilerServiceCallback ProfilerServiceCallback { get; set; }
        public int ClientPid { get; }

        public ProfilingSession(int clientPid)
        {
            this.ClientPid = clientPid;
            this.metadataRegistry = new MetadataRegistry();
        }

        public void RegisterMethod(MethodMetadata method)
        {
            this.metadataRegistry.RegisterMethod(method);
        }

        public void OnMethodEnter(ulong token, int threadId)
        {
            Console.WriteLine($"[Enter - {threadId}] {this.GetMethodDescription(token)}");
        }

        public void OnMethodExit(ulong token, int threadId)
        {
            Console.WriteLine($"[Exit  - {threadId}] {this.GetMethodDescription(token)}");
        }

        private string GetMethodDescription(ulong token)
        {
            MethodMetadata metadata = this.metadataRegistry.ResolveMethodToken(token);
            if (metadata == null)
            {
                return $"<Unknown token: {token}>";
            }

            return $"{metadata.TypeName}.{metadata.Signature}";
        }
    }
}
