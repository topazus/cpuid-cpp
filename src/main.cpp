#include <cstdint>
#include <format>
#include <iostream>
#include <cpuid.h>
#include <algorithm>
#include <charconv>
#include <vector>
#include <fmt/ranges.h>
// use qstring in qt6
#include <QString>
#include "utils.hpp"
#include "cpuid.hpp"

#define MAX_INTEL_TOP_LVL 4

static inline void do_cpuid(uint32_t selector, uint32_t *data)
{
  __asm__ volatile("cpuid"
                   : "=a"(data[0]),
                     "=b"(data[1]),
                     "=c"(data[2]),
                     "=d"(data[3])
                   : "a"(selector),
                     "b"(0),
                     "c"(0),
                     "d"(0));
}
auto cpuid_fn(uint32_t selector, uint32_t *result)
{
  do_cpuid(selector, result);
  printf("cpuid_fn(0x%08x) eax:0x%08x ebx:0x%08x ecx:0x%08x edx:0x%08x\n",
         selector, result[0], result[1], result[2], result[3]);
}
struct CPUVendorID
{
  unsigned int ebx;
  unsigned int edx;
  unsigned int ecx;

  std::string toString() const
  {
    return std::string(reinterpret_cast<const char *>(this), 12);
  }
};
auto vendor()
{
  CPUID cpuID(0); // Get CPU vendor
  // store vendor in C char array
  char vendor_c[12];

  // store vendor in std::string
  std::string vendor;
  vendor += std::string((const char *)&cpuID.EBX(), 4);
  vendor += std::string((const char *)&cpuID.EDX(), 4);
  vendor += std::string((const char *)&cpuID.ECX(), 4);

  std::cout << std::format("CPU vendor = {}\n", vendor);

  std::cout << std::format("vendor eax: {}\n", cpuID.EAX());
  std::cout << std::format("vendor ebx: {}\n", cpuID.EBX());
  std::cout << std::format("vendor ecx: {}\n", cpuID.ECX());
  std::cout << std::format("vendor edx: {}\n", cpuID.EDX());

  //
  unsigned int level = 0;
  unsigned int eax = 0;
  unsigned int ebx;
  unsigned int ecx;
  unsigned int edx;
  __get_cpuid(level, &eax, &ebx, &ecx, &edx);

  CPUVendorID vendorID{.ebx = ebx, .edx = edx, .ecx = ecx};

  auto vendorIDString = vendorID.toString();
  std::cout << std::format("vendorIDString: {}\n", vendorIDString);
}
class CPUInfo
{
public:
  CPUInfo();
  std::string vendor() const { return mVendorId; }
  std::string model() const { return mModelName; }
  int cores() const { return mNumCores; }
  float cpuSpeedInMHz() const { return mCPUMHz; }
  bool isSSE() const { return mIsSSE; }
  bool isSSE2() const { return mIsSSE2; }
  bool isSSE3() const { return mIsSSE3; }
  bool isSSE41() const { return mIsSSE41; }
  bool isSSE42() const { return mIsSSE42; }
  bool isAVX() const { return mIsAVX; }
  bool isAVX2() const { return mIsAVX2; }
  bool isHyperThreaded() const { return mIsHTT; }
  int logicalCpus() const { return mNumLogCpus; }

private:
  // Bit positions for data extractions
  static const uint32_t SSE_POS = 0x02000000;
  static const uint32_t SSE2_POS = 0x04000000;
  static const uint32_t SSE3_POS = 0x00000001;
  static const uint32_t SSE41_POS = 0x00080000;
  static const uint32_t SSE42_POS = 0x00100000;
  static const uint32_t AVX_POS = 0x10000000;
  static const uint32_t AVX2_POS = 0x00000020;
  static const uint32_t LVL_NUM = 0x000000FF;
  static const uint32_t LVL_TYPE = 0x0000FF00;
  static const uint32_t LVL_CORES = 0x0000FFFF;

