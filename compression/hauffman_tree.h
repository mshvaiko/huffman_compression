#ifndef COMPRESSION_HAUFFMAN_TREE_H
#define COMPRESSION_HAUFFMAN_TREE_H

#include <memory>
#include <string>

#include "compressor_base.h"

namespace compression {

class HuffmanTree : public compressor_base {
 private:
  class HuffmanTreeImpl;
  std::unique_ptr<HuffmanTreeImpl> impl_;

 public:
  HuffmanTree(const std::string& input_file_path, const std::string& output_file_path);
  ~HuffmanTree() override;

  bool encode() override;
  bool decode() override;
};

}  // namespace compression

#endif  // COMPRESSION_HAUFFMAN_TREE_H