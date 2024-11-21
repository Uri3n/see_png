#include <Context.hpp>
#include <print>

SPNG_NOINLINE
auto spng::Context::get() -> Context& {
  static Context ctx;
  return ctx;
}

auto spng::Context::debug_print() const -> void {
  std::println("-- CONTEXT");
  std::print("files   :: ");
  for(const auto& iname : ifilenames_) {
    std::print("{}, ", iname);
  }

  std::print("\nextract :: ");
  for(const auto& chunk_name : extract_chunks_) {
    std::print("{}, ", chunk_name);
  }

  std::print("\ndump    :: ");
  for(const auto& chunk_name : dump_chunks_) {
    std::print("{}, ", chunk_name);
  }

  std::print("flags   :: ");
  std::string _flags;
  if(flags_ & NoSumm)  _flags += "NoSummary | ";
  if(flags_ & Verbose) _flags += "Verbose | ";
  if(flags_ & Silent)  _flags += "Silent | ";

  if(!_flags.empty()) {
    _flags.erase(_flags.size() - 3);
  }
  std::println("{}", _flags);
}

