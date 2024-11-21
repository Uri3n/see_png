#include <Chunks.hpp>
#include <Endian.hpp>
#include <Panic.hpp>
#include <Fmt.hpp>
#include <unordered_map>
#include <algorithm>
#include <ranges>
#include <print>
#include <cstring>
#include <array>
#include <stdexcept>

auto spng::Chunk::_throw_bad_chunk() const -> void {
  std::string buff;
  buff.append("This PNG has a corrupted or invalid chunk,\n");
  buff.append("data cannot be read from it...\n");
  buff.append(fmt("At offset (header): {:8X}\n", offset_));
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

auto spng::Chunk::_default_print_impl() const -> void {
  // Chunk name: bold, magenta
  std::print("-- ");
  set_console(ConFg::Magenta);
  set_console(ConStyle::Bold);
  std::println("{}", type_string());
  reset_console();

  // Offset info: yellow
  set_console(ConFg::Yellow);
  std::print("{:<12} ", "Offsets");
  reset_console();
  std::println(
    ": Header: 0x{:<X}, Data: 0x{:<X}",
    offset_, offset_ + sizeof(Header)
  );

  // Chunk Length
  set_console(ConFg::Yellow);
  std::print("{:<12} ", "Length");
  reset_console();
  std::println(": {}", length());

  // Last one. CRC32 checksum
  set_console(ConFg::Yellow);
  std::print("{:<12} ", "CRC32");
  reset_console();
  std::println(": {:4X}", checksum());
}

auto spng::Chunk::print() const -> void {
  switch(type()) {
    case Type::IHDR: return as<Ihdr>().print();
    case Type::sRGB: return as<Srgb>().print();
    case Type::pHYs: return as<Phys>().print();
    case Type::gAMA: return as<Gama>().print();
    case Type::cHRM: return as<Chrm>().print();
    case Type::hIST: return as<Hist>().print();
    case Type::PLTE: return as<Plte>().print();
    case Type::tIME: return as<Time>().print();
    case Type::sPLT: return as<Splt>().print();
    default: break;
  }

  _default_print_impl();
  std::println("");
}

auto spng::Ihdr::print() const -> void {
  _default_print_impl();
  std::string _interlace;

  switch(interlace_method()) {
    case Interlace::None:
      _interlace = "None";
      break;
    case Interlace::Adam7:
      _interlace = "Adam7";
      break;
    case Interlace::Invalid:
      _interlace = "Unknown";
      break;
    default: break;
  }

  std::string _color_type;
  switch(color_type()) {
    case ColorType::GrayScale:
      _color_type = "Grayscale";
      break;
    case ColorType::GrayscaleAlpha:
      _color_type = "Grayscale + alpha";
      break;
    case ColorType::IndexedColor:
      _color_type = "Indexed color";
      break;
    case ColorType::TrueColor:
      _color_type = "RGB";
      break;
    case ColorType::TruecolorAlpha:
      _color_type = "RGB + alpha";
      break;
    default: break;
  }

  const auto _compression =
  compression_method() == Compression::Deflate
    ? "Deflate (0)"
    : "Unknown";

  const auto _filtering =
  filter_method() == FilterMethod::Default
    ? "Default (0)"
    : "Unknown";

  auto display_value = [&]<typename T>(
    const std::string& name, T&& val ) -> void
  {
    set_console(ConFg::Yellow);
    std::print("{:<12} ", name);
    reset_console();
    std::println(": {}", val);
  };

  display_value("Width", width());
  display_value("Height", height());
  display_value("Bit Depth", (int)bit_depth());
  display_value("Interlacing", _interlace);
  display_value("Color", _color_type);
  display_value("Compression", _compression);
  display_value("Filtering", _filtering);
  std::println("");
}

auto spng::Gama::print() const -> void {
  _default_print_impl();
  set_console(ConFg::Yellow);
  std::print("{:<12} ", "Gamma");
  reset_console();
  std::println(": {}\n", gamma());
}

auto spng::Plte::print() const -> void {
  _default_print_impl();
  set_console(ConFg::Yellow);
  std::print("{:<12} ", "Entries");
  reset_console();
  std::println(": {}\n", num_entries());
}

auto spng::Hist::print() const -> void {
  _default_print_impl();
  set_console(ConFg::Yellow);
  std::print("{:<12} ", "Entries");
  reset_console();
  std::println(": {}\n", num_entries());
}

auto spng::Splt::print() const -> void {
  _default_print_impl();
  auto display_value = [&]<typename T>(
    const std::string& name, T&& val ) -> void
  {
    set_console(ConFg::Yellow);
    std::print("{:<12} ", name);
    reset_console();
    std::println(": {}", val);
  };

  display_value("Name", name());
  display_value("Sample Depth", (uint16_t)sample_depth());
  display_value("Entries", num_entries());
  std::println("");
}

auto spng::Chrm::print() const -> void {
  _default_print_impl();
  const ConvertedLayout vals = values();

  auto display_value = [&]<typename T>(
    const std::string& name, T&& val ) -> void
  {
    set_console(ConFg::Yellow);
    std::print("{:<12} ", name);
    reset_console();
    std::println(": {}", val);
  };

  display_value(
    "WhitePoint",
    fmt("X={}, Y={}",
    vals.wp_x,
    vals.wp_y));
  display_value(
    "RedPrimary",
    fmt("X={}, Y={}",
    vals.red_x,
    vals.red_y));
  display_value(
    "GreenPrimary",
    fmt("X={}, Y={}",
    vals.green_x,
    vals.green_y));
  display_value(
    "BluePrimary",
    fmt("X={}, Y={}",
    vals.blue_x,
    vals.blue_y));
  std::println("");
}

auto spng::Time::print() const -> void {
  _default_print_impl();
  Layout vals = values();

  // Imagine if C++ had switch expressions :((
  const auto _month = [&]() -> std::string {
    switch(vals.month) {
      case 1: return "Jan";
      case 2: return "Feb";
      case 3: return "March";
      case 4: return "April";
      case 5: return "May";
      case 6: return "June";
      case 7: return "July";
      case 8: return "Aug";
      case 9: return "Sept";
      case 10: return "Oct";
      case 11: return "Nov";
      case 12: return "Dec";
      default: return "Invalid";
    }
  }();

  set_console(ConFg::Yellow);
  std::print("{:<12} ", "TimeStamp");
  reset_console();

  std::println(
    ": {} {} {} {}:{}:{} UTC\n",
    _month,
    static_cast<uint32_t>(vals.day),
    vals.year,
    static_cast<uint32_t>(vals.hour),
    static_cast<uint32_t>(vals.minute),
    static_cast<uint32_t>(vals.second)
  );
}

auto spng::Srgb::print() const -> void {
  _default_print_impl();
  std::string _intent;

  switch(intent()) {
    case RenderingIntent::Perceptual:
      _intent = "Perceptual";
      break;
    case RenderingIntent::RelativeColorimetric:
      _intent = "Relative, Colorimetric";
      break;
    case RenderingIntent::Saturation:
      _intent = "Saturation";
      break;
    case RenderingIntent::AbsoluteColorimetric:
      _intent = "Absolute, Colorimetric";
      break;
    case RenderingIntent::Invalid:
      _intent = "Invalid!";
      break;
    default: break;
  }

  set_console(ConFg::Yellow);
  std::print("{:<12} ", "Intent");
  reset_console();
  std::println(": {}\n", _intent);
}

auto spng::Phys::print() const -> void {
  _default_print_impl();

  const auto [ppu_x, ppu_y] = ppu();
  std::string _units;

  switch(units()) {
    case Units::Unspecified:
      _units = "Not Specified";
      break;
    case Units::Meters:
      _units = "pp/meter";
      break;
    case Units::Invalid:
      _units = "Invalid";
      break;
    default: break;
  }

  auto display_value = [&]<typename T>(
    const std::string& name, T&& val ) -> void
  {
    set_console(ConFg::Yellow);
    std::print("{:<12} ", name);
    reset_console();
    std::println(": {}", val);
  };

  display_value("Units", _units);
  display_value("Dimensions", fmt("{}x{}", ppu_x, ppu_y));
  std::println("");
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

    switch(layout->color_type) {
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

    if(layout->interlace == 0) {
      return Interlace::None;
    } if(layout->interlace == 1) {
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
    return layout->compression == 0
      ? Compression::Deflate
      : Compression::Invalid;
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
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
    _throw_bad_chunk();
  }

  UNREACHABLE;
}

auto spng::Srgb::intent() const -> RenderingIntent {
  const auto len = length();
  const auto ptr = buff_.lock();
  uint8_t the_intent = 4;

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Srgb::Layout)) {
    throw std::runtime_error("Invalid sRGB chunk size.");
  }

  try {
    const auto* intent_ptr = (uint8_t*)(&ptr->at(offset_ + sizeof(Header)));
    the_intent = *intent_ptr;
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  return the_intent > 3
    ? RenderingIntent::Invalid
    : static_cast<RenderingIntent>(the_intent);
}

auto spng::Phys::ppu() const -> std::array<uint32_t, 2> {
  std::array<uint32_t, 2> the_ppus{0, 0};

  const auto len = length();
  const auto ptr = buff_.lock();
  const size_t last_byte {
    + offset_
    + sizeof(Header)
    + (len - 1)
  };

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Phys::Layout) || last_byte >= ptr->size()) {
    throw std::runtime_error("Invalid pHYs chunk size.");
  }

  try {
    const auto* layout = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    the_ppus[0] = maybe_bitswap(layout->ppu_x, Endian::Big);
    the_ppus[1] = maybe_bitswap(layout->ppu_y, Endian::Big);
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  return the_ppus;
}

