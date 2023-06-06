#include "industrial-process.h"

#include "plc-application.h"

#include "ns3/nstime.h"
#include "ns3/simulator.h"

#include <iostream>

PlcState
WaterTank::UpdateState(const PlcState& plcIn, PlcState plcOut)
{
    bool level1On = plcIn.GetDigitalState(WT_LEVEL1);
    bool level2On = plcIn.GetDigitalState(WT_LEVEL2);

    bool pupmOn = plcOut.GetDigitalState(WT_PUMP);
    bool valveOn = plcOut.GetDigitalState(WT_VALVE);

    if (level2On)
    {
        // Turn pump off and valve on
        plcOut.SetDigitalState(WT_PUMP, false);
        plcOut.SetDigitalState(WT_VALVE, true);
    }
    else if (!level1On)
    {
        // Turn pump on and valve off
        plcOut.SetDigitalState(WT_PUMP, true);
        plcOut.SetDigitalState(WT_VALVE, false);
    }

    return plcOut;
}

PlcState
WaterTank::UpdateProcess(PlcState plcIn, const PlcState& plcOut)
{
    ns3::Time current = ns3::Simulator::Now();
    //std::clog << "[WaterTank] At time " << current.As(Time::S) << ", before sensor update\n";

    bool pupmOn = plcOut.GetDigitalState(WT_PUMP);
    bool valveOn = plcOut.GetDigitalState(WT_VALVE);

    if (pupmOn)
    {
        m_currHeight += s_pumpFlow * (current.ToDouble(ns3::Time::S) - m_prevTime.ToDouble(ns3::Time::S)) / s_tankWidth;
    }
    if (valveOn)
    {
        m_currHeight -= s_valveFlow * (current.ToDouble(ns3::Time::S) - m_prevTime.ToDouble(ns3::Time::S)) / s_tankWidth;
    }

    m_prevTime = current;

    // Set level sensors
    plcIn.SetDigitalState(WT_LEVEL1, m_currHeight >= s_level1);
    plcIn.SetDigitalState(WT_LEVEL2, m_currHeight >= s_level2);

    return plcIn;
}

PlcState
SemaphoreLights::UpdateState(const PlcState& plcIn, PlcState plcOut)
{
    //std::clog << "Updating state [SemaphoreLights - " << Simulator::Now().As(Time::S) << "]\n";
    return plcOut;
}

PlcState
SemaphoreLights::UpdateProcess(PlcState plcIn, const PlcState& plcOut)
{
    return plcIn;
}

