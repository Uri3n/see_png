#ifndef HEXDUMP_HPP
#define HEXDUMP_HPP
#include <FlatBuffer.hpp>
#include <span>
#include <cstdint>

namespace spng {
  auto hexdump(const std::span<char>& bytes) -> void;
}

#endif //HEXDUMP_HPP
