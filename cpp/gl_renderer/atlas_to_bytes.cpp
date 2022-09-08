#include "atlas.h"
#include <fstream>
#include <vector>

// std::vector<char> readall(const char *path) {
//   std::ifstream ifs(path, std::ios::binary);

//   ifs.seekg(0, std::ios::end);
//   auto length = ifs.tellg();
//   ifs.seekg(0, std::ios::beg);

//   // read file
//   std::vector<char> buffer(length);
//   ifs.read(buffer.data(), buffer.size());
//   return buffer;
// }

int main(int argc, char **argv) {
  // auto data = readall(argv[1]);

  std::ofstream ofs(argv[1], std::ios::binary);
  ofs.write((const char *)atlas_texture, ATLAS_WIDTH * ATLAS_HEIGHT);

  return 0;
}
