#pragma once

#include "plc-application.h"

/*
 * This class represents the physics of the whole system.
 * It uses the singleton pattern, where s_Instance is the
 * unique instance of the class in the whole program.
 */
class IndustrialPlant
{
public:
    static void RegisterPLC(PlcApplication *plc);

    static void RegisterProcess(IndustrialProcess *process);

private:
    IndustrialPlant() = default;

    /*
     * Creates Industrial Plant and schedules it for updates
     */
    static void InitPlant();

    static inline void ScheduleUpdate(ns3::Time dt);

    void DoUpdate();

    void Sort();

    static IndustrialPlant *s_Instance;

    std::vector<IndustrialProcess*> m_Processes;
    std::vector<PlcApplication*> m_Plcs;
    ns3::Time m_Interval;
    ns3::Time m_Step = ns3::Seconds(0.0);
    bool m_Sorted;
};

