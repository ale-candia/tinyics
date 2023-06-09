#pragma once

#include "sensor.h"
#include "utils.h"

#include "ns3/fatal-error.h"

class PlcState
{
  public:
    void SetDigitalState(uint8_t pos, bool value)
    {
        SetBitBE(m_DigitalPorts, pos, value);
    }

    bool GetDigitalState(uint8_t pos) const
    {
        return GetBitBE(m_DigitalPorts, pos);
    }

    uint8_t GetBits(uint16_t start, uint16_t num) const
    {
        return GetBitsInRangeBE(start, num, m_DigitalPorts);
    }

    /*
     * Store the value of the input variable into a 16 bit register
     *
     * This simulates the internal ADC of the PLC converting a current
     * in the range 4-20mA to a digital 16-bit value.
     */
    void SetAnalogState(uint8_t pos, double value)
    {
        if (pos > 2)
            NS_FATAL_ERROR("Index out of bounds '" << (int)pos << "' for analog input");

        // Convert the 4-20mA into a 16-bit digital number
        m_AnalogPorts[pos] = static_cast<uint16_t>(NormalizeInRange(value, 4, 20) *
                                                   std::numeric_limits<uint16_t>::max());
    }

    /*
     * Set the analog value but from a sensor directly
     */
    void SetAnalogState(uint8_t pos, const AnalogSensor& value)
    {
        SetAnalogState(pos, value.GetOutputValue());
    }

    /*
     * Retrieve the value stored in the PLC
     */
    uint16_t GetAnalogState(uint8_t pos) const
    {
        if (pos > 2)
            NS_FATAL_ERROR("Index out of bounds '" << (int)pos << "' for analog input");

        return m_AnalogPorts[pos];
    }

  private:
    uint8_t m_DigitalPorts;
    uint16_t m_AnalogPorts[2];
};

