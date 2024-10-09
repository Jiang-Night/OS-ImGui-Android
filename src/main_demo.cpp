#include "OS-ImGui.h"
#include "OS-ImGui_Struct.h"
#include "imgui.h"
#include <cstdio>
#include <cstdlib>
#include <iostream>

bool isLine = false;
bool isLine1 = false;
bool isLine2 = false;
bool isLine3 = false;
bool isLine4 = false;
bool isLine5 = false;
bool isLine6 = false;

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

  std::cout << "Hello, World!" << std::endl;
  return 0;
}
