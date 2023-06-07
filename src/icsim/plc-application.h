#pragma once

#include "industrial-application.h"
#include "industrial-process.h"
#include "modbus-request.h"

#include "ns3/address.h"
#include "ns3/application.h"

namespace ns3
{
    class Socket;
    class Packet;
}

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
    void LinkProcess(std::shared_ptr<IndustrialProcess> ip);
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

    static constexpr uint16_t s_port = 502;       //!< Port on which we listen for incoming packets
    ns3::Time m_interval;
    ns3::Time m_step = ns3::Seconds(0.0);
    ns3::Ptr<ns3::Socket> m_socket;  //!< IPv4 Socket
    PlcState m_in;       //!< State of the PLC input ports
    PlcState m_out;       //!< State of the PLC out ports
    std::shared_ptr<IndustrialProcess> m_industrialProcess; //!< Industrial process being controlled by the PLC
    std::map<MB_FunctionCode, std::shared_ptr<RequestProcessor>> m_RequestProcessors;

    friend class IndustrialNetworkBuilder;
};

