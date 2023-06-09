#include "industrial-process.h"

#include "ns3/nstime.h"

WaterTank::WaterTank()
{
    m_currHeight = AnalogSensor(0, 10);
}

PlcState
WaterTank::UpdateState(const PlcState& measured, PlcState plcOut)
{
    // The value returned by the PLC is a 16-bit value (called word) that 
    // has to be denormalized into the actual physical value
    double height = DenormalizeU16InRange(measured.GetAnalogState(LEVEL_SENSOR_POS), 0, 10);

    bool pumpOn = plcOut.GetDigitalState(PUMP_POS);
    bool valveOn = plcOut.GetDigitalState(VALVE_POS);

    if (height > s_level2)
    {
        // Turn pump off and valve on
        plcOut.SetDigitalState(PUMP_POS, false);
        plcOut.SetDigitalState(VALVE_POS, true);
    }
    else if (height < s_level1)
    {
        // Turn pump on and valve off
        plcOut.SetDigitalState(PUMP_POS, true);
        plcOut.SetDigitalState(VALVE_POS, false);
    }

    return plcOut;
}

PlcState
WaterTank::UpdateProcess(PlcState state, const PlcState& input)
{
    auto current = ns3::Simulator::Now().ToDouble(ns3::Time::S);

    bool pupmOn = input.GetDigitalState(PUMP_POS);
    bool valveOn = input.GetDigitalState(VALVE_POS);

    if (pupmOn)
    {
        m_currHeight += s_pumpFlow * (current - m_prevTime) / s_tankWidth;
    }
    if (valveOn)
    {
        m_currHeight -= s_valveFlow * (current - m_prevTime) / s_tankWidth;
    }

    m_prevTime = current;

    // update level sensor
    state.SetAnalogState(LEVEL_SENSOR_POS, m_currHeight);

    return state;
}

