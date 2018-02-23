namespace ProfilerCore
{
    using System.Collections.Concurrent;
    using System.Collections.Generic;

    internal sealed class MethodMetadata
    {
        public ulong Token { get; set; }
        public string AssemblyName { get; set; }
        public string TypeName { get; set; }
        public string Signature { get; set; }
    }

    internal sealed class MetadataRegistry
    {
        private readonly ConcurrentDictionary<ulong, MethodMetadata> methodMetadatas;

        public MetadataRegistry()
        {
            this.methodMetadatas = new ConcurrentDictionary<ulong, MethodMetadata>();
        }

        public void RegisterMethod(MethodMetadata method)
        {
            this.methodMetadatas[method.Token] = method;
        }

        public MethodMetadata ResolveMethodToken(ulong token)
        {
            MethodMetadata metadata;
            return !this.methodMetadatas.TryGetValue(token, out metadata) ? null : metadata;
        }
    }
}
