#ifndef FLATBUFFER_HPP
#define FLATBUFFER_HPP
#include <vector>
#include <memory>
#include <cstdint>
#include <stdexcept>

static_assert(sizeof(uint8_t) == 1);
namespace spng::FlatBuffer {
  using Byte   = uint8_t;
  using Buffer = std::vector<Byte>;
  using Shared = std::shared_ptr<Buffer>;
  using Weak   = std::weak_ptr<Buffer>;

  // Factories and such
  inline auto make_shared(size_t size) -> Shared;
  inline auto make_weak(const Shared &shared) -> Weak;
}

namespace fb = spng::FlatBuffer;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline auto fb::make_shared(const size_t size) -> Shared {
  if(size == 0) {
    throw std::runtime_error("Invalid file buffer.");
  }

  return std::make_shared<Buffer>(size);
}

inline auto fb::make_weak(const Shared &shared) -> Weak {
  if(!shared) {
    throw std::runtime_error("Null or invalid pointer.");
  }

  return { shared };
}

#endif //FLATBUFFER_HPP
