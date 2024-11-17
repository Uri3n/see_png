#ifndef FMT_HPP
#define FMT_HPP
#include <string>
#include <format>

namespace spng {
  template<typename ... Args>
  std::string fmt(std::format_string<Args...> fmt, Args... args);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename ... Args>
auto spng::fmt(const std::format_string<Args...> fmt, Args... args) -> std::string {
  std::string output;
  try {
    output = std::vformat(fmt.get(), std::make_format_args(args...));
  } catch(const std::format_error& e) {
    output = std::string("!! std::vformat: ") + e.what();
  } catch(...) {
    output = "!! format error";
  }

  return output;
}

#endif //FMT_HPP
