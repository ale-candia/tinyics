#pragma once

#include "ns3/application.h"
#include "ns3/internet-module.h"

/**
 * Base class for industrial applications.
 *
 * Forces industrial applications to have a name and keep their
 * address in IPv4, this address is only set by the
 * IndustrialNetworkBuilder.
 */
class IndustrialApplication : public ns3::Application
{
public:
    IndustrialApplication(const char* name) : m_name(name) {}

    inline ns3::Ipv4Address GetAddress() const
    {
        return m_appIpv4Address;
    }

    inline std::string GetName() const
    {
        return m_name;
    }

protected:
    ns3::Ipv4Address m_appIpv4Address;

private:
    std::string m_name;

    inline void SetAddress(ns3::Ipv4Address addr) {
        m_appIpv4Address = addr;
    }

    friend class IndustrialNetworkBuilder;
};
