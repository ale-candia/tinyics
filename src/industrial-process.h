#pragma once

#include <iostream>

#include "ns3/simulator.h"
#include "ns3/nstime.h"

struct PlcState;

enum IndustrialProcessType
{
    WATER_TANK,
    SEMAPHORE_LIGHT
};

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

    
    /// Updates the state of the PLC 
    virtual void UpdateState(const PlcState& plcIn, PlcState& plcOut) = 0;

    /// Updates the state of the Process 
    virtual void UpdateProcess(PlcState& plcIn, const PlcState& plcOut) = 0;
};

// Positions for the sensors and actuators in the WaterTank process

// SENSORS
#define WT_LEVEL1 0x01
#define WT_LEVEL2 0x02

// ACTUATORS
#define WT_PUMP 0x01
#define WT_VALVE 0x02

/**
 * A Water Tank system
 *
 * It contains two level sensors, a pump and a valve
 */
class WaterTank : public IndustrialProcess
{
public:
    ~WaterTank() = default;
    void UpdateState(const PlcState& plcIn, PlcState& plcOut) override;
    void UpdateProcess(PlcState& plcIn, const PlcState& plcOut) override;

private:
    static constexpr float s_tankWidth = 1; // cross-sectional area of the tank
    static constexpr float s_pumpFlow = 0.1; // 0.1 m/s = 10cm/s
    static constexpr float s_valveFlow = 0.05; // 0.05 m/s = 5cm/s
    static constexpr float s_level1 = 0.2; // lower level sensor height
    static constexpr float s_level2 = 0.5; // higher level sensor height

    ns3::Time m_prevTime;
    float m_currHeight;
};

class SemaphoreLights : public IndustrialProcess
{
public:
    void UpdateState(const PlcState& plcIn, PlcState& plcOut) override;
    void UpdateProcess(PlcState& plcIn, const PlcState& plcOut) override;
};

/// Factory to build the default Industrial Processes
class IndustrialProcessFactory
{
public:
    static std::unique_ptr<IndustrialProcess> Create(IndustrialProcessType ipType)
    {
        switch (ipType)
        {
            case WATER_TANK:
                return std::make_unique<WaterTank>();
            
            case SEMAPHORE_LIGHT:
                return std::make_unique<SemaphoreLights>();
            
            default:
                return nullptr;
        }
    }
};

