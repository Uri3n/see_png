#ifndef INFILEREF_HPP
#define INFILEREF_HPP
#include <FlatBuffer.hpp>
#include <filesystem>
#include <cstdint>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace fs = std::filesystem;
namespace spng {
  class InFileRef;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class spng::InFileRef {
public:
  [[nodiscard]] auto size() const -> uintmax_t;
  [[nodiscard]] auto read(uintmax_t amnt) const -> FlatBuffer::Shared;
  [[nodiscard]] auto name() const -> std::string;
  explicit InFileRef(const std::string &file_name);
private:
  fs::path path_;
};

#endif //INFILEREF_HPP
