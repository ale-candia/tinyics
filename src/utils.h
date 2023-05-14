#pragma once

#include <tuple>

/*
 * Take an uint16_t and return the higher and lower parts of the split
 */
std::tuple<uint8_t, uint8_t> SplitUint16(uint16_t value);

/*
 * Take two uint8_t and join them into a uint16_t
 */
uint16_t CombineUint8(uint8_t higher, uint8_t lower);

/*
 * Get the bits in a specific range
 *
 * Example:
 *  input: start 0 - numOfBits 5 - bits 0110 1011 (uint8_t 107)
 *  returns: 0110 1000 (uint8_t 104)
 */
uint8_t GetBitsInRange(uint16_t start, uint16_t numOfBits, uint8_t bits);