  // Attributes
  std::string mVendorId;
  std::string mModelName;
  int mNumSMT;
  int mNumCores;
  int mNumLogCpus;
  float mCPUMHz;
  bool mIsHTT;
  bool mIsSSE;
  bool mIsSSE2;
  bool mIsSSE3;
  bool mIsSSE41;
  bool mIsSSE42;
  bool mIsAVX;
  bool mIsAVX2;
};

CPUInfo::CPUInfo()
{
  // Get vendor name EAX=0
  CPUID2 cpuID0(0, 0);
  uint32_t HFS = cpuID0.EAX();
  mVendorId += std::string((const char *)&cpuID0.EBX(), 4);
  mVendorId += std::string((const char *)&cpuID0.EDX(), 4);
  mVendorId += std::string((const char *)&cpuID0.ECX(), 4);
  // Get SSE instructions availability
  CPUID2 cpuID1(0x1, 0);
  mIsHTT = cpuID1.EDX() & AVX_POS;
  mIsSSE = cpuID1.EDX() & SSE_POS;
  mIsSSE2 = cpuID1.EDX() & SSE2_POS;
  mIsSSE3 = cpuID1.ECX() & SSE3_POS;
  mIsSSE41 = cpuID1.ECX() & SSE41_POS;
  mIsSSE42 = cpuID1.ECX() & SSE41_POS;
  mIsAVX = cpuID1.ECX() & AVX_POS;
  // Get AVX2 instructions availability
  CPUID2 cpuID7(7, 0);
  mIsAVX2 = cpuID7.EBX() & AVX2_POS;

  std::string upVId = mVendorId;
  for_each(upVId.begin(), upVId.end(), [](char &in)
           { in = ::toupper(in); });
  // Get num of cores
  if (upVId.find("INTEL") != std::string::npos)
  {
    if (HFS >= 11)
    {
      for (int lvl = 0; lvl < MAX_INTEL_TOP_LVL; ++lvl)
      {
        CPUID2 cpuID4(0x0B, lvl);
        uint32_t currLevel = (LVL_TYPE & cpuID4.ECX()) >> 8;
        switch (currLevel)
        {
        case 0x01:
          mNumSMT = LVL_CORES & cpuID4.EBX();
          break;
        case 0x02:
          mNumLogCpus = LVL_CORES & cpuID4.EBX();
          break;
        default:
          break;
        }
      }
      mNumCores = mNumLogCpus / mNumSMT;
    }
    else
    {
      if (HFS >= 1)
      {
        mNumLogCpus = (cpuID1.EBX() >> 16) & 0xFF;
        if (HFS >= 4)
        {
          mNumCores = 1 + (CPUID2(4, 0).EAX() >> 26) & 0x3F;
        }
      }
      if (mIsHTT)
      {
        if (!(mNumCores > 1))
        {
          mNumCores = 1;
          mNumLogCpus = (mNumLogCpus >= 2 ? mNumLogCpus : 2);
        }
      }
      else
      {
        mNumCores = mNumLogCpus = 1;
      }
    }
  }
  else if (upVId.find("AMD") != std::string::npos)
  {
    if (HFS >= 1)
    {
      mNumLogCpus = (cpuID1.EBX() >> 16) & 0xFF;
      if (CPUID2(0x80000000, 0).EAX() >= 8)
      {
        mNumCores = 1 + (CPUID2(0x80000008, 0).ECX() & 0xFF);
      }
    }
    if (mIsHTT)
    {
      if (!(mNumCores > 1))
      {
        mNumCores = 1;
        mNumLogCpus = (mNumLogCpus >= 2 ? mNumLogCpus : 2);
      }
    }
    else
    {
      mNumCores = mNumLogCpus = 1;
    }
  }
  else
  {
    std::cout << "Unexpected vendor id\n";
  }
  // Get processor brand string
  // This seems to be working for both Intel & AMD vendors
  for (int i = 0x80000002; i < 0x80000005; ++i)
  {
    CPUID2 cpuID(i, 0);
    mModelName += std::string((const char *)&cpuID.EAX(), 4);
    mModelName += std::string((const char *)&cpuID.EBX(), 4);
    mModelName += std::string((const char *)&cpuID.ECX(), 4);
    mModelName += std::string((const char *)&cpuID.EDX(), 4);
  }
}
auto test_cpuinfo()
{
  CPUInfo cinfo;

  std::cout << std::format("CPU vendor = {}\n", cinfo.vendor());
  std::cout << std::format("CPU Brand String = {}\n", cinfo.model());
  std::cout << std::format("# of cores = {}\n", cinfo.cores());
  std::cout << std::format("# of logical cores = {}\n", cinfo.logicalCpus());
  std::cout << std::format("Is CPU Hyper threaded = {}\n", cinfo.isHyperThreaded());
  std::cout << std::format("CPU SSE = {}\n ", cinfo.isSSE());
  std::cout << std::format("CPU SSE2 = {}\n", cinfo.isSSE2());
  std::cout << std::format("CPU SSE3 = {}\n", cinfo.isSSE3());
  std::cout << std::format("CPU SSE41 = {}\n", cinfo.isSSE41());
  std::cout << std::format("CPU SSE42 = {}\n", cinfo.isSSE42());
  std::cout << std::format("CPU AVX = {}\n", cinfo.isAVX());
  std::cout << std::format("CPU AVX2 = {}\n", cinfo.isAVX2());
}
static void cpuid_set_generic_info()
{
  uint32_t reg[4];
  char str[128], *p;

  /* do cpuid 0 to get vendor */
  char cpuid_vendor[16];
  cpuid_fn(0, reg);
  // bcopy((char *)&reg[ebx], cpuid_vendor[0], 4); /* ug */
  // bcopy((char *)&reg[ecx], cpuid_vendor[8], 4);
  // bcopy((char *)&reg[edx], cpuid_vendor[4], 4);
  // cpuid_vendor[12] = 0;
}
#define MAX_DIGITS 15
auto test_char()
{
  int n = 9876; // number to be converted
  char number_array[MAX_DIGITS + sizeof(char)];

  // conversion to char array
  std::to_chars(number_array, number_array + MAX_DIGITS, n);

  std::cout << "Number converted to char array is:";
  std::cout << std::format("{}", number_array[0]);
}

