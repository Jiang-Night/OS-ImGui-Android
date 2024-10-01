#ifndef IMGUI_EXCEPTION_HPP
#define IMGUI_EXCEPTION_HPP

#include <exception>
#include <iostream>

namespace OSImGui {
class OSException : public std::exception {
  public:
    OSException() : Error_("[OS-Exception] Unkown Error") {}
    OSException(std::string Error) : Error_("[OS-Exception] " + Error) {}
    char const *what() const throw() { return Error_.c_str(); }

  private:
    std::string Error_ = "";
};
} // namespace OSImGui

#endif