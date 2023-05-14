#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"

#include "scada-application.h"

using namespace ns3;

TypeId
ScadaApplication::GetTypeId()
{
    static TypeId tid = TypeId("ScadaApplication")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddAttribute("Interval",
                                          "The time to wait between packets",
                                          TimeValue(Seconds(1.0)),
                                          MakeTimeAccessor(&ScadaApplication::m_interval),
                                          MakeTimeChecker())
                            .AddAttribute("RemotePort",
                                          "The destination port of the outbound packets",
                                          UintegerValue(502),
                                          MakeUintegerAccessor(&ScadaApplication::m_peerPort),
                                          MakeUintegerChecker<uint16_t>());
    return tid;
}

ScadaApplication::ScadaApplication(const char* name) : IndustrialApplication(name)
{
    FreeSockets();
    //m_data = nullptr;
    m_started = false;
}

ScadaApplication::~ScadaApplication()
{
    FreeSockets();

    //delete[] m_data;
    //m_data = nullptr;
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
ScadaApplication::AddRTU(Address ip, uint16_t port)
{
    m_peerAddresses.push_back(ip);
    m_peerPort = port;
}

void
ScadaApplication::AddRTU(Address addr)
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
    m_stopTime = Seconds(20.0);

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

    ScheduleUpdate(Seconds(0.));
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
ScadaApplication::SetFill(uint8_t unitID)
{
    m_ModBusADU.SetTransactionID(m_transactionId);
    m_transactionId++;

    m_ModBusADU.SetUnitID(unitID);

    m_ModBusADU.SetFunctionCode(MB_FunctionCode::ReadCoils);
}

void
ScadaApplication::SetFill(uint8_t unitId, MB_FunctionCode fc, std::vector<uint16_t> data)
{
    m_ModBusADU.SetTransactionID(m_transactionId);
    m_transactionId++;

    m_ModBusADU.SetUnitID(unitId);

    m_ModBusADU.SetFunctionCode(fc);

    m_ModBusADU.SetData<uint16_t>(data);
}

void ScadaApplication::ScheduleUpdate(Time dt)
{
    // m_sendEvent = Simulator::Schedule(dt, &ScadaApplication::Send, this);
    //Simulator::Schedule(dt, &ScadaApplication::SendAll, this);
    Simulator::Schedule(dt, &ScadaApplication::DoUpdate, this);
}

void
ScadaApplication::SendAll()
{
    for (int i = 0; i < m_sockets.size(); i++)
    {
        // Once the API is implemented
        Ptr<Socket> socket = m_sockets[i];
        Address address = m_peerAddresses[i];

        // Is there a way to do this without a raw pointer?
        ScadaReadings *readconfigs;
        try
        {
            readconfigs = &m_ReadConfigs.at(address);
        }
        catch (std::out_of_range exception)
        {
            continue;
        }

        std::vector<uint16_t> data(2);

        if (readconfigs->m_readings.numCoil > 0 && !readconfigs->m_pendingCoils)
        {
            readconfigs->m_pendingCoils = true;

            data[0] = readconfigs->m_readings.startCoil;
            data[1] = readconfigs->m_readings.numCoil;
            SetFill(i+1, MB_FunctionCode::ReadCoils, data);
            Ptr<Packet> p = m_ModBusADU.ToPacket();
            socket->Send(p);
        }
        if (readconfigs->m_readings.numInRegs > 0 && !readconfigs->m_pendingInReg)
        {
            readconfigs->m_pendingInReg = true;

            data[0] = readconfigs->m_readings.startInRegs;
            data[1] = readconfigs->m_readings.numInRegs;
            SetFill(i+1, MB_FunctionCode::ReadInputRegisters, data);
            Ptr<Packet> p = m_ModBusADU.ToPacket()->CreateFragment(0, m_ModBusADU.GetBufferSize());
            socket->Send(p);
            socket->GetTxAvailable();
        }
        if (readconfigs->m_readings.numDiscreteIn > 0 && !readconfigs->m_pendingDiscreteIn)
        {
            readconfigs->m_pendingDiscreteIn = true;

            data[0] = readconfigs->m_readings.startDiscreteIn;
            data[1] = readconfigs->m_readings.numDiscreteIn;
            SetFill(i+1, MB_FunctionCode::ReadDiscreteInputs, data);
            Ptr<Packet> p = m_ModBusADU.ToPacket();
            socket->Send(p);
        }
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
    while ((packet = socket->RecvFrom(from)))
    {
        if (InetSocketAddress::IsMatchingType(from))
        {
            // Inbound Modbus ADU
            std::vector<ModbusADU> inboundADUs = ModbusADU::GetModbusADUs(packet);

            for(const ModbusADU& adu : inboundADUs)
            {

                // Is there a way to do this without a raw pointer?
                ScadaReadings *readconfigs;
                try
                {
                    // Use the unit identifier to retrieve address, since the from
                    // address has lost the Address format
                    readconfigs = &m_ReadConfigs.at(m_peerAddresses[adu.GetUnitID() - 1]);
                }
                catch (std::out_of_range exception)
                {
                    continue;
                }
                // process packet
                uint8_t fc = adu.GetFunctionCode();

                if (fc == MB_FunctionCode::ReadCoils)
                {
                    readconfigs->m_pendingCoils = false;
                }

                else if (fc == MB_FunctionCode::ReadInputRegisters)
                {
                    readconfigs->m_pendingInReg = false;
                }

                else if (fc == MB_FunctionCode::ReadDiscreteInputs)
                {
                    readconfigs->m_pendingDiscreteIn = false;
                }
            }
        }
    }

}

void
ScadaApplication::DoUpdate()
{
    // Update all statuses


    // Send Data
    SendAll();

    if (Simulator::Now() < m_stopTime)
    {
        ScheduleUpdate(m_interval);
    }
}

void
ScadaApplication::SetReadConfigForPlc(
    Ptr<PlcApplication> plc,
    std::tuple<uint16_t, uint16_t> coils,
    std::tuple<uint16_t, uint16_t> discreteIn,
    std::tuple<uint16_t, uint16_t> inputReg
){
    ScadaReadingsConfig readConfig;
    readConfig.startCoil = std::get<0>(coils);
    readConfig.numCoil = std::get<1>(coils);

    readConfig.startDiscreteIn = std::get<0>(discreteIn);
    readConfig.numDiscreteIn = std::get<1>(discreteIn);

    readConfig.startInRegs = std::get<0>(inputReg);
    readConfig.numInRegs = std::get<1>(inputReg);

    m_ReadConfigs.insert( std::pair<Address, ScadaReadings>(plc->GetAddress(), readConfig));
}

