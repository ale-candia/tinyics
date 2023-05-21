#include "plc-application.h"
#include "modbus.h"
#include "utils.h"

#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"

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
    m_port = 502;
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
        ns3::InetSocketAddress local = ns3::InetSocketAddress(ns3::Ipv4Address::GetAny(), m_port);
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
            // Inbound Modbus ADUs
            std::vector<ModbusADU> inboundAdus = ModbusADU::GetModbusADUs(packet);

            for (const ModbusADU& adu : inboundAdus)
            {

                // Copy incomming ADU head (Still need to change Lenght Field accordingly)
                ModbusADU::CopyBase(adu, m_modbusADU);

                uint8_t fc = m_modbusADU.GetFunctionCode();

                if (fc == MB_FunctionCode::ReadCoils)
                {
                    uint16_t startAddress = CombineUint8(adu.GetDataByte(0), adu.GetDataByte(1));
                    uint16_t numCoils = CombineUint8(adu.GetDataByte(2), adu.GetDataByte(3));

                    // 0 <= startAddres <= 7 (max address)
                    // 1 <= numCoils <= 8 (max amount of coils to read)
                    if (startAddress < 7 && 0 < numCoils && numCoils <= 8 - startAddress)
                    {
                        std::vector<uint8_t> data(2);
                        data[0] = 1; // Set bit count
                        data[1] = GetBitsInRange(startAddress, numCoils, m_out.digitalPorts);

                        m_modbusADU.SetData(data);
                        ns3::Ptr<ns3::Packet> p = m_modbusADU.ToPacket();
                        socket->SendTo(p, 0, from);
                    }

                }

                if (fc == MB_FunctionCode::ReadDiscreteInputs)
                {
                    uint16_t startAddress = CombineUint8(adu.GetDataByte(0), adu.GetDataByte(1));
                    uint16_t numInputs = CombineUint8(adu.GetDataByte(2), adu.GetDataByte(3));

                    // 0 <= startAddres <= 7 (max address)
                    // 1 <= numInputs <= 8 (max amount of digital inputs to read)
                    if (startAddress < 7 && 0 < numInputs && numInputs <= 8 - startAddress)
                    {
                        std::vector<uint8_t> data(2);
                        data[0] = 1; // Set bit count
                        data[1] = GetBitsInRange(startAddress, numInputs, m_in.digitalPorts);

                        m_modbusADU.SetData(data);
                        ns3::Ptr<ns3::Packet> p = m_modbusADU.ToPacket();
                        socket->SendTo(p, 0, from);
                    }
                }

                if (fc == MB_FunctionCode::ReadInputRegisters)
                {
                    uint16_t startAddress = CombineUint8(adu.GetDataByte(0), adu.GetDataByte(1));
                    uint16_t numInputs = CombineUint8(adu.GetDataByte(2), adu.GetDataByte(3));

                    // 0 <= startAddres <= 7 (max address)
                    // 1 <= numCoils <= 2 (max amount of coils to read)
                    if (startAddress < 7 && 0 < numInputs && numInputs <= 2)
                    {
                        std::vector<uint8_t> data(1 + 2 * numInputs);
                        data[0] = 2 * numInputs; // Set bit count

                        for (int i = 1; i < 2 * numInputs + 1; i+=2)
                        {
                            auto [higher, lower] = SplitUint16(m_in.analogPorts[i-1]);

                            data[i] = static_cast<uint8_t>(higher);
                            data[i + 1] = static_cast<uint8_t>(lower);
                        }

                        m_modbusADU.SetData(data);
                        ns3::Ptr<ns3::Packet> p = m_modbusADU.ToPacket();
                        socket->SendTo(p, 0, from);
                    }
                }

                // Exception 01
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

