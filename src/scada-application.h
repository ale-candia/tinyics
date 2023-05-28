#pragma once

#include "industrial-application.h"
#include "modbus.h"
#include "modbus-command.h"
//#include "scada-state.h"
#include "plc-application.h"

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3
{
    class Socket;
    class Packet;
}

enum VarType
{
    Coil,
    DigitalInput,
    InputRegister,
    LocalVariable,
};

class Var
{
public:
    Var(VarType type, uint8_t pos)
        : m_type(type), m_pos(pos), m_value(0) {}

    Var(VarType type, uint8_t pos, uint16_t value)
        : m_type(type), m_pos(pos), m_value(value) {}

    VarType GetType() const { return m_type; }
    
    uint16_t GetValue() const { return m_value; }

    void SetValue(uint16_t val) { m_value = val; }
    
private:
    VarType m_type;
    uint16_t m_value;
    uint8_t m_pos;
};

/**
 * A ScadaApplication (acts as a Modbus TCP client)
 *
 * Every packet sent should be returned by the server and received here.
 */
class ScadaApplication : public IndustrialApplication
{
public:
    /**
     * Get the type ID.
     *
     * returns the object TypeId
     */
    static ns3::TypeId GetTypeId();

    ScadaApplication(const char* name);

    ~ScadaApplication() override;

    /**
     * Set the remote address
     *
     * \param addr remote address
     */
    void AddRTU(ns3::Ipv4Address addr);

    void AddVariable(const ns3::Ptr<PlcApplication>& plc, const std::string& name, VarType type, uint8_t pos);
    void AddVariable(const std::string& name, uint16_t value);

    void SetScadaLoop(std::function<void(std::map<std::string, Var>&)> loop);

protected:
    void DoDispose() override;

private:
    void StartApplication() override;
    void StopApplication() override;

    /**
     * Schedule the next packet transmission
     *
     * \param dt time interval between packets.
     */
    void ScheduleUpdate(ns3::Time dt);

    /// Send a packet to all connected devices
    void SendAll();

    void DoUpdate();

    /**
     * Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * \param socket the socket the packet was received to.
     */
    void HandleRead(ns3::Ptr<ns3::Socket> socket);
    void FreeSockets();

    ns3::Time m_interval;                           //!< Packet inter-send time
    std::vector<ns3::Ptr<ns3::Socket>> m_sockets;   //!< Socket per RTU
    std::vector<ns3::Address> m_peerAddresses;      //!< Address per RTU
    static constexpr uint16_t s_PeerPort = 502;     //!< Remote peer port
    bool m_started;                                 //!< Whether the app has already started
    uint16_t m_transactionId;                       //!< TransactionId for the Modbus ADU
    std::vector<std::map<MB_FunctionCode, Command>> m_Commands; //!< Commands to execute for each RTU
    std::map<std::string, Var> m_vars;

    std::function<void(std::map<std::string, Var>&)> m_loop;
};

