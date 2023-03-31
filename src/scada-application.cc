#include "scada-application.h"

#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

using namespace ns3;

TypeId
ScadaApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ScadaApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<ScadaApplication>()
            .AddAttribute("MaxPackets",
                          "The maximum number of packets the application will send",
                          UintegerValue(5),
                          MakeUintegerAccessor(&ScadaApplication::m_count),
                          MakeUintegerChecker<uint32_t>())
            .AddAttribute("Interval",
                          "The time to wait between packets",
                          TimeValue(Seconds(1.0)),
                          MakeTimeAccessor(&ScadaApplication::m_interval),
                          MakeTimeChecker())
            .AddAttribute("RemoteAddress",
                          "The destination Address of the outbound packets",
                          AddressValue(),
                          MakeAddressAccessor(&ScadaApplication::m_peerAddress),
                          MakeAddressChecker())
            .AddAttribute("RemotePort",
                          "The destination port of the outbound packets",
                          UintegerValue(2222),
                          MakeUintegerAccessor(&ScadaApplication::m_peerPort),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("PacketSize",
                          "Size of echo data in outbound packets",
                          UintegerValue(100),
                          MakeUintegerAccessor(&ScadaApplication::SetDataSize, &ScadaApplication::GetDataSize),
                          MakeUintegerChecker<uint32_t>())
            .AddTraceSource("Tx",
                            "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&ScadaApplication::m_txTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("Rx",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&ScadaApplication::m_rxTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("TxWithAddresses",
                            "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&ScadaApplication::m_txTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback")
            .AddTraceSource("RxWithAddresses",
                            "A packet has been received",
                            MakeTraceSourceAccessor(&ScadaApplication::m_rxTraceWithAddresses),
                            "ns3::Packet::TwoAddressTracedCallback");
    return tid;
}

ScadaApplication::ScadaApplication()
{
    m_sent = 0;
    m_socket = nullptr;
    // m_sendEvent = EventId();
    m_data = nullptr;
    m_dataSize = 0;
}

ScadaApplication::~ScadaApplication()
{
    m_socket = nullptr;

    delete[] m_data;
    m_data = nullptr;
    m_dataSize = 0;
}

void
ScadaApplication::SetRemote(Address ip, uint16_t port)
{
    m_peerAddress = ip;
    m_peerPort = port;
}

void
ScadaApplication::SetRemote(Address addr)
{
    m_peerAddress = addr;
}

void
ScadaApplication::DoDispose()
{
    Application::DoDispose();
}

void
ScadaApplication::StartApplication()
{

    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
            if (m_socket->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
        else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true)
        {
            if (m_socket->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(m_peerAddress);
        }
        else
        {
            NS_FATAL_ERROR("[ScadaApplication] Incompatible address type: " << m_peerAddress);
        }
    }

    m_socket->SetRecvCallback(MakeCallback(&ScadaApplication::HandleRead, this));
    m_socket->SetAllowBroadcast(true);
    ScheduleTransmit(Seconds(0.));
}

void
ScadaApplication::StopApplication()
{
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        m_socket = nullptr;
    }

    // Simulator::Cancel(m_sendEvent);
}

void
ScadaApplication::SetDataSize(uint32_t dataSize)
{
    //
    // If the client is setting the echo packet data size this way, we infer
    // that she doesn't care about the contents of the packet at all, so
    // neither will we.
    //
    delete[] m_data;
    m_data = nullptr;
    m_dataSize = 0;
    m_size = dataSize;
}

uint32_t
ScadaApplication::GetDataSize() const
{
    return m_size;
}

void
ScadaApplication::SetFill(std::string fill)
{
    uint32_t dataSize = fill.size() + 1;

    if (dataSize != m_dataSize)
    {
        delete[] m_data;
        m_data = new uint8_t[dataSize];
        m_dataSize = dataSize;
    }

    memcpy(m_data, fill.c_str(), dataSize);

    //
    // Overwrite packet size attribute.
    //
    m_size = dataSize;
}

