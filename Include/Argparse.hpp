#ifndef ARGPARSE_HPP
#define ARGPARSE_HPP
#include <string_view>

namespace spng {
  // A record of a singular valid commandline flag,
  // it's description, and whether it's required or
  // not.
  struct FlagDescriptor;

  // Print all flags that the program accepts.
  auto print_flags() -> void;

  // Initializes the global context object,
  // given the values passed to the main() function.
  // returns false if any invalid arguments were passed.
  auto init_context_from_args(int argc, char** argv) -> bool;
}

struct spng::FlagDescriptor {
  std::string_view lf;
  std::string_view sf;
  std::string_view desc;
  bool req = false;
};

#endif //ARGPARSE_HPP
