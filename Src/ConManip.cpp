#include <ConManip.hpp>

#if defined(SEE_PNG_WIN32)
#include <Windows.h>

auto spng::are_console_virtual_sequences_enabled() -> bool {
  DWORD mode_stdout = 0;
  DWORD mode_stderr = 0;

  ::GetConsoleMode(::GetStdHandle(STD_OUTPUT_HANDLE), &mode_stdout);
  ::GetConsoleMode(::GetStdHandle(STD_ERROR_HANDLE), &mode_stderr);
  return (mode_stdout & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
      && (mode_stderr & ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

auto spng::enable_console_virtual_sequences() -> void {
  HANDLE h_stdout   = ::GetStdHandle(STD_OUTPUT_HANDLE);
  HANDLE h_stderr   = ::GetStdHandle(STD_ERROR_HANDLE);
  DWORD mode_stdout = 0;
  DWORD mode_stderr = 0;

  if(h_stdout == INVALID_HANDLE_VALUE || h_stderr == INVALID_HANDLE_VALUE) {
    return;
  }

  ::GetConsoleMode(h_stdout, &mode_stdout);
  ::GetConsoleMode(h_stderr, &mode_stderr);
  ::SetConsoleMode(h_stdout, mode_stdout | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  ::SetConsoleMode(h_stderr, mode_stderr | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

auto spng::maybe_enable_console_virtual_sequences() -> void {
  if(are_console_virtual_sequences_enabled())
    return;
  enable_console_virtual_sequences();
}

#endif // #if defined(SEE_PNG_WIN32)
