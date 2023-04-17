#include "industrial-network-builder.h"

IndustrialNetworkBuilder::IndustrialNetworkBuilder(Ipv4Address network, Ipv4Mask mask)
{
    // This are all values typically found in Ethernet/IP networks
    m_csma = std::make_shared<CsmaHelper>();
    m_csma -> SetChannelAttribute("DataRate", DataRateValue(DataRate(8000000)));     // 1Gb/s
    m_csma -> SetChannelAttribute("Delay", TimeValue(MicroSeconds(87)));
    m_csma -> SetDeviceAttribute("Mtu", UintegerValue(1500));
    m_csma -> EnablePcapAll("iiot-scenario", false);

    m_ipv4Address = std::make_shared<Ipv4AddressHelper>();
    m_ipv4Address -> SetBase(network, mask);
}

void
IndustrialNetworkBuilder::AddToNetwork(Ptr<IndustrialApplication> app)
{
    if (app->GetNode()) {
        std::clog << "Node already in the network\n"
            << "Node Info: \n"
            << "\tType => " << app->GetInstanceTypeId() << '\n'
            << "\tNodeId => " << app->GetNode()->GetId() << '\n';
        throw std::exception();
    }

    Ptr<Node> node = CreateObject<Node>();
    node->AddApplication(app);

    m_applications.push_back(app);
}

void
IndustrialNetworkBuilder::BuildNetwork()
{
    NodeContainer nodes = GetAllNodes();

    InternetStackHelper internet;
    internet.Install(nodes);
    NetDeviceContainer devices = m_csma -> Install(nodes);

    Ipv4InterfaceContainer i = m_ipv4Address -> Assign(devices);

    for (int app = 0; app < m_applications.size(); app++)
    {
        auto industrialApp = m_applications[app];
        industrialApp->SetAddress(i.GetAddress(app));
    }
}

NodeContainer
IndustrialNetworkBuilder::GetAllNodes()
{
    NodeContainer n;
    for (int i = 0; i < m_applications.size(); i++)
    {
        n.Add(m_applications[i]->GetNode());
    }

    return n;
}

void
IndustrialNetworkBuilder::EnablePcap(std::string filePrefix)
{
    if (m_csma)
    {
        m_csma->EnablePcapAll(filePrefix);
    }
}