void
ScadaApplication::SetFill(uint8_t fill, uint32_t dataSize)
{
    if (dataSize != m_dataSize)
    {
        delete[] m_data;
        m_data = new uint8_t[dataSize];
        m_dataSize = dataSize;
    }

    memset(m_data, fill, dataSize);

    //
    // Overwrite packet size attribute.
    //
    m_size = dataSize;
}

void
ScadaApplication::SetFill(uint8_t* fill, uint32_t fillSize, uint32_t dataSize)
{
    if (dataSize != m_dataSize)
    {
        delete[] m_data;
        m_data = new uint8_t[dataSize];
        m_dataSize = dataSize;
    }

    if (fillSize >= dataSize)
    {
        memcpy(m_data, fill, dataSize);
        m_size = dataSize;
        return;
    }

    //
    // Do all but the final fill.
    //
    uint32_t filled = 0;
    while (filled + fillSize < dataSize)
    {
        memcpy(&m_data[filled], fill, fillSize);
        filled += fillSize;
    }

    //
    // Last fill may be partial
    //
    memcpy(&m_data[filled], fill, dataSize - filled);

    //
    // Overwrite packet size attribute.
    //
    m_size = dataSize;
}

void
ScadaApplication::ScheduleTransmit(Time dt)
{
    // m_sendEvent = Simulator::Schedule(dt, &ScadaApplication::Send, this);
    Simulator::Schedule(dt, &ScadaApplication::Send, this);
}

void
ScadaApplication::Send()
{
    Ptr<Packet> p;
    if (m_dataSize)
    {
        //
        // If m_dataSize is non-zero, we have a data buffer of the same size that we
        // are expected to copy and send.  This state of affairs is created if one of
        // the Fill functions is called.  In this case, m_size must have been set
        // to agree with m_dataSize
        //
        p = Create<Packet>(m_data, m_dataSize);
    }
    else
    {
        //
        // If m_dataSize is zero, the client has indicated that it doesn't care
        // about the data itself either by specifying the data size by setting
        // the corresponding attribute or by not calling a SetFill function.  In
        // this case, we don't worry about it either.  But we do allow m_size
        // to have a value different from the (zero) m_dataSize.
        //
        p = Create<Packet>(m_size);
    }
    Address localAddress;
    m_socket->GetSockName(localAddress);
    // call to the trace sinks before the packet is actually sent,
    // so that tags added to the packet can be sent as well
    m_txTrace(p);
    if (Ipv4Address::IsMatchingType(m_peerAddress))
    {
        m_txTraceWithAddresses(
            p,
            localAddress,
            InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
    }
    m_socket->Send(p);
    ++m_sent;

#if 0
    if (Ipv4Address::IsMatchingType(m_peerAddress))
    {
        std::clog << "At time " << Simulator::Now().As(Time::S) << " client sent " << m_size
                               << " bytes to " << Ipv4Address::ConvertFrom(m_peerAddress)
                               << " port " << m_peerPort << std::endl;
    }
    else if (InetSocketAddress::IsMatchingType(m_peerAddress))
    {
        std::clog <<
            "At time " << Simulator::Now().As(Time::S) << " client sent " << m_size << " bytes to "
                       << InetSocketAddress::ConvertFrom(m_peerAddress).GetIpv4() << " port "
                       << InetSocketAddress::ConvertFrom(m_peerAddress).GetPort() << std::endl;
    }
#endif

    if (m_sent < m_count)
    {
        ScheduleTransmit(m_interval);
    }
}

/**
 * Here we should decode the incomming data to affect the ScadaApplication's state
 */
void
ScadaApplication::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
        if (InetSocketAddress::IsMatchingType(from))
        {
            uint32_t dataSize = packet->GetSize();
            if (dataSize > 0)
            {
                // uint8_t dataBuffer[dataSize];
                // packet->CopyData(dataBuffer, dataSize);
                // std::clog << "At time " << Simulator::Now().As(Time::S) << ": " << (int)dataBuffer[0] << '\n';
            }
        }
        socket->GetSockName(localAddress);
        m_rxTrace(packet);
        m_rxTraceWithAddresses(packet, from, localAddress);
    }
}
