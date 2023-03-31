#include "plc-application.h"

#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/uinteger.h"

#include "ns3/ipv4-address-generator.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address-helper.h"

using namespace ns3;

TypeId
PlcApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("PlcApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<PlcApplication>()
            .AddAttribute("Port",
                          "Port on which we listen for incoming packets.",
                          UintegerValue(2222),
                          MakeUintegerAccessor(&PlcApplication::m_port),
                          MakeUintegerChecker<uint16_t>())
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&PlcApplication::m_rxTrace),
                            "Packet::TracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&PlcApplication::m_rxTraceWithAddresses),
                            "Packet::TwoAddressTracedCallback");
    return tid;
}

PlcApplication::PlcApplication()
{
    m_state = PlcState();
    this -> SetStartTime(Seconds(1.0));
    this -> SetStopTime(Seconds(10.0));
}

PlcApplication::~PlcApplication()
{
    m_socket = nullptr;
    m_industrialProcess = nullptr;
}

void
PlcApplication::DoDispose()
{
    Application::DoDispose();
}

void
PlcApplication::StartApplication()
{
    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
    }

    m_socket->SetRecvCallback(MakeCallback(&PlcApplication::HandleRead, this));
}

void
PlcApplication::StopApplication()
{
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
PlcApplication::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);

        if (InetSocketAddress::IsMatchingType(from))
        {
            if (m_industrialProcess)
            {
                m_industrialProcess -> UpdateState(m_state);

                // uint8_t i = 3;

                // Ptr<Packet> p = Create<Packet>(&i, sizeof(i));
                // socket->SendTo(p, 0, from);
            }
            else {
                NS_FATAL_ERROR("No industrial process specified for PLC: " << this -> GetInstanceTypeId().GetName());
            }
        }
    }
}

void
PlcApplication::LinkProcess(IndustrialProcessType ipType)
{
    if (m_industrialProcess)
    {
        NS_FATAL_ERROR("Industrial Process Already Specified for PLC(" << this << ')');
    }

    m_industrialProcess = IndustrialProcessFactory::Create(ipType);
}
