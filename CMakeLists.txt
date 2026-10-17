cmake_minimum_required(VERSION 3.15)
project(RollCall LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RollCall WIN32 main.cpp)
target_compile_definitions(RollCall PRIVATE UNICODE _UNICODE)

if(MSVC)
  target_compile_options(RollCall PRIVATE /utf-8 /W3)      # 源码为 UTF-8(含中文)
  set_property(TARGET RollCall PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")  # 静态 CRT,免装运行库
else()
  target_compile_options(RollCall PRIVATE -O2)
  target_link_options(RollCall PRIVATE -municode -static -s)
endif()

# 输出名与原程序一致
set_target_properties(RollCall PROPERTIES OUTPUT_NAME "随机点名助手")

# 把示例名单复制到 exe 旁边
add_custom_command(TARGET RollCall POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_SOURCE_DIR}/person.txt $<TARGET_FILE_DIR:RollCall>/person.txt)
