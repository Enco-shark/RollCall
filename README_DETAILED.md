# 随机点名助手 RollCall · 详细说明

> 本文档是 [README.md](README.md) 的详细补充，面向想要深入了解原理、进行二次开发或排查问题的读者。快速上手请直接阅读 [README.md](README.md)。

## 目录

- [项目简介](#项目简介)
- [功能详解](#功能详解)
- [运行机制与原理](#运行机制与原理)
- [源码结构说明](#源码结构说明)
- [详细构建教程](#详细构建教程)
- [概率权重机制](#概率权重机制)
- [名单文件格式](#名单文件格式)
- [二次开发与定制](#二次开发与定制)
- [常见问题 FAQ](#常见问题-faq)
- [目录结构](#目录结构)

## 项目简介

随机点名助手是一款基于 **C++17 + 原生 Win32 API** 编写的 Windows 桌面小工具。点击「选一个」按钮后，姓名标签会快速滚动，最终随机停在一个名字上，可用于课堂点名、活动抽奖、随机抽取等场景。

本项目是原 aardio 版（吾爱破解 micksoft）的 C++/Win32 重写版，主要目的在于去除对 aardio 运行环境的依赖，用标准 C++ 与 Win32 API 实现同等功能，并做到：

- 单文件、零依赖、免安装运行库
- 体积小、启动快
- 逻辑清晰、易于二次开发

## 功能详解

| 功能 | 说明 |
| ---- | ---- |
| 一键随机点名 | 点击「选一个」按钮抽取姓名 |
| 滚动动画 | 抽取时姓名高速滚动，营造随机感 |
| 热加载名单 | 每次点击重新读取 `person.txt`，改名单即时生效 |
| 可调概率 | 源码中为指定姓名设置权重（详见[概率权重机制](#概率权重机制)） |
| 编码自适应 | 自动识别 UTF-8（含/不含 BOM）与 GBK |
| 自动建单 | 首次运行若名单不存在会自动创建空文件 |
| 高 DPI 适配 | 按系统 DPI 等比缩放，高分屏不模糊 |
| 零依赖 | 静态链接 CRT，目标机器无需安装 VC++ 运行库 |

## 运行机制与原理

### 窗口创建

程序入口为 `wWinMain`，流程如下：

1. `SetProcessDPIAware()` 声明 DPI 感知，随后读取系统 DPI，用 `MulDiv(v, dpi, 96)` 将逻辑坐标（基于 96 DPI）换算为物理像素。
2. 注册窗口类 `RollCallWnd`，创建固定大小窗口：外框尺寸约 **399×199**（逻辑像素），样式为 `WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX`，即不可最大化的固定边框窗口。
3. 创建两个子控件：
   - `STATIC` 标签（用于显示姓名）：黑体、加粗、红色文字、爱丽丝蓝背景（`RGB(0xF0,0xF8,0xFF)`）
   - `BUTTON` 按钮（「选一个」）

### 抽取流程

1. 点击按钮 → 触发 `WM_COMMAND`，先调用 `loadNames()` 重新加载名单。
2. 若名单为空，弹窗提示「请在person.txt中添加人名数据！」。
3. 否则禁用按钮，设置滚动计数 `g_left = 30`（`kRolls`），启动定时器（周期 `kInterval = 20ms`）。
4. 定时器每 20ms 触发一次 `WM_TIMER`：
   - 滚动阶段：调用 `showRolling()`，在所有姓名中均匀随机挑一个显示；**概率为 0 的姓名同样会在滚动中出现**。
   - 最后一次：调用 `showRandom()`，按权重加权随机确定最终结果；概率为 0 的姓名不会成为结果。
5. 计数归零后停止定时器、恢复按钮并聚焦。

### 名单读取与编码

`loadNames()` 以二进制方式读取 `person.txt` 全部内容，交给 `decode()` 转码：

1. 若开头是 UTF-8 BOM（`EF BB BF`），先跳过。
2. 先用 UTF-8（`CP_UTF8`）且开启 `MB_ERR_INVALID_CHARS` 严格解码；失败则回退到 GBK（`CP_936`）解码。
3. 解码后按行拆分，`trim()` 去掉首尾空白，空行跳过，逐行存入 `g_names`，并按 `weightFor()` 计算权重存入 `g_weights`。

### 随机数引擎

使用 `std::mt19937`（梅森旋转算法），以 `std::random_device` 作为种子，随机性远优于 `rand()`。

## 源码结构说明

全部逻辑集中在单文件 [main.cpp](main.cpp) 中：

| 函数 / 变量 | 职责 |
| ----------- | ---- |
| `kRolls` / `kInterval` | 滚动次数（30）与定时周期（20ms） |
| `exeDir()` | 获取 exe 所在目录（用于定位 `person.txt`） |
| `decode()` | UTF-8/GBK 自适应解码 |
| `trim()` | 去除字符串首尾空白 |
| `weightFor()` | 硬编码的权重配置（个性化概率在此修改） |
| `loadNames()` | 读取并解析名单，生成 `g_names` + `g_weights` |
| `showRolling()` | 滚动动画：均匀随机选名 |
| `showRandom()` | 最终结果：按权重加权随机 |
| `WndProc()` | 窗口消息处理（点击、定时器、绘制、销毁） |
| `wWinMain()` | 程序入口：建窗、创建控件、消息循环 |

## 详细构建教程

### 准备工作

- 系统：Windows 7 及以上
- 工具链二选一：
  - **Visual Studio 2019 / 2022**（勾选「使用 C++ 的桌面开发」工作负载，并安装 CMake 组件）
  - **MinGW-w64** + **CMake**

### 方式一：Visual Studio 图形界面构建（推荐）

1. 安装 [Visual Studio](https://visualstudio.microsoft.com/zh-hans/)，安装时务必勾选「使用 C++ 的桌面开发」。
2. 用 VS 打开项目根目录：菜单 **文件 → 打开 → 文件夹**，选择本目录。
3. VS 会自动识别 `CMakeLists.txt`，等待 CMake 配置完成。
4. 顶部选择配置为 `x64-Debug` 或 `x64-Release`，菜单 **生成 → 全部生成**。
5. 产物位于 `out\build\<配置>\随机点名助手.exe`。

### 方式二：Visual Studio 命令行构建

打开「Developer Command Prompt for VS」，执行：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release
```

- `-A Win32` 生成 32 位程序；如需 64 位请改用 `-A x64`。
- 执行完第一条命令后，`build\RollCall.sln` 即为 Visual Studio 解决方案文件，也可直接用 VS 打开。
- 产物位于 `build\Release\随机点名助手.exe`。

### 方式三：MinGW 命令行构建

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

产物位于 `build\随机点名助手.exe`。

> MinGW 下通过 `-municode -static -s` 实现静态链接与体积优化。

## 概率权重机制

程序支持为特定姓名设置不同的抽取概率，实现「提高某些名字的概率、降低某些名字（或号数）的概率」。

### 配置位置

权重的配置在源码 [main.cpp](main.cpp) 的 `weightFor()` 函数中完成（**硬编码，不通过 `person.txt` 设置**）：

```cpp
static double weightFor(const std::wstring& name) {
    // 李四: 概率设为零(永不抽中)
    if (name == L"李四") return 0;

    // 如需为其他姓名调整概率, 仿照下面示例添加:
    // if (name == L"张三") return 3;    // 张三: 3 倍概率
    // if (name == L"王五") return 0.5;  // 王五: 一半概率

    return 1;   // 其余姓名保持正常概率
}
```

### 权重含义

| 权重 | 效果 |
| :--: | ---- |
| `1` | 正常概率（默认） |
| `> 1` | 提高概率（如 `3` 约为正常的 3 倍） |
| `0 < w < 1` | 降低概率（如 `0.5` 约为正常的一半） |
| `0` | 永不抽中 |
| 负数 | 视作 `0`，永不抽中 |

### 算法原理

1. 抽取时，先计算所有权重大于 0 的姓名权重之和 `total`。
2. 在 `[0, total)` 区间取一个随机数 `r`。
3. 依次用 `r` 减去每个姓名的权重，减到 `r <= 0` 时命中该姓名。

因此某姓名的最终概率 = 该姓名权重 ÷ 所有权重之和，即「加权随机」。

### 滚动与结果的区分

- **滚动阶段**：使用 `showRolling()`，在**全部姓名**中均匀随机（概率为 0 的姓名也会一闪而过）。
- **最终定格**：使用 `showRandom()`，按权重加权随机（概率为 0 的姓名不会被选中）。

这样做既保留了「滚动的随机观感」，又保证概率规则只作用于最终结果。

## 名单文件格式

- 文件名固定为 `person.txt`，放在 exe 同目录。
- 每行一个姓名，支持中文、数字（学号）等任意字符。
- 空行与首尾空白会被自动忽略。
- 编码支持 UTF-8（含/不含 BOM）和 GBK，会自动识别。
- 修改名单后无需重启，下一次点击即生效。

示例：

```text
张三
李四
王五
赵六
```

## 二次开发与定制

以下改动均只需编辑 [main.cpp](main.cpp)，重新编译即可：

| 想改什么 | 修改位置 |
| -------- | -------- |
| 窗口标题 | `kTitle` 常量 |
| 按钮文字 | `kButton` 常量 |
| 空名单提示 | `kEmptyMsg` 常量 |
| 滚动次数 / 速度 | `kRolls` / `kInterval` 常量 |
| 窗口大小 | `wWinMain` 中 `S(399)`、`S(199)` |
| 字体与颜色 | `g_font` 创建参数、`WM_CTLCOLORSTATIC` 中的 `RGB(...)` |
| 各姓名概率 | `weightFor()` 函数 |

## 常见问题 FAQ

**Q1：运行时提示缺 `vcruntime140.dll` 等运行库？**
本程序已静态链接 CRT，正常无需安装运行库。若仍报错，请确认没有用其他生成器覆盖，或改用 `MinGW` 的 `-static` 构建。

**Q2：名单里有名字但点「选一个」没反应？**
确认 `person.txt` 与 exe 在同一目录；确认文件编码为 UTF-8 或 GBK；确认每行一个姓名且非全空行。

**Q3：为什么某姓名在滚动时会闪过，却永远不会成为结果？**
因为该姓名的权重被设为 `0`（见 [概率权重机制](#概率权重机制)）。滚动阶段会显示所有姓名，最终结果才应用权重。

**Q4：中文显示乱码？**
检查 `person.txt` 保存编码，推荐另存为 **UTF-8**。程序会自动兼容 GBK。

**Q5：如何彻底取消概率功能、恢复完全随机？**
将 `weightFor()` 中的 `if (name == L"李四") return 0;` 删除，仅保留 `return 1;` 即可。

## 目录结构

```text
RollCall/
├── CMakeLists.txt       # CMake 构建脚本
├── main.cpp             # 程序源码（Win32 / C++17）
├── person.txt           # 示例名单（每行一个姓名）
├── README.md            # 项目简介与快速上手
├── README_DETAILED.md   # 详细说明（本文档）
├── LICENSE              # MIT 许可证
├── CONTRIBUTING.md      # 贡献指南
└── .gitignore           # Git 忽略规则
```

## 许可证

本项目采用 [MIT License](LICENSE) 开源协议。二次开发、分发请保留版权声明。

## 致谢

- 原版 aardio 程序（吾爱破解 micksoft）
- 所有使用与反馈本项目的人