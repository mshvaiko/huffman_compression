#ifndef COMPRESSOR_COMPRESSOR_BASE_H_
#define COMPRESSOR_COMPRESSOR_BASE_H_

#include <stddef.h>

#include <cstdint>

namespace compression {

class compressor_base {
 public:
  virtual ~compressor_base() = default;

  virtual bool encode() = 0;
  virtual bool decode() = 0;
};

}  // namespace compression

#endif  // COMPRESSOR_COMPRESSOR_BASE_H_