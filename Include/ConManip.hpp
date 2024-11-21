#ifndef CONCOLOURS_HPP
#define CONCOLOURS_HPP
#include <cstdint>
#include <string>
#include <print>

namespace spng {
  enum class ConFg : uint16_t {
    None    = 0,
    Green   = 32,
    Yellow  = 33,
    Blue    = 34,
    Magenta = 35,
    Cyan    = 36,
    White   = 37,
    Red     = 91,
  };

  enum class ConStyle : uint16_t {
    Bold      = 1,
    Underline = 4,
  };

#if defined(SEE_PNG_WIN32)
  auto are_console_virtual_sequences_enabled() -> bool;
  auto enable_console_virtual_sequences() -> void;
  auto maybe_enable_console_virtual_sequences() -> void;
#endif
  auto set_console(ConFg fg) -> void;
  auto set_console(ConStyle cs) -> void;
  auto reset_console() -> void;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline auto spng::set_console(const ConStyle cs) -> void {
#if defined(SEE_PNG_WIN32)
  maybe_enable_console_virtual_sequences();
#endif
  std::print("\x1b[{}m", std::to_string(static_cast<uint16_t>(cs)));
}

inline auto spng::set_console(const ConFg fg) -> void {
#if defined(SEE_PNG_WIN32)
  maybe_enable_console_virtual_sequences();
#endif
  std::print("\x1b[{}m", std::to_string(static_cast<uint16_t>(fg)));
}

inline auto spng::reset_console() -> void {
#if defined(SEE_PNG_WIN32)
  maybe_enable_console_virtual_sequences();
#endif
  std::print("\x1b[m");
}

#endif //CONCOLOURS_HPP
