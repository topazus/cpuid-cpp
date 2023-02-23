#ifndef UTILS_HPP
#define UTILS_HPP
#include <vector>

/* Only for 32bit values */
#define bit32(n) (1U << (n))
#define bitmask32(h, l) ((bit32(h) | (bit32(h) - 1)) & ~(bit32(l) - 1))
#define bitfield32(x, h, l) (((x & bitmask32(h, l)) >> l))

// extract bits from a 32-bit value, from bit start to bit end
// e.g. extract_bits(0x12345678, 31, 24) returns 0x12
// e.g. extract_bits(0x12345678, 23, 16) returns 0x34
#define extract_bits(x, start, end) bitfield32(x, end, start)
#define extract_bit(x, n) extract_bits(x, n, n)

// https://github.com/LibreOffice/online/blob/d0edfeabbdc969a9a66cf90976a63c2f4403a6d3/wsd/ProofKey.cpp#L41-L83
std::vector<unsigned char> getBytesLE(const unsigned char *bytesInHostOrder, const size_t n);

std::vector<unsigned char> getBytesBE(const unsigned char *bytesInHostOrder, const size_t n);

// Returns passed number as vector of bytes (little-endian)
template <typename T>
std::vector<unsigned char> ToLEBytes(const T &x)
{
  return getBytesLE(reinterpret_cast<const unsigned char *>(&x), sizeof(x));
}

// Returns passed number as vector of bytes (network order = big-endian)
template <typename T>
std::vector<unsigned char> ToNetworkOrderBytes(const T &x)
{
  return getBytesBE(reinterpret_cast<const unsigned char *>(&x), sizeof(x));
}

std::string num_to_str(int num);

#endif // UTILS_HPP