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
                            .SetGroupName("Applications");
    return tid;
}

ScadaApplication::ScadaApplication(const char* name) : IndustrialApplication(name)
{
    m_started = false;
    m_interval = ns3::Seconds(1.0);
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
ScadaApplication::AddRTU(ns3::Ipv4Address addr)
{
    m_peerAddresses.push_back(addr);
    m_Commands.push_back(std::map<MB_FunctionCode, Command>());
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
    for (const ns3::Address& address : m_peerAddresses)
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
            socket->Connect(ns3::InetSocketAddress(ns3::Ipv4Address::ConvertFrom(address), s_PeerPort));
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
        const ns3::Address& address = m_peerAddresses[i];

        for (const auto& command : m_Commands[i])
        {
            command.second.Execute(socket, m_transactionId, i+1);
            m_transactionId++;
        }

    }
}

/**
 * Here we should decode the incomming data to affect the ScadaApplication's state
 */
void
ScadaApplication::HandleRead(ns3::Ptr<ns3::Socket> socket)
{
    /*
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
                try
                {
                    // Use the unit identifier to retrieve address, since the from
                    // address has lost the Address format
                    ScadaReadings& readconfigs = m_readConfigs.at(m_peerAddresses[adu.GetUnitID() - 1]);

                    // process packet
                    uint8_t fc = adu.GetFunctionCode();

                    if (fc == MB_FunctionCode::ReadCoils)
                    {
                        readconfigs.m_pendingCoils = false;
                    }

                    else if (fc == MB_FunctionCode::ReadInputRegisters)
                    {
                        readconfigs.m_pendingInReg = false;
                    }

                    else if (fc == MB_FunctionCode::ReadDiscreteInputs)
                    {
                        readconfigs.m_pendingDiscreteIn = false;
                    }
                }
                catch (std::out_of_range exception)
                {
                    continue;
                }
            }
        }
    }
    */

}

void
ScadaApplication::DoUpdate()
{
    // Update all statuses
    if (m_loop)
    {
        m_loop(m_vars);
    }

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
    const std::string& name,
    VarType type,
    uint8_t pos
){
    // Add the variable
    m_vars.insert(std::pair<std::string, Var>(name, Var(type, pos)));

    const ns3::Address& plcAddr = plc -> GetAddress();

    // Cannot add a Command to an unregistered PLC (Here we could add it internally)
    auto it = std::find(m_peerAddresses.begin(), m_peerAddresses.end(), plcAddr);
    if (it == m_peerAddresses.end())
        NS_FATAL_ERROR("RTU '" << plc->GetName() << "' is not register in '" << GetName() << '\'');

    int idx = it - m_peerAddresses.begin();

    auto& commandMap = m_Commands[idx];

    // Map VarType to the corresponding modbus function code
    MB_FunctionCode fc;
    switch (type)
    {
        case VarType::Coil:
            fc = MB_FunctionCode::ReadCoils;
            break;
        case VarType::DigitalInput:
            fc = MB_FunctionCode::ReadDiscreteInputs;
            break;
        case VarType::InputRegister:
            fc = MB_FunctionCode::ReadInputRegisters;
            break;
        case VarType::LocalVariable:
            return; // If its a local variable we don't need to add a command
    }

    // If there isn't a command for the Function Code Add it
    if (commandMap.find(fc) == commandMap.end())
    {
        commandMap.insert(std::pair<MB_FunctionCode, Command>(
            fc,
            Command(fc, 0, 0)
        ));
    }

    commandMap.at(fc).SetByteCount(pos);
}

void
ScadaApplication::SetScadaLoop(std::function<void(std::map<std::string, Var>&)> loop)
{
    m_loop = loop;
}

void
ScadaApplication::AddVariable(
    const std::string& name,
    uint16_t value = 0
){
    m_vars.insert(std::pair<std::string, Var>(name, Var(VarType::LocalVariable, 0, value)));
}

