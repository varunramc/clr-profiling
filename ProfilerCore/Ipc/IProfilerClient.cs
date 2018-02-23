namespace ProfilerCore.Ipc
{
    public interface IProfilerClient
    {
        void OnMethodEnter(ulong methodToken, int threadId);
        void OnMethodExit(ulong methodToken, int threadId);
    }
}