auto spng::Phys::units() const -> Units {
  auto the_units = Units::Invalid;
  const auto len = length();
  const auto ptr = buff_.lock();
  const size_t last_byte {
    + offset_
    + sizeof(Header)
    + (len - 1)
  };

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Phys::Layout) || last_byte >= ptr->size()) {
    throw std::runtime_error("Invalid pHYs chunk size.");
  }

  try {
    const auto* layout = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    const auto raw = layout->unit_t;

    // 0: Unspecified units
    // 1: Pixels Per Meter
    // Otherwise the value is invalid...

    if(raw == 0) {
      the_units = Units::Unspecified;
    } else if(raw == 1) {
      the_units = Units::Meters;
    }
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  return the_units;
}

auto spng::Gama::gamma() const -> double {
  const auto len = length();
  const auto ptr = buff_.lock();
  const size_t last_byte {
    + offset_
    + sizeof(Header)
    + (len - 1)
  };

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Gama::Layout) || last_byte >= ptr->size()) {
    throw std::runtime_error("Invalid gAMA chunk size.");
  }

  try {
    const auto* layout  = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    const auto raw_val  = maybe_bitswap(layout->gamma, Endian::Big);
    if(raw_val == 0) {
      throw std::runtime_error(
        "Corrupted gAMA chunk: Gamma value is 0.");
    }

    return static_cast<double>(raw_val) / 100000.00;
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  UNREACHABLE;
}

