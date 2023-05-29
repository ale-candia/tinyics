#include "utils.h"

std::tuple<uint8_t, uint8_t>
SplitUint16(uint16_t value)
{
    uint8_t lower = value & 0xFF;             // Extract the lower 8 bits
    uint8_t higher = (value >> 8) & 0xFF;     // Extract the higher 8 bits
    return std::make_tuple(higher, lower);
}

uint16_t
CombineUint8(uint8_t higher, uint8_t lower) {
    return (static_cast<uint16_t>(higher) << 8) | lower;
}

uint8_t
GetBitsInRangeBE(uint16_t start, uint16_t numOfBits, uint8_t bits)
{
    uint8_t mask = (1 << numOfBits) - 1;

    bits >>= start; // Shift value to right-align the desired bits
    bits &= mask; // Apply mask to extract desired bits

    return bits;
}

