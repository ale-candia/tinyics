#pragma once

#include "modbus.h"
#include "plc-state.h"

#include "ns3/socket.h"

using Socket = ns3::Ptr<ns3::Socket>;

class RequestProcessor
{
public:
    ~RequestProcessor() = default;
    /**
     * Processes and sends the response using the provided socket
     */
    virtual void Execute(Socket sock, const ModbusADU& adu, const PlcState& state) const = 0;
};

class ReadDigitalIO : public RequestProcessor
{
public:
    void Execute(Socket sock, const ModbusADU& adu, const PlcState& state) const override;
};

class ReadRegisters : public RequestProcessor
{
    void Execute(Socket sock, const ModbusADU& adu, const PlcState& state) const override;
};

