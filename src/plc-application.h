#pragma once

#include "industrial-application.h"
#include "industrial-process.h"
#include "modbus.h"

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/ptr.h"

namespace ns3
{
    class Socket;
    class Packet;
}

struct PlcState
{
    uint8_t digitalPorts;
    uint16_t analogPorts[2];
};

class PlcApplication : public IndustrialApplication
{
public:
    /**
     * Get the type ID.
     *
     * returns the object TypeId
     */
    static ns3::TypeId GetTypeId();
    PlcApplication(const char* name);
    ~PlcApplication() override;

    /// Link the PLC to and Industrial Process to control
    void LinkProcess(IndustrialProcessType ip);

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
    void HandleAccept(ns3::Ptr<ns3::Socket> s, const ns3::Address& from);

    /**
     * Schedule Update for the PLC and Process state
     *
     * The update of the PLC and the Industrial Process is independent
     * of the network traffic. So these are simulated separately at
     * their own rate.
     */
    void ScheduleUpdate(ns3::Time dt);

    /// Do the state update
    void UpdateOutput();

    uint16_t m_port;       //!< Port on which we listen for incoming packets.
    ns3::Ptr<ns3::Socket> m_socket;  //!< IPv4 Socket
    PlcState m_in;       //!< State of the PLC input ports
    PlcState m_out;       //!< State of the PLC out ports
    std::unique_ptr<IndustrialProcess> m_industrialProcess; //!< Industrial process being controlled by the PLC
    ModbusADU m_modbusADU;  //!< The modbus ADU to be sent over the network

    friend class IndustrialNetworkBuilder;
};

