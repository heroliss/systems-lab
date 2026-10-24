using System.Text.Json;
namespace SystemsLab;

internal sealed class PagingPage : UserControl
{
    readonly TextBox sequence = Theme.Input("7 0 1 2 0 3 0 4 2 3 0 3 2", true);
    readonly NumericUpDown frames = Theme.Number(3, 1, 16);
    readonly ComboBox algorithm = Theme.Combo("FIFO · 先进先出", "LRU · 最近最少使用", "OPT · 最佳置换");
    readonly Label status = Theme.Label("输入访问序列，比较三种策略；点击时间线可定位任意一步。", 10, Theme.Muted);
    readonly Canvas chart = new(), timeline = new();
    readonly Panel timelineHost = new() { AutoScroll = true, BackColor = Theme.Inset };
    readonly TrackBar scroll = new() { Dock = DockStyle.Fill, Minimum = 0, SmallChange = 1, LargeChange = 1, TickStyle = TickStyle.None, BackColor = Theme.Inset, AutoSize = false, Height = 24 };
    readonly System.Windows.Forms.Timer timer = new() { Interval = 600 };
    readonly Button play;
    JsonElement results; int current;
    internal PagingPage()
    {
        var next = Theme.Button("下一步", (_, _) => Step(1));
        var prev = Theme.Button("上一步", (_, _) => Step(-1));
        play = Theme.Button("自动演示", (_, _) => TogglePlayback());
        timer.Tick += (_, _) => { if (results.ValueKind != JsonValueKind.Array || current >= Steps().GetArrayLength() - 1) StopPlayback(); else Step(1); };
        var top = Theme.Row(Theme.Label("页框数"), frames, algorithm, Theme.Button("重新计算", (_, _) => Compute(), true), Theme.Button("Belady 示例", (_, _) => { sequence.Text = "1 2 3 4 1 2 5 1 2 3 4 5"; frames.Value = 3; Compute(); }));
        timelineHost.Controls.Add(timeline);
        timelineHost.Resize += (_, _) => SizeTimeline();
        Controls.Add(Theme.Layout((Theme.Label("请求分页 · 看见每一次命中与置换", 18), SizeType.Absolute, 48), (Theme.Label("相同访问序列、相同页框数；OPT 使用未来信息，只作为理论参照。", 10, Theme.Muted), SizeType.Absolute, 35), (top, SizeType.Absolute, 48), (sequence, SizeType.Absolute, 72), (chart, SizeType.Absolute, 100), (Theme.Row(prev, next, play), SizeType.Absolute, 48), (timelineHost, SizeType.Percent, 100), (scroll, SizeType.Absolute, 24), (status, SizeType.Absolute, 46)));
        chart.Render = DrawChart;
        timeline.Render = DrawTimeline;
        timeline.MouseDown += (_, e) => { if (results.ValueKind != JsonValueKind.Array) return; int cell = Math.Max(34, (timeline.Width - 62) / Math.Min(24, Math.Max(1, Steps().GetArrayLength()))); current = Math.Clamp(scroll.Value + (e.X - 62) / cell, 0, Steps().GetArrayLength() - 1); RefreshView(); };
        algorithm.SelectedIndexChanged += (_, _) => RefreshView();
        scroll.ValueChanged += (_, _) => timeline.Invalidate();
        sequence.TextChanged += (_, _) => InvalidateResults();
        frames.ValueChanged += (_, _) => InvalidateResults();
        Compute();
    }
    JsonElement Run() => results[algorithm.SelectedIndex]; JsonElement Steps() => Run().GetProperty("steps");
    void InvalidateResults()
    {
        StopPlayback();
        results = default;
        current = 0;
        scroll.Value = 0;
        scroll.Maximum = 0;
        status.Text = "输入已修改，请点击重新计算。";
        status.ForeColor = Theme.Amber;
        chart.Invalidate();
        timeline.Invalidate();
    }
    void Compute()
    {
        try
        {
            StopPlayback();
            var values = sequence.Text.Split(new[] { ' ', ',', '，', ';', '\r', '\n', '\t' }, StringSplitOptions.RemoveEmptyEntries).Select(int.Parse).ToArray();
            results = Native.Paging(values, (int)frames.Value).GetProperty("results");
            current = 0;
            scroll.Maximum = Math.Max(0, values.Length - 1);
            scroll.Value = 0;
            RefreshView();
        }
        catch (Exception e) { InvalidateResults(); status.Text = "无法计算：使用 1–4096 个非负页号，每个不超过 100 万。"; status.ForeColor = Theme.Red; System.Diagnostics.Debug.WriteLine(e); }
    }
    void Step(int delta)
    {
        if (results.ValueKind != JsonValueKind.Array)
            return;
        current = Math.Clamp(current + delta, 0, Steps().GetArrayLength() - 1);
        int visible = Math.Max(1, (timeline.Width - 62) / 36);
        if (current < scroll.Value || current >= scroll.Value + visible)
            scroll.Value = Math.Clamp(current - visible / 2, 0, scroll.Maximum);
        RefreshView();
    }
    void TogglePlayback()
    {
        if (results.ValueKind != JsonValueKind.Array)
            return;
        if (timer.Enabled)
        {
            StopPlayback();
            return;
        }
        if (current == Steps().GetArrayLength() - 1)
        {
            current = 0;
            scroll.Value = 0;
            RefreshView();
        }
        play.Text = "暂停演示";
        timer.Start();
    }
    void StopPlayback()
    {
        timer.Stop();
        play.Text = "自动演示";
    }
    void RefreshView()
    {
        if (results.ValueKind != JsonValueKind.Array)
            return;
        SizeTimeline();
        var s = Steps()[current];
        status.Text = $"第 {current + 1} / {Steps().GetArrayLength()} 次访问 · 页面 {s.GetProperty("page")} · {(s.GetProperty("hit").GetBoolean() ? "命中" : s.GetProperty("victim").GetInt32() < 0 ? "缺页，使用空闲页框" : $"缺页，淘汰页面 {s.GetProperty("victim")}")}     |     绿色 = 命中，橙色 = 缺页";
        status.ForeColor = Theme.Muted;
        chart.Invalidate();
        timeline.Invalidate();
    }
    void SizeTimeline()
    {
        int count = results.ValueKind == JsonValueKind.Array ? Steps()[0].GetProperty("frames").GetArrayLength() : 3;
        timeline.Size = new Size(Math.Max(200, timelineHost.ClientSize.Width - 20), Math.Max(timelineHost.ClientSize.Height, count * 30 + 100));
    }
    void DrawChart(Graphics g, Rectangle r)
    {
        if (results.ValueKind != JsonValueKind.Array)
            return;
        int n = Steps().GetArrayLength();
        for (int a = 0; a < 3; a++)
        {
            var v = results[a];
            int count = v.GetProperty("faults").GetInt32();
            float y = 12 + a * 28;
            Theme.TextAt(g, v.GetProperty("algorithm").GetString()!, new(16, y, 52, 24), a == algorithm.SelectedIndex ? Theme.Mint : Theme.Muted, 10, FontStyle.Bold);
            Theme.Fill(g, Theme.Surface, new(78, y + 5, Math.Max(1, r.Width - 340), 13));
            Theme.Fill(g, new[] { Theme.Mint, Theme.Blue, Theme.Amber }[a], new(78, y + 5, (r.Width - 340) * (float)count / n, 13));
            Theme.TextAt(g, $"{count} 次缺页 · {(double)count / n:P1}", new(r.Width - 245, y, 230, 24));
        }
    }
    void DrawTimeline(Graphics g, Rectangle r)
    {
        if (results.ValueKind != JsonValueKind.Array)
            return;
        var steps = Steps();
        int capacity = steps[0].GetProperty("frames").GetArrayLength(), cell = Math.Max(34, (r.Width - 62) / Math.Min(24, steps.GetArrayLength())), height = Math.Clamp((r.Height - 82) / capacity, 23, 44);
        for (int row = 0; row < capacity; row++)
            Theme.TextAt(g, $"框 {row + 1}", new(4, 43 + row * height, 54, height), Theme.Muted, 9);
        for (int col = scroll.Value, x = 62; col < steps.GetArrayLength() && x < r.Width; col++, x += cell)
        {
            var s = steps[col];
            bool selected = col == current;
            var color = s.GetProperty("hit").GetBoolean() ? Theme.Mint : Theme.Amber;
            if (selected)
                Theme.Fill(g, Color.FromArgb(35, 56, 73), new(x, 0, cell - 2, r.Height));
            Theme.TextAt(g, s.GetProperty("page").ToString(), new(x + 4, 4, cell - 6, 30), color, 11, FontStyle.Bold);
            var fs = s.GetProperty("frames");
            for (int row = 0; row < capacity; row++)
            {
                Theme.Fill(g, selected ? Color.FromArgb(42, 70, 84) : Theme.Surface, new(x + 2, 43 + row * height, cell - 5, height - 3));
                int v = fs[row].GetInt32();
                Theme.TextAt(g, v < 0 ? "—" : v.ToString(), new(x + 7, 43 + row * height, cell - 12, height - 3), v == s.GetProperty("page").GetInt32() ? color : Theme.Text, 10);
            }
            Theme.TextAt(g, s.GetProperty("hit").GetBoolean() ? "命中" : "缺页", new(x + 3, 48 + capacity * height, cell - 6, 26), color, 8);
        }
    }
    protected override void Dispose(bool disposing)
    {
        if (disposing)
            timer.Dispose();
        base.Dispose(disposing);
    }
}