auto spng::Chrm::values() const -> ConvertedLayout {
  const auto len = length();
  const auto ptr = buff_.lock();
  const size_t last_byte {
    + offset_
    + sizeof(Header)
    + (len - 1)
  };

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Chrm::Layout) || last_byte >= ptr->size()) {
    throw std::runtime_error("Invalid cHRM chunk size.");
  }

  try {
    const auto* layout  = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    ConvertedLayout the_values {
      .wp_x    = (double)maybe_bitswap(layout->wp_x, Endian::Big),
      .wp_y    = (double)maybe_bitswap(layout->wp_y, Endian::Big),
      .red_x   = (double)maybe_bitswap(layout->red_x, Endian::Big),
      .red_y   = (double)maybe_bitswap(layout->red_y, Endian::Big),
      .green_x = (double)maybe_bitswap(layout->green_x, Endian::Big),
      .green_y = (double)maybe_bitswap(layout->green_y, Endian::Big),
      .blue_x  = (double)maybe_bitswap(layout->blue_x, Endian::Big),
      .blue_y  = (double)maybe_bitswap(layout->blue_y, Endian::Big)
    };

    if(the_values.wp_x    == 0.00 ||
      the_values.wp_y     == 0.00 ||
      the_values.red_x    == 0.00 ||
      the_values.red_y    == 0.00 ||
      the_values.green_x  == 0.00 ||
      the_values.green_y  == 0.00 ||
      the_values.blue_x   == 0.00 ||
      the_values.blue_y   == 0.00 ){
      throw std::runtime_error(
        "Corrupted Chrm chunk: one or more values are 0.");
    }

    the_values.wp_x     /= 100000.00;
    the_values.wp_y     /= 100000.00;
    the_values.red_x    /= 100000.00;
    the_values.red_y    /= 100000.00;
    the_values.green_x  /= 100000.00;
    the_values.green_y  /= 100000.00;
    the_values.blue_x   /= 100000.00;
    the_values.blue_y   /= 100000.00;
    return the_values;
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  UNREACHABLE;
}

