cmake_minimum_required(VERSION 3.15)
project(SprintHookByDarkP1xel C CXX)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This ASI targets 32-bit GTA:SA. Configure with -A Win32 (MSVC) or a 32-bit MinGW toolchain.")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_library(minhook STATIC
    libs/minhook/src/buffer.c
    libs/minhook/src/hook.c
    libs/minhook/src/trampoline.c
    libs/minhook/src/hde/hde32.c
    libs/minhook/src/hde/hde64.c
)
target_include_directories(minhook PUBLIC libs/minhook/include)

add_library(sprinthook SHARED src/main.cpp)
target_link_libraries(sprinthook PRIVATE minhook)
set_target_properties(sprinthook PROPERTIES PREFIX "" SUFFIX ".asi")

if(MINGW)
    target_link_options(sprinthook PRIVATE -static)
endif()
