## ros2_zenoh_pico_sub

一个基于 zenoh-cpp（后端使用 zenoh-pico）的最小订阅示例，订阅键 `rt/chatter` 并打印收到的消息。

### 特性
- **拷贝即用（Linux）**：
  - 已内置 `third_party/zenoh-pico` 静态库和 `third_party/zenoh-cpp/include` 头文件。
  - CMake 优先链接静态 `libstdc++` 与 `libgcc`，尽量减少目标机运行依赖。
  - 若仅提供了共享库 `.so`，构建后会将其复制到可执行文件同目录，并设置 RPATH 为 `$ORIGIN`。

### 先决条件
- Linux（x86_64 等与内置库 ABI 兼容的架构）
- CMake >= 3.16
- 支持 C++17 的编译器（gcc/clang）
- pthread（CMake 自动链接）

注意：若将本项目拷贝到另一台机器，需保证 CPU 架构和 glibc 版本与内置的 `libzenohpico` 兼容；否则请在目标机重新构建或替换为兼容的 `third_party/zenoh-pico/lib/`。

### 构建
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

生成的可执行文件位于：
```
build/ros2_zenoh_pico_sub/ros2_zenoh_pico_sub
```

### 运行
```bash
# 可选：调试日志
export Z_LOG=info   # 或 debug

./build/ros2_zenoh_pico_sub/ros2_zenoh_pico_sub
```

程序会订阅 `rt/chatter` 并打印样本，如：
```
>> [ROS2 Sub] PUT ('rt/chatter' : 'hello')
```

### 与 ROS 2 互通（两种方式，择一）

- 方式 A：使用 ROS 2 ↔ zenoh 桥接，无需替换 RMW
  1) 启动 zenoh 路由器（任选其一）
     - Docker：
       ```bash
       docker run --rm -it --network host ghcr.io/eclipse-zenoh/zenoh-router:latest
       ```
     - 本机二进制：
       ```bash
       zenohd -l tcp/0.0.0.0:7447
       ```
  2) 启动 ROS 2 ↔ zenoh 桥（与路由器同机或同网段）
     - Docker：
       ```bash
       docker run --rm -it --network host ghcr.io/eclipse-zenoh/zenoh-plugin-ros2dds:latest
       ```
  3) 发布 ROS 2 话题（另一个终端）：
     ```bash
     ros2 topic pub /chatter std_msgs/msg/String '{data: "hello ros2 world"}'
     ```
  4) 本程序应收到对应的 `rt/chatter` 消息。

- 方式 B：切换 ROS 2 到 rmw_zenoh
  1) 安装：
     ```bash
     sudo apt install ros-$ROS_DISTRO-rmw-zenoh-cpp
     ```
  2) 切换并发布：
     ```bash
     export RMW_IMPLEMENTATION=rmw_zenoh_cpp
     ros2 topic pub /chatter std_msgs/msg/String '{data: "hello ros2 world"}'
     ```

提示：若网络不支持多播发现，建议先启动 `zenohd`，或将桥接/发布与本程序放在同一主机。

### 常见问题
- 没有收到 ROS 2 发布的数据？
  - 请确认已使用“方式 A 桥接”或“方式 B 切换 RMW”。
  - 观察本程序输出是否显示“Declaring subscriber on 'rt/chatter'...”。
  - 需要时将日志调到 `debug`：
    ```bash
    export Z_LOG=debug
    ./build/ros2_zenoh_pico_sub/ros2_zenoh_pico_sub
    ```

### 修改订阅键
- 默认键为 `rt/chatter`，可在 `src/main.cpp` 中修改：
  ```cpp
  const KeyExpr keyexpr("rt/chatter");
  ```

### 目录结构（简要）
```
third_party/
  zenoh-pico/           # 内置库与头文件
  zenoh-cpp/include/    # C++ 绑定头文件
src/main.cpp            # 示例订阅程序
CMakeLists.txt
```

### 许可
- 示例源码与内置第三方遵循其各自的许可证（见源码头部与第三方仓库）。


