#include "modbus-command.h"

Command::Command(MB_FunctionCode fc, uint16_t ref, uint16_t value) 
    : m_FunctionCode(fc), m_Ref(ref), m_Value(value) {}

uint16_t
Command::GetStart() const
{
    return m_Ref;
}

void
Command::Execute(ns3::Ptr<ns3::Socket> socket, uint16_t tid, uint8_t uid) const
{
    std::vector<uint16_t> data(2);

    // if (m_pending) return;

    data[0] = m_Ref;
    data[1] = m_Value;

    ModbusADU adu = ModbusADU();
    adu.SetTransactionID(tid);
    adu.SetUnitID(uid);
    adu.SetFunctionCode(m_FunctionCode);

    adu.SetData<uint16_t>(data);
    ns3::Ptr<ns3::Packet> p = adu.ToPacket();
    socket->Send(p);
}

ReadCommand::ReadCommand(MB_FunctionCode fc, uint16_t ref, uint16_t byteCount)
    : Command(fc, ref, byteCount)
{
    if (fc == MB_FunctionCode::ReadCoils || fc == MB_FunctionCode::ReadDiscreteInputs)
    {
        m_MaxPos = 7;
    }
    else m_MaxPos = 1;
}

void
ReadCommand::SetReadCount(uint8_t pos)
{
    uint16_t start = m_Ref;
    uint16_t num = m_Value;
    uint16_t end = 0;

    if (num == 0 && start == 0)
        start = end = pos;

    else if (pos < start)
        start = pos;

    else if (pos > end)
        end = (pos > m_MaxPos) ? m_MaxPos : pos;

    m_Ref = start;
    m_Value = end - start + 1;
}

WriteCommand::WriteCommand(MB_FunctionCode fc, uint16_t ref, uint16_t value, uint8_t uid)
    : Command(fc, ref, 0)
{
    m_Value = (value > 0) ? 0xFF00 : 0x0000;
    m_UID = uid;
}

void
WriteCommand::Execute(ns3::Ptr<ns3::Socket> socket, uint16_t tid) const
{
    Command::Execute(socket, tid, m_UID);
}

