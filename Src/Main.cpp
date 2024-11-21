#include <Carrier.hpp>
#include <Argparse.hpp>
#include <ConManip.hpp>
#include <print>
#include <iostream>
#include <csignal>
#include <cstdlib>

#include "Context.hpp"

#define TEST_FILE "Tests\\corrupted_3.png"
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

/*
 TODO:
 add support for these chunks:
 IDAT (zlib header), ITXT, TEXT, ZTXT

 after that tidy everything up.
*/

int main(int argc, char** argv) try {
  std::signal(SIGINT, handle_kb_interrupt);
  /*if(argc < 2) {
    print_banner();
    print_flags();
    return 0;
  }

  if(!init_context_from_args(argc, argv)) {
    return 1;
  }

  Context::get().debug_print();*/

  auto ref = FileRef(TEST_FILE);
  auto carrier = Carrier(ref);

  for(const auto& chunk : carrier.chunks()) {
    chunk.print();
  }

  carrier.print_summary();
  return 0;
} catch(const std::exception& e) {
  std::cerr << "EXCEPTION: " << e.what() << std::endl;
}
