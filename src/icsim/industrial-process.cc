#include "industrial-process.h"

#include "ns3/nstime.h"

WaterTank::WaterTank()
{
    m_currHeight = AnalogSensor(0, 10);
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

