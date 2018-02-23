namespace ProfilerCore
{
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using NLog;

    internal sealed class SessionRegistry
    {
        private static readonly Logger Logger = LogManager.GetCurrentClassLogger();

        private readonly ConcurrentDictionary<int, ProfilingSession> sessions =
            new ConcurrentDictionary<int, ProfilingSession>();

        private ProfilingSession Get(int clientPid)
        {
            return this.sessions.GetOrAdd(
                clientPid,
                pid =>
                {
                    SessionRegistry.Logger.Info(
                        $"Found new profiling session from process {pid}");
                    return new ProfilingSession(pid);
                });
        }

        public ProfilingSession this[int clientPid] => this.Get(clientPid);
    }
}
