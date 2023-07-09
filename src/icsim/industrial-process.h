#pragma once

#include "plc-state.h"

#include "ns3/nstime.h"
#include "ns3/simulator.h"

/**
 * An Industrial Process to be controlled by a PLC
 *
 * This is called from the PLC to update the state of the process as
 * well as the state of the inputs and outputs of the PLC.
 */
class IndustrialProcess
{
public:
    virtual ~IndustrialProcess() = default;

    // Returns the updated measurements from the PLC
    virtual PlcState UpdateProcess(PlcState plcIn, const PlcState& plcOut) = 0;
};

/**
 * A Water Tank system
 *
 * It contains two level sensors, a pump and a valve
 */
class WaterTank : public IndustrialProcess
{
public:
    // SENSORS
    static constexpr double LEVEL_SENSOR_POS = 0;

    // ACTUATORS
    static constexpr uint8_t PUMP_POS = 0;
    static constexpr uint8_t VALVE_POS = 1;

    WaterTank();
    ~WaterTank() = default;

    PlcState UpdateProcess(PlcState plcIn, const PlcState& plcOut) override;

private:
    static constexpr float s_tankWidth = 1;    // cross-sectional area of the tank
    static constexpr float s_pumpFlow = 0.1;   // 0.1 m/s = 10cm/s
    static constexpr float s_valveFlow = 0.05; // 0.05 m/s = 5cm/s

    double m_prevTime;
    AnalogSensor m_currHeight;
};

