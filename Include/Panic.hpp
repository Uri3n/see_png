#ifndef PANIC_HPP
#define PANIC_HPP
#include <cstdlib>
#include <string>
#include <print>
#include <ConManip.hpp>

#define PANIC(MSG) ::spng::_panic_impl(__FILE__, __LINE__, MSG)
#define FATAL(MSG) ::spng::_exit_impl(MSG)
#define UNREACHABLE PANIC("Unreachable branch executed.")
#define ASSERT(COND) if(!(COND)) PANIC("Assertion \"" #COND "\" failed!")

namespace spng {
  [[noreturn]] void _exit_impl(const std::string& msg);
  [[noreturn]] void _panic_impl(const std::string& file, int line, const std::string& msg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

[[noreturn]] inline void
spng::_panic_impl(const std::string& file, const int line, const std::string& msg) {
  // Red bold header
  set_console(ConFg::Red);
  set_console(ConStyle::Bold);
  std::println("PANIC :: {}", msg);

  // Location details
  reset_console();
  std::println("In file \"{}\" at line {}.", file, line);
  ::exit(EXIT_FAILURE);
}

[[noreturn]] inline void
spng::_exit_impl(const std::string& msg) {
  set_console(ConFg::Red);
  set_console(ConStyle::Bold);
  std::println("FATAL :: {}", msg);
  ::exit(1);
}

#endif //PANIC_HPP
