#pragma once

#include "industrial-application.h"
#include "modbus-command.h"
#include "modbus-response.h"
#include "plc-application.h"

#include "ns3/application.h"
#include "ns3/ipv4-address.h"

namespace ns3
{
    class Socket;
    class Packet;
}

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

    ScadaApplication(const char* name, double rate = 500);

    ~ScadaApplication() override;

    /**
     * Set the remote address
     *
     * \param addr remote address
     */
    void AddRTU(ns3::Ipv4Address addr);

    void AddVariable(const ns3::Ptr<PlcApplication>& plc, const std::string& name, VarType type, uint8_t pos);

    /*
     * Run the update/logic of the SCADA
     *
     * Is expected to be overwritten, if not SCADA is used in read only mode if variables were defined
     */
    virtual void Update(const std::map<std::string, Var>& vars) {}
    void Write(const std::map<std::string, uint16_t>& vars);

    void SetRefreshRate(uint64_t rate);

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
    void ScheduleRead();

    /**
     * Get index for the RTU in the Address vector. Crashes if not found
     *
     * \returns int index of the RTU
     */
    int GetRTUIndex(const ns3::Address& rtuAddr);

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

    ns3::Time m_Interval;                           //!< Packet inter-send time
    ns3::Time m_Step;                           //!< Packet inter-send time
    std::vector<ns3::Ptr<ns3::Socket>> m_Sockets;   //!< Socket per RTU
    std::vector<ns3::Address> m_PeerAddresses;      //!< Address per RTU
    uint16_t m_TransactionId;                       //!< TransactionId for the Modbus ADU
    uint16_t m_PendingPackets = 0;
    std::vector<std::map<MB_FunctionCode, ReadCommand>> m_ReadCommands; //!< Commands to execute for each RTU
    std::map<std::string, Var> m_Vars;
    std::map<MB_FunctionCode, std::shared_ptr<ResponseProcessor>> m_ResponseProcessors;
    std::list<WriteCommand> m_WriteCommands;

    static constexpr uint16_t s_PeerPort = 502;     //!< Remote peer port
};

