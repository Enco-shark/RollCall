# 随机点名助手 RollCall

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Windows-0078D4.svg)]()
[![Language](https://img.shields.io/badge/language-C%2B%2B17-f34b7d.svg)]()
[![Build](https://img.shields.io/badge/build-CMake-064F8C.svg)]()

一款基于 **C++17** 与原生 **Win32 API** 编写的 Windows 随机点名（抽名）小工具。点击「选一个」按钮，姓名快速滚动后随机停在一个名字上，适用于课堂点名、活动抽奖、随机抽取等场景。

本项目为原 aardio 版（吾爱破解 micksoft）的 C++/Win32 重写版。

## ✨ 功能特性

- 🎲 **一键随机点名**：点击「选一个」按钮随机抽取姓名
- 🎞 **滚动动画**：抽取时姓名滚动 30 次（每次 20ms）后定格
- 📂 **热加载名单**：每次点击即时重新读取 `person.txt`，修改名单无需重启
- 🌏 **编码自适应**：自动识别 UTF-8（含 BOM）与 GBK 编码
- 🔍 **自动建单**：首次运行自动创建空名单文件
- 🖥 **高 DPI 适配**：固定大小窗口（399×199）
- 🚀 **零依赖**：静态链接 CRT，无需安装运行库，单文件即可运行

## 🖼 界面预览

程序窗口：上方为显示姓名的标签，下方为「选一个」按钮。

> 可将软件截图保存为 `docs/screenshot.png` 并在 README 中引用。

## 🏗 构建

### 环境要求

- Windows 7 及以上
- 下列工具链之一：
  - Visual Studio 2019 / 2022（勾选「使用 C++ 的桌面开发」工作负载及 CMake 组件）
  - MinGW-w64 + CMake

### 使用 Visual Studio 构建

1. 安装 [Visual Studio](https://visualstudio.microsoft.com/zh-hans/)，勾选「使用 C++ 的桌面开发」。
2. 菜单 **文件 → 打开 → 文件夹**，选择项目根目录。
3. 菜单 **生成 → 全部生成**。

或使用命令行（开发者命令提示符）：

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release
```

产物位于 `build\Release\随机点名助手.exe`。

### 使用 MinGW 构建

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

产物位于 `build\随机点名助手.exe`。

## 📖 使用教程

1. 将 `随机点名助手.exe` 与名单文件 `person.txt` 放在同一目录下。
2. 编辑 `person.txt`，每行填写一个姓名（支持 UTF-8 或 GBK 编码）。例如：

   ```text
   张三
   李四
   王五
   赵六
   ```

3. 双击运行 `随机点名助手.exe`。
4. 点击「选一个」按钮，姓名快速滚动后随机停在一个名字上。
5. 若 `person.txt` 不存在，程序会在首次运行时自动创建。

## 📁 目录结构

```text
RollCall/
├── CMakeLists.txt   # CMake 构建脚本
├── main.cpp         # 程序源码（Win32 / C++17）
├── person.txt       # 示例名单（每行一个姓名）
├── README.md        # 项目说明
├── LICENSE          # MIT 许可证
├── CONTRIBUTING.md  # 贡献指南
└── .gitignore       # Git 忽略规则
```

## 📄 许可证

本项目采用 [MIT License](LICENSE) 开源协议。

## 🤝 贡献指南

欢迎提交 Issue 与 Pull Request，详细规范见 [CONTRIBUTING.md](CONTRIBUTING.md)。

## 🙏 致谢

- 原版 aardio 程序（吾爱破解 micksoft）
- 所有使用与反馈本项目的人