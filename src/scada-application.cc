#include "scada-application.h"

#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"

using namespace ns3;

TypeId
ScadaApplication::GetTypeId()
{
    static TypeId tid = TypeId("ScadaApplication")
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
                            .AddAttribute("RemotePort",
                                          "The destination port of the outbound packets",
                                          UintegerValue(502),
                                          MakeUintegerAccessor(&ScadaApplication::m_peerPort),
                                          MakeUintegerChecker<uint16_t>())
                            .AddAttribute("PacketSize",
                                          "Size of echo data in outbound packets",
                                          UintegerValue(100),
                                          MakeUintegerAccessor(&ScadaApplication::SetDataSize,
                                                               &ScadaApplication::GetDataSize),
                                          MakeUintegerChecker<uint32_t>());
    return tid;
}

ScadaApplication::ScadaApplication()
{
    m_sent = 0;
    FreeSockets();
    m_data = nullptr;
    m_dataSize = 0;
    m_started = false;
}

ScadaApplication::~ScadaApplication()
{
    FreeSockets();

    delete[] m_data;
    m_data = nullptr;
    m_dataSize = 0;
}

void
ScadaApplication::FreeSockets()
{
    for (auto socket : m_sockets)
    {
        socket = nullptr;
    }
}

void
ScadaApplication::AddRemote(Address ip, uint16_t port)
{
    m_peerAddresses.push_back(ip);
    m_peerPort = port;
}

void
ScadaApplication::AddRemote(Address addr)
{
    m_peerAddresses.push_back(addr);
}

void
ScadaApplication::DoDispose()
{
    Application::DoDispose();
}

void
ScadaApplication::StartApplication()
{
    if (m_started)
    {
        std::cerr << "[WARNING] SCADA application was attempted to start twice\n";
        return;
    }
    m_started = true;

    for (Address address : m_peerAddresses)
    {
        TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
        Ptr<Socket> socket = Socket::CreateSocket(GetNode(), tid);
        if (Ipv4Address::IsMatchingType(address) == true)
        {
            if (socket->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(address), m_peerPort));
        }
        else if (InetSocketAddress::IsMatchingType(address) == true)
        {
            if (socket->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            socket->Connect(address);
        }
        else
        {
            NS_FATAL_ERROR("[ScadaApplication] Incompatible address type: " << address);
        }

        socket->SetRecvCallback(MakeCallback(&ScadaApplication::HandleRead, this));
        socket->SetAllowBroadcast(true);

        m_sockets.push_back(socket);
    }

    //ScheduleTransmit(Seconds(0.));
}

void
ScadaApplication::StopApplication()
{
    for (auto socket : m_sockets)
    {
        socket->Close();
        socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        socket = nullptr;
    }
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
    for (int i = 0; i < m_sockets.size(); i++)
    {
        Ptr<Socket> socket = m_sockets[i];
        Address address = m_peerAddresses[i];

        Address localAddress;
        socket->GetSockName(localAddress);
        socket->Send(p);
        ++m_sent;
    }

    if (m_sent < m_count * m_sockets.size())
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
                uint8_t dataBuffer[dataSize];
                packet->CopyData(dataBuffer, dataSize);
                std::clog << "At time " << Simulator::Now().As(Time::S) << ": "
                          << (int)dataBuffer[0] << '\n';
            }
        }
        socket->GetSockName(localAddress);
    }
}
