// SENSORS
#define WT_LEVEL1 0x01
#define WT_LEVEL2 0x02

// ACTUATORS
#define WT_PUMP 0x01
#define WT_VALVE 0x02

#include "industrial-process.h"

#include "plc-application.h"

#include "ns3/nstime.h"
#include "ns3/simulator.h"

#include <iostream>

using namespace ns3;

bool
GetStatus(uint8_t digitalPorts, uint8_t coil)
{
    // Using an enum is safer for this
    return digitalPorts & coil;
}

void
SetStatus(PlcState& plcIO, uint8_t coil, bool up)
{
    if (up)
    {
        plcIO.digitalPorts |= coil;
    }
    else
    {
        plcIO.digitalPorts &= ~coil;
    }
}

void
WaterTank::UpdateState(const PlcState& plcIn, PlcState& plcOut)
{
    bool level1On = GetStatus(plcIn.digitalPorts, WT_LEVEL1);
    bool level2On = GetStatus(plcIn.digitalPorts, WT_LEVEL2);

    bool pupmOn = GetStatus(plcOut.digitalPorts, WT_PUMP);
    bool valveOn = GetStatus(plcOut.digitalPorts, WT_VALVE);

    if (level2On)
    {
        // Turn pump off and valve on
        SetStatus(plcOut, WT_PUMP, false);
        SetStatus(plcOut, WT_VALVE, true);
    }
    else if (!level1On)
    {
        // Turn pump on and valve off
        SetStatus(plcOut, WT_PUMP, true);
        SetStatus(plcOut, WT_VALVE, false);
    }
}

void
WaterTank::UpdateProcess(PlcState& plcIn, const PlcState& plcOut)
{
    Time current = Simulator::Now();
    //std::clog << "[WaterTank] At time " << current.As(Time::S) << ", before sensor update\n";

    bool pupmOn = GetStatus(plcOut.digitalPorts, WT_PUMP);
    bool valveOn = GetStatus(plcOut.digitalPorts, WT_VALVE);

    if (pupmOn)
    {
        m_currHeight += s_pumpFlow * (current.ToDouble(Time::S) - m_prevTime.ToDouble(Time::S)) / s_tankWidth;
    }
    if (valveOn)
    {
        m_currHeight -= s_valveFlow * (current.ToDouble(Time::S) - m_prevTime.ToDouble(Time::S)) / s_tankWidth;
    }

    m_prevTime = current;

    // Set level sensors
    SetStatus(plcIn, WT_LEVEL1, m_currHeight >= s_level1);
    SetStatus(plcIn, WT_LEVEL2, m_currHeight >= s_level2);

    //std::clog << "- Height:  " << m_currHeight << "\n";
    //std::clog << "- Pump:  " << pupmOn << "\n";
    //std::clog << "- Valve:  " << valveOn << "\n";
    //std::clog << "- Level 1:  " << GetStatus(plcIn.digitalPorts, WT_LEVEL1) << "\n";
    //std::clog << "- Level 2:  " << GetStatus(plcIn.digitalPorts, WT_LEVEL2) << "\n";
}

void
SemaphoreLights::UpdateState(const PlcState& plcIn, PlcState& plcOut)
{
    //std::clog << "Updating state [SemaphoreLights - " << Simulator::Now().As(Time::S) << "]\n";
}

void
SemaphoreLights::UpdateProcess(PlcState& plcIn, const PlcState& plcOut)
{
}

