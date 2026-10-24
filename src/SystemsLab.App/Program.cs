using System.Text.Json;
namespace SystemsLab;

internal static class Program
{
    [STAThread]
    static void Main(string[] args)
    {
        if (args.Contains("--verify"))
        {
            int i = Array.IndexOf(args, "--verify");
            string output = args.ElementAtOrDefault(i + 1) ?? "interop-checks.json";
            var checks = new List<string>();
            void Check(bool ok, string name)
            {
                if (!ok)
                    throw new InvalidOperationException(name);
                checks.Add(name);
            }
            void Reject(Action action, string name)
            {
                bool rejected = false;
                try { action(); }
                catch (ArgumentException) { rejected = true; }
                Check(rejected, name);
            }
            Check(Native.Paging([7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2], 3).GetProperty("results")[2].GetProperty("faults").GetInt32() == 7, "PInvoke paging OPT");
            Check(Native.Devices([(1, 0), (2, 1)]).GetProperty("granted").Ints().SequenceEqual([1, 0]), "PInvoke hierarchy");
            Check(Native.Primes(1000000, 0, CancellationToken.None).GetProperty("count").GetInt32() == 78498, "PInvoke million sieve");
            Check(Native.Bank([7, 5, 3, 3, 2, 2, 9, 0, 2, 2, 2, 2, 4, 3, 3], [0, 1, 0, 2, 0, 0, 3, 0, 2, 2, 1, 1, 0, 0, 2], [3, 3, 2]).GetProperty("safe").GetBoolean(), "PInvoke banker safety");
            int[] maximum = [3, 3, 3], allocation = [0, 0, 0], available = [3, 3, 3];
            Check(Native.Bank(maximum, allocation, available, 0, [1, 0, 2]).GetProperty("granted").GetBoolean(), "PInvoke banker request with exact dimensions");
            Reject(() => Native.Bank(maximum, allocation, available, 0), "Managed boundary rejects missing request");
            Reject(() => Native.Bank(maximum, allocation, available, 0, []), "Managed boundary rejects empty request");
            Reject(() => Native.Bank(maximum, allocation, available, 0, [1]), "Managed boundary rejects short request");
            Reject(() => Native.Bank(maximum, allocation, available, 0, [1, 0, 0, 0]), "Managed boundary rejects long request");
            Reject(() => Native.Bank(maximum, [0], available), "Managed boundary rejects short allocation");
            Reject(() => Native.Bank(maximum, [0, 0, 0, 0], available), "Managed boundary rejects long allocation");
            Reject(() => Native.Bank([3, 3], [0, 0], available), "Managed boundary rejects incomplete matrix row");
            Reject(() => Native.Bank([], [], []), "Managed boundary rejects empty resources");
            Reject(() => Native.Bank(maximum, allocation, available, 1, [0, 0, 0]), "Managed boundary rejects invalid process");
            Reject(() => Native.Bank(null!, allocation, available), "Managed boundary rejects null maximum");
            Reject(() => Native.Bank(maximum, null!, available), "Managed boundary rejects null allocation");
            Reject(() => Native.Bank(maximum, allocation, null!), "Managed boundary rejects null resources");
            Reject(() => Native.Paging(null!, 3), "Managed boundary rejects null pages");
            Reject(() => Native.Paging([], 3), "Managed boundary rejects empty pages");
            Reject(() => Native.Paging([1], 33), "Managed boundary rejects excessive page frames");
            Reject(() => Native.Devices(null!), "Managed boundary rejects null device queue");
            Reject(() => Native.Devices([(1, 0), (1, 1)]), "Managed boundary rejects duplicate device process");
            Reject(() => Native.Devices([(1, 4)]), "Managed boundary rejects invalid device");
            Check(Native.Devices([]).GetProperty("granted").Ints().Length == 0, "PInvoke empty device queue");
            File.WriteAllText(output, JsonSerializer.Serialize(new
            {
                passed = true,
                checks
            }, new JsonSerializerOptions { WriteIndented = true }));
            return;
        }
        ApplicationConfiguration.Initialize();
        Application.Run(new MainForm(args));
    }
}
internal sealed class MainForm : Form
{
    readonly Panel content = new() { Dock = DockStyle.Fill }; readonly UserControl[] pages = [new PagingPage(), new DevicePage(), new BankerPage(), new PrimePage()]; readonly List<Button> nav = [];
    internal MainForm(string[] args)
    {
        Text = "Liss Systems Lab · 系统与算法实验";
        BackColor = Theme.Background;
        ForeColor = Theme.Text;
        Font = Theme.Font();
        ClientSize = new(1380, 900);
        MinimumSize = new(1120, 800);
        StartPosition = FormStartPosition.CenterScreen;
        var header = new Panel { Dock = DockStyle.Top, Height = 88, BackColor = Theme.Surface, Padding = new Padding(26, 14, 20, 12) };
        header.Controls.Add(Theme.Label("Liss Systems Lab", 21));
        var subtitle = Theme.Label("系统机制与算法的交互实验", 10, Theme.Muted);
        subtitle.Dock = DockStyle.Bottom;
        header.Controls.Add(subtitle);
        header.Controls[0].Location = new Point(26, 8);
        var side = new FlowLayoutPanel { Dock = DockStyle.Left, Width = 184, BackColor = Theme.Surface, FlowDirection = FlowDirection.TopDown, Padding = new Padding(12, 22, 8, 10), WrapContents = false };
        string[] names = ["请求分页", "设备管理", "银行家算法", "质数实验"];
        for (int i = 0; i < names.Length; i++)
        {
            int index = i;
            var b = Theme.Button(names[i], (_, _) => Select(index));
            b.Width = 158;
            b.Height = 50;
            b.AutoSize = false;
            b.TextAlign = ContentAlignment.MiddleLeft;
            nav.Add(b);
            side.Controls.Add(b);
        }
        var about = Theme.Label("C++20 算法核心\n.NET 桌面可视化\n\n离线运行 · Windows x64", 9, Theme.Muted);
        about.Margin = new Padding(8, 32, 4, 4);
        side.Controls.Add(about);
        Controls.Add(content);
        Controls.Add(side);
        Controls.Add(header);
        Select(0);
        Shown += async (_, _) => { int at = Array.IndexOf(args, "--showcase"); if (at >= 0) { string page = args.ElementAtOrDefault(at + 1) ?? "paging"; int index = Array.IndexOf(new[] { "paging", "devices", "banker", "primes" }, page); Select(Math.Max(0, index)); if (index == 3) await ((PrimePage)pages[3]).Showcase(); } };
    }
    void Select(int index)
    {
        content.Controls.Clear();
        for (int i = 0; i < nav.Count; i++)
        {
            nav[i].BackColor = i == index ? Color.FromArgb(37, 66, 72) : Theme.Surface;
            nav[i].ForeColor = i == index ? Theme.Mint : Theme.Text;
        }
        pages[index].Dock = DockStyle.Fill;
        content.Controls.Add(pages[index]);
    }
    protected override void Dispose(bool disposing)
    {
        if (disposing)
            foreach (var p in pages)
                p.Dispose();
        base.Dispose(disposing);
    }
}
