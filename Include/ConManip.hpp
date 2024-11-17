#ifndef CONCOLOURS_HPP
#define CONCOLOURS_HPP
#include <cstdint>

namespace spng {
  enum class ConFg : uint16_t {
    None   = 0,
    Green  = 32,
    Yellow = 33,
    Blue   = 34,
    Cyan   = 36,
    Red    = 91,
  };

  enum class ConStyle : uint16_t {
    Bold      = 1,
    Underline = 4,
  };

#if defined(SEE_PNG_WIN32)
  bool are_console_virtual_sequences_enabled();
  void enable_console_virtual_sequences();
  void maybe_enable_console_virtual_sequences();
#endif
  void set_console(ConFg fg);
  void set_console(ConStyle cs);
  void reset_console();
}

#endif //CONCOLOURS_HPP
