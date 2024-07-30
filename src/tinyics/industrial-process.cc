#include "industrial-process.h"
#include "industrial-plant.h"

#include "ns3/nstime.h"

void
IndustrialProcess::DoUpdate()
{
    if (m_Measurements != nullptr && m_Input != nullptr)
    {
        UpdateProcess(m_Measurements, m_Input);
    }
    else
    {
        // throw fatal error
    }
}

void
IndustrialProcess::LinkPLC(uint8_t priority, PlcState *measurement, const PlcState *input)
{
    m_Priority = priority;
    m_Measurements = measurement;
    m_Input = input;

    IndustrialPlant::RegisterProcess(this);
}

uint8_t
IndustrialProcess::GetPriority() const
{
    return m_Priority;
}

