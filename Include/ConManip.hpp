#ifndef CONCOLOURS_HPP
#define CONCOLOURS_HPP
#include <cstdint>
#include <string>

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

#endif //CONCOLOURS_HPP
