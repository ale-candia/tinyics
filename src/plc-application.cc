#include "plc-application.h"
#include "modbus.h"
#include "utils.h"

#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

ns3::TypeId
PlcApplication::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("PlcApplication")
        .SetParent<Application>()
        .SetGroupName("Applications");

    return tid;
}

PlcApplication::PlcApplication(const char* name) : IndustrialApplication(name)
{
    // Setup Modbus Responses (Can also be created dynamically/on demand if needed)
    m_RequestProcessors.insert(std::pair(
        MB_FunctionCode::ReadCoils,
        std::make_shared<DigitalReadRequest>(DigitalReadRequest())
    ));
    m_RequestProcessors.insert(std::pair(
        MB_FunctionCode::ReadDiscreteInputs,
        m_RequestProcessors.at(MB_FunctionCode::ReadCoils)
    ));
    m_RequestProcessors.insert(std::pair(
        MB_FunctionCode::ReadInputRegisters,
        std::make_shared<ReadRegistersRequest>(ReadRegistersRequest())
    ));
    m_RequestProcessors.insert(std::pair(
        MB_FunctionCode::WriteSingleCoil,
        std::make_shared<WriteCoilRequest>(WriteCoilRequest())
    ));
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
    m_stopTime = ns3::Seconds(20.0);

    if (!m_socket)
    {
        // Create TCP socket
        ns3::TypeId tid = ns3::TypeId::LookupByName("ns3::TcpSocketFactory");
        m_socket = ns3::Socket::CreateSocket(GetNode(), tid);

        // Bind the socket to the local address
        ns3::InetSocketAddress local = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), s_port);
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
    }

    m_socket->Listen();
    m_socket->SetAcceptCallback(
        ns3::MakeNullCallback<bool, ns3::Ptr<ns3::Socket>,
        const ns3::Address&>(),
        MakeCallback(&PlcApplication::HandleAccept, this)
    );

    ScheduleUpdate(ns3::Seconds(0.));
}

void
PlcApplication::StopApplication()
{
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(ns3::MakeNullCallback<void, ns3::Ptr<ns3::Socket>>());
    }
}

void
PlcApplication::HandleAccept(ns3::Ptr<ns3::Socket> s, const ns3::Address& from)
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
            for (const ModbusADU& adu : ModbusADU::GetModbusADUs(packet))
            {
                MB_FunctionCode fc = adu.GetFunctionCode();

                if (fc == MB_FunctionCode::ReadCoils || fc == MB_FunctionCode::WriteSingleCoil)
                {
                    if (m_RequestProcessors.at(fc))
                        m_RequestProcessors.at(fc)->Execute(socket, from, adu, m_out);
                }
                else
                    m_RequestProcessors.at(fc)->Execute(socket, from, adu, m_in);
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
    if (m_industrialProcess)
    {
        // Join these two together in industrial process and only make one call from here
        m_industrialProcess->UpdateProcess(m_in, m_out);
        m_industrialProcess->UpdateState(m_in, m_out);
    }
    else
    {
        NS_FATAL_ERROR("No industrial process specified for PLC: " << this->GetInstanceTypeId().GetName());
    }
    if (ns3::Simulator::Now() < m_stopTime)
    {
        ScheduleUpdate(ns3::Seconds(1.0));
    }
}

void
PlcApplication::ScheduleUpdate(ns3::Time dt)
{
    ns3::Simulator::Schedule(dt, &PlcApplication::UpdateOutput, this);
}

