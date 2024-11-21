///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This file contains the necessary definitions
// for all standard chunks according to the PNG specification,
// both critical (required to be in every PNG file),
// and ancillary (optional). For more information on the layouts
// of PNG chunks and their uses, visit http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html .
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef CHUNKS_HPP
#define CHUNKS_HPP
#include <CompileAttrs.hpp>
#include <FileRef.hpp>
#include <FlatBuffer.hpp>
#include <cstdint>
#include <string_view>
#include <concepts>
#include <memory>
#include <optional>
#include <array>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define SEE_PNG_CHUNK_LIST             \
  X(IHDR) /* IHDR chunk, metadata */   \
  X(IDAT) /* IDAT chunk, raw data */   \
  X(IEND) /* IEND chunk, EOF */        \
  X(bKGD) /* Background color chunk */ \
  X(cHRM) /* Chromacity chunk */       \
  X(gAMA) /* Gamma chunk */            \
  X(hIST) /* Histogram chunk */        \
  X(iCCP) /* ICC profile chunk */      \
  X(IPCT) /* ICC - Deprecated */       \
  X(sBIT) /* Significant Bits */       \
  X(sRGB) /* sRGB rendering intent */  \
  X(tEXt) /* Uncompressed text */      \
  X(zTXt) /* Compressed text */        \
  X(iTXt) /* UTF-8 encoded text */     \
  X(tIME) /* Time (last modified) */   \
  X(pHYs) /* Pixel dimensions */       \
  X(sPLT) /* Suggested Palette */      \
  X(tRNS) /* Transparency Info */      \
  X(PLTE) /* Palette Chunk*/           \
  X(acTL) /* APNG - Anim Ctrl*/        \
  X(fRAc) /* APNG - Frame Ctrl */      \
  X(fdAT) /* APNG - Frame data */      \

namespace spng {
  class Chunk;
  class Ihdr;
  class Plte;
  class Srgb;
  class Phys;
  class Chrm;
  class Gama;
  class Plte;
  class Hist;
  class Time;
  class Splt;
}

namespace spng {
  template<class T>
  concept IsChunk = std::derived_from<T, Chunk>;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class spng::Chunk {
protected:
  auto _throw_bad_chunk() const -> void;
  auto _default_print_impl() const -> void;
public:
  PACKED_STRUCT(Header, {
    uint32_t length;  // Total size of the chunk.
    int8_t type[4];   // 4 Byte value that specifies the chunk type.
  });

  enum class Type : uint8_t {
    #define X(CHUNK_TYPE) CHUNK_TYPE,
    SEE_PNG_CHUNK_LIST
    Unknown,
    #undef X
  };

  // For conversion to other chunk types.
  template<class T> requires IsChunk<T>
  auto as() const -> T;
  virtual auto print() const -> void;

  [[nodiscard]] auto next()        const -> std::optional<Chunk>;
  [[nodiscard]] auto type()        const -> Type;
  [[nodiscard]] auto type_string() const -> std::string;
  [[nodiscard]] auto length()      const -> uint32_t;
  [[nodiscard]] auto checksum()    const -> uint32_t;

  FlatBuffer::Weak buff_; // weak pointer to the file buff
  size_t offset_ = 0;     // offset to the start of the chunk header.

  virtual ~Chunk() = default;
  Chunk() = default;
  explicit Chunk(const FlatBuffer::Shared& buff)
    : buff_(buff) {}
};

class spng::Ihdr final : public Chunk {
public:
  PACKED_STRUCT(Layout, {
    uint32_t width;      // Image width in pixels
    uint32_t height;     // Image height in pixels
    uint8_t bit_depth;   // Bit depth (e.g., 1, 2, 4, 8, 16)
    uint8_t color_type;  // Color type (e.g., 0: Grayscale, 2: Truecolor)
    uint8_t compression; // Compression method (always 0)
    uint8_t filter;      // Filter method (always 0)
    uint8_t interlace;   // Interlace method (0: None, 1: Adam7)
  });

  enum class ColorType : uint8_t {
    GrayScale = 0,       // Grayscale (no color)
    TrueColor = 2,       // RGB
    IndexedColor = 3,    // PLTE chunk is used for colour values.
    GrayscaleAlpha = 4,  // Grayscale + opacity value (alpha)
    TruecolorAlpha = 6,  // RGB + opacity value (alpha)
  };

  enum class Interlace : uint8_t {
    None = 0,            // no interlacing used.
    Adam7 = 1,           // adam7 interlacing is used.
    Invalid = 2,         // The value in the IHDR chunk is invalid.
  };

  enum class Compression : uint8_t {
    Deflate = 0,         // "zlib-style" deflate compression is used.
    Invalid = 1,         // the compression value in the IHDR chunk is invalid.
  };

  enum class FilterMethod : uint8_t {
    Default = 0,         // The filter method is stored at the beginning of each scanline.
    Invalid = 1,         // The filter value in the IHDR chunk is invalid.
  };

  auto print()                            const -> void override;
  [[nodiscard]] auto bit_depth()          const -> uint8_t;
  [[nodiscard]] auto width()              const -> uint32_t;
  [[nodiscard]] auto height()             const -> uint32_t;
  [[nodiscard]] auto compression_method() const -> Compression;
  [[nodiscard]] auto filter_method()      const -> FilterMethod;
  [[nodiscard]] auto interlace_method()   const -> Interlace;
  [[nodiscard]] auto color_type()         const -> ColorType;

