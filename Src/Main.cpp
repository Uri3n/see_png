#include <print>
#include <iostream>
#include <Png.hpp>

#define TEST_FILE "C:\\Users\\diago\\Desktop\\example.png"
using namespace spng;

int main() try {
  auto ref = FileRef(TEST_FILE);
  auto carrier = Carrier(ref);
  auto meta = carrier.metadata();

  std::println("bit depth: {}", (int)meta.bit_depth());
  std::println("interlace: {}", (int)meta.interlace_method());
  std::println("color type: {}", (int)meta.color_type());
  std::println("filter: {}", (int)meta.filter_method());
  std::println("height: {}", (int)meta.height());
  std::println("width: {}", (int)meta.width());
  return 0;
} catch(const std::exception& e) {
  std::cerr << "EXCEPTION: " << e.what() << std::endl;
}
