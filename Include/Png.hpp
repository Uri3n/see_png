#ifndef PNG_HPP
#define PNG_HPP
#include <CompileAttrs.hpp>
#include <FileRef.hpp>
#include <FlatBuffer.hpp>
#include <cstdint>
#include <vector>
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
  class Carrier;
  class Chunk;
  class Ihdr;
  class Plte;
  class Trns;
  class Idat;
  class Iend;
  class Srgb;
  class Phys;
}

namespace spng {
  template<class T>
  concept IsChunk = std::derived_from<T, Chunk>;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class spng::Chunk {
protected:
  auto _throw_bad_chunk() const -> void;
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

  [[nodiscard]] auto next()        const -> std::optional<Chunk>;
  [[nodiscard]] auto type()        const -> Type;
  [[nodiscard]] auto type_string() const -> std::string;
  [[nodiscard]] auto length()      const -> uint32_t;
  [[nodiscard]] auto checksum()    const -> uint32_t;

  FlatBuffer::Weak buff_; // weak pointer to the file buff
  size_t offset_ = 0;     // offset to the start of the chunk header.

  ~Chunk() = default;
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

  [[nodiscard]] auto bit_depth()          const -> uint8_t;
  [[nodiscard]] auto width()              const -> uint32_t;
  [[nodiscard]] auto height()             const -> uint32_t;
  [[nodiscard]] auto compression_method() const -> Compression;
  [[nodiscard]] auto filter_method()      const -> FilterMethod;
  [[nodiscard]] auto interlace_method()   const -> Interlace;
  [[nodiscard]] auto color_type()         const -> ColorType;

  ~Ihdr() = default;
  explicit Ihdr(const FlatBuffer::Shared& buff)
    : Chunk(buff) {}
};

class spng::Carrier {
  Carrier& _verify_signature();
  Carrier&  _gather_chunks();
public:
  Carrier(const Carrier&)             = delete;
  Carrier& operator=(const Carrier&)  = delete;

  [[nodiscard]] auto metadata() const -> Ihdr;
  [[nodiscard]] auto chunks() -> const std::vector<Chunk>&;

  explicit Carrier(const FileRef& file);
  explicit Carrier(const FlatBuffer::Buffer& file);
private:
  std::vector<Chunk> chunks_;
  FlatBuffer::Shared buff_;
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

  auto intent() -> RenderingIntent;
  ~Srgb() = default;
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

  enum class PhysUnits : uint8_t {
    Unspecified = 0, // The chunk specifies pixel aspect ratio only.
    Meters = 1,      // The chunk specifies pixels per meter.
    Invalid = 2,     // The value is invalid / corrupted.
  };

  auto ppu() -> std::array<uint32_t, 2>;
  auto units() -> PhysUnits;

  ~Phys() = default;
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

  Ihdr new_chunk(ptr);
  new_chunk.offset_ = this->offset_;
  return new_chunk;
}

inline auto spng::Carrier::metadata() const -> Ihdr {
  return chunks_.at(0).as<Ihdr>();
}

inline auto spng::Carrier::chunks() -> const std::vector<Chunk>& {
  return chunks_;
}

#endif //PNG_HPP
