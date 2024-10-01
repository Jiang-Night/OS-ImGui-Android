
#pragma once
#include "OS-ImGui.h"
#include "OS-ImGui_Struct.h"
#include "imgui_internal.h"

namespace OSImGui {
void OSImGui::Text(std::string Text, Vec2 Pos, ImColor Color, float FontSize) {
    float TextWidth = ImGui::GetFont()->CalcTextSizeA(FontSize, FLT_MAX, 0.f, Text.c_str()).x;
    ImVec2 Pos_ = {Pos.x - TextWidth / 2, Pos.y};
    ImGui::GetForegroundDrawList()->AddText(ImGui::GetFont(), FontSize, Pos_, Color, Text.c_str());
}

void OSImGui::StrokeText(std::string Text, Vec2 Pos, ImColor Color, float FontSize) {
    this->Text(Text, Vec2(Pos.x - 1, Pos.y + 1), ImColor(0, 0, 0), FontSize);
    this->Text(Text, Vec2(Pos.x - 1, Pos.y - 1), ImColor(0, 0, 0), FontSize);
    this->Text(Text, Vec2(Pos.x + 1, Pos.y + 1), ImColor(0, 0, 0), FontSize);
    this->Text(Text, Vec2(Pos.x + 1, Pos.y - 1), ImColor(0, 0, 0), FontSize);
    this->Text(Text, Pos, Color, FontSize);
}

void OSImGui::Rectangle(Vec2 Pos, Vec2 Size, ImColor Color, float Thickness, float Rounding) {
    ImGui::GetForegroundDrawList()->AddRect(Pos.ToImVec2(), (Pos + Size).ToImVec2(), Color, Rounding, 0, Thickness);
}

void OSImGui::RectangleFilled(Vec2 Pos, Vec2 Size, ImColor Color, float Rounding, int Nums) {
    ImDrawList *DrawList = ImGui::GetForegroundDrawList();
    ImDrawCornerFlags rounding_corners = ImDrawCornerFlags_All;
    ImVec2 a = Pos.ToImVec2();
    ImVec2 b = {Pos.x + Size.x, Pos.y + Size.y};
    Rounding = ImMin<float>(Rounding, fabsf(Size.x) * (((rounding_corners & ImDrawCornerFlags_Top) == ImDrawCornerFlags_Top) || ((rounding_corners & ImDrawCornerFlags_Bot) == ImDrawCornerFlags_Bot) ? 0.5f : 1.0f) - 1.0f);
    Rounding = ImMin<float>(Rounding, fabsf(Size.y) * (((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f) - 1.0f);
    if (Rounding <= 0.0f || rounding_corners == 0) {
        DrawList->PathLineTo(a);
        DrawList->PathLineTo(ImVec2(b.x, a.y));
        DrawList->PathLineTo(b);
        DrawList->PathLineTo(ImVec2(a.x, b.y));
    } else {
        DrawList->PathArcTo(ImVec2(a.x + Rounding, a.y + Rounding), Rounding, IM_PI, IM_PI / 2.f * 3.f, Nums);
        DrawList->PathArcTo(ImVec2(b.x - Rounding, a.y + Rounding), Rounding, IM_PI / 2.f * 3.f, IM_PI * 2.f, Nums);
        DrawList->PathArcTo(ImVec2(b.x - Rounding, b.y - Rounding), Rounding, 0.f, IM_PI / 2.f, Nums);
        DrawList->PathArcTo(ImVec2(a.x + Rounding, b.y - Rounding), Rounding, IM_PI / 2.f, IM_PI, Nums);
    }
    DrawList->PathFillConvex(Color);
}

void OSImGui::Line(Vec2 From, Vec2 To, ImColor Color, float Thickness) {
    ImGui::GetForegroundDrawList()->AddLine(From.ToImVec2(), To.ToImVec2(), Color, Thickness);
}

void OSImGui::Circle(Vec2 Center, float Radius, ImColor Color, float Thickness, int Num) {
    ImGui::GetForegroundDrawList()->AddCircle(Center.ToImVec2(), Radius, Color, Num, Thickness);
}
void OSImGui::CircleFilled(Vec2 Center, float Radius, ImColor Color, int Num) {
    ImGui::GetForegroundDrawList()->AddCircleFilled(Center.ToImVec2(), Radius, Color, Num);
}

void OSImGui::ConnectPoints(std::vector<Vec2> Points, ImColor Color, float Thickness) {
    if (Points.size() <= 0)
        return;
    for (int i = 0; i < Points.size() - 1; i++) {
        Line(Points[i], Points[i + 1], Color, Thickness);
        if (i == Points.size() - 2)
            Line(Points[i + 1], Points[0], Color, Thickness);
    }
}

void OSImGui::Arc(ImVec2 Center, float Radius, ImColor Color, float Thickness, float Angle_begin, float Angle_end, float Nums) {
    ImDrawList *DrawList = ImGui::GetForegroundDrawList();
    float angle = (Angle_end - Angle_begin) / Nums;
    for (int i = 0; i < Nums; i++) {
        float angle_ = i * angle + Angle_begin - IM_PI / 2;
        DrawList->PathLineTo({Center.x - Radius * cos(angle_), Center.y - Radius * sin(angle_)});
    }
    DrawList->PathStroke(Color, false, Thickness);
}
void OSImGui::MyCheckBox(const char *str_id, bool *v) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *DrawList = ImGui::GetWindowDrawList();
    float Height = ImGui::GetFrameHeight();
    float Width = Height * 1.7f;
    float Radius = Height / 2 - 2;

    ImGui::InvisibleButton(str_id, ImVec2(Width, Height));
    if (ImGui::IsItemClicked())
        *v = !(*v);
    // 组件移动动画
    float t = *v ? 1.0f : 0.f;
    ImGuiContext &g = *GImGui;
    float AnimationSpeed = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id)) {
        float T_Animation = ImSaturate(g.LastActiveIdTimer / AnimationSpeed);
        t = *v ? (T_Animation) : (1.0f - T_Animation);
    }
    // 鼠标悬停颜色
    ImU32 Color;
    if (ImGui::IsItemHovered())
        Color = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.24f, 0.15f, 1.0f), ImVec4(0.55f, 0.85f, 0.13f, 1.000f), t));
    else
        Color = ImGui::GetColorU32(ImLerp(ImVec4(0.90f, 0.29f, 0.20f, 1.0f), ImVec4(0.60f, 0.90f, 0.18f, 1.000f), t));
    // 组件绘制
    DrawList->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + Width, p.y + Height), Color, Height);
    DrawList->AddCircleFilled(ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2), p.y + Radius + 2), Radius, IM_COL32(255, 255, 255, 255), 360);
    DrawList->AddCircle(ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2), p.y + Radius + 2), Radius, IM_COL32(20, 20, 20, 80), 360, 1);

    ImGui::SameLine();
    ImGui::Text(str_id);
}

