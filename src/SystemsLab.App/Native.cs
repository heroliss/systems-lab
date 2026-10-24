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
    internal static JsonElement Paging(int[] p, int frames)
    {
        ArgumentNullException.ThrowIfNull(p);
        if (p.Length is < 1 or > 4096 || frames is < 1 or > 32)
            throw new ArgumentException("访问序列需包含 1–4096 项，页框数需为 1–32。 ");
        if (p.Any(v => v is < 0 or > 1000000))
            throw new ArgumentException("页号需为 0–1000000 的整数。");
        return Read(sl_paging(p, p.Length, frames));
    }
    internal static JsonElement Bank(int[] mx, int[] al, int[] av, int pid = -1, int[]? request = null)
    {
        ArgumentNullException.ThrowIfNull(mx);
        ArgumentNullException.ThrowIfNull(al);
        ArgumentNullException.ThrowIfNull(av);
        if (av.Length is < 1 or > 10 || mx.Length == 0 || mx.Length % av.Length != 0 || al.Length != mx.Length)
            throw new ArgumentException("资源数需为 1–10，最大需求与已分配矩阵必须具有相同的完整维度。");
        int processes = mx.Length / av.Length;
        if (processes is < 1 or > 50 || pid < -1 || pid >= processes)
            throw new ArgumentException("进程数需为 1–50，请求进程必须在矩阵范围内。");
        // The C ABI receives dimensions, not managed array lengths. Validate before native copies.
        if (pid >= 0 && (request is null || request.Length != av.Length))
            throw new ArgumentException($"请求向量必须包含 {av.Length} 项，分别对应每一种资源。");
        if (mx.Any(v => v is < 0 or > 1000000) || al.Any(v => v is < 0 or > 1000000) || av.Any(v => v is < 0 or > 1000000)
            || (pid >= 0 && request!.Any(v => v is < 0 or > 1000000)))
            throw new ArgumentException("资源数量需为 0–1000000 的整数。");
        for (int i = 0; i < mx.Length; i++)
            if (al[i] > mx[i])
                throw new ArgumentException("已分配资源不能超过最大需求。");
        return Read(sl_banker(processes, av.Length, mx, al, av, pid, pid >= 0 ? request! : new int[av.Length]));
    }
    internal static JsonElement Devices(List<(int pid, int device)> q)
    {
        ArgumentNullException.ThrowIfNull(q);
        var requests = q.ToArray();
        if (requests.Length > 1000 || requests.Any(v => v.pid < 0 || v.device is < 0 or > 3)
            || requests.Select(v => v.pid).Distinct().Count() != requests.Length)
            throw new ArgumentException("最多保留 1000 个请求；进程编号需非负且唯一，设备编号需为 0–3。");
        return Read(sl_devices(requests.Select(v => v.pid).ToArray(), requests.Select(v => v.device).ToArray(), requests.Length));
    }
    internal static JsonElement Primes(int limit, int method, CancellationToken token)
    {
        Cancel callback = () => token.IsCancellationRequested ? 1 : 0;
        var r = Read(sl_primes(limit, method, callback));
        GC.KeepAlive(callback);
        return r;
    }
    internal static int[] Ints(this JsonElement e) => e.EnumerateArray().Select(v => v.GetInt32()).ToArray();
}
