namespace IpcGen
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.Linq;
    using System.Text.RegularExpressions;

    public sealed class IpcContractCompiler
    {
        private void WriteWithColor(string message, ConsoleColor color)
        {
            ConsoleColor curColor = Console.ForegroundColor;
            Console.ForegroundColor = color;
            Console.WriteLine(message);
            Console.ForegroundColor = curColor;
        }

        private string GetPackagePath(string packageName)
        {
            return
                Directory.EnumerateDirectories(@"..\..\packages\")
                    .First(dir => Regex.IsMatch(dir, $@"{Regex.Unescape(packageName)}[\.\d]+", RegexOptions.IgnoreCase));
        }

        private void CompileProtobuf(string outputDir, string language, string protoFile, string grpcPlugin)
        {
            if (!Directory.Exists($"./{outputDir}"))
            {
                Directory.CreateDirectory($"./{outputDir}");
            }

            string protobufCompilerPath = Path.Combine(
                this.GetPackagePath("Grpc.Tools"),
                @"tools\windows_x64\protoc.exe");
            // protobufCompilerPath = @"C:\apps\protobuf-3.2.0\install\bin\protoc.exe";    // TMP:

            string commandLine =
                $" --{language}_out {outputDir} --grpc_out {outputDir}" +
                $" --plugin=protoc-gen-grpc={grpcPlugin} {protoFile}";

            //Console.WriteLine("{0} {1}", protobufCompilerPath, commandLine);
            ProcessStartInfo startInfo = new ProcessStartInfo
            {
                FileName = protobufCompilerPath,
                Arguments = commandLine,
                CreateNoWindow = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                UseShellExecute = false
            };

            this.WriteWithColor($"Running: \n{protobufCompilerPath} {commandLine}", ConsoleColor.Cyan);

            using (Process process = Process.Start(startInfo))
            {
                process.OutputDataReceived += (sender, args) => this.WriteWithColor(args.Data, ConsoleColor.White);
                process.ErrorDataReceived += (sender, args) => this.WriteWithColor(args.Data, ConsoleColor.Red);

                process.BeginOutputReadLine();
                process.BeginErrorReadLine();

                process.WaitForExit();
                if (process.ExitCode != 0)
                {
                    this.WriteWithColor("Protobuf compilation failed", ConsoleColor.Red);
                }
                else
                {
                    this.WriteWithColor($"Successfully compiled {protoFile} for {language}", ConsoleColor.Green);
                }
            }
        }

        public void Run()
        {
            string grpcPluginFolder = Path.Combine(
                this.GetPackagePath("grpc.cpp.redist"),
                @"build\native\bin\Win32\v140\Release");

            string outputPath = @"..\..\IpcShared";
            string grpcCppPlugin = Path.Combine(grpcPluginFolder, "grpc_cpp_plugin.exe");
            string grpcCsharpPlugin = Path.Combine(grpcPluginFolder, "grpc_csharp_plugin.exe");

            this.CompileProtobuf(outputPath, "csharp", "HostIpc.proto", grpcCsharpPlugin);
            this.CompileProtobuf(outputPath, "cpp", "HostIpc.proto", grpcCppPlugin);
        }

        public static void Main(string[] args)
        {
            new IpcContractCompiler().Run();
        }
    }
}
