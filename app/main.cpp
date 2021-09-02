#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <thread>
#include <unordered_map>

#include "hauffman_tree.h"

constexpr auto kIntArraySize = 1000000;

void int_array_generator(const std::string& filename) {
  std::ofstream file(filename);
  int counter = 1;
  for (int i = 0; i < kIntArraySize; ++i) {
    file << counter << ", ";
    if (counter > 100) {
      counter = 1;
    }
    counter++;
  }
  file.close();
}

int main(int argc, char** argv) {
  if (argc == 4) {
    compression::HuffmanTree huffmanTree(argv[2], argv[3]);
    if (std::string(argv[1]) == "-encode") {
      std::cout << "encoding...\n";
      huffmanTree.encode();
      std::cout << "encoding finnished\n";
      std::cout << "encoded to file: " << argv[3] << std::endl;
    } else if (std::string(argv[1]) == "-decode") {
      std::cout << "decoding..\n";
      huffmanTree.decode();
      std::cout << "decoding finnished\n";
      std::cout << "decoded to file: " << argv[3] << std::endl;
    }
  } else if (argc == 3) {
    if (std::string(argv[1]) == "-int_gen") {
      std::cout << "int generating...\n";
      int_array_generator(argv[2]);
      std::cout << "generating finnished!\n";
    }
  } else {
    std::cout << "Usage:\n";
    std::cout << argv[0] << " -int_gen <filename>";
    std::cout << argv[0] << " [-encode|-decode] <input_file> <output_file>";
  }

  return 0;
}
