#include "industrial-plant.h"

IndustrialPlant *IndustrialPlant::s_Instance = nullptr;

void
IndustrialPlant::ScheduleUpdate()
{
    if (s_Instance)
    {
        s_Instance->m_Step += s_Instance->m_Interval;
        ns3::Simulator::Schedule(s_Instance->m_Step - ns3::Simulator::Now(), &IndustrialPlant::DoUpdate, s_Instance);
    }
}

void
IndustrialPlant::SetRefreshRate(uint64_t rate)
{
    InitPlant();
    s_Instance->m_Interval = ns3::MilliSeconds(rate);
}

void
IndustrialPlant::RegisterPLC(PlcApplication *plc)
{
    InitPlant();
    s_Instance->m_Plcs.push_back(plc);
}

void
IndustrialPlant::RegisterProcess(IndustrialProcess *process)
{
    InitPlant();
    s_Instance->m_Processes.push_back(process);
}

void
IndustrialPlant::InitPlant()
{
    if(s_Instance)
        return;

    s_Instance = new IndustrialPlant();
    s_Instance->m_Interval = ns3::MilliSeconds(50);

    ScheduleUpdate();
}

void
IndustrialPlant::DoUpdate()
{
    /*
     * We can use another data structure that stays sorted when inserted
     * to avoid this step.
     */
    if (!m_Sorted)
        Sort();

    for (auto process : m_Processes)
        if (process) process->DoUpdate();

    for (auto plc : m_Plcs)
        if (plc) plc->DoUpdate();

    if (m_Processes.size() == 0 && m_Plcs.size() == 0)
        return;

    ScheduleUpdate();
}

void
IndustrialPlant::Sort()
{

    /*
     * Only Industrial Processes are sorted since PLC logics are independent
     */
    struct
    {
        bool operator()(IndustrialProcess* a, IndustrialProcess* b) const
        {
            if (a && b)
                return a->GetPriority() > b->GetPriority();

            return false;
        }
    } GreaterThan;

    std::sort(m_Processes.begin(), m_Processes.end(), GreaterThan);

    m_Sorted = true;
}

