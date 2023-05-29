#include "modbus-command.h"

Command::Command(MB_FunctionCode fc, uint16_t ref, uint16_t byteCount)
    : m_FunctionCode(fc), m_Start(ref), m_ByteCount(byteCount)
{
    if (fc == MB_FunctionCode::ReadCoils || fc == MB_FunctionCode::ReadDiscreteInputs)
    {
        m_MaxPos = 7;
    }
    else m_MaxPos = 1;
}

void
Command::SetByteCount(uint8_t pos)
{
    uint16_t start = m_Start;
    uint16_t num = m_ByteCount;
    uint16_t end = 0;

    if (num == 0 && start == 0)
        start = end = pos;

    else if (pos < start)
        start = pos;

    else if (pos > end)
        end = (pos > m_MaxPos) ? m_MaxPos : pos;

    m_Start = start;
    m_ByteCount = end - start + 1;
}

uint16_t
Command::GetStart() const
{
    return m_Start;
}

void
Command::Execute(ns3::Ptr<ns3::Socket> socket, uint16_t tid, uint8_t uid) const
{
    std::vector<uint16_t> data(2);

    // if (m_pending) return;

    data[0] = m_Start;
    data[1] = m_ByteCount;

    ModbusADU adu = ModbusADU();
    adu.SetTransactionID(tid);
    adu.SetUnitID(uid);
    adu.SetFunctionCode(m_FunctionCode);

    adu.SetData<uint16_t>(data);
    ns3::Ptr<ns3::Packet> p = adu.ToPacket();
    socket->Send(p);
}


