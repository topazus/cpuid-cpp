
#include "utils.hpp"

#include <algorithm>
#include <string>
#include <vector>
/* Only for 32bit values */
#define bit32(n) (1U << (n))
#define bitmask32(h, l) ((bit32(h) | (bit32(h) - 1)) & ~(bit32(l) - 1))
#define bitfield32(x, h, l) (((x & bitmask32(h, l)) >> l))

// macros to functions
// export unsigned bit32(unsigned n) { return 1U << n; }
// export unsigned bitmask32(unsigned h, unsigned l) {
//   return (bit32(h) | (bit32(h) - 1)) & ~(bit32(l) - 1);
// }
// export unsigned bitfield32(unsigned x, unsigned h, unsigned l) {
//   return ((x & bitmask32(h, l)) >> l);
// }

// extract bits from a 32-bit value, from bit start to bit end
// e.g. extract_bits(0x12345678, 31, 24) returns 0x12
// e.g. extract_bits(0x12345678, 23, 16) returns 0x34
#define extract_bits(x, start, end) bitfield32(x, end, start)
#define extract_bit(x, n) extract_bits(x, n, n)
// export unsigned extract_bits(unsigned x, unsigned start, unsigned end) {
//   return bitfield32(x, end, start);
// }
// export unsigned extract_bit(unsigned x, unsigned n) {
//   return bitfield32(x, n, n);
// }

typedef unsigned int uint32_t;

// Returns the memory representation of an integer in little-endian byte order
unsigned char *intToBytesLE(uint32_t value) {
  // Allocate memory for a byte array of size 4
  unsigned char *bytes = new unsigned char[4];

  // Copy the bytes of the integer into the byte array in little-endian order
  bytes[0] = static_cast<unsigned char>(value);
  bytes[1] = static_cast<unsigned char>(value >> 8);
  bytes[2] = static_cast<unsigned char>(value >> 16);
  bytes[3] = static_cast<unsigned char>(value >> 24);

  return bytes;
}

// https://doc.rust-lang.org/std/primitive.u32.html#method.to_le_bytes

// https://github.com/LibreOffice/online/blob/d0edfeabbdc969a9a66cf90976a63c2f4403a6d3/wsd/ProofKey.cpp#L41-L83
std::vector<unsigned char> getBytesLE(const unsigned char *bytesInHostOrder,
                                      const size_t n) {
  std::vector<unsigned char> ret(n);
#if !defined __BYTE_ORDER__
  static_assert(false, "Byte order is not detected on this platform!");
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  std::copy_n(bytesInHostOrder, n, ret.begin());
#else
  std::copy_n(bytesInHostOrder, n, ret.rbegin());
#endif
  return ret;
}

std::vector<unsigned char> getBytesBE(const unsigned char *bytesInHostOrder,
                                      const size_t n) {
  std::vector<unsigned char> ret(n);
#if !defined __BYTE_ORDER__
  static_assert(false, "Byte order is not detected on this platform!");
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  std::copy_n(bytesInHostOrder, n, ret.rbegin());
#else
  std::copy_n(bytesInHostOrder, n, ret.begin());
#endif
  return ret;
}

std::string num_to_str(int num) {
  auto vec = ToLEBytes(num);
  std::string s(vec.begin(), vec.end());
  return s;
}

char *num_to_str2(int num) {
  auto vec = intToBytesLE(num);
  return reinterpret_cast<char *>(vec);
}
