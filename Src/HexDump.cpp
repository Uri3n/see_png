#include <Fmt.hpp>
#include <HexDump.hpp>
#include <ConManip.hpp>
#include <print>

auto spng::hexdump(const std::span<char>& bytes) -> void {
  static_assert(sizeof(char) == 1);
  size_t off = 0;
  if(bytes.empty()) {
    return;
  }

  std::string line_bin;
  std::string line_ascii;
  line_bin.reserve(42);
  line_ascii.reserve(42);

  auto is_printable = [](const uint8_t ch) -> bool {
    return ch >= 32 && ch <= 126;
  };

  auto do_offprint = [](const size_t _off) -> void {
    set_console(ConFg::White);
    set_console(ConStyle::Bold);
    std::print("{:08X}: ", _off);
    reset_console();
  };

  auto do_lineprint = [&]() -> void {
    std::print("{:<45}", line_bin);
    for(const auto ch : line_ascii) {
      if(ch != '.') {
        set_console(ConFg::Green);
        set_console(ConStyle::Bold);
        std::print("{}", ch);
        reset_console();
      } else {
        std::print("{}", ch);
      }
    }
    std::println("");
  };

  do_offprint(0);
  while(off < bytes.size()) {
    if(off && off % 16 == 0) {
      do_lineprint();
      do_offprint(off);
      line_bin.clear();
      line_ascii.clear();
    } if(off && off % 2 == 0 && !line_bin.empty()) {
      line_bin += ' ';
    }

    line_bin += spng::fmt("{:02X}", (uint8_t)bytes[off]);
    line_ascii += is_printable((uint8_t)bytes[off])
      ? bytes[off]
      : '.';
    ++off;
  }

  if(!line_bin.empty() && !line_ascii.empty()) {
    do_lineprint();
  }
  std::println("");
}