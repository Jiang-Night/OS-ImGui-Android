#ifndef OS_IMGUI_H
#define OS_IMGUI_H
#include "OS-ImGui_Base.h"
#include "OS-ImGui_Struct.h"
#include "imgui.h"

namespace OSImGui {

class OSImGui : public OSImGui_Base, public Singleton<OSImGui> {
public:
    // 文本
    void Text(std::string Text, Vec2 Pos, ImColor Color, float FontSize = 15);
    // 描边文本
    void
    StrokeText(std::string Text, Vec2 Pos, ImColor Color, float FontSize = 15);
    // 矩形
    void Rectangle(Vec2 Pos, Vec2 Size, ImColor Color, float Thickness, float Rounding = 0);
    void RectangleFilled(Vec2 Pos, Vec2 Size, ImColor Color, float Rounding = 0, int Nums = 15);
    // 线
    void Line(Vec2 From, Vec2 To, ImColor Color, float Thickness);
    // 圆形
    void Circle(Vec2 Center, float Radius, ImColor Color, float Thickness, int Num = 50);
    void CircleFilled(Vec2 Center, float Radius, ImColor Color, int Num = 50);
    // 连接点
    void ConnectPoints(std::vector<Vec2> Points, ImColor Color, float Thickness);
    // 圆弧
    void Arc(ImVec2 Center, float Radius, ImColor Color, float Thickness, float Aangle_begin, float Angle_end, float Nums = 15);
    // 勾选框
    void MyCheckBox(const char *str_id, bool *v);
    void MyCheckBox2(const char *str_id, bool *v);
    void MyCheckBox3(const char *str_id, bool *v);
    void MyCheckBox4(const char *str_id, bool *v);
    // 半圆
    void HalfCircle(Vec2 Pos, float radius, ImVec4 color, int segments, int thickness, float arc_degree);
    // 图片
    void Image(Vec2 Pos, int w, int h, ImTextureID Texture);
    // 水印
    void Watermark(std::string licenseKey);
    // 轮廓方框
    void OutlineBox(int x, int y, int w, int h, ImVec4 color, float thickness);
    // 左Switch
    bool SwitchForLeft(const char *str_id, bool *v);
    // 右Switch
    bool SwitchForRight(const char *str_id, bool *v);
};

}; // namespace OSImGui

inline OSImGui::OSImGui &Gui = OSImGui::OSImGui::get();

#endif // OS_IMGUI_H