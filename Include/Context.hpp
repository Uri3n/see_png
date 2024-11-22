#ifndef CONTEXT_HPP
#define CONTEXT_HPP
#include <CompileAttrs.hpp>
#include <vector>
#include <string>
#include <filesystem>

namespace spng {
  class Context;
}

class spng::Context {
public:
  enum Flags : uint8_t {
    None     = 0U,
    Verbose  = 1U,
    Silent   = 1U << 1,
    NoSumm   = 1U << 2,
  };

  std::vector<std::string> ifilenames_;
  std::vector<std::string> extract_chunks_;
  std::vector<std::string> dump_chunks_;
  uint8_t flags_ = None;

  [[nodiscard]] SPNG_NOINLINE
  static auto get() -> Context&;

  auto debug_print() const -> void;
private:
  Context() = default;
};

#endif //CONTEXT_HPP
