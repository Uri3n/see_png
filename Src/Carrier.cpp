#include <Carrier.hpp>
#include <Panic.hpp>

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
  constexpr std::array<uint8_t, 8> png_magic = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A
  };

  if(png_magic.size() >= buff_->size()) {
    throw std::runtime_error("File is too small.");
  }

  for(size_t i = 0; i < png_magic.size(); i++) {
    if(buff_->at(i) != png_magic.at(i)) {
      throw std::runtime_error("Invalid PNG signature.");
    }
  }

  return *this;
}

auto spng::Carrier::print_summary() const -> void {
  ASSERT(buff_ != nullptr);
  ASSERT(!chunks_.empty());

  // Title
  std::print("-- ");
  set_console(ConFg::Magenta);
  set_console(ConStyle::Bold);
  std::println("Chunk Summary:");
  reset_console();

  // Category headers
  set_console(ConFg::White);
  set_console(ConStyle::Bold);
  std::println("{:<5} {:<8} {:<7}", "Type", "Offset", "Length");
  reset_console();
  std::println("{:=<5} {:=<8} {:=<7}", "=", "=", "=");

  // Print each chunk.
  for(const auto& chunk : chunks_) {
    set_console(ConFg::Magenta);
    std::print("{:<5} ", chunk.type_string());
    reset_console();

    set_console(ConFg::Green);
    std::println("0x{:<6X} {:<7}", chunk.offset_, chunk.length());
    reset_console();
  }

  std::println("\nTotal Chunks : {}", chunks_.size());
  std::println("Size (Bytes) : {}", buff_->size());

  set_console(ConFg::Green);
  set_console(ConStyle::Bold);
  std::println("Summary complete.");
  reset_console();
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
