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
2. 注册窗口类 `RollCallWnd`，创建固定大小窗口：外框尺寸约 **400×225**（逻辑像素），样式为 `WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX`，即不可最大化的固定边框窗口。
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

全部逻辑集中在单文件 [main.cpp](main.cpp) 中。下面先给出概览表，再按行号顺序逐段说明每一处代码与注释的**作用**及对程序行为的**影响**。

### 总体概览

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

### 逐段详解

#### 1. 文件头注释与预处理（第 1–13 行）

```cpp
// 随机点名助手 - C++/Win32 重写版
// 还原自原 aardio 程序: 窗体 400x225, 标签显示姓名, 按钮"选一个",
// 读取程序目录下的 person.txt, 点击后滚动 30 次(每次 20ms)后停在随机姓名上。
#ifndef UNICODE
#define UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <string>
#include <vector>
#include <random>
#include <cstdio>
#include <sstream>
```

- **文件头注释（第 1–3 行）**：说明程序是「还原原 aardio 程序」的重写版，并概括关键参数（窗口 399×199、滚动 30 次、每次 20ms）。**作用**：帮助阅读者快速理解程序全貌与来源；**影响**：纯注释，不参与编译与运行。
- **`#define UNICODE` / `_UNICODE`（第 4–7 行）**：开启 Windows API 的宽字符版本。**影响**：使 `CreateWindowExW`、`SetWindowTextW` 等宽字符 API 与 `L"..."` 字符串正常工作，没有它中文会乱码。用 `#ifndef` 包裹是为了避免与编译器已有定义冲突。
- **头文件（第 8–13 行）**：`windows.h` 提供 Win32 API；`string` 提供字符串类型；`vector` 存名单；`random` 提供随机引擎与分布；`cstdio` 提供文件读取；`sstream` 提供按行拆分。

#### 2. 常量与全局变量（第 15–28 行）

```cpp
static const wchar_t* kTitle   = L"随机点名助手(吾爱破解 micksoft)";
static const wchar_t* kButton  = L"选一个";
static const wchar_t* kEmptyMsg = L"请在person.txt中添加人名数据！";
static const int  kRolls    = 30;   // 滚动次数
static const UINT kInterval = 20;   // 毫秒
enum { ID_BTN = 1001, ID_LBL = 1002, ID_TIMER = 1 };

static HWND g_lbl, g_btn;
static HFONT g_font;
static HBRUSH g_bg;
static std::vector<std::wstring> g_names;
static std::vector<double> g_weights;   // 每个姓名对应的权重(默认 1)
static int g_left = 0;
static std::mt19937 g_rng{std::random_device{}()};
```

- **字符串常量 `kTitle`/`kButton`/`kEmptyMsg`**：集中管理窗口标题、按钮文字、空名单提示。**影响**：直接决定用户看到的标题、按钮文字与弹窗内容。
- **`kRolls`、`kInterval` 及注释「滚动次数」「毫秒」**：滚动 30 次、每次 20ms，总动画约 0.6 秒；改这两个值即可调整动画时长与次数。
- **`enum { ID_BTN, ID_LBL, ID_TIMER }`**：控件与定时器标识，`WM_COMMAND`、`WM_CTLCOLORSTATIC`、`WM_TIMER` 据此区分来源。
- **全局句柄 `g_lbl`/`g_btn`/`g_font`/`g_bg`**：标签、按钮、字体、背景画刷，生命周期贯穿整个程序，故存为全局。
- **`g_names` / `g_weights`**：名单与对应权重，二者下标一一对应；`g_weights` 注释「每个姓名对应的权重(默认 1)」点明含义。**影响**：是 `showRandom()` 加权随机的数据来源。
- **`g_left`**：剩余滚动次数。
- **`g_rng`**：梅森旋转随机引擎，用 `std::random_device` 播种，随机性优于 `rand()`。

#### 3. `exeDir()`（第 30–35 行）

