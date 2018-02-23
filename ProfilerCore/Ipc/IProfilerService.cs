namespace ProfilerCore.Ipc
{
    using System.ServiceModel;

    [ServiceContract(CallbackContract = typeof(IProfilerServiceCallback))]
    internal interface IProfilerService
    {
        [OperationContract(IsOneWay = true)]
        void RegisterClient(int pid);

        [OperationContract(IsOneWay = true)]
        void OnMethodEnter(ulong methodToken, int threadId);

        [OperationContract(IsOneWay = true)]
        void OnMethodExit(ulong methodToken, int threadId);
    }
}