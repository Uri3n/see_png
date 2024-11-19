#include <print>
#include <iostream>
#include <Carrier.hpp>

#define TEST_FILE "Tests\\timestamp.png"
using namespace spng;

int main() try {
  auto ref = FileRef(TEST_FILE);
  auto carrier = Carrier(ref);

  for(const auto& chunk : carrier.chunks()) {
    chunk.print();
  }

  carrier.print_summary();
  return 0;
} catch(const std::exception& e) {
  std::cerr << "EXCEPTION: " << e.what() << std::endl;
}
