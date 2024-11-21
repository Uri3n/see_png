#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#include <CompileAttrs.hpp>
#include <cstdint>
#include <vector>
#include <string>

namespace spng {
  class Context;
}

class spng::Context {
public:
  std::vector<std::string> ifilenames_;
  std::vector<std::string> extract_chunks_;
  std::vector<std::string> dump_chunks_;

  std::string output_dir_;
  bool verbose_ = false;

  [[nodiscard]] SPNG_NOINLINE
  static auto get() -> Context&;

  auto debug_print() const -> void;
private:
  Context() = default;
};

#endif //CONTEXT_HPP