  ~Ihdr() override = default;
  explicit Ihdr(const FlatBuffer::Shared& buff)
    : Chunk(buff) {}
};

class spng::Plte final : public Chunk {
public:
  PACKED_STRUCT(Entry, {
    uint8_t red;   // 0 = black, 255 = red.
    uint8_t green; // 0 = black, 255 = green.
    uint8_t blue;  // 0 = black, 255 = blue.
  });

  [[nodiscard]] auto num_entries() const -> size_t;
  auto print() const -> void override;

  ~Plte() override = default;
  explicit Plte(const FlatBuffer::Shared& buff)
    : Chunk(buff) {}
};

// This name is semi-confusing maybe,
// but it refers to the PNG "tIME" chunk, not
// some sort of clock or system time.
class spng::Time final : public Chunk {
public:
  PACKED_STRUCT(Layout, {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
  });

  struct ConvertedLayout {
    std::string_view year;
    std::string_view month;
    uint8_t day     = 0;
    uint8_t hour    = 0;
    uint8_t minute  = 0;
    uint8_t second  = 0;
  };

  auto print() const -> void override;
  [[nodiscard]] auto values() const -> Layout;

  ~Time() override = default;
  explicit Time(const FlatBuffer::Shared& buff)
    : Chunk(buff) {}
};

class spng::Splt final : public Chunk {
public:
  auto print() const -> void override;
  [[nodiscard]] auto sample_depth()  const -> uint8_t;
  [[nodiscard]] auto name()          const -> std::string;
  [[nodiscard]] auto num_entries()   const -> size_t;

  ~Splt() override = default;
  explicit Splt(const FlatBuffer::Shared& buff)
    : Chunk(buff) {}
};

class spng::Hist final : public Chunk {
public:
  [[nodiscard]] auto num_entries() const -> size_t;
  auto print() const -> void override;

  ~Hist() override = default;
  explicit Hist(const FlatBuffer::Shared& buff)
    : Chunk(buff) {}
};

class spng::Chrm final : public Chunk {
public:
  PACKED_STRUCT(Layout, {
    uint32_t wp_x;     // X-coord: white point (CIE 1931), times 100,000.
    uint32_t wp_y;     // Y-coord: white point (CIE 1931), times 100,000.
    uint32_t red_x;    // X-coord: red primary (CIE 1931), times 100,000.
    uint32_t red_y;    // Y-coord: red primary (CIE 1931), times 100,000.
    uint32_t green_x;  // X-coord: green primary (CIE 1931), times 100,000.
    uint32_t green_y;  // Y-coord: green primary (CIE 1931), times 100,000.
    uint32_t blue_x;   // X-coord: blue primary (CIE 1931), times 100,000.
    uint32_t blue_y;   // Y-coord: blue primary (CIE 1931), times 100,000.
  });

  struct ConvertedLayout {
    double wp_x     = 0.00;
    double wp_y     = 0.00;
    double red_x    = 0.00;
    double red_y    = 0.00;
    double green_x  = 0.00;
    double green_y  = 0.00;
    double blue_x   = 0.00;
    double blue_y   = 0.00;
  };

  auto print() const -> void override;
  [[nodiscard]] auto values() const -> ConvertedLayout;

  ~Chrm() override = default;
  explicit Chrm(const FlatBuffer::Shared& buff)
    : Chunk(buff) {}
};

class spng::Gama final : public Chunk {
public:
  PACKED_STRUCT(Layout, {
    uint32_t gamma; // Gamma, times 100,000.
  });

  auto print() const -> void override;
  [[nodiscard]] auto gamma() const -> double;

  ~Gama() override = default;
  explicit Gama(const FlatBuffer::Shared& buff)
    : Chunk(buff) {}
};

class spng::Srgb final : public Chunk {
public:
  PACKED_STRUCT(Layout, {
    uint8_t bintent;
  });

  enum class RenderingIntent : uint8_t {
    Perceptual = 0,
    RelativeColorimetric = 1,
    Saturation = 2,
    AbsoluteColorimetric = 3,
    Invalid = 4,
  };

  auto print() const -> void override;
  [[nodiscard]] auto intent() const -> RenderingIntent;

  ~Srgb() override = default;
  explicit Srgb(const FlatBuffer::Shared& buff)
    : Chunk(buff) {}
};

class spng::Phys final : public Chunk {
public:
  PACKED_STRUCT(Layout, {
    uint32_t ppu_x;  // Pixels per unit (X-axis)
    uint32_t ppu_y;  // Pixels per unit (Y-axis)
    uint8_t unit_t;  // Unit kind: 0 (unknown), 1 (meters)
  });

  enum class Units : uint8_t {
    Unspecified = 0, // The chunk specifies pixel aspect ratio only.
    Meters = 1,      // The chunk specifies pixels per meter.
    Invalid = 2,     // The value is invalid / corrupted.
  };

  auto print() const -> void override;
  [[nodiscard]] auto ppu() const -> std::array<uint32_t, 2>;
  [[nodiscard]] auto units() const -> Units;

  ~Phys() override = default;
  explicit Phys(const FlatBuffer::Shared& buff)
    : Chunk(buff) {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T> requires spng::IsChunk<T>
auto spng::Chunk::as() const -> T {
  const auto ptr = buff_.lock();
  if(ptr == nullptr) {
    throw std::runtime_error("Invalid file buffer.");
  }

  T new_chunk(ptr);
  new_chunk.offset_ = this->offset_;
  return new_chunk;
}

#endif //CHUNKS_HPP
