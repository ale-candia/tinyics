#pragma once

#include "ns3/socket.h"

#include "modbus.h"
#include "plc-state.h"

using Socket = ns3::Ptr<ns3::Socket>;

class RequestProcessor
{
public:
    ~RequestProcessor() = default;

    /**
     * Processes the incoming request and sends the response through the provided socket
     */
    static void Execute(MB_FunctionCode fc,
                        Socket sock,
                        const ns3::Address &from,
                        const ModbusADU &adu,
                        PlcState &state);

private:
    static void DigitalReadRequest(Socket sock,
                                   const ns3::Address &from,
                                   const ModbusADU &adu,
                                   PlcState &state);

    static void ReadRegistersRequest(Socket sock,
                                     const ns3::Address &from,
                                     const ModbusADU &adu,
                                     PlcState &state);

    static void WriteCoilRequest(Socket sock,
                                 const ns3::Address &from,
                                 const ModbusADU &adu,
                                 PlcState &state);
};
