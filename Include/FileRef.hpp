#ifndef FILEREF_HPP
#define FILEREF_HPP
#include <FlatBuffer.hpp>
#include <filesystem>
#include <cstdint>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace fs = std::filesystem;
namespace spng {
  class FileRef;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class spng::FileRef {
public:
  [[nodiscard]] auto size() const -> uintmax_t;
  [[nodiscard]] auto read(uintmax_t amnt) const -> FlatBuffer::Shared;
  [[nodiscard]] auto name() const -> std::string;

  explicit FileRef(const std::wstring &file_name);
  explicit FileRef(const std::string &file_name);
private:
  fs::path path_;
};

#endif //FILEREF_HPP
