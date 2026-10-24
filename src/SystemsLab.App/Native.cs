using System.Runtime.InteropServices;
using System.Text.Json;
namespace SystemsLab;

internal static class Native
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)] internal delegate int Cancel();
    [DllImport("SystemsCore", CallingConvention = CallingConvention.Cdecl)] private static extern IntPtr sl_paging(int[] p, int n, int frames);
    [DllImport("SystemsCore", CallingConvention = CallingConvention.Cdecl)] private static extern IntPtr sl_banker(int n, int m, int[] max, int[] allocation, int[] available, int pid, int[] request);
    [DllImport("SystemsCore", CallingConvention = CallingConvention.Cdecl)] private static extern IntPtr sl_devices(int[] pids, int[] devices, int n);
    [DllImport("SystemsCore", CallingConvention = CallingConvention.Cdecl)] private static extern IntPtr sl_primes(int limit, int method, Cancel cancel);
    private static JsonElement Read(IntPtr p)
    {
        using var doc = JsonDocument.Parse(Marshal.PtrToStringUTF8(p) ?? throw new InvalidOperationException("核心没有返回结果"));
        if (doc.RootElement.TryGetProperty("error", out var error))
            throw new ArgumentException(error.GetString());
        return doc.RootElement.Clone();
    }
    internal static JsonElement Paging(int[] p, int frames) => Read(sl_paging(p, p.Length, frames));
    internal static JsonElement Bank(int[] mx, int[] al, int[] av, int pid = -1, int[]? request = null) => Read(sl_banker(mx.Length / av.Length, av.Length, mx, al, av, pid, request ?? new int[av.Length]));
    internal static JsonElement Devices(List<(int pid, int device)> q) => Read(sl_devices(q.Select(v => v.pid).ToArray(), q.Select(v => v.device).ToArray(), q.Count));
    internal static JsonElement Primes(int limit, int method, CancellationToken token)
    {
        Cancel callback = () => token.IsCancellationRequested ? 1 : 0;
        var r = Read(sl_primes(limit, method, callback));
        GC.KeepAlive(callback);
        return r;
    }
    internal static int[] Ints(this JsonElement e) => e.EnumerateArray().Select(v => v.GetInt32()).ToArray();
}
