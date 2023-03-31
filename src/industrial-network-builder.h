#pragma once

#include "plc-application.h"
#include "scada-application.h"

#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"


class IndustrialNetworkBuilder
{
public:
    IndustrialNetworkBuilder(Ipv4Address network, Ipv4Mask mask);
    ~IndustrialNetworkBuilder() = default;

    void AddToNetwork(Ptr<IndustrialApplication> app);
    void BuildNetwork();

private:
    NodeContainer GetAllNodes();

    std::shared_ptr<CsmaHelper> m_csma;
    std::shared_ptr<Ipv4AddressHelper> m_ipv4Address;
    std::vector<Ptr<IndustrialApplication>> m_applications;
};
