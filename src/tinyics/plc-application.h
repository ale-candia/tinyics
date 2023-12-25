#pragma once

#include "ns3/address.h"
#include "ns3/application.h"

#include "industrial-application.h"
#include "industrial-process.h"
#include "modbus-request.h"

namespace ns3
{
class Socket;
class Packet;
} // namespace ns3

class PlcApplication : public IndustrialApplication
{
public:
    /**
     * Get the type ID.
     *
     * returns the object TypeId
     */
    static ns3::TypeId GetTypeId();
    PlcApplication(const char *name);
    ~PlcApplication() override;

    /*
     * Link the PLC to and Industrial Process to control
     */
    void LinkProcess(std::shared_ptr<IndustrialProcess> ip, uint8_t priority = 0);

    /*
     * Run the update/logic of the PLC
     *
     * It is expected to be overwritten, if not, the PLC does nothing.
     */
    virtual void Update(const PlcState *measured, PlcState *plc_out)
    {
    }

protected:
    void DoDispose() override;

private:
    void StartApplication() override;
    void StopApplication() override;

    /*
     * Processes each packet received from the socket using Modbus.
     *
     * Note: There are a lot of verifications that are currently not
     * being considered. And exceptions that are currently not being
     * thrown.
     */
    void HandleRead(ns3::Ptr<ns3::Socket> socket);

    /// Accept callback for the socket
    void HandleAccept(ns3::Ptr<ns3::Socket> s, const ns3::Address &from);

    /// Do the state update
    void DoUpdate();

    static constexpr uint16_t s_Port = 502; //!< Port on which we listen for incoming packets
    ns3::Ptr<ns3::Socket> m_Socket;         //!< IPv4 Sockets
    PlcState m_In;                          //!< State of the PLC input ports
    PlcState m_Out;                         //!< State of the PLC out ports
    std::shared_ptr<IndustrialProcess> m_IndustrialProcess; //!< process being controlled

    friend class IndustrialNetworkBuilder;
    friend class IndustrialPlant;
};
