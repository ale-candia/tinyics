#pragma once

#include "industrial-element.h"
#include "industrial-process.h"

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
    uint8_t analogPorts[2];
};

using namespace ns3;

class PlcApplication : public IndustrialApplication
{
public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    PlcApplication();
    ~PlcApplication() override;

    void LinkProcess(IndustrialProcessType ip);

protected:
    void DoDispose() override;

private:
    void StartApplication() override;
    void StopApplication() override;

    void HandleRead(Ptr<Socket> socket);
    void HandleAccept(Ptr<Socket> s, const Address& from);

    void ScheduleUpdate(Time dt);
    void UpdateOutput();

    uint16_t m_port;       //!< Port on which we listen for incoming packets.
    Ptr<Socket> m_socket;  //!< IPv4 Socket
    PlcState m_in;       //!< State of the PLC input ports
    PlcState m_out;       //!< State of the PLC out ports
    std::unique_ptr<IndustrialProcess> m_industrialProcess; //!< Industrial process being controlled by the PLC

    friend class IndustrialNetworkBuilder;
};
