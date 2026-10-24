using System.Text.Json;
namespace SystemsLab;

internal sealed class BankerPage : UserControl
{
    readonly TextBox max = Theme.Input("7 5 3\r\n3 2 2\r\n9 0 2\r\n2 2 2\r\n4 3 3", true), allocation = Theme.Input("0 1 0\r\n2 0 0\r\n3 0 2\r\n2 1 1\r\n0 0 2", true), available = Theme.Input("3 3 2"), request = Theme.Input("1 0 2");
    readonly NumericUpDown pid = Theme.Number(1, 0, 49); readonly DataGridView grid = Theme.Grid("进程", "最大需求 Max", "已分配 Allocation", "剩余需求 Need");
    readonly Label status = Theme.Label("", 11), explanation = Theme.Label("", 10, Theme.Muted); readonly Canvas path = new(); JsonElement result;
    internal BankerPage()
    {
        max.Height = 130;
        allocation.Height = 130;
        available.Width = 135;
        request.Width = 135;
        var matrices = new TableLayoutPanel { ColumnCount = 2, Dock = DockStyle.Fill };
        matrices.ColumnStyles.Add(new(SizeType.Percent, 50));
        matrices.ColumnStyles.Add(new(SizeType.Percent, 50));
        matrices.Controls.Add(Theme.Layout((Theme.Label("最大需求矩阵 · 每行一个进程"), SizeType.Absolute, 32), (max, SizeType.Percent, 100)), 0, 0);
        matrices.Controls.Add(Theme.Layout((Theme.Label("已分配矩阵 · 列为资源 A / B / C…"), SizeType.Absolute, 32), (allocation, SizeType.Percent, 100)), 1, 0);
        Controls.Add(Theme.Layout((Theme.Label("银行家算法 · 分配前先验证安全性", 18), SizeType.Absolute, 48), (Theme.Label("试分配、寻找安全序列；无法通过安全检查时保持原状态。", 10, Theme.Muted), SizeType.Absolute, 35), (matrices, SizeType.Absolute, 190), (Theme.Row(Theme.Label("可用资源"), available, Theme.Button("检查安全性", (_, _) => Evaluate(), true), Theme.Label("进程 P"), pid, Theme.Label("请求"), request, Theme.Button("申请资源", (_, _) => Evaluate(true))), SizeType.Absolute, 50), (status, SizeType.Absolute, 44), (path, SizeType.Absolute, 78), (grid, SizeType.Percent, 100), (explanation, SizeType.Absolute, 58)));
        path.Render = DrawPath;
        Evaluate();
    }
    static int[] Vector(string text) => text.Split(new[] { ' ', ',', '，', ';', '\t', '\r', '\n' }, StringSplitOptions.RemoveEmptyEntries).Select(int.Parse).ToArray();
    static int[][] Matrix(string text) => text.Split(new[] { '\n', ';' }, StringSplitOptions.RemoveEmptyEntries).Select(Vector).Where(r => r.Length > 0).ToArray();
    void Evaluate(bool apply = false)
    {
        try
        {
            var mx = Matrix(max.Text);
            var al = Matrix(allocation.Text);
            var av = Vector(available.Text);
            if (mx.Length != al.Length || mx.Length == 0 || av.Length < 1 || mx.Any(r => r.Length != av.Length) || al.Any(r => r.Length != av.Length))
                throw new ArgumentException("两个矩阵行列必须一致，并与可用资源维数对应。");
            if (mx.Length > 50 || av.Length > 10)
                throw new ArgumentException("最多 50 个进程和 10 类资源。");
            result = Native.Bank(mx.SelectMany(r => r).ToArray(), al.SelectMany(r => r).ToArray(), av, apply ? (int)pid.Value : -1, apply ? Vector(request.Text) : null);
            if (result.GetProperty("granted").GetBoolean())
            {
                av = result.GetProperty("available").Ints();
                al = result.GetProperty("allocation").Ints().Chunk(av.Length).ToArray();
                available.Text = string.Join(' ', av);
                allocation.Text = string.Join("\r\n", al.Select(r => string.Join(' ', r)));
            }
            string code = result.GetProperty("status").GetString()!;
            status.Text = code switch
            {
                "GRANTED" => "已分配 · 存在安全执行顺序",
                "SAFE" => "安全状态 · 存在安全执行顺序",
                "UNSAFE" => apply ? "请求未分配 · 会进入不安全状态" : "不安全状态 · 当前无法找到完整安全序列",
                "EXCEEDS_NEED" => "请求未分配 · 超过该进程剩余需求",
                "INSUFFICIENT" => "请求未分配 · 当前可用资源不足",
                _ => code
            };
            status.ForeColor = result.GetProperty("safe").GetBoolean() ? Theme.Mint : Theme.Amber;
            grid.Rows.Clear();
            var need = result.GetProperty("need").Ints();
            for (int i = 0; i < mx.Length; i++)
                grid.Rows.Add($"P{i}", string.Join("  ", mx[i]), string.Join("  ", al[i]), string.Join("  ", need.Skip(i * av.Length).Take(av.Length)));
            explanation.Text = "序列按当前保留的分配计算；安全检查模拟完成并返还资源，不会实际结束进程。不安全不等于已死锁。";
            path.Invalidate();
        }
        catch (Exception e) { status.Text = "输入无效：" + (e is ArgumentException ? e.Message : "请使用非负整数，并检查矩阵维数。"); status.ForeColor = Theme.Red; }
    }
    void DrawPath(Graphics g, Rectangle r)
    {
        if (result.ValueKind != JsonValueKind.Object)
            return;
        int x = 16;
        var seq = result.GetProperty("sequence").Ints();
        if (seq.Length == 0)
        {
            Theme.TextAt(g, "没有可执行的安全前缀", new(16, 10, r.Width - 30, 48), Theme.Muted);
            return;
        }
        int width = Math.Clamp((r.Width - 45) / seq.Length, 36, 102);
        foreach (int p in seq)
        {
            Theme.Fill(g, Theme.Surface, new(x, 14, width - 18, 42));
            Theme.TextAt(g, $"P{p}", new(x + 10, 14, width - 28, 42), Theme.Mint, 12, FontStyle.Bold);
            x += width;
            if (x < r.Width - 20)
            {
                using var pen = new Pen(Theme.Muted, 1);
                g.DrawLine(pen, x - 14, 35, x - 2, 35);
                g.DrawLine(pen, x - 6, 31, x - 2, 35);
                g.DrawLine(pen, x - 6, 39, x - 2, 35);
            }
        }
    }
}
