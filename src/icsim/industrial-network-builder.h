#pragma once

#include "plc-application.h"
#include "scada-application.h"

#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

// These are all values typically found in Ethernet/IP networks

// 1Gb/s
#define ETH_DATA_RATE ns3::DataRateValue(ns3::DataRate(8000000))

#define ETH_DELAY ns3::TimeValue(ns3::MicroSeconds(87))

#define ETH_MTU ns3::UintegerValue(1500)

/// Class used to manage and build networks
class IndustrialNetworkBuilder
{
public:
    /**
     * Create an instance of the network builder
     *
     * Sets the network and mask for the network and the DataRate,
     * Delay and MTU for the Ethernet channel
     */
    IndustrialNetworkBuilder(ns3::Ipv4Address network, ns3::Ipv4Mask mask);
    ~IndustrialNetworkBuilder() = default;

    /**
     * Add a new Industrial Application to the network
     *
     * This internally adds the node into the network even if though
     * an Application is passed to it.
     */
    void AddToNetwork(ns3::Ptr<IndustrialApplication> app);

    /**
     * Builds the network
     *
     * This gives each node in the network an IP Address it also
     * prints these values to the console.
     */
    void BuildNetwork();

    /// Enables capturing packets in a pcap file
    void EnablePcap(std::string prefix);

private:
    /// Get all nodes in the network using the ns3 NodeContainer data structure
    ns3::NodeContainer GetAllNodes();

    ns3::CsmaHelper m_csma;
    ns3::Ipv4AddressHelper m_ipv4Address;
    std::vector<ns3::Ptr<IndustrialApplication>> m_applications;
};

