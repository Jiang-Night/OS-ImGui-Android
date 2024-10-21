#include "OS-ImGui_Base.h"
#include "NativeSurfaceUtils.h"
#include "OS-ImGui_Exception.hpp"
#include "font.h"
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include "touch.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/native_window.h>
#include <cstdio>

int screen_x, screen_y, Orientation, touchScreenX, touchScreenY, _ScreenX, _ScreenY;

namespace OSImGui {

EGLDisplay m_EglDisplay = EGL_NO_DISPLAY;
EGLSurface m_EglSurface = EGL_NO_SURFACE;
EGLContext m_EglContext = EGL_NO_CONTEXT;

bool CreateOpenGLWindow(ANativeWindow *m_Window) {
    const EGLint egl_attributes[] = {EGL_BLUE_SIZE,
                                     8,
                                     EGL_GREEN_SIZE,
                                     8,
                                     EGL_RED_SIZE,
                                     8,
                                     EGL_ALPHA_SIZE,
                                     8,
                                     EGL_DEPTH_SIZE,
                                     16,
                                     EGL_RENDERABLE_TYPE,
                                     EGL_OPENGL_ES3_BIT,
                                     EGL_SURFACE_TYPE,
                                     EGL_WINDOW_BIT,
                                     EGL_NONE};

    m_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(m_EglDisplay, nullptr, nullptr);
    EGLint num_configs = 0;
    eglChooseConfig(m_EglDisplay, egl_attributes, nullptr, 0, &num_configs);
    EGLConfig egl_config;
    eglChooseConfig(m_EglDisplay, egl_attributes, &egl_config, 1, &num_configs);
    EGLint egl_format;
    eglGetConfigAttrib(m_EglDisplay, egl_config, EGL_NATIVE_VISUAL_ID,
                       &egl_format);
    ANativeWindow_setBuffersGeometry(m_Window, 0, 0, egl_format);

    const EGLint egl_context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 3,
                                             EGL_NONE};
    m_EglContext = eglCreateContext(m_EglDisplay, egl_config, EGL_NO_CONTEXT,
                                    egl_context_attributes);
    m_EglSurface =
        eglCreateWindowSurface(m_EglDisplay, egl_config, m_Window, nullptr);
    eglMakeCurrent(m_EglDisplay, m_EglSurface, m_EglSurface, m_EglContext);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    return true;
}

void updateScreen() {
    android::detail::ui::DisplayState info;
    static android::detail::SurfaceComposerClient client;
    client.GetDisplayInfo(&info);
    screen_x = info.layerStackSpaceRect.width;
    screen_y = info.layerStackSpaceRect.height;
    touchScreenX = std::min(screen_x, screen_y);
    touchScreenY = std::max(screen_x, screen_y);
    _ScreenX = std::max(screen_x, screen_y) / 2;
    _ScreenY = std::min(screen_x, screen_y) / 2;
    switch (info.orientation) {
    case android::detail::ui::Rotation::Rotation0:
        Orientation = 0;
        break;
    case android::detail::ui::Rotation::Rotation90:
        Orientation = 1;
        break;
    case android::detail::ui::Rotation::Rotation180:
        Orientation = 2;
        break;
    case android::detail::ui::Rotation::Rotation270:
        Orientation = 3;
        break;
    }
}

bool OSImGui_Base::InitImGui(ANativeWindow *SurfaceWindow) {
    ANativeWindow_acquire(SurfaceWindow);
    CreateOpenGLWindow(SurfaceWindow);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiStyle *style = &ImGui::GetStyle();
    ImGui::StyleColorsClassic(style);
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.FontGlobalScale = 1.3f;
    ImFont *font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(
        font_compressed_data_base85, 36.f, nullptr,
        io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    IM_ASSERT(font != NULL);
    ImGui::GetStyle().ScaleAllSizes(3.0f);

    if (!ImGui_ImplOpenGL3_Init("#version 300 es"))
        throw OSException("ImGui_ImplOpenGL3_Init call failed.");

    if (!ImGui_ImplAndroid_Init(SurfaceWindow))
        throw OSException("ImGui_ImplAndroid_Init call failed.");

    return true;
}

void OSImGui_Base::CleanImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    eglMakeCurrent(m_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(m_EglDisplay, m_EglContext);
    eglDestroySurface(m_EglDisplay, m_EglSurface);
    eglTerminate(m_EglDisplay);
    m_EglDisplay = EGL_NO_DISPLAY;
    m_EglSurface = EGL_NO_SURFACE;
    m_EglContext = EGL_NO_CONTEXT;
}

void OSImGui_Base::NewWindow(std::string WindowName,
                             std::function<void()> CallBack,
                             std::function<void()> TextureCallBack) {
    std::string WName;

    if (!CallBack)
        throw OSException("CallBack is not null");

    if (WindowName.empty())
        throw OSException("WindowName is empty");

    this->CallBackFn = CallBack;

    Surface.InitSurfaceWindow(WName);

    if (!Surface.GetSurfaceWindow())
        throw OSException("Create Window Error");

    try {
        InitImGui(Surface.GetSurfaceWindow());
    } catch (OSException e) {
        throw e;
    }

    updateScreen();
    InitTouch(false);

    if (TextureCallBack) {
        this->TextureCallBackFn = TextureCallBack;
        this->TextureCallBackFn();
    }

    MainLoop();
}

void OSImGui_Base::MainLoop() {
    while (!EndFlags) {
        updateScreen();
        glViewport(0, 0, (int)screen_x, (int)screen_y);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplAndroid_NewFrame();
        ImGui::NewFrame();

        this->CallBackFn();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        eglSwapBuffers(m_EglDisplay, m_EglSurface);
    }
    CleanImGui();
}

NativeWindows::NativeWindows() {
    // TODO 暂时还不知道写什么 先放着
}

void NativeWindows::InitSurfaceWindow(std::string Name) {
    this->SurfaceWindow = android::ANativeWindowCreator::Create(Name.c_str());
}

NativeWindows::~NativeWindows() {
    if (SurfaceWindow)
        android::ANativeWindowCreator::Destroy(SurfaceWindow);
}

} // namespace OSImGui