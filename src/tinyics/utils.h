#pragma once

#include <tuple>
#include <cstdint>

/*
 * Take an uint16_t and return the higher and lower parts of the split
 */
std::tuple<uint8_t, uint8_t> SplitUint16(uint16_t value);

/*
 * Take two uint8_t and join them into a uint16_t
 */
uint16_t CombineUint8(uint8_t higher, uint8_t lower);

/*
 * Get the bits in a specific range (using Big-Endian byte order)
 *
 * Example:
 *  input: start 0 - numOfBits 5 - bits 0110 1011 (uint8_t 107)
 *  returns: 0110 1000 (uint8_t 104)
 */
uint8_t GetBitsInRangeBE(uint16_t start, uint16_t numOfBits, uint8_t bits);

/*
 * Set the bit in the specified position to the specified value
 */
void SetBitBE(uint8_t& byte, uint8_t position, bool value);

/*
 * Get the bit in the specified position
 */
bool GetBitBE(const uint8_t& byte, uint8_t position);

/*
 * Normalize value to the specified range. A.K.A. Linearly Scaling
 *
 * (Fails if input is not in the range)
 */
double NormalizeInRange(double value, double min, double max);

/*
 * For a given uint16_t value scale it to the range [min, max]
 */
double DenormalizeU16InRange(uint16_t value, double min, double max);

