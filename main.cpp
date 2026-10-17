// 随机点名助手 - C++/Win32 重写版
// 还原自原 aardio 程序: 窗体 399x199, 标签显示姓名, 按钮"选一个",
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

static std::wstring exeDir() {
    wchar_t p[MAX_PATH];
    GetModuleFileNameW(nullptr, p, MAX_PATH);
    std::wstring s = p;
    return s.substr(0, s.find_last_of(L"\\/"));
}

static std::wstring decode(const std::string& b) {
    size_t off = (b.size() >= 3 && (unsigned char)b[0] == 0xEF && (unsigned char)b[1] == 0xBB && (unsigned char)b[2] == 0xBF) ? 3 : 0;
    UINT cp = CP_UTF8; DWORD fl = MB_ERR_INVALID_CHARS;
    int n = MultiByteToWideChar(cp, fl, b.data() + off, (int)(b.size() - off), nullptr, 0);
    if (n <= 0) { cp = 936; fl = 0; n = MultiByteToWideChar(cp, fl, b.data() + off, (int)(b.size() - off), nullptr, 0); } // 回退 GBK
    std::wstring w(n, 0);
    if (n) MultiByteToWideChar(cp, fl, b.data() + off, (int)(b.size() - off), &w[0], n);
    return w;
}

static std::wstring trim(const std::wstring& s) {
    size_t a = s.find_first_not_of(L" \t\r\n"), b = s.find_last_not_of(L" \t\r\n");
    return a == std::wstring::npos ? L"" : s.substr(a, b - a + 1);
}

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

// 滚动动画: 所有姓名(含概率为 0 者)均匀随机闪烁
static void showRolling() {
    if (g_names.empty()) return;
    std::uniform_int_distribution<size_t> d(0, g_names.size() - 1);
    SetWindowTextW(g_lbl, g_names[d(g_rng)].c_str());
}

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

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_COMMAND:
        if (LOWORD(w) == ID_BTN && HIWORD(w) == BN_CLICKED) {
            loadNames();                                   // 每次点击重新读取, 修改名单即时生效
            if (g_names.empty()) { MessageBoxW(h, kEmptyMsg, kTitle, MB_ICONERROR); return 0; }
            EnableWindow(g_btn, FALSE);
            g_left = kRolls;
            SetTimer(h, ID_TIMER, kInterval, nullptr);
        }
        return 0;
    case WM_TIMER:
        if (w == ID_TIMER) {
            if (g_left == 1) showRandom();   // 最后一次: 按权重确定最终结果
            else showRolling();              // 滚动过程: 均匀随机(概率为 0 的姓名也会闪烁)
            if (--g_left <= 0) { KillTimer(h, ID_TIMER); EnableWindow(g_btn, TRUE); SetFocus(g_btn); }
        }
        return 0;
    case WM_CTLCOLORSTATIC:
        if ((HWND)l == g_lbl) {
            SetTextColor((HDC)w, RGB(255, 0, 0));          // aardio color=255 => 红
            SetBkColor((HDC)w, RGB(0xF0, 0xF8, 0xFF));     // bgcolor=0xF0F8FF => 爱丽丝蓝
            return (LRESULT)g_bg;
        }
        break;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, PWSTR, int nShow) {
    SetProcessDPIAware();
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc; wc.hInstance = hi; wc.lpszClassName = L"RollCallWnd";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassW(&wc);

    HDC sdc = GetDC(nullptr); int dpi = GetDeviceCaps(sdc, LOGPIXELSX); ReleaseDC(nullptr, sdc);
    auto S = [&](int v) { return MulDiv(v, dpi, 96); };

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;   // 固定大小对话框边框, 无最大化
    RECT rc{0, 0, S(399), S(199)}; AdjustWindowRect(&rc, style, FALSE);
    int W = rc.right - rc.left, H = rc.bottom - rc.top;
    HWND hw = CreateWindowExW(0, wc.lpszClassName, kTitle, style,
        (GetSystemMetrics(SM_CXSCREEN) - W) / 2, (GetSystemMetrics(SM_CYSCREEN) - H) / 2, W, H,
        nullptr, nullptr, hi, nullptr);

    g_bg = CreateSolidBrush(RGB(0xF0, 0xF8, 0xFF));
    g_font = CreateFontW(-S(80), 0, 0, 0, FW_BOLD, 0, 0, 0,
                         DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"黑体");
    g_lbl = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTER | WS_BORDER,
        S(8), S(12), S(384), S(104), hw, (HMENU)ID_LBL, hi, nullptr);
    g_btn = CreateWindowExW(0, L"BUTTON", kButton, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        S(144), S(132), S(100), S(48), hw, (HMENU)ID_BTN, hi, nullptr);
    HFONT ui = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessageW(g_btn, WM_SETFONT, (WPARAM)ui, TRUE);
    SendMessageW(g_lbl, WM_SETFONT, (WPARAM)g_font, TRUE);

    loadNames();   // 启动即读取(并在 person.txt 不存在时创建)
    ShowWindow(hw, nShow); UpdateWindow(hw);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(hw, &msg)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    }
    return 0;
}
