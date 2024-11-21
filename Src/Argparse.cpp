#include <Argparse.hpp>
#include <ConManip.hpp>
#include <Context.hpp>
#include <Panic.hpp>
#include <print>
#include <string>
#include <vector>
#include <ranges>
#include <filesystem>

// ~ Flag List ~
// -v --verbose
// -ec --extract-chunks chunk1,chunk2,chunk3
// -dc --dump-chunks chunk1,chunk2,chunk3
// Last argument is input files
// More can be added later.

static constexpr spng::FlagDescriptor flag_list[] {{
  .lf   = "--verbose",
  .sf   = "-v",
  .desc = "Display verbose info about each chunk.",
},{
  .lf   = "--extract-chunks",
  .sf   = "-ec",
  .desc = "Comma-delimited chunk names "
          "to be saved in the form <FILENAME>.<CHUNK>.bin",
},{
  .lf   = "--dump-chunks",
  .sf   = "-dc",
  .desc = "Comma delimited chunk names "
          "to be hex-dumped to the console.",
},{
  .lf   = "--silent",
  .sf   = "-s",
  .desc = "Don't display any non-error output.",
},{
  .lf   = "--no-summary",
  .sf   = "-ns",
  .desc = "Don't display any chunk summaries.",
}};

auto spng::print_help() -> void {
  set_console(ConFg::White);
  set_console(ConStyle::Bold);
  std::println("-- Flags:");
  reset_console();

  for(const auto &[lf, sf, desc] : flag_list) {
    // Flag name
    set_console(ConFg::Magenta);
    std::print("{:<17} {:<3}", lf, sf);
    reset_console();
    // Description
    std::println(" :: {} ", desc);
  }

  set_console(ConFg::White);
  set_console(ConStyle::Bold);
  std::println("\n-- Examples:");
  reset_console();

  std::println("see_png -v file1.png,file2.png");
  std::println("see_png --verbose --dump_chunks IHDR,IEND,IDAT myfile.png");
  std::println("see_png --extract-chunks tEXt --silent myfile.png\n");
}

auto spng::init_context_from_args(const int argc, char** argv) -> bool {
  ASSERT(argc > 1);
  ASSERT(argv != nullptr);

  std::vector<std::string> strings;
  size_t ind = 0;

  // Copy into a vector, so that we can
  // get useful bounds checking.
  for(int i = 1; i < argc; i++) {
    strings.emplace_back(argv[i]);
  }

  // The argument is invalid.
  auto einvalid_arg = [&]() -> void {
    set_console(ConFg::Red);
    std::print("INVALID argument ");
    reset_console();
    std::println(":: \"{}\"", strings.at(ind));
  };

  // Argument has already been passed.
  // Do not allow arguments to be passed
  // more than once because it makes me very mad >:((
  auto ealready_passed = [&]() -> void {
    set_console(ConFg::Red);
    std::print("Argument already passed");
    reset_console();
    std::println(":: \"{}\"", strings.at(ind));
  };

  // parse_current():
  // - Parses the string at the current index, ind.
  // - Returns false on failure.
  auto parse_current = [&]() -> bool {
    if(strings.at(ind) == "--verbose" || strings.at(ind) == "-v") {
      if(Context::get().flags_ & Context::Verbose) {
        ealready_passed();
        return false;
      }
      Context::get().flags_ |= Context::Verbose;
      return true;
    }

    if(strings.at(ind) == "--silent" || strings.at(ind) == "-s") {
      if(Context::get().flags_ & Context::Silent) {
        ealready_passed();
        return false;
      }
      Context::get().flags_ |= Context::Silent;
      return true;
    }

    if(strings.at(ind) == "--no-summary" || strings.at(ind) == "-ns") {
      if(Context::get().flags_ & Context::NoSumm) {
        ealready_passed();
        return false;
      }
      Context::get().flags_ |= Context::NoSumm;
      return true;
    }

    if(strings.at(ind) == "--extract-chunks" || strings.at(ind) == "-ec") {
      if(!Context::get().extract_chunks_.empty()) {
        ealready_passed();
        return false;
      }
      const auto chunk_names = strings.at(ind + 1);
      for(const auto& name : std::ranges::views::split(chunk_names, ',')) {
        Context::get().extract_chunks_.emplace_back(name.begin(), name.end());
      }
      ++ind;
      return true;
    }

    if(strings.at(ind) == "--dump-chunks" || strings.at(ind) == "-dc") {
      if(!Context::get().dump_chunks_.empty()) {
        ealready_passed();
        return false;
      }
      const auto chunk_names = strings.at(ind + 1);
      for(const auto& name : std::ranges::views::split(chunk_names, ',')) {
        Context::get().dump_chunks_.emplace_back(name.begin(), name.end());
      }
      ++ind;
      return true;
    }

    // Last argument should be a list of the PNG
    // files to run on...
    if(ind == strings.size() - 1) {
      for(const auto& name : std::ranges::views::split(strings.at(ind), ',')) {
        Context::get().ifilenames_.emplace_back(name.begin(), name.end());
      }
      return true;
    }

    einvalid_arg();
    return false;
  };

  try {
    for( ; ind < strings.size(); ++ind ) {
      if(!parse_current()) return false;
    }
  } catch(const std::out_of_range& _) {
    // We expected a value AFTER the current
    // string in the input stream. It was OOB.
    set_console(ConFg::Red);
    std::println("!! INVALID ARGUMENT");
    reset_console();
    std::println("Expected another value after "
      "\"{}\" at position {}, but "
      "there wasn't anything there.",
      strings.back(),
      ind + 1);
    return false;
  }

  // Make sure we have input file(s) to use...
  if(Context::get().ifilenames_.empty()) {
    set_console(ConFg::Red);
    std::println("Invalid command line arguments: "
      "expected one or more comma delimited input "
      "files as the last argument.");
    reset_console();
    return false;
  }

  return true;
}


