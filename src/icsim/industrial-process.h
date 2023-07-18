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

    void DoUpdate();
    
    // Returns the updated measurements from the PLC
    virtual void UpdateProcess(PlcState* plcIn, const PlcState* plcOut) = 0;

    void LinkPLC(uint8_t priority, PlcState* measurement, const PlcState* input);

    uint8_t GetPriority() const;

private:
    PlcState* m_Measurements;
    const PlcState* m_Input;
    uint8_t m_Priority = 0;
};

