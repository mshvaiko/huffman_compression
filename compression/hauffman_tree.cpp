#include "hauffman_tree.h"

#include <algorithm>
#include <array>
#include <execution>
#include <fstream>
#include <future>
#include <iostream>
#include <queue>
#include <vector>
#include <bitset>

#include "thread_pool.h"

namespace {

constexpr auto kAsciiCharactersCount = 128;
constexpr auto kZeroSign = '0';
constexpr auto kOneSign = '1';
constexpr auto kBitsInByte = 8;

}  // namespace

namespace compression {

struct huffman_node {
  char id{};           // character
  int freq{};          // frequency of the character
  std::string code{};  // huffman code for the character
  huffman_node* left{nullptr};
  huffman_node* right{nullptr};
  huffman_node(char id, int freq) : id(id), freq(freq) {}
  huffman_node() {}

  friend bool operator>(const huffman_node& c1, const huffman_node& c2);
};

bool operator>(const huffman_node& c1, const huffman_node& c2) { return c1.freq > c2.freq; }

using node_ptr = huffman_node*;

class HuffmanTree::HuffmanTreeImpl {
  std::array<node_ptr, kAsciiCharactersCount> node_array_;
  std::fstream in_file_{};
  std::fstream out_file_{};
  node_ptr root_{nullptr};
  std::string in_file_name_;
  std::string out_file_name_;
  char id_{};
  std::priority_queue<node_ptr, std::vector<node_ptr>> frequency_pq_;  // priority queue of frequency from high to low
  thread_pool pool_;

  void createNodeArray();
  void traverse(node_ptr, const std::string&);  // traverse the huffman tree and get
                                                // huffman code for a character
  int binaryToDecimal(const std::string&);          // convert a 8-bit 0/1 string of binary
                                                // code to a decimal integer
  std::string decimalToBinary(int);           // convert a decimal integer to a 8-bit 0/1 string of binary code
  inline void buildTree(std::string&,
                         char);  // build the huffman tree according to information from file

