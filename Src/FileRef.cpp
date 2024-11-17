#include <FileRef.hpp>
#include <Defer.hpp>
#include <Fmt.hpp>
#include <ios>
#include <fstream>

spng::FileRef::FileRef(const std::string& file_name) {
  if(!fs::exists(file_name)) {
    throw std::runtime_error(fmt("\"{}\" does not exist.", file_name));
  } if(fs::is_directory(file_name)) {
    throw std::runtime_error(fmt("\"{}\" is a directory.", file_name));
  } if(!fs::is_regular_file(file_name)) {
    throw std::runtime_error(fmt("\"{}\" is not a regular file.", file_name));
  }

  path_ = file_name;
}

spng::FileRef::FileRef(const std::wstring& file_name) {
#if !defined(SEE_PNG_WIN32)
  throw std::runtime_error(
    "Internal: FileRef::FileRef:"
    "unexpected wide string on a POSIX platform.");
#else
  if(!fs::exists(file_name)) {
    throw std::runtime_error("File does not exist.");
  } if(fs::is_directory(file_name)) {
    throw std::runtime_error("Path is a directory, not a file.");
  } if(!fs::is_regular_file(file_name)) {
    throw std::runtime_error("File is not regular.");
  }

  path_ = file_name;
#endif
}

auto spng::FileRef::read(const uintmax_t amnt) const
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

auto spng::FileRef::name() const -> std::string {
  return path_.string();
}

auto spng::FileRef::size() const -> uintmax_t {
  return file_size(path_);
}


