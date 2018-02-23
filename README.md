# CLR Profiler

## Overview
This is an experimental CLR profiler primarily meant for continuous profiling of ASP.NET requests. It works by injecting a native profiler(based on the CLR Profiling API) into an ASP.NET process. The profiler uses IL rewriting to add instrumentation hooks on specific methods. Profiling data is streamed to a profiling server using GRPC.

## Components
* ***Profiler:*** This is the profiling server that listens to profiling data streamed from the profiling host.
* ***ProfilerHost:*** This is the native profiling library that is hosted in the target process and does IL rewriting.
* ***ProfilerCore:*** A managed library that is shared by both the server and the profiling host.
* ***ProfilingTarget:*** A sample managed app to test the profiler.

## Current Status
The profiler is in working condition and is able to record method start/end timestamps by injecting enter/exit hooks. The profiler loads up during CLR host initialization, scans every loaded assembly for classes matching a namespace filter, and rewrites all the class methods' IL to add the enter/exit hooks.

## Pending Work

### Async/await methods
A major missing feature is support for async code. The hooks do work, but the recorded times will only reflect the execution time until the first yield. This can solved by identifying the state machine class that the compiler generates for async methods and mapping state transition calls to the original async method.

### Sampling requests to improve perf
Instrumenting every single request would be prohibitively expensive. Instead, sampling could be used to profile with a certain probability that is inversely porportional to the load.
When the profiling host decides to profile a particular request, it sets a flag in a thread local variable. This flag is propagated through thread switches by modifying the execution context save/resume methods:
* `System.Threading.ExecutionContext.Capture()`
* `System.Threading.ExecutionContext.RunInternal()`

All the instrumented method hooks only work if the flag is set, thus adding no overhead when a request is not being profiled.

### Associating method hook calls with a ASP.NET request
To be able to link a hook call to a particular ASP.NET request, a unique ID needs to be generated for a request and saved in a thread local variable that is propagated via context switches. All hook events can now simply pass the ID in their thread local variable and the profiling server can link the event with the request.