 public:
  HuffmanTreeImpl(const std::string&, const std::string&);
  ~HuffmanTreeImpl();
  bool createFrequencyPriorityQueue();
  void create_huffman_tree();
  void calculateHuffmanCodes();
  bool encodeSave();
  bool decodeSave();
  void recreateHuffmanTree();
};

void HuffmanTree::HuffmanTreeImpl::createNodeArray() {
  std::atomic<int> i = 0;
  std::for_each(std::execution::par, node_array_.begin(), node_array_.end(), [&](auto&& item) {
    item = new huffman_node(std::atomic_exchange_explicit(&i, i + 1, std::memory_order_acquire), 0);
  });
}

void HuffmanTree::HuffmanTreeImpl::traverse(node_ptr node, const std::string& code) {
  if (!node->left && !node->right) {
    node->code = code;
  } else {
    traverse(node->left, code + kZeroSign);
    traverse(node->right, code + kOneSign);
  }
}

int HuffmanTree::HuffmanTreeImpl::binaryToDecimal(const std::string& in) {
  return stoi(in, 0, 2);
}

std::string HuffmanTree::HuffmanTreeImpl::decimalToBinary(int in) {
  std::string temp = "";
  std::string result = "";
  while (in) {
    temp += (kZeroSign + in % 2);
    in /= 2;
  }
  result.append(kBitsInByte - temp.size(), kZeroSign);  // append '0' ahead to let the result become fixed length of 8
  for (int i = temp.size() - 1; i >= 0; i--) {
    result += temp[i];
  }
  return result;
}

inline void HuffmanTree::HuffmanTreeImpl::buildTree(std::string& path,
                                                     char a_code) {  // build a new branch according to the inpue
                                                                     // code and ignore the already existed branches
  node_ptr current = root_;
  for (size_t i = 0; i < path.size(); i++) {
    if (path[i] == kZeroSign) {
      if (!current->left) current->left = new huffman_node;
      current = current->left;
    } else if (path[i] == kOneSign) {
      if (!current->right) current->right = new huffman_node;
      current = current->right;
    }
  }
  current->id = a_code;  // attach id to the leaf
}

HuffmanTree::HuffmanTreeImpl::HuffmanTreeImpl(const std::string& in, const std::string& out)
    : in_file_name_(in), out_file_name_(out), pool_(std::thread::hardware_concurrency()) {
  pool_.run();
  createNodeArray();
}

HuffmanTree::HuffmanTreeImpl::~HuffmanTreeImpl() {}

bool HuffmanTree::HuffmanTreeImpl::createFrequencyPriorityQueue() {
  in_file_.open(in_file_name_, std::ios::in);

  if (!in_file_.is_open()) {
    std::cout << "Failed to open input file...";
    return false;
  }

  while (!in_file_.eof()) {
    in_file_.get(id_);
    node_array_[id_]->freq++;
  }
  in_file_.close();

  auto parallel_exec = [&] {
    // first part
    pool_.async_job([&] {
      for (int i = 0; i < kAsciiCharactersCount / 2; i++) {
        if (node_array_[i]->freq) {
          frequency_pq_.push(node_array_[i]);
        }
      }
    });

    // second part
    pool_.async_job([&] {
      for (int i = kAsciiCharactersCount / 2; i < kAsciiCharactersCount; i++) {
        if (node_array_[i]->freq) {
          frequency_pq_.push(node_array_[i]);
        }
      }
    });
  };
  parallel_exec();
  return true;
}

void HuffmanTree::HuffmanTreeImpl::create_huffman_tree() {
  std::priority_queue<node_ptr, std::vector<node_ptr>> temp(frequency_pq_);
  while (temp.size() > 1) {  // create the huffman tree with highest frequecy
                             // characher being leaf from bottom to top
    root_ = new huffman_node;
    root_->freq = 0;
    root_->left = temp.top();
    root_->freq += temp.top()->freq;
    temp.pop();
    root_->right = temp.top();
    root_->freq += temp.top()->freq;
    temp.pop();
    temp.push(root_);
  }
}

void HuffmanTree::HuffmanTreeImpl::calculateHuffmanCodes() {
  pool_.dispatch([this] { traverse(this->root_, ""); });
}

bool HuffmanTree::HuffmanTreeImpl::encodeSave() {
  in_file_.open(in_file_name_, std::ios::in);
  out_file_.open(out_file_name_, std::ios::out | std::ios::binary);

  if (!in_file_.is_open() || !out_file_.is_open()) {
    std::cout << "Failed to open input or output file...";
    return false;
  }

  std::string in = "";
  std::string s = "";

  in += static_cast<char>(frequency_pq_.size());  // the first byte saves the size of the priority queue
  std::priority_queue<node_ptr, std::vector<node_ptr>> temp(frequency_pq_);
  while (!temp.empty()) {  // get all characters and their huffman codes for output
    node_ptr current = temp.top();
    in += current->id;
    s.assign(127 - current->code.size(),
             kZeroSign);  // set the codes with a fixed 128-bit string form[000¡­¡­1 +
                    // real code]
    s += kOneSign;       //'1' indicates the start of huffman code
    s.append(current->code);
    auto in_substr = s.substr(0, kBitsInByte);
    in += static_cast<char>(binaryToDecimal(in_substr));
    for (int i = 0; i < 15; i++) {  // cut into 8-bit binary codes that can convert into saving
                                    // char needed for binary file
      s = s.substr(kBitsInByte);
      auto in_sub_str = s.substr(0, kBitsInByte);
      in += static_cast<char>(binaryToDecimal(in_sub_str));
    }
    temp.pop();
  }
  s.clear();

  in_file_.get(id_);
  while (!in_file_.eof()) {  // get the huffman code
    s += node_array_[id_]->code;
    while (s.size() > kBitsInByte) {  // cut into 8-bit binary codes that can convert into
                            // saving char needed for binary file
      auto in_substr = s.substr(0, kBitsInByte);
      in += static_cast<char>(binaryToDecimal(in_substr));
      s = s.substr(kBitsInByte);
    }
    in_file_.get(id_);
  }
  int count = kBitsInByte - s.size();
  if (s.size() < kBitsInByte) {  // append number of 'count' '0' to the last few codes to
                       // create the last byte of text
    s.append(count, kZeroSign);
  }
  in += static_cast<char>(binaryToDecimal(s));  // save number of 'count' at last
  in += static_cast<char>(count);

  out_file_.write(in.c_str(), in.size());
  in_file_.close();
  out_file_.close();

  return true;
}

void HuffmanTree::HuffmanTreeImpl::recreateHuffmanTree() {
  in_file_.open(in_file_name_, std::ios::in | std::ios::binary);
  unsigned char size;  // unsigned char to get number of node of humman tree
  in_file_.read(reinterpret_cast<char*>(&size), 1);
  root_ = new huffman_node;
  for (int i = 0; i < size; i++) {
    char a_code;
    unsigned char h_code_c[16];  // 16 unsigned char to obtain the binary code
    in_file_.read(&a_code, 1);
    in_file_.read(reinterpret_cast<char*>(h_code_c), 16);
    std::string h_code_s = "";
    for (int i = 0; i < 16; i++) {  // obtain the oringinal 128-bit binary
                                    // string
      h_code_s += decimalToBinary(h_code_c[i]);
    }
    int j = 0;
    while (h_code_s[j] == kZeroSign) {  // delete the added '000¡­¡­1' to get the real huffman code
      j++;
    }
    h_code_s = h_code_s.substr(j + 1);
    buildTree(h_code_s, a_code);
  }
  in_file_.close();
}

bool HuffmanTree::HuffmanTreeImpl::decodeSave() {
  in_file_.open(in_file_name_, std::ios::in | std::ios::binary);
  out_file_.open(out_file_name_, std::ios::out);
  if (!in_file_.is_open() || !out_file_.is_open()) {
    std::cout << "Failed to open input or output file...";
    return false;
  }
  unsigned char size;  // get the size of huffman tree
  in_file_.read(reinterpret_cast<char*>(&size), 1);
  in_file_.seekg(-1,
                std::ios::end);  // jump to the last one byte to get the number
                                 // of '0' append to the string at last
  char count0;
  in_file_.read(&count0, 1);
  in_file_.seekg(1 + 17 * size,
                std::ios::beg);  // jump to the position where text starts

  std::vector<unsigned char> text;
  unsigned char textseg;
  in_file_.read(reinterpret_cast<char*>(&textseg), 1);

  while (!in_file_.eof()) {  // get the text byte by byte using unsigned char
    text.push_back(textseg);
    in_file_.read(reinterpret_cast<char*>(&textseg), 1);
  }

  node_ptr current = root_;
  std::string path;
  for (size_t i = 0; i < text.size() - 1; i++) {  // translate the huffman code
    path = decimalToBinary(text[i]);
    if (i == text.size() - 2) path = path.substr(0, 8 - count0);
    for (size_t j = 0; j < path.size(); j++) {
      if (path[j] == kZeroSign)
        current = current->left;
      else
        current = current->right;
      if (!current->left && !current->right) {
        out_file_.put(current->id);
        current = root_;
      }
    }
  }
  in_file_.close();
  out_file_.close();
  return true;
}

HuffmanTree::HuffmanTree(const std::string& input_file_path, const std::string& output_file_path)
    : impl_(std::make_unique<HuffmanTreeImpl>(input_file_path, output_file_path)) {}

HuffmanTree::~HuffmanTree() = default;

bool HuffmanTree::encode() {
  if (!impl_->createFrequencyPriorityQueue()) {
    return false;
  }
  impl_->create_huffman_tree();
  impl_->calculateHuffmanCodes();

  return impl_->encodeSave();
}

bool HuffmanTree::decode() {
  impl_->recreateHuffmanTree();
  return impl_->decodeSave();
}

}  // namespace compression