void OSImGui::MyCheckBox2(const char *str_id, bool *v) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *DrawList = ImGui::GetWindowDrawList();
    float Height = ImGui::GetFrameHeight();
    float Width = Height * 1.7f;
    float Radius = Height / 2 - 2;

    ImGui::InvisibleButton(str_id, ImVec2(Width, Height));
    if (ImGui::IsItemClicked())
        *v = !(*v);
    // 组件移动动画
    float t = *v ? 1.0f : 0.f;
    ImGuiContext &g = *GImGui;
    float AnimationSpeed = 0.15f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id)) {
        float T_Animation = ImSaturate(g.LastActiveIdTimer / AnimationSpeed);
        t = *v ? (T_Animation) : (1.0f - T_Animation);
    }
    // 鼠标悬停颜色
    ImU32 Color;
    if (ImGui::IsItemHovered())
        Color = ImGui::GetColorU32(ImLerp(ImVec4(0.08f, 0.18f, 0.21f, 1.0f), ImVec4(0.10f, 0.48f, 0.68f, 1.000f), t));
    else
        Color = ImGui::GetColorU32(ImLerp(ImVec4(0.12f, 0.22f, 0.25f, 1.0f), ImVec4(0.14f, 0.52f, 0.72f, 1.000f), t));
    // 组件绘制
    DrawList->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + Width, p.y + Height), Color, 360);
    DrawList->AddCircleFilled(ImVec2(p.x + Radius + 2 + t * (Width - (Radius + 2) * 2), p.y + Radius + 2), Radius + 2, IM_COL32(255, 255, 255, 255), 360);
    DrawList->AddCircleFilled(ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2), p.y + Radius + 2), Radius, IM_COL32(230, 230, 230, 255), 360);
    if (*v)
        DrawList->AddText(ImVec2(p.x + 45, p.y + 2), ImColor{255, 255, 255, 255}, str_id);
    else
        DrawList->AddText(ImVec2(p.x + 45, p.y + 2), ImColor{185, 185, 185, 255}, str_id);
}

