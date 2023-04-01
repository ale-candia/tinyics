#pragma once

#include "industrial-process.h"
#include "industrial-element.h"

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include <memory>

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
    void SetAddress(Address addr);

private:
    void StartApplication() override;
    void StopApplication() override;

    /**
     * \brief Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * \param socket the socket the packet was received to.
     */
    void HandleRead(Ptr<Socket> socket);


    uint16_t m_port;       //!< Port on which we listen for incoming packets.
    Ptr<Socket> m_socket;  //!< IPv4 Socket
    PlcState m_state;       //!< State of the PLC ports
    std::shared_ptr<IndustrialProcess> m_industrialProcess; //!< Industrial process being controlled by the PLC

    friend class IndustrialNetworkBuilder;
};
