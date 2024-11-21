#ifndef CARRIER_HPP
#define CARRIER_HPP
#include <Chunks.hpp>
#include <FlatBuffer.hpp>
#include <vector>

namespace spng {
  class Carrier;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class spng::Carrier {
  Carrier& _verify_signature();
  Carrier&  _gather_chunks();
public:
  Carrier(const Carrier&)             = delete;
  Carrier& operator=(const Carrier&)  = delete;

  auto print_summary() const -> void;
  [[nodiscard]] auto metadata()  const -> Ihdr;
  [[nodiscard]] auto chunks()    const -> const std::vector<Chunk>&;

  explicit Carrier(const InFileRef& file);
  explicit Carrier(const FlatBuffer::Buffer& file);
private:
  std::vector<Chunk> chunks_;
  FlatBuffer::Shared buff_;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline auto spng::Carrier::metadata() const
-> Ihdr {
  return chunks_.at(0).as<Ihdr>();
}

inline auto spng::Carrier::chunks() const
-> const std::vector<Chunk>& {
  return chunks_;
}

#endif //CARRIER_HPP
