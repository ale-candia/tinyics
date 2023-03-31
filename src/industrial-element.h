#pragma once
// #include "industrial-network-builder.h"

// enum IndustrialElementType {
//     PLC,
//     SCADA
// };

// class IndustrialElement {
// public:
//     IndustrialElement(IndustrialElementType type);

//     IndustrialElementType GetType();
//     void ConnectTo(IndustrialElement element);
// private:
//     void SetAddress(Address addr);
//     Address GetAddress();

//     IndustrialElementType m_type;
//     Ptr<Node> m_node;
//     Ptr<Application> m_industrialApplication;
//     Address m_address;

//     friend class IndustrialNetworkBuilder;
// };

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
