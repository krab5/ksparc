/*
 * utils.h -- defines some utility functions
 */
#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <cstddef>
#include "logger.h"

// Get the two's complement of a 32-bit number
#define COMPL32(b)    ((~b)+0x00000001)
// Get the two's complement of a 64-bit number
#define COMPL64(b)    ((~b)+0x0000000000000001)
// Test if a number is negative
#define ISNEG(b)      ((b >> 31) == 1)

/**
 * Get a portion of a 32 bits value
 * @param data data from which to extract
 * @param from starting point
 * @param size number of bits to retrieve
 * @returns the data
 */
uint32_t sub(uint32_t data, uint32_t from, uint32_t size);
/**
 * Sign-extend a value
 * @param data to sign-extend
 * @param size size of the original value
 */
uint32_t signext(uint32_t data, uint32_t size);
/**
 * Sign-extend a 64 bits value
 * @param data to sign-extend
 * @param size size of the original value
 */
uint64_t signext64(uint64_t data, uint32_t size);

#ifdef _DEBUG
#include <iostream>
#include <bitset>
#include <sstream>
#include <iomanip>

#define LOG(s) std::cout << s << std::endl

/**
 * Print a value in binary
 */
std::string printBinary(int32_t val);
#endif

#endif // UTILS_H

