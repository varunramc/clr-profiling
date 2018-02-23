namespace ProfilerCore
{
    using System;
    using System.Threading;
    using ProfilerCore.Ipc;

    public sealed class ProfilingHook
    {
        private static readonly IProfilerClient ProfilerClient;

        private static Action onExecCtxCaptured;
        private static Action onExecCtxResumed;

        static ProfilingHook()
        {
            ProfilingHook.ProfilerClient = ProfilerIpc.CreateClient();
            ProfilingHook.onExecCtxCaptured = ProfilingHook.OnExecutionContextCaptured;
            ProfilingHook.onExecCtxResumed = ProfilingHook.OnExecutionContextResumed;
        }

        public static void OnMethodEnter(ulong methodToken)
        {
            Console.WriteLine("> Enter: " + methodToken);
            ProfilingHook.ProfilerClient.OnMethodEnter(methodToken, Thread.CurrentThread.ManagedThreadId);
        }

        public static void OnMethodExit(ulong methodToken)
        {
            Console.WriteLine("> Exit: " + methodToken);
            ProfilingHook.ProfilerClient.OnMethodExit(methodToken, Thread.CurrentThread.ManagedThreadId);
        }

        private static void OnExecutionContextCaptured()
        {
            Console.WriteLine("~~~~~~~~~~~~~~~~~~> Context captured");
        }

        private static void OnExecutionContextResumed()
        {
            Console.WriteLine("~~~~~~~~~~~~~~~~~~> Context resumed");
        }
    }
}
