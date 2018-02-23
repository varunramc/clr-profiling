namespace ProfilerCore.Ipc
{
    using System.ServiceModel;

    [ServiceContract]
    internal interface IProfilerServiceCallback
    {
        [OperationContract(IsOneWay = true)]
        void StopNotifications();
    }
}