void OSImGui::MyCheckBox3(const char *str_id, bool *v) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *DrawList = ImGui::GetWindowDrawList();
    float Height = ImGui::GetFrameHeight();
    float Width = Height;
    float Left = 8;
    float Right = Left * 1.5f;
    ImGui::InvisibleButton(str_id, ImVec2(Width, Height));

    if (ImGui::IsItemClicked())
        *v = !(*v);
    // 组件移动动画
    float t = *v ? 1.0f : 0.f;
    ImGuiContext &g = *GImGui;
    float AnimationSpeed = 0.12f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id)) {
        float T_Animation = ImSaturate(g.LastActiveIdTimer / AnimationSpeed);
        t = *v ? (T_Animation) : (1.0f - T_Animation);
    }
    // 鼠标悬停颜色
    ImU32 Color;
    ImU32 TickColor1, TickColor2;
    if (ImGui::IsItemHovered())
        Color = ImGui::GetColorU32(ImLerp(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), ImVec4(0.05f, 0.85f, 0.25f, 1.000f), t));
    else
        Color = ImGui::GetColorU32(ImLerp(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), ImVec4(0.1f, 0.9f, 0.3f, 1.000f), t));

    TickColor1 = IM_COL32(255, 255, 255, 255 * t);
    TickColor2 = IM_COL32(180, 180, 180, 255 * (1 - t));

    float Size = Width;
    float Scale = (float)(Size) / 20.0f;
    // 底色
    DrawList->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + Width, p.y + Height), Color, 5, 15);
    // 选中勾
    DrawList->AddLine(ImVec2(p.x + 3 * Scale, p.y + Size / 2 - 2 * Scale), ImVec2(p.x + Size / 2 - 1 * Scale, p.y + Size - 5 * Scale), TickColor1, 3 * Scale);
    DrawList->AddLine(ImVec2(p.x + Size - 3 * Scale - 1, p.y + 3 * Scale + 1), ImVec2(p.x + Size / 2 - 1 * Scale, p.y + Size - 5 * Scale), TickColor1, 3 * Scale);
    // 未选中勾
    DrawList->AddLine(ImVec2(p.x + 3 * Scale, p.y + Size / 2 - 2 * Scale), ImVec2(p.x + Size / 2 - 1 * Scale, p.y + Size - 5 * Scale), TickColor2, 3 * Scale);
    DrawList->AddLine(ImVec2(p.x + Size - 3 * Scale - 1, p.y + 3 * Scale + 1), ImVec2(p.x + Size / 2 - 1 * Scale, p.y + Size - 5 * Scale), TickColor2, 3 * Scale);
    ImGui::SameLine();
    ImGui::Text(str_id);
}

void OSImGui::MyCheckBox4(const char *str_id, bool *v) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *DrawList = ImGui::GetWindowDrawList();
    float Height = ImGui::GetFrameHeight();
    float Width = Height;
    ImGui::InvisibleButton(str_id, ImVec2(Width, Height));

    if (ImGui::IsItemClicked())
        *v = !(*v);
    // 组件动画
    float t = *v ? 1.0f : 0.f;
    ImGuiContext &g = *GImGui;
    float AnimationSpeed = 0.12f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id)) {
        float T_Animation = ImSaturate(g.LastActiveIdTimer / AnimationSpeed);
        t = *v ? (T_Animation) : (1.0f - T_Animation);
    }
    // bg 0.74 0.72 0.81-> 0.69 0.77 0.76
    ImU32 BgColor;
    if (ImGui::IsItemHovered())
        BgColor = ImGui::GetColorU32(ImVec4(0.69f, 0.69f, 0.69f, 1.0f));
    else
        BgColor = ImGui::GetColorU32(ImVec4(0.74f, 0.74f, 0.74f, 1.0f));
    DrawList->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + Width, p.y + Width), BgColor);

    ImU32 FrColor;
    FrColor = ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, 0.5f * t));
    DrawList->AddRectFilled(ImVec2(p.x + Width / 5, p.y + Width / 5), ImVec2(p.x + Width - Width / 5, p.y + Width - Width / 5), FrColor);

    ImGui::SameLine();
    ImGui::Text(str_id);
}
void OSImGui::HalfCircle(Vec2 Pos, float radius, ImVec4 color, int segments, int thickness, float arc_degree) {
    ImGui::GetForegroundDrawList()->AddHalfCircle(ImVec2(Pos.x, Pos.y), radius, ImGui::ColorConvertFloat4ToU32(color), segments, thickness, arc_degree);
}

void OSImGui::Image(Vec2 Pos, int w, int h, ImTextureID Texture) {
    ImGui::GetForegroundDrawList()->AddImage(Texture, ImVec2(Pos.x, Pos.y), ImVec2(Pos.x + w, Pos.y + h));
}

