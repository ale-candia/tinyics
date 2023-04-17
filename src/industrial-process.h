#pragma once

#include <iostream>

#include "ns3/simulator.h"
#include "ns3/nstime.h"

struct PlcState;

using namespace ns3;

enum IndustrialProcessType
{
    WATER_TANK,
    SEMAPHORE_LIGHT
};

class IndustrialProcess
{
public:
    virtual ~IndustrialProcess() = default;

    virtual void UpdateState(const PlcState& plcIn, PlcState& plcOut) = 0;
    virtual void UpdateProcess(PlcState& plcIn, const PlcState& plcOut) = 0;
};

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

    Time m_prevTime;
    float m_currHeight;
};

class SemaphoreLights : public IndustrialProcess
{
public:
    void UpdateState(const PlcState& plcIn, PlcState& plcOut) override;
    void UpdateProcess(PlcState& plcIn, const PlcState& plcOut) override;
};

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

