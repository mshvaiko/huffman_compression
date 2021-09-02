#include <gtest/gtest.h>

#include "hauffman_tree.h"

#include <fstream>

namespace {

constexpr auto kIntArraySize = 1000000;

}  // namespace

class HuffmanTreeTest : public ::testing::Test {
 protected:
  void SetUp() override {
    std::ofstream file(input_int_array_file_);
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
  std::string input_int_array_file_ = "int_array.txt";
  std::string encoded_file_{"encoded_int_array.copmressed"};
  std::string decoded_file_{"decoded_int_array.txt"};
};

TEST_F(HuffmanTreeTest, HuffmanTree_encode_to_file_with_positive_result) {
  compression::HuffmanTree instance(input_int_array_file_, encoded_file_);
  bool result = instance.encode();
  EXPECT_TRUE(result);
}

TEST_F(HuffmanTreeTest, HuffmanTree_decode_to_file_with_positive_result) {
  compression::HuffmanTree instance(encoded_file_, decoded_file_);
  bool result = instance.decode();
  EXPECT_TRUE(result);
}

TEST_F(HuffmanTreeTest, HuffmanTree_Compare_input_and_decoded_file) {
  std::fstream input_file(input_int_array_file_);
  std::fstream output_file(decoded_file_);

  char input_file_sign;
  char output_file_sign;

  while (!input_file.eof() || !output_file.eof()) {
    input_file.get(input_file_sign);
    output_file.get(output_file_sign);
    EXPECT_EQ(input_file_sign, output_file_sign);
  }
}

TEST_F(HuffmanTreeTest, HuffmanTree_check_compressed_file_less_than_origin) {
  std::ifstream origin_flie(input_int_array_file_, std::ifstream::ate | std::ifstream::binary);
  std::ifstream compressed_flie(encoded_file_, std::ifstream::ate | std::ifstream::binary);

  EXPECT_GT(origin_flie.tellg(), compressed_flie.tellg());
}
