#include "utils.h"

#include <ns3/fatal-error.h>

std::tuple<uint8_t, uint8_t>
SplitUint16(uint16_t value)
{
    uint8_t lower = value & 0xFF;         // Extract the lower 8 bits
    uint8_t higher = (value >> 8) & 0xFF; // Extract the higher 8 bits
    return std::make_tuple(higher, lower);
}

uint16_t
CombineUint8(uint8_t higher, uint8_t lower)
{
    return (static_cast<uint16_t>(higher) << 8) | lower;
}

uint8_t
GetBitsInRangeBE(uint16_t start, uint16_t numOfBits, uint8_t bits)
{
    uint8_t mask = (1 << numOfBits) - 1;

    bits >>= start; // Shift value to right-align the desired bits
    bits &= mask;   // Apply mask to extract desired bits

    return bits;
}

void
SetBitBE(uint8_t& byte, uint8_t pos, bool value)
{
    if (pos < 8)
    {
        uint8_t mask = 1 << pos;
        if (value)
        {
            byte |= mask; // Set bit to 1
        }
        else
        {
            byte &= ~mask; // Set bit to 0
        }
    }
}

bool
GetBitBE(const uint8_t& byte, uint8_t pos)
{
    if (pos > 7)
        NS_FATAL_ERROR("Index out of rage '" << (int)pos << "' for byte, ");

    return GetBitsInRangeBE(pos, 1, byte);
}

double
NormalizeInRange(double value, double min, double max)
{
    if (value < min or value > max)
    {
        NS_FATAL_ERROR(
                "Linearly mapped value should be in the specified range. Called with "
                << value << ", " << min << ", " << max);
    }

    return (value - min) / (max - min);
}

double
DenormalizeU16InRange(uint16_t value, double min, double max)
{
    double scalingFactor = (max - min) / std::numeric_limits<uint16_t>::max();

    return scalingFactor * value;
}

