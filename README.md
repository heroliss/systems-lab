# Liss Systems Lab

一个离线 Windows 系统与算法实验台，把早期的四个 C++ 控制台实验组织为可以观察、修改输入和重复运行的工作区。

[作品页](https://heroliss.github.io/liss-works/projects/systems-lab/) · [Windows 下载](https://github.com/heroliss/systems-lab/releases/latest)

## 四个工作区

| 模块 | 可以体验的内容 |
| --- | --- |
| 请求分页 | 修改页号序列与页框数，同时比较 FIFO / LRU / OPT 缺页数；时间线显示实际物理槽位，可逐步查看、定位和自动播放。OPT 使用未来信息，是理论参照。 |
| 设备管理 | 观察设备→控制器→通道的排他资源链；申请、等待、释放和取消，按请求顺序重新调度。它是教学模拟，不会控制真实硬件。 |
| 银行家算法 | 编辑最大需求、已分配和可用资源；检查安全序列，试分配资源并在不安全时保持原分配。最多 50 个进程 / 10 类资源。 |
| 质数实验 | 分段埃氏筛与基于已知质数的试除法；范围、数量、最大质数、实际耗时、分布和本次会话记录。支持后台运行与协作取消。 |

## 运行

Windows 10/11 x64，完整解压后运行 `Liss.SystemsLab.exe`。自包含包附带 .NET 桌面运行时，`SystemsCore.dll` 必须与 EXE 保持在同一目录，无需安装 SDK、Qt 或 Visual Studio。

- `--showcase paging`、`devices`、`banker`、`primes`：直接打开相应工作区；质数场景会实际计算一百万以内的质数。
- 银行家示例中 P1 请求 `1 0 2` 可安全分配；恢复初始矩阵后 P4 请求 `3 3 0` 会被安全检查拒绝。
- 分页输入支持空格、逗号与换行，最多 4096 次访问、16 个可视页框。改变页框数后点击“重新计算”。
- 质数上限为一亿。试除法在大范围会较慢，可点击停止；停止结果明确为部分区间，不与完整结果混同。首 1000 个质数用于文本浏览，其余仍计入完整数量与区间分布。

## 实现

`native/` 是不依赖 UI 的 C++20 算法核心；`src/SystemsLab.App/` 是 .NET 10 WinForms 界面。P/Invoke 边界传入受验证的整数数组，核心按值计算快照并返回 JSON；原生结果使用线程局部缓冲，调用侧在同一个工作线程立即复制为独立 JSON 值。后台计算不直接访问界面控件。

分页的槽位不会因 LRU 排序而移动，只有命中时间戳和替换位置变化；OPT 的下一次引用通过预索引查找。银行家算法使用临时向量执行安全检查，拒绝请求不修改输入。设备调度将完整资源链作为一次原子申请，避免只拿到部分资源的中间状态。分段筛一次标记 65,536 个整数，试除法只遍历平方根以内的已知质数。

计时是当前机器的一次实际运行，包含算法内部工作，并非跨机器的性能排名。教学模型不等同于生产操作系统的完整行为。

## 构建与验证

需要当前 Visual Studio C++ 桌面工具、Windows SDK、.NET 10 SDK 和 PowerShell。

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1 -Publish
.\artifacts\SystemsLab-1.0.0-win-x64\Liss.SystemsLab.exe --verify interop.json
powershell -ExecutionPolicy Bypass -File scripts/package.ps1
```

构建脚本使用安装的默认 MSVC 平台工具集，不要求旧 v100。原生回归覆盖已知 FIFO/LRU/OPT 结果、固定种子随机引用的 OPT 下界和槽位不变量、银行家安全/拒绝/回滚、资源链排他与排队、两种质数实现的交叉核对、一百万 `π(n)=78498`、取消与非法输入。断言数量包含参数化和逐步不变量检查，不等同于独立用户场景数。`--verify` 另外检查发行包中的原生互操作。

## 来源

原始“请求分页存储管理 / 设备管理器 / 银行家算法 / 质数实验”源码按原字节保留在 `legacy/`，SHA-256 位于 `legacy/source-manifest.json`。现代版在保留实验主题和算法思路的基础上重新组织了数据所有权、输入边界与可视化，原始代码不参与新版本编译。没有复制旧二进制、构建缓存或个人数据。

未为本项目额外授予开源许可证。分发包中的 .NET Runtime / WinForms 声明仅适用于相应平台组件，见 `licenses/`。MSVC 运行库由当前合法安装的工具链静态链接；不需要收件者单独安装 VC Redistributable。
