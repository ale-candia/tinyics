#include "industrial-network-builder.h"
#include "ns3/names.h"

IndustrialNetworkBuilder::IndustrialNetworkBuilder(ns3::Ipv4Address network, ns3::Ipv4Mask mask)
{
    m_csma = ns3::CsmaHelper();
    m_csma.SetChannelAttribute("DataRate", ETH_DATA_RATE);     // 1Gb/s
    m_csma.SetChannelAttribute("Delay", ETH_DELAY);
    m_csma.SetDeviceAttribute("Mtu", ETH_MTU);

    m_ipv4Address = ns3::Ipv4AddressHelper();
    m_ipv4Address.SetBase(network, mask);
}

void
IndustrialNetworkBuilder::AddToNetwork(ns3::Ptr<IndustrialApplication> app)
{
    if (app->GetNode()) {
        std::clog << "Node already in the network\n"
            << "Node Info: \n"
            << "\tType => " << app->GetInstanceTypeId() << '\n'
            << "\tNodeId => " << app->GetNode()->GetId() << '\n';
        throw std::exception();
    }

    ns3::Ptr<ns3::Node> node = ns3::CreateObject<ns3::Node>();
    node->AddApplication(app);
    ns3::Names::Add(app->GetName(), node);

    m_applications.push_back(app);
}

void
IndustrialNetworkBuilder::BuildNetwork()
{
    ns3::NodeContainer nodes = GetAllNodes();

    ns3::InternetStackHelper internet;
    internet.Install(nodes);
    ns3::NetDeviceContainer devices = m_csma.Install(nodes);

    ns3::Ipv4InterfaceContainer i = m_ipv4Address.Assign(devices);

    for (int app = 0; app < m_applications.size(); app++)
    {
        auto industrialApp = m_applications[app];
        industrialApp->SetAddress(i.GetAddress(app));

        std::clog << industrialApp->GetName() << ": " << industrialApp->GetAddress() << '\n';
    }
}

ns3::NodeContainer
IndustrialNetworkBuilder::GetAllNodes()
{
    ns3::NodeContainer n;
    for (int i = 0; i < m_applications.size(); i++)
    {
        n.Add(m_applications[i]->GetNode());
    }

    return n;
}

void
IndustrialNetworkBuilder::EnablePcap(std::string filePrefix)
{
    m_csma.EnablePcapAll(filePrefix);
}

