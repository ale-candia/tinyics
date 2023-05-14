#include "plc-application.h"
#include "modbus.h"

#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "utils.h"

using namespace ns3;

TypeId
PlcApplication::GetTypeId()
{
    static TypeId tid = TypeId("PlcApplication")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddAttribute("Port",
                                          "Port on which we listen for incoming packets.",
                                          UintegerValue(502),
                                          MakeUintegerAccessor(&PlcApplication::m_port),
                                          MakeUintegerChecker<uint16_t>());
    return tid;
}

PlcApplication::PlcApplication(const char* name) : IndustrialApplication(name)
{}

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
        if (InetSocketAddress::IsMatchingType(from))
        {
            // Inbound Modbus ADU
            std::vector<ModbusADU> inboundAdus = ModbusADU::GetModbusADUs(packet);

            for (const ModbusADU& adu : inboundAdus)
            {

                // Copy incomming ADU head (Still need to change Lenght Field accordingly)
                ModbusADU::CopyBase(adu, m_modbusADU);

                // process packet
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
                        Ptr<Packet> p = m_modbusADU.ToPacket();
                        socket->SendTo(p, 0, from);
                    }

                }

                if (fc == MB_FunctionCode::ReadDiscreteInputs)
                {
                    uint16_t startAddress = CombineUint8(adu.GetDataByte(0), adu.GetDataByte(1));
                    uint16_t numInputs = CombineUint8(adu.GetDataByte(2), adu.GetDataByte(3));

                    // 0 <= startAddres <= 7 (max address)
                    // 1 <= numCoils <= 8 (max amount of coils to read)
                    if (startAddress < 7 && 0 < numInputs && numInputs <= 8 - startAddress)
                    {
                        std::vector<uint8_t> data(2);
                        data[0] = 1; // Set bit count
                        data[1] = GetBitsInRange(startAddress, numInputs, m_in.digitalPorts);

                        m_modbusADU.SetData(data);
                        Ptr<Packet> p = m_modbusADU.ToPacket();
                        socket->SendTo(p, 0, from);
                    }
                }

                if (fc == MB_FunctionCode::ReadInputRegisters)
                {
                    uint16_t startAddress = CombineUint8(adu.GetDataByte(0), adu.GetDataByte(1));
                    uint16_t numInputs = CombineUint8(adu.GetDataByte(2), adu.GetDataByte(3));

                    if (numInputs > 2)
                    {
                        NS_FATAL_ERROR("[UNSUPPORTED] A maximum of 2 input registers can be read");
                    }

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
                        Ptr<Packet> p = m_modbusADU.ToPacket();
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

