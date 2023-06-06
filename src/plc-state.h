#pragma once

#include "utils.h"

#include <cstdint>

#include "ns3/core-module.h"

class PlcState
{
public:
    void SetDigitalState(uint8_t pos, bool value) {
        SetBitBE(m_DigitalPorts, pos, value);
    }

    bool GetDigitalState(uint8_t pos) const {
        return GetBitBE(m_DigitalPorts, pos);
    }

    uint8_t GetBits(uint16_t start, uint16_t num) const {
        return GetBitsInRangeBE(start, num, m_DigitalPorts);
    }

    void SetAnalogState(uint8_t pos, uint16_t value) {
        if (pos < 2) m_AnalogPorts[pos] = value;
    }

    uint16_t GetAnalogState(uint8_t pos) const {
        if (pos > 3)
            NS_FATAL_ERROR("Index out of bounds '" << (int)pos << "' for analog input");

        return m_AnalogPorts[pos];
    }

private:
    uint8_t m_DigitalPorts;
    uint16_t m_AnalogPorts[2];
};

