#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

template <typename T>
void PrintPaddedHex(T v) {
  const auto saved_width = std::cout.width();
  const auto saved_flags = std::cout.flags();
  auto width = (std::numeric_limits<T>::digits + 3) / 4;
  std::cout << "0x" << std::setfill('0') << std::setw(width) << std::hex << v;
  std::cout.width(saved_width);
  std::cout.flags(saved_flags);
}

template <typename T>
std::vector<T> GenerateTable(const T polynomial, const size_t table_size) {
  auto kBitCount = std::numeric_limits<T>::digits;
  const T kTopBit = static_cast<T>(1) << (kBitCount - 1);
  // std::cout << std::dec << "kBitCount=" << kBitCount << ", kTopBit=";
  // PrintPaddedHex(kTopBit);
  // std::cout << std::endl;
  std::vector<T> table;
  T remainder;
  for (size_t dividend = 0; dividend < table_size; ++dividend) {
    // Start with the dividend followed by zeros.
    remainder = static_cast<T>(dividend << (kBitCount - 8));

    // Perform modulo-2 division, a bit at a time.
    for (int bit = 8; bit > 0; --bit) {
      // Try to divide the current data bit.
      if (remainder & kTopBit) {
        remainder = (remainder << 1) ^ polynomial;
      } else {
        remainder = (remainder << 1);
      }
    }

    table.push_back(remainder);
  }
  return table;
}

template <typename T>
void GenerateAndPrintTable(const T polynomial, const size_t table_size) {
  std::cout << "Table of size " << table_size << std::hex << ", polynomial: ";
  PrintPaddedHex(polynomial);
  std::cout << std::endl;
  auto table = GenerateTable<T>(polynomial, table_size);
  for (size_t ndx = 0; ndx < table.size(); ++ndx) {
    if (ndx > 0) {
      std::cout << ",";
    }
    if (ndx % 6 == 0) {
      std::cout << std::endl << "    ";
    } else {
      std::cout << " ";
    }
    PrintPaddedHex(table[ndx]);
  }
  std::cout << std::dec << std::endl << std::endl;
}

int main() {
  // This produces the table used by Crc32 to compute the CRC; the table was
  // originally copied from https://www.arduino.cc/en/Tutorial/EEPROMCrc, but I
  // wanted to be sure I understood how to generate the table.
  GenerateAndPrintTable<uint32_t>(0x1db71064, 16);
  return EXIT_SUCCESS;
}
