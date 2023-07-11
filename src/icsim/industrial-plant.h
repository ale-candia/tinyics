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

private:
    IndustrialPlant() = default;

    /*
     * Creates Industrial Plant and schedules it for updates
     */
    static void InitPlant();

    static inline void ScheduleUpdate(ns3::Time dt);

    void DoUpdate();

    static IndustrialPlant *s_Instance;

    std::vector<PlcApplication*> m_Plcs;
    ns3::Time m_interval;
    ns3::Time m_step = ns3::Seconds(0.0);
};