```cpp
static std::wstring exeDir() {
    wchar_t p[MAX_PATH];
    GetModuleFileNameW(nullptr, p, MAX_PATH);
    std::wstring s = p;
    return s.substr(0, s.find_last_of(L"\\/"));
}
```

- **作用**：`GetModuleFileNameW` 取当前 exe 完整路径，再截取到最后一个 `\\` 或 `/` 之前的目录部分。
- **影响**：`loadNames()` 据此定位 exe 同目录下的 `person.txt`，保证无论从哪个目录运行都能正确找到名单。

#### 4. `decode()`（第 37–45 行）

```cpp
static std::wstring decode(const std::string& b) {
    size_t off = (b.size() >= 3 && (unsigned char)b[0]==0xEF && (unsigned char)b[1]==0xBB && (unsigned char)b[2]==0xBF) ? 3 : 0;
    UINT cp = CP_UTF8; DWORD fl = MB_ERR_INVALID_CHARS;
    int n = MultiByteToWideChar(cp, fl, b.data()+off, (int)(b.size()-off), nullptr, 0);
    if (n <= 0) { cp = 936; fl = 0; n = MultiByteToWideChar(cp, fl, b.data()+off, (int)(b.size()-off), nullptr, 0); } // 回退 GBK
    std::wstring w(n, 0);
    if (n) MultiByteToWideChar(cp, fl, b.data()+off, (int)(b.size()-off), &w[0], n);
    return w;
}
```

- **`off` 逻辑**：检测开头是否为 UTF-8 BOM（`EF BB BF`），是则跳过 3 字节，避免 BOM 被当成内容。
- **首选 UTF-8**：用 `CP_UTF8` + `MB_ERR_INVALID_CHARS` 严格解码；非法 UTF-8 时返回失败（`n<=0`）。
- **注释「回退 GBK」的作用与影响**：当 UTF-8 解码失败，改用代码页 936（GBK）重试。**影响**：让同一程序同时兼容「UTF-8」与「ANSI/GBK」两种名单文件，是解决中文名单乱码的关键处理。

#### 5. `trim()`（第 47–50 行）

```cpp
static std::wstring trim(const std::wstring& s) {
    size_t a = s.find_first_not_of(L" \t\r\n"), b = s.find_last_not_of(L" \t\r\n");
    return a == std::wstring::npos ? L"" : s.substr(a, b-a+1);
}
```

- **作用**：去掉字符串首尾的空格、制表符、回车、换行。
- **影响**：避免 Windows 换行符（`\r`）或行首尾多余空格被误记为姓名；整行空白时返回空串，供 `loadNames()` 跳过空行。

#### 6. `weightFor()`（第 52–63 行）

```cpp
// 权重配置(硬编码): 在此处为指定姓名设置抽中概率权重, 无需修改 person.txt
// 默认权重为 1(正常概率); 权重越大越容易被抽中, 0 表示永不抽中。
static double weightFor(const std::wstring& name) {
    // 李四: 概率设为零(永不抽中)
    if (name == L"李四") return 0;

    // 如需为其他姓名调整概率, 仿照下面示例添加:
    // if (name == L"张三") return 3;    // 张三: 3 倍概率
    // if (name == L"王五") return 0.5;  // 王五: 一半概率

    return 1;   // 其余姓名保持正常概率
}
```

- 这是「可调概率」功能的配置入口，逻辑全部通过注释说明。
- **块注释（第 52–53 行）**：说明权重在代码中硬编码配置、默认值含义。
- **`if (name == L"李四") return 0;`（第 55–56 行）**：把「李四」权重设为 0。**影响**：李四在最终结果中概率为 0（永不抽中），但滚动动画中仍会闪过（见 `showRolling` 与 `showRandom` 的分工）。
- **被注释掉的示例（第 58–60 行）**：给出「3 倍」「一半」两种写法模板。注释不参与编译；取消注释即可快速设置某姓名概率，是留给二次开发者的「活文档」。
- **`return 1;`（第 62 行）**：兜底返回 1，未特别配置的姓名保持正常概率。

