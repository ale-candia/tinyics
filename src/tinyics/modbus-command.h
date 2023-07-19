#pragma once

#include "modbus.h"

#include "ns3/socket.h"

class Command
{
public:
    Command(MB_FunctionCode fc, uint16_t ref, uint16_t value); 
    ~Command() = default;

    uint16_t GetStart() const;

    void Execute(ns3::Ptr<ns3::Socket> socket, uint16_t tid, uint8_t uid) const;

protected:
    uint16_t m_Ref;
    uint16_t m_Value;
    MB_FunctionCode m_FunctionCode;
};

class ReadCommand : public Command
{
public:
    ReadCommand(MB_FunctionCode fc, uint16_t ref, uint16_t value); 

    void SetReadCount(uint8_t pos);

private:
    uint8_t m_MaxPos;
};

class WriteCommand : public Command
{
public:
    WriteCommand(MB_FunctionCode fc, uint16_t ref, uint16_t value, uint8_t uid);

    void Execute(ns3::Ptr<ns3::Socket> socket, uint16_t tid) const;

    uint8_t GetUID() const { return m_UID; }

private:
    uint8_t m_UID;
};

