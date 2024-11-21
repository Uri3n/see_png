#include <InFileRef.hpp>
#include <Defer.hpp>
#include <Fmt.hpp>
#include <ios>
#include <fstream>

spng::InFileRef::InFileRef(const std::string& file_name) {
  if(!fs::exists(file_name)) {
    throw std::ios_base::failure(fmt("\"{}\" does not exist.", file_name));
  } if(fs::is_directory(file_name)) {
    throw std::ios_base::failure(fmt("\"{}\" is a directory.", file_name));
  } if(!fs::is_regular_file(file_name)) {
    throw std::ios_base::failure(fmt("\"{}\" is not a regular file.", file_name));
  }

  path_ = file_name;
}

auto spng::InFileRef::read(const uintmax_t amnt) const
-> FlatBuffer::Shared {
  std::ifstream stream(path_.string(), std::ios::binary);
  spng_defer_if(stream.is_open(), [&] {
    stream.close();
  });

  // - Check if stream is open
  // - Check if file size is valid
  if(!stream.is_open()) {
    throw std::ios_base::failure(fmt("Could not open file {}.", path_.string()));
  } if(const auto _size = size(); !_size || amnt > _size) {
    throw std::ios_base::failure("Invalid file size.");
  }

  const auto buff = FlatBuffer::make_shared(amnt);
  stream.read(
    reinterpret_cast<char*>(buff->data()),
    static_cast<std::streamsize>(buff->size()
  ));

  return buff;
}

auto spng::InFileRef::name() const -> std::string {
  return path_.string();
}

auto spng::InFileRef::size() const -> uintmax_t {
  return file_size(path_);
}


