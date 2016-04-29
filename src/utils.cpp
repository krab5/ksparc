/*
 * utils.cpp -- some utility functions
 */
#include "utils.h"

// Subbits of an int
uint32_t sub(uint32_t data, uint32_t from, uint32_t size) {
  return (data & ((0xFFFFFFFF >> (32 - size)) << from)) >> from;
}

// Sign extension
uint32_t signext(uint32_t data, uint32_t size) {
  if ((data >> (size - 1)) == 1) { // it is negative
    return data | (0xFFFFFFFF << size);
  } else {
    return data;
  }
}

// Sign extension for 64 bits
uint64_t signext64(uint64_t data, uint32_t size) {
  if ((data >> (size - 1)) == 1) { // it is negative
    return data | (0xFFFFFFFFFFFFFFFF << size);
  } else {
    return data;
  }
}

#ifdef _DEBUG
// Print a binary number for debugging
std::string printBinary(int32_t val) {
  std::ostringstream os;
  std::bitset<32> b(val);
  os << b;
  return os.str();
}
#endif //_DEBUG


