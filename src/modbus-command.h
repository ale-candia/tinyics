#include "modbus.h"

#include "ns3/socket.h"

class Command
{
public:
    Command(MB_FunctionCode fc, uint16_t ref, uint16_t byteCount); 

    void SetByteCount(uint8_t pos);

    uint16_t GetStart() const;

    void Execute(ns3::Ptr<ns3::Socket> socket, uint16_t tid, uint8_t uid) const;

private:
    uint16_t m_Start;
    uint16_t m_ByteCount;
    uint8_t m_MaxPos;
    MB_FunctionCode m_FunctionCode;
};

