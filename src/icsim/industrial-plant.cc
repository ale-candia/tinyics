#include "industrial-plant.h"

IndustrialPlant *IndustrialPlant::s_Instance = nullptr;

void
IndustrialPlant::ScheduleUpdate(ns3::Time dt)
{
    ns3::Simulator::Schedule(dt, &IndustrialPlant::DoUpdate, s_Instance);
}

void
IndustrialPlant::RegisterPLC(PlcApplication *plc)
{
    InitPlant();
    s_Instance->m_Plcs.push_back(plc);
}

void
IndustrialPlant::InitPlant()
{
    if(s_Instance)
        return;

    s_Instance = new IndustrialPlant();
    s_Instance->m_interval = ns3::Seconds(0.5);

    ScheduleUpdate(ns3::Seconds(0.));
}

void
IndustrialPlant::DoUpdate()
{
    for (auto plc : m_Plcs)
        if (plc) plc->DoUpdate();

    if (ns3::Simulator::Now() < ns3::Seconds(20.0))
    {
        m_step += m_interval;
        ScheduleUpdate(m_step - ns3::Simulator::Now());
    }
}

