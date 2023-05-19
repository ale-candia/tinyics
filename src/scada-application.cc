#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"

#include "scada-application.h"

ns3::TypeId
ScadaApplication::GetTypeId()
{
    static ns3::TypeId tid = ns3::TypeId("ScadaApplication")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddAttribute("Interval",
                                          "The time to wait between packets",
                                          ns3::TimeValue(ns3::Seconds(1.0)),
                                          MakeTimeAccessor(&ScadaApplication::m_interval),
                                          ns3::MakeTimeChecker())
                            .AddAttribute("RemotePort",
                                          "The destination port of the outbound packets",
                                          ns3::UintegerValue(502),
                                          MakeUintegerAccessor(&ScadaApplication::m_peerPort),
                                          ns3::MakeUintegerChecker<uint16_t>());
    return tid;
}

ScadaApplication::ScadaApplication(const char* name) : IndustrialApplication(name)
{
    FreeSockets();
    m_started = false;
}

ScadaApplication::~ScadaApplication()
{
    FreeSockets();
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
ScadaApplication::AddRTU(ns3::Address ip, uint16_t port)
{
    m_peerAddresses.push_back(ip);
    m_peerPort = port;
}

void
ScadaApplication::AddRTU(ns3::Address addr)
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
    m_stopTime = ns3::Seconds(20.0);

    // Setup communication with each remote terminal unit
    for (ns3::Address address : m_peerAddresses)
    {
        // Create a socket
        ns3::TypeId tid = ns3::TypeId::LookupByName("ns3::TcpSocketFactory");
        ns3::Ptr<ns3::Socket> socket = ns3::Socket::CreateSocket(GetNode(), tid);

        if (ns3::Ipv4Address::IsMatchingType(address) == true)
        {
            if (socket->Bind() == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            socket->Connect(ns3::InetSocketAddress(ns3::Ipv4Address::ConvertFrom(address), m_peerPort));
        }
        else
        {
            NS_FATAL_ERROR("[ScadaApplication] Incompatible address type: " << address);
        }

        socket->SetRecvCallback(MakeCallback(&ScadaApplication::HandleRead, this));
        socket->SetAllowBroadcast(true);

        m_sockets.push_back(socket);
    }

    ScheduleUpdate(ns3::Seconds(0.));
}

void
ScadaApplication::StopApplication()
{
    for (auto socket : m_sockets)
    {
        socket->Close();
        socket->SetRecvCallback(ns3::MakeNullCallback<void, ns3::Ptr<ns3::Socket>>());
        socket = nullptr;
    }
}

void
ScadaApplication::SetFill(uint8_t unitId, MB_FunctionCode fc, std::vector<uint16_t> data)
{
    m_modBusADU.SetTransactionID(m_transactionId);
    m_transactionId++;

    m_modBusADU.SetUnitID(unitId);

    m_modBusADU.SetFunctionCode(fc);

    m_modBusADU.SetData<uint16_t>(data);
}

void ScadaApplication::ScheduleUpdate(ns3::Time dt)
{
    ns3::Simulator::Schedule(dt, &ScadaApplication::DoUpdate, this);
}

void
ScadaApplication::SendAll()
{
    for (int i = 0; i < m_sockets.size(); i++)
    {
        // Once the API is implemented
        ns3::Ptr<ns3::Socket> socket = m_sockets[i];
        ns3::Address address = m_peerAddresses[i];

        // Is there a way to do this without a raw pointer?
        ScadaReadings *readconfigs;
        try
        {
            readconfigs = &m_readConfigs.at(address);
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
            ns3::Ptr<ns3::Packet> p = m_modBusADU.ToPacket();
            socket->Send(p);
        }
        if (readconfigs->m_readings.numInRegs > 0 && !readconfigs->m_pendingInReg)
        {
            readconfigs->m_pendingInReg = true;

            data[0] = readconfigs->m_readings.startInRegs;
            data[1] = readconfigs->m_readings.numInRegs;
            SetFill(i+1, MB_FunctionCode::ReadInputRegisters, data);
            ns3::Ptr<ns3::Packet> p = m_modBusADU.ToPacket();
            socket->Send(p);
            socket->GetTxAvailable();
        }
        if (readconfigs->m_readings.numDiscreteIn > 0 && !readconfigs->m_pendingDiscreteIn)
        {
            readconfigs->m_pendingDiscreteIn = true;

            data[0] = readconfigs->m_readings.startDiscreteIn;
            data[1] = readconfigs->m_readings.numDiscreteIn;
            SetFill(i+1, MB_FunctionCode::ReadDiscreteInputs, data);
            ns3::Ptr<ns3::Packet> p = m_modBusADU.ToPacket();
            socket->Send(p);
        }
    }
}

/**
 * Here we should decode the incomming data to affect the ScadaApplication's state
 */
void
ScadaApplication::HandleRead(ns3::Ptr<ns3::Socket> socket)
{
    ns3::Ptr<ns3::Packet> packet;
    ns3::Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        if (ns3::InetSocketAddress::IsMatchingType(from))
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
                    readconfigs = &m_readConfigs.at(m_peerAddresses[adu.GetUnitID() - 1]);
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

    if (ns3::Simulator::Now() < m_stopTime)
    {
        ScheduleUpdate(m_interval);
    }
}

void
ScadaApplication::AddVariable(
    const ns3::Ptr<PlcApplication>& plc,
    std::string name,
    VarType type,
    uint8_t pos
){
    // Add the variable
    m_vars.insert(std::pair<std::string, Var>(name, Var(type, pos)));

    ns3::Address plcAddr = plc -> GetAddress();
    // Add read config if it doesn't exist
    if (m_readConfigs.find(plcAddr) == m_readConfigs.end())
    {
        m_readConfigs.insert(
            std::pair<ns3::Address, ScadaReadings>(plcAddr, ScadaReadingsConfig())
        );
    }

    // Update the Read Configurations
    if (type == VarType::Coil)
    {
        uint16_t start = m_readConfigs[plcAddr].m_readings.startCoil;
        uint16_t num = m_readConfigs[plcAddr].m_readings.numCoil;
        uint16_t end;

        if (num == 0 && start == 0)
            start = end = pos;

        else if (pos < start)
            start = pos;

        else if (pos > end)
            end = (pos > 7) ? 7 : pos;

        m_readConfigs[plcAddr].m_readings.startCoil = start;
        m_readConfigs[plcAddr].m_readings.numCoil = end - start + 1;
    }

    if (type == VarType::DigitalInput)
    {
        uint16_t start = m_readConfigs[plcAddr].m_readings.startDiscreteIn;
        uint16_t num = m_readConfigs[plcAddr].m_readings.numDiscreteIn;
        uint16_t end;

        if (num == 0 && start == 0)
            start = end = pos;

        else if (pos < start)
            start = pos;

        else if (pos > end)
            end = (pos > 7) ? 7 : pos;

        m_readConfigs[plcAddr].m_readings.startDiscreteIn = start;
        m_readConfigs[plcAddr].m_readings.numDiscreteIn = end - start + 1;
    }

    if (type == VarType::InputRegister)
    {
        uint16_t start = m_readConfigs[plcAddr].m_readings.startInRegs;
        uint16_t num = m_readConfigs[plcAddr].m_readings.numInRegs;
        uint16_t end;

        if (num == 0 && start == 0)
            start = end = pos;

        else if (pos < start)
            start = pos;

        else if (pos > end)
            end = (pos > 1) ? 1 : pos;

        m_readConfigs[plcAddr].m_readings.startInRegs = start;
        m_readConfigs[plcAddr].m_readings.numInRegs = end - start + 1;
    }
}

void
ScadaApplication::AddVariable(
    const ns3::Ptr<PlcApplication>& plc,
    std::string name,
    VarType type
){
    m_vars.insert(std::pair<std::string, Var>(name, Var(type, 0)));
}

