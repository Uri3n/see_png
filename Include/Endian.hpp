#ifndef ENDIAN_HPP
#define ENDIAN_HPP
#include <bit>
#include <type_traits>

namespace spng {
  // Endianness constants
  // Don't allow mixed endianness
  enum class Endian : uint8_t {
    Big,
    Little
  };

  // Swaps the endianness of the given value
  // of type T, which must be integral
  template<typename T> requires std::is_integral_v<T>
  auto bitswap(T val) -> T;

  // Swaps the endianness of the given value of
  // type T IF the provided endianness of the value
  // does not match the endianness of the platform
  // this program is running on.
  template<typename T> requires std::is_integral_v<T>
  auto maybe_bitswap(T val, Endian e) -> T;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T> requires std::is_integral_v<T>
auto spng::bitswap(const T val) -> T {
  return std::byteswap<T>(val);
}

template<typename T> requires std::is_integral_v<T>
auto spng::maybe_bitswap(const T val, const Endian e) -> T {
  auto plat_end = Endian::Little;
  if constexpr (std::endian::native == std::endian::big) {
    plat_end = Endian::Big;
  }

  return plat_end != e ? bitswap<T>(val) : val;
}

#endif //ENDIAN_HPP
