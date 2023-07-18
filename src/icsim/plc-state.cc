#include "plc-state.h"

void
PlcState::SetDigitalState(uint8_t pos, bool value)
{
    SetBitBE(m_DigitalPorts, pos, value);
}

bool
PlcState::GetDigitalState(uint8_t pos) const
{
    return GetBitBE(m_DigitalPorts, pos);
}

uint8_t
PlcState::GetBits(uint16_t start, uint16_t num) const
{
    return GetBitsInRangeBE(start, num, m_DigitalPorts);
}

void
PlcState::SetAnalogState(uint8_t pos, double value)
{
    if (pos > 2)
        NS_FATAL_ERROR("Index out of bounds '" << (int)pos << "' for analog input");

    // Convert the 4-20mA into a 16-bit digital number
    m_AnalogPorts[pos] = static_cast<uint16_t>(NormalizeInRange(value, 4, 20) *
                                               std::numeric_limits<uint16_t>::max());
}

void
PlcState::SetAnalogState(uint8_t pos, const AnalogSensor& value)
{
    SetAnalogState(pos, value.GetOutputValue());
}

uint16_t
PlcState::GetAnalogState(uint8_t pos) const
{
    if (pos > 2)
        NS_FATAL_ERROR("Index out of bounds '" << (int)pos << "' for analog input");

    return m_AnalogPorts[pos];
}

