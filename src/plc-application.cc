#include "plc-application.h"

#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"

using namespace ns3;

TypeId
PlcApplication::GetTypeId()
{
    static TypeId tid = TypeId("PlcApplication")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<PlcApplication>()
                            .AddAttribute("Port",
                                          "Port on which we listen for incoming packets.",
                                          UintegerValue(502),
                                          MakeUintegerAccessor(&PlcApplication::m_port),
                                          MakeUintegerChecker<uint16_t>());
    return tid;
}

PlcApplication::PlcApplication()
{
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
    m_stopTime = Seconds(20.0);

    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
    }

    m_socket->Listen();
    m_socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address&>(),
                                MakeCallback(&PlcApplication::HandleAccept, this));

    ScheduleUpdate(Seconds(0.));
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
PlcApplication::HandleAccept(Ptr<Socket> s, const Address& from)
{
    s->SetRecvCallback(MakeCallback(&PlcApplication::HandleRead, this));
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

        if (InetSocketAddress::IsMatchingType(from))
        {
            if (m_industrialProcess)
            {
                // Send state to SCADA
                // uint8_t i = 3;

                // Ptr<Packet> p = Create<Packet>(&i, sizeof(i));
                // socket->SendTo(p, 0, from);
            }
            else
            {
                NS_FATAL_ERROR("No industrial process specified for PLC: "
                               << this->GetInstanceTypeId().GetName());
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

void
PlcApplication::UpdateOutput()
{
    // Join these two together in industrial process and only make one call from here
    m_industrialProcess->UpdateProcess(m_in, m_out);
    m_industrialProcess->UpdateState(m_in, m_out);

    if (Simulator::Now() < m_stopTime)
    {
        ScheduleUpdate(Seconds(1.0));
    }
}

void
PlcApplication::ScheduleUpdate(Time dt)
{
    Simulator::Schedule(dt, &PlcApplication::UpdateOutput, this);
}
