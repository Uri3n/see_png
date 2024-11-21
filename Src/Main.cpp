#include <Carrier.hpp>
#include <Argparse.hpp>
#include <ConManip.hpp>
#include <Fmt.hpp>
#include <FileCycle.hpp>
#include <Context.hpp>
#include <print>
#include <csignal>
#include <cstdlib>
using namespace spng;

static auto handle_kb_interrupt(int signal) -> void {
  std::println("\nKeyboard Interrupt. Exiting...");
  std::exit(signal);
}

static auto print_banner() -> void {
  constexpr auto banner =
    R"(  ___  ___  ___   _ __  _ __   __ _ )" "\n"
    R"( / __|/ _ \/ _ \ | '_ \| '_ \ / _` |)" "\n"
    R"( \__ \  __/  __/ | |_) | | | | (_| |)" "\n"
    R"( |___/\___|\___| | .__/|_| |_|\__, |)" "\n"
    R"(                 | |           __/ |)" "\n"
    R"(                 |_|          |___/ )" "\n";
  set_console(ConFg::Magenta);
  set_console(ConStyle::Bold);
  std::println("{}", banner);
  reset_console();
}

int main(int argc, char** argv) {
  std::signal(SIGINT, handle_kb_interrupt);
  if(argc < 2) {
    print_banner();
    print_help();
    return 0;
  }

  if(!init_context_from_args(argc, argv)) {
    return 1;
  }

  for(const auto& input : Context::get().ifilenames_) {
    if(!do_file_cycle(input)) return 1;
  }

  return 0;
}
