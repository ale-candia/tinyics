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

using namespace ns3;

class PlcApplication : public IndustrialApplication
{
public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    PlcApplication(const char* name);
    ~PlcApplication() override;

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
    void HandleRead(Ptr<Socket> socket);
    void HandleAccept(Ptr<Socket> s, const Address& from);

    void ScheduleUpdate(Time dt);
    void UpdateOutput();

    uint16_t m_port;       //!< Port on which we listen for incoming packets.
    Ptr<Socket> m_socket;  //!< IPv4 Socket
    PlcState m_in;       //!< State of the PLC input ports
    PlcState m_out;       //!< State of the PLC out ports
    std::unique_ptr<IndustrialProcess> m_industrialProcess; //!< Industrial process being controlled by the PLC
    ModbusADU m_modbusADU;

    friend class IndustrialNetworkBuilder;
};

