#include <FileCycle.hpp>
#include <InFileRef.hpp>
#include <HexDump.hpp>
#include <Carrier.hpp>
#include <Context.hpp>
#include <ConManip.hpp>
#include <Fmt.hpp>
#include <print>
#include <filesystem>
#include <ios>
#include <stdexcept>
#include <algorithm>


auto spng::do_file_cycle(const std::string& file) -> bool {
  try {
    // Load file into memory
    const InFileRef ref(file);
    const Carrier carrier(ref);

    // Get context flags,
    // chunks to extract, chunks to dump,
    // base file name for extracted chunks.
    const uint32_t flags     = Context::get().flags_;
    const auto& extr_chunks  = Context::get().extract_chunks_;
    const auto& dump_chunks  = Context::get().dump_chunks_;
    const auto file_name     = std::filesystem::path(file).filename();

    // Display file name
    if(!(flags & Context::Silent)) {
      set_console(ConFg::White);
      set_console(ConStyle::Underline);
      set_console(ConStyle::Bold);
      std::println("{}:", file);
      reset_console();
    }

    for(const auto& chunk : carrier.chunks()) {
      const auto ch_type = chunk.type_string();
      if(!(flags & Context::Silent) && flags & Context::Verbose) {
        chunk.print();
      } if(std::ranges::find(dump_chunks, ch_type) != dump_chunks.end()) {
        chunk.hexdump();
      } if(std::ranges::find(extr_chunks, ch_type) != extr_chunks.end()) {
        chunk.extract_to(fmt("{}.{}.bin", file_name.string(), ch_type));
      }
    }

    if(!(flags & Context::Silent) && !(flags & Context::NoSumm)) {
      carrier.print_summary();
    }
  } catch(const std::ios_base::failure& e) {
    set_console(ConFg::Red);
    set_console(ConStyle::Bold);
    std::print("FILE I/O :: ");
    reset_console();
    std::println("For {} :: {}", file, e.what());
    return false;
  }
  catch(const std::runtime_error& e) {
    set_console(ConFg::Red);
    set_console(ConStyle::Bold);
    std::print("FILE CORRUPTION :: ");
    reset_console();
    std::println("For {} :: {}", file, e.what());
    return false;
  }
  catch(const std::exception& e) {
    set_console(ConFg::Red);
    set_console(ConStyle::Bold);
    std::print("INTERNAL ERROR :: ");
    reset_console();
    std::println("For {} :: {}", file, e.what());
    return false;
  }
  catch(...) {
    return false;
  }

  return true;
}