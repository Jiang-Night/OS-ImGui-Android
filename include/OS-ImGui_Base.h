#ifndef IMGUI_BASE_H
#define IMGUI_BASE_H
#include "OS-ImGui_Struct.h"
#include <android/native_window.h>
#include <functional>
namespace OSImGui {

class NativeWindows {
private:
    ANativeWindow *SurfaceWindow;

public:
    NativeWindows();
    ~NativeWindows();
    void InitSurfaceWindow(std::string Name);
    ANativeWindow *GetSurfaceWindow() { return SurfaceWindow; }
};

static NativeWindows Surface;

class OSImGui_Base {
public:
    // 回调函数
    std::function<void()> CallBackFn = nullptr;
    // 退出标识
    bool EndFlags = false;

public:
    // 创建窗口
    void NewWindow(std::string WindowName,
                   std::function<void()> CallBack);
    void Quit() { EndFlags = true; }

    void MainLoop();                              // 类似循环
    bool InitImGui(ANativeWindow *SurfaceWindow); // 初始化imgui窗口
    void CleanImGui();                            // 清除imgui窗口
};

} // namespace OSImGui

#endif // IMGUI_BASE_H