cmake_minimum_required(VERSION 3.21)

set(NDK_PATH /Users/jiangnight/Library/Android/sdk/ndk/r27_goron)
set(CMAKE_SYSTEM_NAME ANDROID) 
set(CMAKE_BUILD_TYPE Release) 
set(CMAKE_SYSTEM_VERSION 21)
set(ANDROID_PLATFORM 21) 
set(ANDROID_ABI arm64-v8a)
set(ANDROID_NDK ${NDK_PATH})
set(CMAKE_TOOLCHAIN_FILE ${NDK_PATH}/build/cmake/android.toolchain.cmake)
set(ANDROID_SDK_ROOT ${NDK_PATH})

project(OSImGui)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mllvm -irobf-cse  -Wno-error=format-security -fexceptions  -fvisibility=hidden -DNDEBUG -O2 -w")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/outputs/${CMAKE_ANDROID_ARCH_ABI}/) # 重定向输出产物

# CMake头文件设置 
FILE(GLOB_RECURSE FILE_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h*)
FILE(GLOB_RECURSE FILE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*)


include_directories( 
        include/
        include/imgui
        include/utils
        )

add_executable(${PROJECT_NAME} 
        ${FILE_SOURCES} 
        )

target_link_libraries(${PROJECT_NAME} PRIVATE
        EGL log android GLESv3
        )