#### 7. `loadNames()`（第 65–87 行）

```cpp
static void loadNames() {
    g_names.clear();
    g_weights.clear();
    std::wstring path = exeDir() + L"\\person.txt";
    if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {     // 不存在则创建空文件
        HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
    }
    std::string data;
    if (FILE* f = _wfopen(path.c_str(), L"rb")) {
        char buf[4096]; size_t n;
        while ((n = fread(buf, 1, sizeof buf, f)) > 0) data.append(buf, n);
        fclose(f);
    }
    std::wstringstream ws(decode(data));
    std::wstring line;
    while (std::getline(ws, line)) {
        line = trim(line);
        if (line.empty()) continue;
        g_names.push_back(line);
        g_weights.push_back(weightFor(line));   // 根据硬编码规则设置权重
    }
}
```

- **清空旧数据**：每次都先 `clear()`，保证「热加载」不会累积重复姓名。
- **首次建单（注释「不存在则创建空文件」）**：探测 `person.txt`，不存在则 `CreateFileW(... CREATE_NEW ...)` 创建空文件。**影响**：首次运行即可看到文件、手动填名单，无需担心文件缺失报错。
- **二进制读取**：`_wfopen(..., L"rb")` + `fread` 整体读入 `std::string data`，避免换行符被篡改，再交给 `decode` 转码。
- **按行拆分**：`decode` 后放入 `wstringstream`，`getline` 逐行读取、`trim` 去空白、空行跳过。
- **同步权重（注释「根据硬编码规则设置权重」）**：每个姓名调用 `weightFor()` 生成权重，保持 `g_names` 与 `g_weights` 下标一致。

#### 8. `showRolling()`（第 89–94 行）

```cpp
// 滚动动画: 所有姓名(含概率为 0 者)均匀随机闪烁
static void showRolling() {
    if (g_names.empty()) return;
    std::uniform_int_distribution<size_t> d(0, g_names.size() - 1);
    SetWindowTextW(g_lbl, g_names[d(g_rng)].c_str());
}
```

- **注释（第 89 行）**：点明其职责是「滚动动画」且用「均匀随机」，即不区分权重。
- **实现**：在 `[0, g_names.size()-1]` 均匀取整数，`SetWindowTextW` 更新标签文字。
- **影响**：概率为 0 的姓名（如李四）在滚动中也会被随机显示出来，符合「动画中可见、结果抽不到」的预期。

#### 9. `showRandom()`（第 96–114 行）

```cpp
// 最终结果: 按权重加权随机(概率为 0 的姓名不会被抽中)
static void showRandom() {
    double total = 0;
    for (double w : g_weights) if (w > 0) total += w;
    size_t idx = g_names.empty() ? 0 : g_names.size() - 1;
    if (total > 0) {                                          // 按权重加权随机
        std::uniform_real_distribution<double> d(0.0, total);
        double r = d(g_rng);
        for (size_t i = 0; i < g_weights.size(); ++i) {
            if (g_weights[i] <= 0) continue;
            r -= g_weights[i];
            if (r <= 0) { idx = i; break; }
        }
    } else if (!g_names.empty()) {                            // 权重全为 0 时退化为均匀随机
        std::uniform_int_distribution<size_t> d(0, g_names.size() - 1);
        idx = d(g_rng);
    }
    SetWindowTextW(g_lbl, g_names[idx].c_str());
}
```

- **注释（第 96 行）**：说明此函数用于「最终结果」、按权重加权、且「概率为 0 的姓名不会被抽中」。
- **求和 `total`**：累加所有 `w > 0` 的权重，作为随机区间上限。
- **加权随机（注释「按权重加权随机」）**：在 `[0, total)` 取随机数 `r`，逐个减权重，`r <= 0` 时命中当前下标。权重越大命中概率越高；权重 `<= 0` 的姓名被 `continue` 跳过，永不命中。**影响**：最终结果严格按各姓名权重比例分布。
- **兜底（注释「权重全为 0 时退化为均匀随机」）**：所有权重都不大于 0 时退化为均匀随机，防止程序卡死或崩溃。

