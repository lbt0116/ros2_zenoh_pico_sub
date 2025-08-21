cmake_minimum_required(VERSION 3.16)

project(ros2_zenoh_pico_sub LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 优先使用本项目 third_party 中的 zenoh-pico 和 zenoh-cpp 头文件/库，无需预先安装
set(_TP_ZP_ROOT "${CMAKE_CURRENT_LIST_DIR}/third_party/zenoh-pico")
set(_TP_ZCPP_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/third_party/zenoh-cpp/include")

find_package(Threads REQUIRED)

# 优先使用静态库，提升可移植性；若无静态库则回退到 .so
if(EXISTS "${_TP_ZP_ROOT}/lib/libzenohpico.a")
    add_library(zenohpico::lib STATIC IMPORTED)
    set_target_properties(zenohpico::lib PROPERTIES
        IMPORTED_LOCATION "${_TP_ZP_ROOT}/lib/libzenohpico.a"
        INTERFACE_INCLUDE_DIRECTORIES "${_TP_ZP_ROOT}/include"
    )
elseif(EXISTS "${_TP_ZP_ROOT}/lib/libzenohpico.so")
    add_library(zenohpico::lib SHARED IMPORTED)
    set_target_properties(zenohpico::lib PROPERTIES
        IMPORTED_LOCATION "${_TP_ZP_ROOT}/lib/libzenohpico.so"
        INTERFACE_INCLUDE_DIRECTORIES "${_TP_ZP_ROOT}/include"
    )
else()
    message(FATAL_ERROR "未找到 third_party/zenoh-pico 库，请将 zenoh-pico 的 include/ 和 lib/ 放入该目录")
endif()

if(NOT EXISTS "${_TP_ZCPP_INCLUDE}/zenoh/zenohc.hxx")
    message(FATAL_ERROR "未找到 third_party/zenoh-cpp/include，请复制 zenoh-cpp 的 include/ 目录过来")
endif()

add_executable(ros2_zenoh_pico_sub src/main.cpp)
target_include_directories(ros2_zenoh_pico_sub PRIVATE "${_TP_ZCPP_INCLUDE}")
target_compile_definitions(ros2_zenoh_pico_sub PRIVATE ZENOHCXX_ZENOHPICO)
target_link_libraries(ros2_zenoh_pico_sub PRIVATE zenohpico::lib Threads::Threads)

# 平台宏：只支持并固定为 Linux，保持项目精简
target_compile_definitions(ros2_zenoh_pico_sub PRIVATE ZENOH_LINUX)

# 若使用共享库，设置可执行文件的运行时RPATH到自身目录，并在安装时放同目录
get_target_property(_ZP_IMPORTED_LOC zenohpico::lib IMPORTED_LOCATION)
if(_ZP_IMPORTED_LOC AND _ZP_IMPORTED_LOC MATCHES "\.so(\.|$)")
    set_target_properties(ros2_zenoh_pico_sub PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
    add_custom_command(TARGET ros2_zenoh_pico_sub POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_ZP_IMPORTED_LOC}" "$<TARGET_FILE_DIR:ros2_zenoh_pico_sub>"
        COMMENT "复制 libzenohpico.so 到可执行文件目录以便拷贝即用")
endif()

# 静态链接 libstdc++/libgcc（提升拷贝即用性；可视需要关闭）
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_link_options(ros2_zenoh_pico_sub PRIVATE -static-libstdc++ -static-libgcc)
endif()

install(TARGETS ros2_zenoh_pico_sub RUNTIME DESTINATION bin)



