#ifndef ARGPARSE_HPP
#define ARGPARSE_HPP
#include <string_view>

namespace spng {
  // A record of a singular valid commandline flag,
  // it's description, and whether it's required or
  // not.
  struct FlagDescriptor;

  // Print all flags that the program accepts,
  // as well as some useful examples.
  auto print_help() -> void;

  // Initializes the global context object,
  // given the values passed to the main() function.
  // returns false if any invalid arguments were passed.
  auto init_context_from_args(int argc, char** argv) -> bool;
}

struct spng::FlagDescriptor {
  std::string_view lf;
  std::string_view sf;
  std::string_view desc;
};

#endif //ARGPARSE_HPP
