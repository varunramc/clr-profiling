namespace ProfilerCore
{
    using System;
    using System.Reflection;
    using System.Runtime.InteropServices;
    using NLog;

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [Guid("16F3E434-F45C-429A-A105-A8B4EE51A964")]
    [ComVisible(true)]
    public interface IProfilerAssemblyResolver
    {
        [ComVisible(true)]
        void ConfigureResolver();
    }

    public sealed class ProfilerAssemblyResolver : IProfilerAssemblyResolver
    {
        private static readonly Logger Log = LogManager.GetCurrentClassLogger();

        public void ConfigureResolver()
        {
            AppDomain.CurrentDomain.AssemblyResolve += this.AppDomainAssemblyResolver;
        }

        private Assembly AppDomainAssemblyResolver(object sender, ResolveEventArgs args)
        {
            if (args.Name.StartsWith("ProfilerCore."))
            {
                ProfilerAssemblyResolver.Log.Info("Resolving profiler core assembly");
                return Assembly.GetExecutingAssembly();
            }

            return null;
        }
    }
}
