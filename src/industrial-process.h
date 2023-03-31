#pragma once

#include <iostream>
#include <stdint.h>

struct PlcState;

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
        std::clog << "Updating WaterTank state\n";
    }
};

class SemaphoreLights : public IndustrialProcess
{
public:
    void UpdateState(PlcState& plcState)
    {
        std::clog << "Updating Semaphore Light state\n";
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