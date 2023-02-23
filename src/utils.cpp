#include <vector>
#include <algorithm>
#include <string>
#include "utils.hpp"
// https://github.com/LibreOffice/online/blob/d0edfeabbdc969a9a66cf90976a63c2f4403a6d3/wsd/ProofKey.cpp#L41-L83
std::vector<unsigned char> getBytesLE(const unsigned char *bytesInHostOrder, const size_t n)
{
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

std::vector<unsigned char> getBytesBE(const unsigned char *bytesInHostOrder, const size_t n)
{
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

std::string num_to_str(int num)
{
  auto vec = ToLEBytes(num);
  std::string s(vec.begin(), vec.end());
  return s;
}

// // Returns passed number as vector of bytes (little-endian)
// template <typename T>
// std::vector<unsigned char> ToLEBytes(const T &x)
// {
//   return getBytesLE(reinterpret_cast<const unsigned char *>(&x), sizeof(x));
// }

// // Returns passed number as vector of bytes (network order = big-endian)
// template <typename T>
// std::vector<unsigned char> ToNetworkOrderBytes(const T &x)
// {
//   return getBytesBE(reinterpret_cast<const unsigned char *>(&x), sizeof(x));
// }