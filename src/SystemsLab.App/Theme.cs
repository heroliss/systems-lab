using System.Drawing.Drawing2D;
namespace SystemsLab;

internal static class Theme
{
    internal static readonly Color Background = Color.FromArgb(14, 21, 31), Surface = Color.FromArgb(23, 33, 47), Inset = Color.FromArgb(18, 27, 39), Line = Color.FromArgb(47, 64, 82), Text = Color.FromArgb(230, 238, 246), Muted = Color.FromArgb(160, 179, 199), Mint = Color.FromArgb(80, 224, 178), Blue = Color.FromArgb(112, 174, 255), Amber = Color.FromArgb(255, 193, 112), Red = Color.FromArgb(255, 132, 151);
    internal static Font Font(float size = 10, FontStyle style = FontStyle.Regular) => new("Microsoft YaHei UI", size, style);
    internal static Label Label(string text, int size = 10, Color? color = null) => new() { Text = text, AutoSize = true, Font = Font(size), ForeColor = color ?? Text, Margin = new Padding(0, 7, 12, 7) };
    internal static Button Button(string text, EventHandler click, bool primary = false)
    {
        var b = new Button { Text = text, AutoSize = true, Height = 38, MinimumSize = new Size(82, 36), Padding = new Padding(10, 3, 10, 3), FlatStyle = FlatStyle.Flat, BackColor = primary ? Mint : Surface, ForeColor = primary ? Background : Text, Font = Font(), Cursor = Cursors.Hand, Margin = new Padding(0, 3, 8, 3) };
        b.FlatAppearance.BorderColor = primary ? Mint : Line;
        b.FlatAppearance.MouseOverBackColor = primary ? Color.FromArgb(112, 238, 199) : Color.FromArgb(36, 51, 70);
        b.Click += click;
        return b;
    }
    internal static TextBox Input(string text, bool multiline = false) => new() { Text = text, Multiline = multiline, AcceptsReturn = multiline, ScrollBars = multiline ? ScrollBars.Vertical : ScrollBars.None, BackColor = Inset, ForeColor = Text, BorderStyle = BorderStyle.FixedSingle, Font = Font(), Height = multiline ? 82 : 31, Margin = new Padding(0, 5, 10, 5) };
    internal static ComboBox Combo(params string[] entries)
    {
        var c = new ComboBox { DropDownStyle = ComboBoxStyle.DropDownList, BackColor = Surface, ForeColor = Text, FlatStyle = FlatStyle.Flat, Font = Font(), Width = 156, Margin = new Padding(0, 5, 10, 5) };
        c.Items.AddRange(entries);
        c.SelectedIndex = 0;
        return c;
    }
    internal static NumericUpDown Number(int value, int min, int max) => new() { Minimum = min, Maximum = max, Value = value, Width = 100, BackColor = Inset, ForeColor = Text, Font = Font(), BorderStyle = BorderStyle.FixedSingle, ThousandsSeparator = true, Margin = new Padding(0, 5, 10, 5) };
    internal static FlowLayoutPanel Row(params Control[] children)
    {
        var p = new FlowLayoutPanel { Dock = DockStyle.Fill, AutoSize = true, WrapContents = true, BackColor = Color.Transparent, Margin = Padding.Empty };
        p.Controls.AddRange(children);
        return p;
    }
    internal static TableLayoutPanel Layout(params (Control control, SizeType kind, float height)[] rows)
    {
        var p = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 1, RowCount = rows.Length, Padding = new Padding(24, 18, 24, 20), BackColor = Background };
        p.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        foreach (var (c, k, h) in rows)
        {
            p.RowStyles.Add(new RowStyle(k, h));
            c.Dock = DockStyle.Fill;
            p.Controls.Add(c);
        }
        return p;
    }
    internal static DataGridView Grid(params string[] columns)
    {
        var g = new DataGridView { Dock = DockStyle.Fill, BackgroundColor = Inset, BorderStyle = BorderStyle.None, ColumnHeadersBorderStyle = DataGridViewHeaderBorderStyle.None, CellBorderStyle = DataGridViewCellBorderStyle.SingleHorizontal, GridColor = Line, EnableHeadersVisualStyles = false, AllowUserToAddRows = false, AllowUserToDeleteRows = false, AllowUserToResizeRows = false, ReadOnly = true, RowHeadersVisible = false, AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.Fill, SelectionMode = DataGridViewSelectionMode.FullRowSelect, MultiSelect = false, Font = Font(), Margin = new Padding(0, 10, 0, 10) };
        g.DefaultCellStyle = new DataGridViewCellStyle { BackColor = Inset, ForeColor = Text, SelectionBackColor = Color.FromArgb(38, 70, 82), SelectionForeColor = Text, Padding = new Padding(9, 4, 9, 4) };
        g.ColumnHeadersDefaultCellStyle = new DataGridViewCellStyle { BackColor = Surface, ForeColor = Muted, Font = Font(), Padding = new Padding(9) };
        g.ColumnHeadersHeight = 42;
        g.RowTemplate.Height = 38;
        foreach (string c in columns)
            g.Columns.Add(c, c);
        return g;
    }
    internal static void TextAt(Graphics g, string text, RectangleF rect, Color? color = null, float size = 10, FontStyle style = FontStyle.Regular)
    {
        using var f = Font(size, style);
        using var brush = new SolidBrush(color ?? Text);
        using var format = new StringFormat { Trimming = StringTrimming.EllipsisCharacter, LineAlignment = StringAlignment.Center };
        g.DrawString(text, f, brush, rect, format);
    }
    internal static void Fill(Graphics g, Color color, RectangleF rect)
    {
        using var b = new SolidBrush(color);
        g.FillRectangle(b, rect);
    }
}
internal sealed class Canvas : Panel
{
    internal Action<Graphics, Rectangle>? Render;
    internal Canvas()
    {
        DoubleBuffered = true;
        BackColor = Theme.Inset;
        ResizeRedraw = true;
    }
    protected override void OnPaint(PaintEventArgs e)
    {
        base.OnPaint(e);
        e.Graphics.SmoothingMode = SmoothingMode.AntiAlias;
        Render?.Invoke(e.Graphics, ClientRectangle);
    }
}
