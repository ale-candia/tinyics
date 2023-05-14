#pragma once

#include "industrial-application.h"
#include "modbus.h"
#include "scada-state.h"
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
} // namespace ns3

using namespace ns3;

/**
 * \brief A ScadaApplication (acts as a Modbus TCP client)
 *
 * Every packet sent should be returned by the server and received here.
 */
class ScadaApplication : public IndustrialApplication
{
public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    ScadaApplication(const char* name);

    ~ScadaApplication() override;

    /**
     * \brief set the remote address and port
     * \param ip remote IP address
     * \param port remote port
     */
    void AddRTU(Address ip, uint16_t port);
    /**
     * \brief set the remote address
     * \param addr remote address
     */
    void AddRTU(Address addr);

    /**
     * Fill the Modbus ADU
     *
     * \param unitID unit identifier of the remote machine (255 if not used)
     */
    void SetFill(uint8_t unitID);

    /**
     * Select the data that is going to be read from the PLC.
     *
     * \param plcName name of the PLC from which we're reading the data
     * \param coils a tuple with values (startCoil, finishCoil)
     * \param discreteIn a tuple with values (startDiscreteInput, finishDiscreteInput)
     * \param coils a tuple with values (startInputRegister, finishInputRegister)
     */
    void SetReadConfigForPlc(
        Ptr<PlcApplication> plc,
        std::tuple<uint16_t, uint16_t> coils,
        std::tuple<uint16_t, uint16_t> discreteIn,
        std::tuple<uint16_t, uint16_t> inputReg
    );

protected:
    void DoDispose() override;

private:
    void StartApplication() override;
    void StopApplication() override;

    /**
     * \brief Schedule the next packet transmission
     * \param dt time interval between packets.
     */
    void ScheduleUpdate(Time dt);
    /**
     * \brief Send a packet to all connected devices
     */
    void SendAll();

    void DoUpdate();

    /**
     * \brief Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * \param socket the socket the packet was received to.
     */
    void HandleRead(Ptr<Socket> socket);
    void FreeSockets();

    void SetFill(uint8_t unitId, MB_FunctionCode fc, std::vector<uint16_t> data);

    Time m_interval;  //!< Packet inter-send time
    ModbusADU m_ModBusADU;  //!< Modbus Application Data Unit
    std::vector<Ptr<Socket>> m_sockets;   //!< Socket
    std::vector<Address> m_peerAddresses; //!< Remote peer address
    uint16_t m_peerPort;                  //!< Remote peer port
    bool m_started; //!< Whether the app has already started
    uint16_t m_transactionId; //!< TransactionId for the Modbus ADU
    std::map<Address, ScadaReadings> m_ReadConfigs;
};

