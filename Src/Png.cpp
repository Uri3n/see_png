#include <Png.hpp>
#include <Endian.hpp>
#include <Panic.hpp>
#include <Fmt.hpp>
#include <unordered_map>
#include <algorithm>
#include <ranges>
#include <print>
#include <cstring>
#include <array>

auto spng::Carrier::_gather_chunks() -> Carrier& {
  ASSERT(buff_ != nullptr);
  ASSERT(buff_->size() > 8);

  auto& ihdr = chunks_.emplace_back();
  ihdr.buff_ = this->buff_;
  ihdr.offset_ = 8;

  if(ihdr.type() != Chunk::Type::IHDR) {
    throw std::runtime_error("corrupted PNG - no IHDR");
  }

  auto curr_chunk     = ihdr.next();
  auto chunk_indices  = std::vector<Chunk::Type>();

  while(curr_chunk.has_value()) {
    chunks_.emplace_back(*curr_chunk);
    chunk_indices.emplace_back(curr_chunk->type());
    curr_chunk = curr_chunk->next();
  }

  if(chunk_indices.empty()) {
    throw std::runtime_error("PNG has no data.");
  } if(chunk_indices.back() != Chunk::Type::IEND) {
    throw std::runtime_error("IEND is not the final PNG chunk.");
  }

  return *this;
}

auto spng::Carrier::_verify_signature() -> Carrier& {
  ASSERT(buff_ != nullptr);
  ASSERT(!buff_->empty());

  // These first 8 "magic" bytes must be
  // at the beginning of the file.
  constexpr std::array<uint8_t, 8> png_signature = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
  };

  if(png_signature.size() >= buff_->size()) {
    throw std::runtime_error("File is too small.");
  }

  for(size_t i = 0; i < png_signature.size(); i++) {
    if(buff_->at(i) != png_signature.at(i)) {
      throw std::runtime_error("Invalid PNG signature.");
    }
  }
  
  return *this;
}

spng::Carrier::Carrier(const FlatBuffer::Buffer& file) {
  if(file.empty()) {
    throw std::runtime_error("Empty file buffer");
  }

  buff_  = FlatBuffer::make_shared(file.size());
  *buff_ = file;
  _verify_signature();
  _gather_chunks();
}

spng::Carrier::Carrier(const FileRef& file) {
  const auto fsize = file.size();
  buff_ = file.read(fsize);
  _verify_signature();
  _gather_chunks();
}

auto spng::Chunk::_throw_bad_chunk() const -> void {
  std::string buff;
  buff.append("This PNG has a corrupted or invalid chunk,\n");
  buff.append("data cannot be read from it...\n");
  buff.append(fmt("At offset: {:8X}\n", offset_));
  throw std::runtime_error(buff);
}

