#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "industrial-plant.h"
#include "modbus.h"
#include "utils.h"

ns3::TypeId
PlcApplication::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("PlcApplication")
        .SetParent<Application>()
        .SetGroupName("Applications");

    return tid;
}

PlcApplication::PlcApplication(const char *name)
    : IndustrialApplication(name)
{
    IndustrialPlant::RegisterPLC(this);
}

PlcApplication::~PlcApplication()
{
    m_Socket = nullptr;
    m_IndustrialProcess = nullptr;
}

void
PlcApplication::DoDispose()
{
    Application::DoDispose();
}

void
PlcApplication::StartApplication()
{
    if (!m_Socket)
    {
        // Create TCP socket
        ns3::TypeId tid = ns3::TypeId::LookupByName("ns3::TcpSocketFactory");
        m_Socket = ns3::Socket::CreateSocket(GetNode(), tid);

        // Bind the socket to the local address
        ns3::InetSocketAddress local = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), s_Port);
        if (m_Socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
    }

    m_Socket->Listen();
    m_Socket->SetAcceptCallback(
        ns3::MakeNullCallback<bool, ns3::Ptr<ns3::Socket>, const ns3::Address &>(),
        MakeCallback(&PlcApplication::HandleAccept, this));
}

void
PlcApplication::StopApplication()
{
    if (m_Socket)
    {
        m_Socket->Close();
        m_Socket->SetRecvCallback(ns3::MakeNullCallback<void, ns3::Ptr<ns3::Socket>>());
    }
}

void
PlcApplication::HandleAccept(ns3::Ptr<ns3::Socket> s, const ns3::Address &from)
{
    s->SetRecvCallback(MakeCallback(&PlcApplication::HandleRead, this));
}

void
PlcApplication::HandleRead(ns3::Ptr<ns3::Socket> socket)
{
    ns3::Ptr<ns3::Packet> packet;
    ns3::Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        if (ns3::InetSocketAddress::IsMatchingType(from))
        {
            for (const ModbusADU &adu : ModbusADU::GetModbusADUs(packet))
            {
                MB_FunctionCode fc = adu.GetFunctionCode();

                if (fc == MB_FunctionCode::ReadCoils || fc == MB_FunctionCode::WriteSingleCoil)
                    RequestProcessor::Execute(fc, socket, from, adu, m_Out);
                else
                    RequestProcessor::Execute(fc, socket, from, adu, m_In);
            }
        }
    }
}

void
PlcApplication::LinkProcess(std::shared_ptr<IndustrialProcess> ip, uint8_t priority)
{
    m_IndustrialProcess = ip;
    m_IndustrialProcess->LinkPLC(priority, &m_In, &m_Out);
}

void
PlcApplication::DoUpdate()
{
    Update(&m_In, &m_Out);
}
