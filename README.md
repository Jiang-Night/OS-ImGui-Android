# OS-ImGui-Android
Android Platform Rapid Development ImGui.

一个简单的imgui库，基于[Dear-ImGui](https://github.com/ocornut/imgui).

项目参考[Liv](https://github.com/TKazer/OS-ImGui).

仅需几分钟，快速搭建imgui环境，让人简单的上手imgui

## 特点
* 代码框架简洁，使用方便，快速开发

* 易于使用
> Only one line of code is needed to call.

* 多样绘制
><img src = "https://github.com/Jiang-Night/OS-ImGui-Android/blob/main/Image/1.jpg" width = 400/>

## 示例
在main.cpp有使用案例
~~~ c++

void Imgui_demo() {
  ImGui::Begin("Test");
  ImGui::Text("Test Windows");
  ImGui::Checkbox("Line", &isLine);
  if (ImGui::Button("exit")) {
    Gui.Quit();
    exit(0);
  }
  Gui.MyCheckBox("Test1", &isLine1);
  Gui.MyCheckBox2("Test2", &isLine2);
  Gui.MyCheckBox3("Test3", &isLine3);
  Gui.MyCheckBox4("Test4", &isLine4);
  Gui.SwitchForRight("Test5", &isLine5);
  Gui.SwitchForLeft("Test6", &isLine6);

  ImGui::End();

  if (isLine) {
    Gui.Line(Vec2{0, 0}, Vec2{1000, 1000}, ImColor{255, 255, 255, 255}, 10);
  }
}


int main() {
  Gui.NewWindow("SurafceName", Imgui_demo);
  return 0;
}
~~~

## 未来待做
- [ ] 适配vulkan
- [ ] 适配内部注入版本
- [ ] 添加更多样式 以及更方便的使用方法