auto spng::Chunk::type() const -> Type {
  const auto str = type_string();
  #define X(CHUNK_TYPE) \
    if(str == #CHUNK_TYPE) return Type::CHUNK_TYPE;
    SEE_PNG_CHUNK_LIST
  #undef X
  return Type::Unknown;
}

auto spng::Chunk::type_string() const -> std::string {
  char type_arr[5] = { 0 };
  const auto ptr   = buff_.lock();

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(offset_ + (sizeof(Header) - 1) >= ptr->size()) {
    _throw_bad_chunk();
  }

  try {
    const auto* header = (Header*)(&ptr->at(offset_));
    type_arr[0] = header->type[0];
    type_arr[1] = header->type[1];
    type_arr[2] = header->type[2];
    type_arr[3] = header->type[3];
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  type_arr[4] = '\0';
  return { type_arr };
}

auto spng::Chunk::checksum() const -> uint32_t {
  uint32_t the_checksum = 0;
  const auto ptr = buff_.lock();

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(offset_ + (sizeof(Header) - 1) >= ptr->size()) {
    _throw_bad_chunk();
  }

  try {
    const auto header  = reinterpret_cast<Header*>(&ptr->at(offset_));
    const auto ch_len  = maybe_bitswap(header->length, Endian::Big);
    const auto address = reinterpret_cast<uint32_t*>(&ptr->at(
      + offset_
      + sizeof(Header)
      + ch_len
    ));

    // Ensure that we can access the LAST byte
    // of the CRC-32 checksum without going OOB.
    const size_t crc_last_byte = {
      + offset_
      + sizeof(Header)
      + ch_len
      + (sizeof(uint32_t) - 1)
    };

    if(crc_last_byte >= ptr->size()) {
      throw std::out_of_range("-");
    }
    the_checksum = *address;
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  return the_checksum;
}

auto spng::Chunk::length() const -> uint32_t {
  uint32_t the_length = 0;
  const auto ptr = buff_.lock();

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(offset_ + (sizeof(Header) - 1) >= ptr->size()) {
    _throw_bad_chunk();
  }

  try {
    auto* header = reinterpret_cast<Header*>(&ptr->at(offset_));
    the_length   = header->length;
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  // If our platform is not BE, swap these bytes.
  return maybe_bitswap(the_length, Endian::Big);
}

auto spng::Chunk::next() const -> std::optional<Chunk> {
  const auto ptr = buff_.lock();
  Chunk chunk;

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if((offset_ + (sizeof(Header) - 1)) >= ptr->size()) {
    _throw_bad_chunk();
  } if(type() == Type::IEND) {
    return std::nullopt;
  }

  try {
    const auto* header = reinterpret_cast<Header*>(&ptr->at(offset_));
    const auto  length = maybe_bitswap(header->length, Endian::Big);
    const size_t ch_offset {
      + offset_           // Offset to the chunk header
      + sizeof(Header)    // Add sizeof length + chunk type array
      + length            // Length of the chunk's data
      + sizeof(uint32_t)  // Length of the CRC-32 checksum after the chunk.
    };

    if(ch_offset >= ptr->size()) {
      return std::nullopt;
    }

    chunk.buff_ = ptr;
    chunk.offset_  = ch_offset;
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  return chunk;
}

auto spng::Ihdr::bit_depth() const -> uint8_t {
  ASSERT(type() == Type::IHDR);
  uint8_t the_bit_depth = 0;
  const auto len = length();
  const auto ptr = buff_.lock();

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Ihdr::Layout)) {
    throw std::runtime_error("invalid IHDR size.");
  }

  try {
    const auto* layout = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    const size_t last_byte = {
      + offset_
      + sizeof(Header)
      + (len - 1)
    };

    if(last_byte >= ptr->size()) {
      throw std::out_of_range("-");
    }

    the_bit_depth = layout->bit_depth;
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  return the_bit_depth;
}

auto spng::Ihdr::color_type() const -> ColorType {
  const auto len = length();
  const auto ptr = buff_.lock();

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Ihdr::Layout)) {
    throw std::runtime_error("invalid IHDR size.");
  }

  try {
    const auto* layout = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    const size_t last_byte = {
      + offset_
      + sizeof(Header)
      + (len - 1)
    };

    if(last_byte >= ptr->size()) {
      throw std::out_of_range("-");
    }

    switch(maybe_bitswap(layout->color_type, Endian::Big)) {
      case 0: return ColorType::GrayScale;
      case 2: return ColorType::TrueColor;
      case 3: return ColorType::IndexedColor;
      case 4: return ColorType::GrayscaleAlpha;
      case 6: return ColorType::TruecolorAlpha;
      default: break;
    }
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  throw std::runtime_error("Invalid PNG color type.");
}

auto spng::Ihdr::width() const -> uint32_t {
  uint32_t the_width = 0;
  const auto len = length();
  const auto ptr = buff_.lock();

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Ihdr::Layout)) {
    throw std::runtime_error("invalid IHDR size.");
  }

  try {
    const auto* layout = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    const size_t last_byte = {
      + offset_
      + sizeof(Header)
      + (len - 1)
    };

    if(last_byte >= ptr->size()) {
      throw std::out_of_range("-");
    }

    the_width = maybe_bitswap(layout->width, Endian::Big);
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  return the_width;
}

auto spng::Ihdr::height() const -> uint32_t {
  uint32_t the_height = 0;
  const auto len = length();
  const auto ptr = buff_.lock();

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Ihdr::Layout)) {
    throw std::runtime_error("invalid IHDR size.");
  }

  try {
    const auto* layout = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    const size_t last_byte = {
      + offset_
      + sizeof(Header)
      + (len - 1)
    };

    if(last_byte >= ptr->size()) {
      throw std::out_of_range("-");
    }

    the_height = maybe_bitswap(layout->height, Endian::Big);
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  return the_height;
}

auto spng::Ihdr::interlace_method() const -> Interlace {
  const auto len = length();
  const auto ptr = buff_.lock();

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Ihdr::Layout)) {
    throw std::runtime_error("invalid IHDR size.");
  }

  try {
    const auto* layout = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    const size_t last_byte = {
      + offset_
      + sizeof(Header)
      + (len - 1)
    };

    if(last_byte >= ptr->size()) {
      throw std::out_of_range("-");
    }

    const auto raw = maybe_bitswap(layout->interlace, Endian::Big);
    if(raw == 0) {
      return Interlace::None;
    } if(raw == 1) {
      return Interlace::Adam7;
    }
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  return Interlace::Invalid;
}

auto spng::Ihdr::compression_method() const -> Compression {
  const auto len = length();
  const auto ptr = buff_.lock();

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Ihdr::Layout)) {
    throw std::runtime_error("invalid IHDR size.");
  }

  try {
    const auto* layout = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    const size_t last_byte = {
      + offset_
      + sizeof(Header)
      + (len - 1)
    };

    if(last_byte >= ptr->size()) {
      throw std::out_of_range("-");
    }

    // Currently only the DEFLATE
    // algorithm is supported (value of 0).
    // Return an error if it's something else...
    return maybe_bitswap(layout->compression, Endian::Big) == 0
      ? Compression::Deflate
      : Compression::Invalid;
  } catch(const std::out_of_range& _) {
    throw std::runtime_error("corrupted PNG - bad IHDR");
  }

  UNREACHABLE;
}

auto spng::Ihdr::filter_method() const -> FilterMethod {
  const auto len = length();
  const auto ptr = buff_.lock();

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Ihdr::Layout)) {
    throw std::runtime_error("invalid IHDR size.");
  }

  try {
    const auto* layout = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    const size_t last_byte = {
      + offset_
      + sizeof(Header)
      + (len - 1)
    };

    if(last_byte >= ptr->size()) {
      throw std::out_of_range("-");
    }

    // Same story as the compression method.
    // The only supported value here (currently) is 0.

    return maybe_bitswap(layout->filter, Endian::Big) == 0
      ? FilterMethod::Default
      : FilterMethod::Invalid;
  } catch(const std::out_of_range& _) {
    throw std::runtime_error("corrupted PNG - bad IHDR");
  }

  UNREACHABLE;
}
