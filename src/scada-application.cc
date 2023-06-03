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

    m_ResponseProcessors.insert(std::pair(MB_FunctionCode::ReadCoils, std::make_shared<DigitalReadResponse>()));                                            
    m_ResponseProcessors.insert(std::pair(MB_FunctionCode::ReadDiscreteInputs, m_ResponseProcessors.at(MB_FunctionCode::ReadCoils)));
    m_ResponseProcessors.insert(std::pair(MB_FunctionCode::ReadInputRegisters, std::make_shared<RegisterReadResponse>()));
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
    m_ReadCommands.push_back(std::map<MB_FunctionCode, ReadCommand>());
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
        auto tid = ns3::TypeId::LookupByName("ns3::TcpSocketFactory");
        auto socket = ns3::Socket::CreateSocket(GetNode(), tid);

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
    std::vector<int> sock;
    for (int i = 0; i < m_sockets.size(); i++)
    {
        auto socket = m_sockets[i];

        for (const auto& command : m_ReadCommands[i])
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
    ns3::Ptr<ns3::Packet> packet;
    ns3::Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        if (ns3::InetSocketAddress::IsMatchingType(from))
        {
            for(const ModbusADU& adu : ModbusADU::GetModbusADUs(packet))
            {
                if (adu.GetFunctionCode() == MB_FunctionCode::WriteSingleCoil)
                    continue;

                auto type = Var::IntoVarType(adu.GetFunctionCode());
                auto idx = adu.GetUnitID() - 1;

                std::vector<Var*> vars;

                // No need to use variables used with different function codes
                for (auto& var : m_vars)
                {
                    if (type == var.second.GetType())
                        vars.push_back(&var.second);
                }

                uint16_t start = m_ReadCommands[idx].at(adu.GetFunctionCode()).GetStart();

                m_ResponseProcessors.at(adu.GetFunctionCode())->Execute(adu, vars, start);
            }

            // Execute writes
            if (m_WriteCommands.size() > 0)
            {
                for (const auto& command : m_WriteCommands)
                {
                    command.Execute(socket, m_transactionId);
                    m_transactionId++;
                }
                m_WriteCommands.clear();
            }
        }
    }

}

void
ScadaApplication::DoUpdate()
{
    // Update all statuses
    if (m_loop)
        m_loop(m_vars);

    // Create write commands if needed
    // TODO: WE NEED TO MAKE SURE INPUT VARIABLES (MEASUREMENTS) ARE NOT BEING CHANGED
    for (auto& var : m_vars)
    {
        if (var.second.Changed() && var.second.GetType() == VarType::Coil)
        {
            m_WriteCommands.emplace_back(WriteCommand(
                MB_FunctionCode::WriteSingleCoil,
                var.second.GetPosition(),
                var.second.GetValue(),
                var.second.GetUID()
            ));
        }
        var.second.SetChanged(false);
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
    // Map VarType to the corresponding modbus function code
    MB_FunctionCode fc = Var::IntoFCRead(type);

    int idx = GetRTUIndex(plc->GetAddress());

    // If the variable is already registered then, ignore this one
    if (m_vars.find(name) != m_vars.end())
    {
        std::clog << "Variable " << name << " already defined, ignoring all future instances\n";
        return;
    }

    m_vars.insert(std::pair(name, Var(type, pos, idx+1)));

    /* Build a command to send the appropriate request */

    auto& commandMap = m_ReadCommands[idx];

    // If there isn't a command for the Function Code add it
    if (commandMap.find(fc) == commandMap.end())
    {
        commandMap.insert(std::pair(fc, ReadCommand(fc, 0, 0)));
    }

    commandMap.at(fc).SetReadCount(pos);
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
    m_vars.insert(std::pair(name, Var(VarType::LocalVariable, value)));
}

int
ScadaApplication::GetRTUIndex(const ns3::Address& rtuAddr)
{
    auto it = std::find(m_peerAddresses.begin(), m_peerAddresses.end(), rtuAddr);
    if (it == m_peerAddresses.end())
        NS_FATAL_ERROR("No RTU with address: '" << rtuAddr << "' in SCADA '" << GetName() << '\'');

    return it - m_peerAddresses.begin();
}

