using System.Text.Json;
namespace SystemsLab;

internal sealed class PrimePage : UserControl
{
    readonly NumericUpDown limit = Theme.Number(1000000, 0, 100000000); readonly ComboBox method = Theme.Combo("分段埃氏筛", "基于已知质数试除"); readonly Label status = Theme.Label("", 11), detail = Theme.Label("", 10, Theme.Muted); readonly Canvas density = new(); readonly TextBox numbers = Theme.Input("", true); readonly DataGridView history = Theme.Grid("算法 / 上限", "状态", "质数数量", "实际耗时 ms"); readonly Button run, stop; CancellationTokenSource? cancellation; JsonElement result; int resultLimit;
    internal PrimePage()
    {
        limit.Width = 145;
        method.Width = 205;
        numbers.ReadOnly = true;
        numbers.Dock = DockStyle.Fill;
        numbers.WordWrap = false;
        numbers.Font = new Font("Consolas", 11);
        history.Columns[0].FillWeight = 175;
        run = Theme.Button("计算质数", async (_, _) => await Run(), true);
        stop = Theme.Button("停止", (_, _) => CancelRun());
        stop.Enabled = false;
        var bottom = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 2 };
        bottom.ColumnStyles.Add(new(SizeType.Percent, 55));
        bottom.ColumnStyles.Add(new(SizeType.Percent, 45));
        bottom.Controls.Add(numbers);
        bottom.Controls.Add(history);
        Controls.Add(Theme.Layout((Theme.Label("质数实验 · 比较计算方式与数的分布", 18), SizeType.Absolute, 48), (Theme.Label("计算 0 到 N（包含 N）中的质数。相同范围可分别运行两种方法，耗时来自本次实际计算。", 10, Theme.Muted), SizeType.Absolute, 40), (Theme.Row(Theme.Label("上限 N"), limit, method, run, stop), SizeType.Absolute, 50), (status, SizeType.Absolute, 70), (density, SizeType.Absolute, 190), (detail, SizeType.Absolute, 60), (Theme.Label("前 1000 个质数与本次会话记录 · 停止项仅代表已完成区间"), SizeType.Absolute, 32), (bottom, SizeType.Percent, 100)));
        density.Render = Draw;
        status.Text = "设置上限后开始计算";
        detail.Text = "分段筛每次处理 65,536 个整数；试除法只检查不超过平方根的已知质数。大范围试除可随时停止。";
    }
    internal async Task Run()
    {
        if (cancellation != null)
            return;
        cancellation = new();
        run.Enabled = false;
        run.Text = "计算中…";
        stop.Enabled = true;
        limit.Enabled = method.Enabled = false;
        int n = (int)limit.Value, a = method.SelectedIndex;
        var token = cancellation.Token;
        status.Text = $"正在运行 {method.Text} · N = {n:N0} …";
        status.ForeColor = Theme.Blue;
        try
        {
            result = await Task.Run(() => Native.Primes(n, a, token));
            resultLimit = n;
            bool canceled = result.GetProperty("canceled").GetBoolean();
            status.Text = $"{(canceled ? "已停止" : "计算完成")} · {result.GetProperty("count").GetUInt64():N0} 个质数 · 最大 {result.GetProperty("last").GetInt32():N0}\r\n实际耗时 {result.GetProperty("elapsed").GetDouble():N2} ms · 已处理到 {result.GetProperty("processed").GetInt32():N0}";
            status.ForeColor = canceled ? Theme.Amber : Theme.Mint;
            history.Rows.Insert(0, $"{(a == 0 ? "分段筛" : "试除")} / {n:N0}", canceled ? "停止 · 部分" : "完成", result.GetProperty("count").GetUInt64().ToString("N0"), result.GetProperty("elapsed").GetDouble().ToString("N2"));
            if (history.Rows.Count > 20)
                history.Rows.RemoveAt(20);
            var values = result.GetProperty("first").Ints();
            numbers.Text = string.Join("\r\n", values.Chunk(5).Select(row => string.Join(" ", row.Select(p => p.ToString().PadLeft(7)))));
            density.Invalidate();
        }
        catch (Exception e) { status.Text = "计算失败：" + e.Message; status.ForeColor = Theme.Red; }
        finally { cancellation.Dispose(); cancellation = null; if (!IsDisposed) { run.Enabled = true; run.Text = "计算质数"; stop.Enabled = false; limit.Enabled = method.Enabled = true; } }
    }
    internal async Task Showcase()
    {
        await Run();
    }
    void CancelRun()
    {
        cancellation?.Cancel();
        stop.Enabled = false;
        status.Text = "正在停止，保留已完成区间的结果…";
    }
    void Draw(Graphics g, Rectangle r)
    {
        if (result.ValueKind != JsonValueKind.Object)
        {
            Theme.TextAt(g, "运行后显示 100 个等宽区间的质数数量", new(20, 20, r.Width - 40, r.Height - 40), Theme.Muted);
            return;
        }
        var bins = result.GetProperty("bins").Ints();
        int max = Math.Max(1, bins.Max());
        float width = (r.Width - 40) / 100f;
        for (int i = 0; i < 100; i++)
        {
            float h = (r.Height - 60) * bins[i] / max;
            Theme.Fill(g, Theme.Mint, new(20 + i * width, r.Height - 32 - h, Math.Max(1, width - 2), h));
        }
        Theme.TextAt(g, "0", new(20, r.Height - 28, 70, 25), Theme.Muted, 9);
        Theme.TextAt(g, $"N = {resultLimit:N0} · 每柱为一个等宽区间", new(r.Width - 360, r.Height - 28, 345, 25), Theme.Muted, 9);
    }
    protected override void Dispose(bool disposing)
    {
        if (disposing)
            cancellation?.Cancel();
        base.Dispose(disposing);
    }
}
