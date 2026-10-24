using System.Text.Json;
namespace SystemsLab;

internal sealed class DevicePage : UserControl
{
    readonly List<(int pid, int device)> requests = []; readonly NumericUpDown pid = Theme.Number(1, 0, 999999); readonly ComboBox device = Theme.Combo("键盘 · 输入设备", "鼠标 · 输入设备", "打印机 · 输出设备", "显示器 · 输出设备");
    readonly DataGridView grid = Theme.Grid("进程", "申请设备", "当前状态", "请求顺序"); readonly Canvas topology = new(); readonly Label status = Theme.Label("", 10, Theme.Muted); JsonElement state;
    static readonly string[] Names = ["键盘", "鼠标", "打印机", "显示器"];
    internal DevicePage()
    {
        device.Width = 200;
        var actions = Theme.Row(Theme.Label("进程 P"), pid, device, Theme.Button("申请设备", (_, _) => Add(), true), Theme.Button("释放 / 取消所选", (_, _) => Remove()), Theme.Button("载入竞争示例", (_, _) => Sample()), Theme.Button("清空", (_, _) => { requests.Clear(); RefreshState(); }));
        Controls.Add(Theme.Layout((Theme.Label("设备管理 · 从通道到设备的资源链", 18), SizeType.Absolute, 48), (Theme.Label("只有设备、控制器、通道同时空闲，请求才能运行；释放后按请求先后重新分配。", 10, Theme.Muted), SizeType.Absolute, 40), (actions, SizeType.Absolute, 50), (topology, SizeType.Percent, 60), (grid, SizeType.Percent, 40), (status, SizeType.Absolute, 50)));
        topology.Render = Draw;
        Sample();
    }
    void Sample()
    {
        requests.Clear();
        requests.AddRange([(1, 0), (2, 1), (3, 2), (4, 3)]);
        pid.Value = 5;
        RefreshState();
    }
    void Add()
    {
        if (requests.Count >= 1000)
        {
            status.Text = "最多保留 1000 个请求，请先释放或取消一些。";
            status.ForeColor = Theme.Amber;
            return;
        }
        if (requests.Any(v => v.pid == (int)pid.Value))
        {
            status.Text = "每个进程同时申请一个设备；请先释放或取消原请求。";
            status.ForeColor = Theme.Amber;
            return;
        }
        requests.Add(((int)pid.Value, device.SelectedIndex));
        RefreshState();
        pid.Value = Math.Min(pid.Maximum, pid.Value + 1);
    }
    void Remove()
    {
        if (grid.SelectedRows.Count == 0)
        {
            status.Text = "请先在下方选择一个请求。";
            return;
        }
        int index = grid.SelectedRows[0].Index;
        if (index < requests.Count)
            requests.RemoveAt(index);
        RefreshState();
    }
    void RefreshState()
    {
        state = Native.Devices(requests);
        grid.Rows.Clear();
        var granted = state.GetProperty("granted").Ints();
        for (int i = 0; i < requests.Count; i++)
            grid.Rows.Add($"P{requests[i].pid}", Names[requests[i].device], granted[i] == 1 ? "运行 · 已获得完整资源链" : "等待 · 资源链被占用", i + 1);
        int active = granted.Count(v => v == 1);
        status.Text = $"{active} 个运行 · {requests.Count - active} 个等待     |     蓝色表示空闲，绿色表示运行，橙色文字表示等待。此处模拟资源分配，不操作系统真实设备。";
        status.ForeColor = Theme.Muted;
        topology.Invalidate();
    }
    void Draw(Graphics g, Rectangle r)
    {
        if (state.ValueKind != JsonValueKind.Object)
            return;
        var owner = state.GetProperty("owner").Ints();
        float top = 35, mid = r.Height * .46f, bottom = r.Height - 80;
        float[] dx = Enumerable.Range(0, 4).Select(i => r.Width * (i + .5f) / 4).ToArray();
        float[] cx = [r.Width * .25f, r.Width * .625f, r.Width * .875f], chx = [r.Width * .25f, r.Width * .75f];
        using var line = new Pen(Theme.Line, 2);
        for (int i = 0; i < 4; i++)
        {
            int co = i < 2 ? 0 : i - 1;
            g.DrawLine(line, cx[co], mid + 38, dx[i], bottom);
        }
        g.DrawLine(line, chx[0], top + 38, cx[0], mid);
        g.DrawLine(line, chx[1], top + 38, cx[1], mid);
        g.DrawLine(line, chx[1], top + 38, cx[2], mid);
        void Node(float x, float y, string title, string detail, bool active, bool waiting = false)
        {
            float w = Math.Min(188, r.Width / 4f - 16);
            Theme.Fill(g, Theme.Surface, new(x - w / 2, y, w, 55));
            Theme.Fill(g, active ? Theme.Mint : waiting ? Theme.Amber : Theme.Blue, new(x - w / 2 + 12, y + 15, 5, 5));
            Theme.TextAt(g, title, new(x - w / 2 + 26, y + 3, w - 32, 25), active ? Theme.Mint : Theme.Text, 10, FontStyle.Bold);
            Theme.TextAt(g, detail, new(x - w / 2 + 12, y + 27, w - 20, 24), waiting ? Theme.Amber : Theme.Muted, 9);
        }
        for (int c = 0; c < 2; c++)
        {
            int p = c == 0 ? owner.Take(2).FirstOrDefault(v => v >= 0, -1) : owner.Skip(2).FirstOrDefault(v => v >= 0, -1);
            Node(chx[c], top, $"通道 CH{c + 1}", p < 0 ? "空闲" : $"P{p} 正在使用", p >= 0);
        }
        for (int c = 0; c < 3; c++)
        {
            int p = c == 0 ? owner.Take(2).FirstOrDefault(v => v >= 0, -1) : owner[c + 1];
            Node(cx[c], mid, new[] { "基本输入控制器", "打印控制器", "显示控制器" }[c], p < 0 ? "空闲" : $"P{p} 正在使用", p >= 0);
        }
        for (int i = 0; i < 4; i++)
        {
            int waiting = requests.Count(q => q.device == i && (owner[i] < 0 || q.pid != owner[i]));
            Node(dx[i], bottom, Names[i], owner[i] < 0 ? (waiting > 0 ? $"{waiting} 个请求等待资源链" : "空闲") : $"P{owner[i]} 运行 · {waiting} 等待", owner[i] >= 0, waiting > 0);
        }
    }
}
