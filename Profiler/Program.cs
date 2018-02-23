namespace Profiler
{
    using System;
    using ProfilerCore;

    public sealed class Program
    {
        public static void Main(string[] args)
        {
            new Program().Run();
        }

        private void Run()
        {
            ProfilingManager profilingMgr = new ProfilingManager();
            profilingMgr.Start();

            Console.WriteLine("Press any key to exit...");
            Console.ReadKey(true);

            profilingMgr.Stop();
        }
    }
}