#### 10. `WndProc()`（第 116–144 行）

窗口消息处理函数，是事件驱动的核心，按 `m` 分支处理：

- **`WM_COMMAND`（第 118–126 行）**：`LOWORD(w)==ID_BTN && HIWORD(w)==BN_CLICKED` 判断按钮点击。点击后：
  1. `loadNames()`（注释「每次点击重新读取, 修改名单即时生效」）实现热加载；
  2. 名单为空则弹窗返回；
  3. `EnableWindow(g_btn, FALSE)` 禁用按钮防连点；
  4. `g_left = kRolls` 重置次数，`SetTimer` 启动定时器。
- **`WM_TIMER`（第 127–133 行）**：每 20ms 触发一次。注释「最后一次: 按权重确定最终结果」「滚动过程: 均匀随机...」说明分支意图——`g_left == 1` 时调用 `showRandom()` 定格，否则调用 `showRolling()` 滚动；`--g_left` 归零时 `KillTimer` 停止、恢复按钮并聚焦。
- **`WM_CTLCOLORSTATIC`（第 134–140 行）**：系统绘制静态文本前询问颜色的消息。仅当目标控件是标签 `g_lbl` 时，设置红色文字、爱丽丝蓝背景并返回 `g_bg` 画刷。注释「aardio color=255 => 红」「bgcolor=0xF0F8FF => 爱丽丝蓝」记录了原版配色来源。**影响**：决定标签的字体颜色与底色；去掉这段会让标签变回默认配色。
- **`WM_DESTROY`**：`PostQuitMessage(0)` 结束消息循环，让程序正常退出。

#### 11. `wWinMain()`（第 146–182 行）

```cpp
int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, PWSTR, int nShow) {
    SetProcessDPIAware();
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc; wc.hInstance = hi; wc.lpszClassName = L"RollCallWnd";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassW(&wc);

    HDC sdc = GetDC(nullptr); int dpi = GetDeviceCaps(sdc, LOGPIXELSX); ReleaseDC(nullptr, sdc);
    auto S = [&](int v) { return MulDiv(v, dpi, 96); };
    // ...
}
```

- **`SetProcessDPIAware()`**：声明进程 DPI 感知，避免系统在高分屏上拉伸窗口导致模糊。
- **注册窗口类**：设置消息处理函数 `WndProc`、实例句柄、类名、箭头光标、按钮底色。
- **DPI 缩放 `S()` 与 `MulDiv(v, dpi, 96)`**：把「96 DPI 基准下的逻辑像素」换算为实际像素。**影响**：使窗口与控件在高分屏等比缩放；后续所有 `S(...)` 都基于此换算。
- **窗口样式（注释「固定大小对话框边框, 无最大化」）**：`WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX` 还原原版固定尺寸、不可最大化、可最小化的窗口。
- **居中创建窗口**：用 `(GetSystemMetrics(SM_CXSCREEN)-W)/2` 等计算居中位置。
- **创建控件与字体**：标签用黑体加粗、字符高约 80 像素（`-S(80)`）、`CLEARTYPE_QUALITY` 抗锯齿；按钮用系统默认 GUI 字体（`DEFAULT_GUI_FONT`）。
- **`loadNames()`（注释「启动即读取(并在 person.txt 不存在时创建)」）**：启动即加载名单，保证点按钮前名单已就绪。
- **消息循环**：`GetMessageW` 取消息，`IsDialogMessageW` 处理对话框快捷键（Tab 切换焦点、回车触发按钮），其余交给 `TranslateMessage/DispatchMessage`。

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
| 窗口大小 | `wWinMain` 中 `S(400)`、`S(225)` |
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