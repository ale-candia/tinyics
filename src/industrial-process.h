#pragma once

#include <iostream>
#include <stdint.h>

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
    ~IndustrialProcess() = default;

    virtual void UpdateState(PlcState& plcState) = 0;
};

class WaterTank : public IndustrialProcess
{
public:
    void UpdateState(PlcState& plcState)
    {
        std::clog << "Updating state [WaterTank - " << Simulator::Now().As(Time::S) << "]\n";
    }
};

class SemaphoreLights : public IndustrialProcess
{
public:
    void UpdateState(PlcState& plcState)
    {
        std::clog << "Updating state [SemaphoreLights - " << Simulator::Now().As(Time::S) << "]\n";
    }
};

class IndustrialProcessFactory
{
public:
    static std::shared_ptr<IndustrialProcess> Create(IndustrialProcessType ipType)
    {
        switch (ipType)
        {
            case WATER_TANK:
                return std::make_shared<WaterTank>();
            
            case SEMAPHORE_LIGHT:
                return std::make_shared<SemaphoreLights>();
            
            default:
                return nullptr;
        }
    }
};