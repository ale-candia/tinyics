#pragma once

#include "ns3/application.h"
#include "ns3/internet-module.h"

using namespace ns3;

class IndustrialApplication : public Application
{
public:
    inline Ipv4Address GetAddress() const {
        return m_appIpv4Address;
    }

protected:
    Ipv4Address m_appIpv4Address;

private:
    inline void SetAddress(Ipv4Address addr) {
        m_appIpv4Address = addr;
    }

    friend class IndustrialNetworkBuilder;
};