auto spng::Plte::num_entries() const -> size_t {
  const auto len = length();
  if(len == 0 || len % 3 != 0) {
    throw std::runtime_error(fmt(
      "PLTE chunk has an invalid size ({})",
      len));
  }

  return len / 3;
}

auto spng::Hist::num_entries() const -> size_t {
  const auto len = length();
  if(len == 0 || len % 2 != 0) {
    throw std::runtime_error(fmt(
      "hIST chunk has an invalid size ({})",
      len));
  }

  return len / 2;
}

auto spng::Time::values() const -> Layout {
  const auto len = length();
  const auto ptr = buff_.lock();

  Layout the_layout = { 0 };
  const size_t last_byte {
    + offset_
    + sizeof(Header)
    + (len - 1)
  };

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(len != sizeof(Time::Layout) || last_byte >= ptr->size()) {
    throw std::runtime_error("Invalid tIME chunk size.");
  }

  try {
    const auto layout  = (Layout*)(&ptr->at(offset_ + sizeof(Header)));
    the_layout.year    = maybe_bitswap(layout->year, Endian::Big);
    the_layout.day     = layout->day;
    the_layout.hour    = layout->hour;
    the_layout.minute  = layout->minute;
    the_layout.month   = layout->month;
    the_layout.second  = layout->second;
  } catch(const std::out_of_range& _) {
    _throw_bad_chunk();
  }

  return the_layout;
}

auto spng::Splt::name() const -> std::string {
  const auto len = length();
  const auto ptr = buff_.lock();
  const size_t last_byte {
    + offset_
    + sizeof(Header)
    + (len - 1)
  };

  std::string the_name;
  the_name.reserve(79);

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(last_byte >= ptr->size()) {
    throw std::runtime_error("Invalid sPLT chunk size.");
  }

  try {
    size_t i = offset_ + sizeof(Header);
    while(ptr->at(i) != '\0') {
      if(the_name.size() > 79 || i >= len) {
        // laziness
        throw std::exception();
      }
      the_name += ptr->at(i);
      ++i;
    }

    if(the_name.empty()) {
      throw std::exception();
    }
  } catch(const std::exception& _) {
    _throw_bad_chunk();
  }

  return the_name;
}

auto spng::Splt::sample_depth() const -> uint8_t {
  const auto len = length();
  const auto nme = name();
  const auto ptr = buff_.lock();
  uint8_t the_sample_depth = 0;

  // The offset to the sample depth byte.
  // We add the offset to the start of the
  // chunk header, plus the header size,
  // plus the size of the chunk's name,
  // and account for the null terminator after that.
  const size_t the_offset {
    + offset_
    + sizeof(Header)
    + nme.size()
    + 1
  };

  // Get the offset to the last byte
  // of the chunk, as per usual.
  const size_t last_byte {
    + offset_
    + sizeof(Header)
    + (len - 1)
  };

  if(!ptr) {
    throw std::runtime_error("Invalid file buffer.");
  } if(the_offset >= ptr->size() || the_offset > last_byte) {
    throw std::runtime_error("Invalid sPLT chunk size.");
  }

  try {
    the_sample_depth = ptr->at(the_offset);
    // The only permitted values for the
    // sample depth byte are 8 and 16.
    // The chunk is corrupted if this isn't the case.
    if(the_sample_depth != 8 && the_sample_depth != 16) {
      throw std::exception();
    }
  } catch(const std::exception& _) {
    _throw_bad_chunk();
  }

  return the_sample_depth;
}

auto spng::Splt::num_entries() const -> size_t {
  const auto len = length();
  const auto nme = name();
  const auto sd  = sample_depth();

  // Calculate the remaining length of the chunk,
  // so that we can determine the number of entries.
  const size_t remaining_len = len - nme.size() - 2;

  // Check if an overflow occurred.
  if(len == 0 || remaining_len > len) {
    _throw_bad_chunk();
  }

  // Make sure the remaining length is valid.
  // If the sample depth is 8, it must be divisible by 6.
  // If the sample depth is 16, it must be divisible by 10.
  if(remaining_len == 0
    || (sd == 16 && remaining_len % 10 != 0)
    || (sd == 8 && remaining_len % 6 != 0)
  ) {
    _throw_bad_chunk();
  }

  return sample_depth() == 8
    ? remaining_len / 6
    : remaining_len / 10;
}

