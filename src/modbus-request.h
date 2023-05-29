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
    virtual void Execute(Socket sock, const ns3::Address& from, const ModbusADU& adu, const PlcState& state) const = 0;
};

class DigitalReadRequest : public RequestProcessor
{
public:
    void Execute(Socket sock, const ns3::Address& from, const ModbusADU& adu, const PlcState& state) const override;
};

class ReadRegistersRequest : public RequestProcessor
{
    void Execute(Socket sock, const ns3::Address& from, const ModbusADU& adu, const PlcState& state) const override;
};

