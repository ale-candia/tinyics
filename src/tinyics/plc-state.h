#pragma once

#include "sensor.h"
#include "utils.h"

#include "ns3/fatal-error.h"

class PlcState
{
  public:
    void SetDigitalState(uint8_t pos, bool value);

    bool GetDigitalState(uint8_t pos) const;

    uint8_t GetBits(uint16_t start, uint16_t num) const;

    /*
     * Store the value of the input variable into a 16 bit register
     *
     * This simulates the internal ADC of the PLC converting a current
     * in the range 4-20mA to a digital 16-bit value.
     */
    void SetAnalogState(uint8_t pos, double value);

    /*
     * Set the analog value but from a sensor directly
     */
    void SetAnalogState(uint8_t pos, const AnalogSensor& value);

    /*
     * Retrieve the value stored in the PLC
     */
    uint16_t GetAnalogState(uint8_t pos) const;

  private:
    uint8_t m_DigitalPorts;
    uint16_t m_AnalogPorts[2];
};

