# 随机点名助手 (C++ / Win32)

## Visual Studio(推荐)
1. 安装 Visual Studio(勾选"使用 C++ 的桌面开发"),安装 CMake 组件。
2. 菜单 文件 → 打开 → 文件夹,选择本目录,直接 生成 → 全部生成。
   或命令行(开发者命令提示符):
       cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
       cmake --build build --config Release
   产物:build\Release\随机点名助手.exe(与 person.txt 同目录)。
   想生成 .sln:上面 cmake 命令执行后,build\RollCall.sln 即是。

## MinGW
       cmake -S . -B build -G "MinGW Makefiles" && cmake --build build

## 使用
person.txt 放在 exe 同目录,每行一个名字(UTF-8 或 GBK)。
