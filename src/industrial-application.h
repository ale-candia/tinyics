#pragma once

#include "ns3/application.h"
#include "ns3/internet-module.h"

using namespace ns3;

class IndustrialApplication : public Application
{
public:
    IndustrialApplication(const char* name) : m_name(name) {}

    inline Ipv4Address GetAddress() const
    {
        return m_appIpv4Address;
    }

    inline std::string GetName() const
    {
        return m_name;
    }

protected:
    Ipv4Address m_appIpv4Address;

private:
    std::string m_name;

    inline void SetAddress(Ipv4Address addr) {
        m_appIpv4Address = addr;
    }

    friend class IndustrialNetworkBuilder;
};
