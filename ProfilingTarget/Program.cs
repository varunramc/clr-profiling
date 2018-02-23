namespace ProfilingTarget
{
    using System;
    using System.Diagnostics;
    using System.Net.Http;
    using System.Threading;
    using System.Threading.Tasks;

    class CilTest
    {

        public async Task<int> DoSomething(int a)
        {
            await Task.Delay(200);
            await GetSomething("https://www.google.com/");
            return a * 5;
        }

        private async Task GetSomething(string address)
        {
            Console.WriteLine(" --- Before get string");
            var str = await new HttpClient().GetStringAsync(address);
            Console.WriteLine(" --- After get string");
            var callstack = Environment.StackTrace;
            Console.WriteLine(str.Length);
        }

        public void RunTask()
        {
            this.Step1();
            Thread.Sleep(100);
            Thread.Sleep(TimeSpan.FromMilliseconds(100));
            this.Step2();
        }

        private void Step1()
        {
            Console.WriteLine("Step - 1");
        }

        private void Step2()
        {
            Console.WriteLine("Step - 2");
        }
    }

    public sealed class Program
    {
        const int MinDepth = 4;

        public static void Main(string[] args)
        {
            Stopwatch stopwatch = Stopwatch.StartNew();

            //(new CilTest()).RunTask();
            (new CilTest()).DoSomething(5).Wait();
            DateTime.Now.ToString();

            Console.WriteLine($"Total time = {stopwatch.ElapsedMilliseconds:N0}ms");
        }
    }
}