// number to byte array
auto num_to_byte_arr(int num)
{
  return std::string((const char *)num, sizeof num);
}
auto test_hybrid_flag()
{
  // The Hybrid Flag can be obtained by calling CPUID with the value “07H” in
  // the EAX register and reading the 15th bit of the EDX register.
  // If the bit is set, the processor supports the Hybrid feature.
  // If 1, the processor is identified as a hybrid part. Additionally, on
  // hybrid parts (CPUID.07H.0H:EDX[15]=1), software must consult the native
  // model ID and core type from the Hybrid Information Enumeration Leaf.
  CPUID2 cpuID(0x07, 0);
  std::cout << std::format("Hybrid Flag = {}\n", (cpuID.EDX() >> 15) & 0x1);

  // The Core Type Flag can be obtained by calling CPUID on each logical
  // processor with a value of “1AH” in the EAX register; this will return each
  // processor’s type in bits 24 - 31 of the EAX register.
  // You can use the core type to determine if a logical processor is either an
  // Efficient-core (E-core, also known as “Gracemont”) (20H), or a
  // Performance-core (P-core, also known as “Golden Cove”) (40H).
  // However, the return value for Core Type does not differentiate between
  // physical and SMT cores for Intel Core processor. Both will be represented
  // as Intel Core (40H).
  CPUID2 cpuID2(0x1A, 0);
  std::cout << std::format("{}\n", cpuID2.EAX());
  std::cout << std::format("Core Type = {}\n", (cpuID2.EAX() >> 24) & 0xFF);
  std::cout << std::format("Core Type = {}\n", extract_bits(cpuID2.EAX(), 24, 31));
}
auto cpuid_01H()
{
  // EAX=01H
  // EAX
  // Type, Family, Model, and Stepping ID
  // The Type, Family, Model, and Stepping ID can be obtained by calling CPUID
  // with the value “01H” in the EAX register and reading the EAX register.
  // The Type is bits 12 - 13,
  // the Family is bits 8 - 11,
  // Model is bits 4 - 7
  // Stepping ID is bits 0 - 3.
  // Extended Model ID: bits 16 - 19
  // Extended Family ID: bits 20 - 27
  CPUID2 cpuID(0x01, 0);
  std::cout << std::format("Type = {:#X}\n", extract_bits(cpuID.EAX(), 12, 13));
  std::cout << std::format("Family = {:#X}\n", extract_bits(cpuID.EAX(), 8, 11));
  std::cout << std::format("Model = {:#X}\n", extract_bits(cpuID.EAX(), 4, 7));
  std::cout << std::format("Stepping ID = {:#X}\n", extract_bits(cpuID.EAX(), 0, 3));
  std::cout << std::format("Extended Model ID = {:#X}\n", extract_bits(cpuID.EAX(), 16, 19));
  std::cout << std::format("Extended Family ID = {:#X}\n", extract_bits(cpuID.EAX(), 20, 27));
  auto real_model = (extract_bits(cpuID.EAX(), 16, 19) << 4) | extract_bits(cpuID.EAX(), 4, 7);
  std::cout << std::format("Real Model = {:#X}\n", real_model);

  // EBX
  // Bits 07-00: Brand Index.
  // Bits 15-08: CLFLUSH line size (Value ∗ 8 = cache line size in bytes; used also by CLFLUSHOPT).
  // Bits 23-16: Maximum number of addressable IDs for logical processors in this physical package*.
  // Bits 31-24: Initial APIC ID**
  auto apic_id = extract_bits(cpuID.EBX(), 24, 31);
  std::cout << std::format("APIC ID = {:#X}\n", apic_id);

  // ECX Feature Information
  // Bit 0: SSE3
  // Bit 1: PCLMULQDQ
  // Bit 2: DTEST64
  // Bit 3: MONITOR
  // Bit 4: DS-CPL
  // Bit 5: VMX
  // Bit 6: SMX
  // Bit 7: EIST
  // Bit 8: TM2
  // Bit 9: SSSE3
  // Bit 10: CNXT-ID
  // Bit 11: SDBG
  // Bit 12: FMA
  // Bit 13: CMPXCHG16B
  // Bit 14: xTPR Update Control
  // Bit 15: PDCM
  // 16 Reserved Reserved
  // Bit 17: PCID
  // Bit 18: DCA
  // Bit 19: SSE4.1
  // Bit 20: SSE4.2
  // Bit 21: x2APIC
  // Bit 22: MOVBE
  // Bit 23: POPCNT
  // Bit 24: TSC-Deadline
  // 25 AESNI
  // 26 XSAVE
  // 27 OSXSAVE
  // 28 AVX
  // 29 F16C
  // 30 RDRAND
  // 31 Not Used Always returns 0.
  std::cout << std::format("SSE3 = {}\n", extract_bits(cpuID.ECX(), 0, 0));
  std::cout << std::format("PCLMULQDQ = {}\n", (cpuID.ECX() >> 1) & 0x1);
  std::cout << std::format("DTEST64 = {}\n", (cpuID.ECX() >> 2) & 0x1);
  std::cout << std::format("MONITOR = {}\n", (cpuID.ECX() >> 3) & 0x1);
  std::cout << std::format("DS-CPL = {}\n", (cpuID.ECX() >> 4) & 0x1);
  std::cout << std::format("VMX = {}\n", (cpuID.ECX() >> 5) & 0x1);
  std::cout << std::format("SMX = {}\n", (cpuID.ECX() >> 6) & 0x1);
  std::cout << std::format("EIST = {}\n", (cpuID.ECX() >> 7) & 0x1);
  std::cout << std::format("TM2 = {}\n", (cpuID.ECX() >> 8) & 0x1);
  std::cout << std::format("SSSE3 = {}\n", (cpuID.ECX() >> 9) & 0x1);
  std::cout << std::format("CNXT-ID = {}\n", (cpuID.ECX() >> 10) & 0x1);
  std::cout << std::format("SDBG = {}\n", (cpuID.ECX() >> 11) & 0x1);
  std::cout << std::format("FMA = {}\n", extract_bits(cpuID.ECX(), 12, 12));
  std::cout << std::format("CMPXCHG16B = {}\n", extract_bits(cpuID.ECX(), 13, 13));
  std::cout << std::format("xTPR Update Control = {}\n", extract_bits(cpuID.ECX(), 14, 14));
  std::cout << std::format("PDCM = {}\n", extract_bits(cpuID.ECX(), 15, 15));
  std::cout << std::format("PCID = {}\n", extract_bits(cpuID.ECX(), 17, 17));
  std::cout << std::format("DCA = {}\n", extract_bits(cpuID.ECX(), 18, 18));
  std::cout << std::format("SSE4.1 = {}\n", extract_bits(cpuID.ECX(), 19, 19));
  std::cout << std::format("SSE4.2 = {}\n", extract_bits(cpuID.ECX(), 20, 20));
  std::cout << std::format("x2APIC = {}\n", extract_bits(cpuID.ECX(), 21, 21));
  std::cout << std::format("MOVBE = {}\n", extract_bits(cpuID.ECX(), 22, 22));
  std::cout << std::format("POPCNT = {}\n", extract_bits(cpuID.ECX(), 23, 23));
  std::cout << std::format("TSC-Deadline = {}\n", extract_bits(cpuID.ECX(), 24, 24));
  std::cout << std::format("AESNI = {}\n", extract_bits(cpuID.ECX(), 25, 25));
  std::cout << std::format("XSAVE = {}\n", extract_bits(cpuID.ECX(), 26, 26));
  std::cout << std::format("OSXSAVE = {}\n", extract_bit(cpuID.ECX(), 27));
  std::cout << std::format("AVX = {}\n", extract_bit(cpuID.ECX(), 28));
  std::cout << std::format("F16C = {}\n", extract_bit(cpuID.ECX(), 29));
  std::cout << std::format("RDRAND = {}\n", extract_bit(cpuID.ECX(), 30));
}
auto test_processor_serial()
{
  // The processor serial number can be obtained by calling CPUID with the
  // value “03H” in the EAX register and reading the EDX register.
  CPUID2 cpuID(0x03, 0);
  std::cout << std::format("Processor Serial Number = {}\n", cpuID.EDX());
}
// initial EAX=0H
// EAX Maximum Input Value for Basic CPUID Information.
// EBX “Genu”
// ECX “ntel”
// EDX “ineI”
auto cpuid_0H()
{
  CPUID2 cpuid(0x0, 0x0);
  std::cout << std::format("EAX = {:#X}\n", cpuid.EAX());
  std::cout << std::format("EBX = {:#X}, {}\n", cpuid.EBX(), num_to_str(cpuid.EBX()));
  std::cout << std::format("ECX = {:#X}, {}\n", cpuid.ECX(), num_to_str(cpuid.ECX()));
  std::cout << std::format("EDX = {:#X}, {}\n", cpuid.EDX(), num_to_str(cpuid.EDX()));
}

int main()
{
  cpuid_0H();
  std::cout << "------------------\n";
  cpuid_01H();
  test_processor_serial();
  test_char();
  test_hybrid_flag();
  vendor();
  test_cpuinfo();
  // std::cout << std::format("{}", num_to_byte_arr(1970169159));
  // fmt::print("{}\n", fmt::join(ToLEBytes(0x12345678), ", "));
  // u32 number to vector of char
  auto char_arr = ToLEBytes(1970169159);
  std::cout << fmt::format("{::#x}\n", char_arr);

  // vector of char to string
  std::string s(char_arr.begin(), char_arr.end());
  std::cout << std::format("{}\n", s);
  return 0;
}