void OSImGui::Watermark(std::string licenseKey) {
    ImGuiIO &io = ImGui::GetIO();
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    // 获取当前窗口的位置和大小
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();
    // 设置文本颜色
    ImU32 text_color = IM_COL32(128, 128, 128, 200);
    // 设置文本大小
    window->DrawList->PushTextureID(io.Fonts->TexID);
    window->DrawList->PushClipRectFullScreen();
    // 计算文字宽高
    ImVec2 textSize = ImGui::CalcTextSize(licenseKey.c_str());
    float textWidth = textSize.x;
    float textHeight = textSize.y;
    // 设置文本间距
    float padding = 60.0f;
    // 绘制在ImGui窗口背景上
    for (float x = padding; x < window_size.x; x += textWidth + padding) {
        for (float y = padding; y < window_size.y; y += textHeight + padding) {
            ImVec2 watermarkPos = ImVec2(window_pos.x + x, window_pos.y + y);
            // 检查文本是否超出窗口范围
            if (watermarkPos.x + textWidth <= window_pos.x + window_size.x && watermarkPos.y + textHeight <= window_pos.y + window_size.y) {
                window->DrawList->AddText(watermarkPos, text_color, licenseKey.c_str());
            }
        }
    }
    // 恢复裁剪区域
    window->DrawList->PopClipRect();
}

void OSImGui::OutlineBox(int x, int y, int w, int h, ImVec4 color, float thickness) {
    this->Rectangle(Vec2{x, y}, Vec2{w, h}, ImVec4(0.f, 0.f, 0.f, 1.f), thickness);
    this->Rectangle(Vec2{x + 1, y + 1}, Vec2{w - 2, h - 2}, color, thickness);
    this->Rectangle(Vec2{x + 2, y + 2}, Vec2{w - 4, h - 4}, ImVec4(0.f, 0.f, 0.f, 1.f), thickness);
}

bool OSImGui::SwitchForLeft(const char *str_id, bool *v) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *DrawList = ImGui::GetWindowDrawList();
    float Height = ImGui::GetFrameHeight();
    float Width = Height * 1.7f;
    float Radius = Height / 2 - 2;

    ImGui::InvisibleButton(str_id, ImVec2(Width, Height));

    if (ImGui::IsItemClicked())
        *v = !(*v);

    // 组件移动动画
    float t = *v ? 1.0f : 0.f;
    ImGuiContext &g = *GImGui;
    float AnimationSpeed = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id)) {
        float T_Animation = ImSaturate(g.LastActiveIdTimer / AnimationSpeed);
        t = *v ? (T_Animation) : (1.0f - T_Animation);
    }

    // 鼠标悬停颜色
    ImU32 Color;
    if (ImGui::IsItemHovered())
        Color = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.24f, 0.15f, 1.0f), ImVec4(0.55f, 0.85f, 0.13f, 1.000f), t));
    else
        Color = ImGui::GetColorU32(ImLerp(ImVec4(0.90f, 0.29f, 0.20f, 1.0f), ImVec4(0.60f, 0.90f, 0.18f, 1.000f), t));

    // 组件绘制
    DrawList->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + Width, p.y + Height), Color, Height);
    DrawList->AddCircleFilled(ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2), p.y + Radius + 2), Radius, IM_COL32(255, 255, 255, 255), 360);
    DrawList->AddCircle(ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2), p.y + Radius + 2), Radius, IM_COL32(20, 20, 20, 80), 360, 1);

    ImGui::SameLine();
    ImGui::Text(str_id);

    return *v; // 返回复选框是否被点击
}

bool OSImGui::SwitchForRight(const char *str_id, bool *v) {
    ImGui::Text(str_id); // 先绘制文本

    ImGui::SameLine(0, 140); // 保持水平布局

    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *DrawList = ImGui::GetWindowDrawList();
    float Height = ImGui::GetFrameHeight();
    float Width = Height * 1.7f;
    float Radius = Height / 2 - 2;

    ImGui::InvisibleButton(str_id, ImVec2(Width, Height));

    if (ImGui::IsItemClicked())
        *v = !(*v);

    // 组件移动动画
    float t = *v ? 1.0f : 0.f;
    ImGuiContext &g = *GImGui;
    float AnimationSpeed = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id)) {
        float T_Animation = ImSaturate(g.LastActiveIdTimer / AnimationSpeed);
        t = *v ? (T_Animation) : (1.0f - T_Animation);
    }

    // 鼠标悬停颜色
    ImU32 Color;
    if (ImGui::IsItemHovered())
        Color = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.24f, 0.15f, 1.0f), ImVec4(0.55f, 0.85f, 0.13f, 1.000f), t));
    else
        Color = ImGui::GetColorU32(ImLerp(ImVec4(0.90f, 0.29f, 0.20f, 1.0f), ImVec4(0.60f, 0.90f, 0.18f, 1.000f), t));

    // 组件绘制
    DrawList->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + Width, p.y + Height), Color, Height);
    DrawList->AddCircleFilled(ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2), p.y + Radius + 2), Radius, IM_COL32(255, 255, 255, 255), 360);
    DrawList->AddCircle(ImVec2(p.x + Radius + t * (Width - Radius * 2) + (t == 0 ? 2 : -2), p.y + Radius + 2), Radius, IM_COL32(20, 20, 20, 80), 360, 1);

    return *v; // 返回复选框是否被点击
}

} // namespace OSImGui
