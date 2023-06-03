#include "modbus-request.h"

void
DigitalReadRequest::Execute(
    Socket sock,
    const ns3::Address& from,
    const ModbusADU& adu,
    PlcState& state) const
{
    uint16_t start = CombineUint8(adu.GetDataByte(0), adu.GetDataByte(1));
    uint16_t num = CombineUint8(adu.GetDataByte(2), adu.GetDataByte(3));

    // 0 <= start <= 7 (max start address)
    // 1 <= num <= 8 (max amount coils/inputs to read)
    if (start < 7 && 0 < num && num <= 8 - start)
    {
        std::vector<uint8_t> data(2);
        data[0] = 1; // Set bit count
        data[1] = GetBitsInRangeBE(start, num, state.digitalPorts);

        ModbusADU response;
        ModbusADU::CopyBase(adu, response);
        response.SetData(data);

        ns3::Ptr<ns3::Packet> p = response.ToPacket();
        sock->SendTo(p, 0, from);
    }
}

void
ReadRegistersRequest::Execute(
    Socket sock,
    const ns3::Address& from,
    const ModbusADU& adu,
    PlcState& state) const
{
    uint16_t start = CombineUint8(adu.GetDataByte(0), adu.GetDataByte(1));
    uint16_t num = CombineUint8(adu.GetDataByte(2), adu.GetDataByte(3));

    // 0 <= startAddres <= 1 (max address)
    // 1 <= numRegisters <= 2 (max amount of input registers to read)
    if (start <= 1 && 0 < num && num <= 2)
    {
        // Last In Register To Read
        uint16_t end = num + start - 1;

        std::vector<uint8_t> data(1 + 2 * num); // Registers are 16 bits
        data[0] = 2 * num; // Set byte count

        uint8_t count = 1;
        for (int i = start; i <= end; i++)
        {
            auto [higher, lower] = SplitUint16(state.analogPorts[i]);

            data[count] = static_cast<uint8_t>(higher);
            data[count + 1] = static_cast<uint8_t>(lower);

            count+=2;
        }

        ModbusADU response;
        ModbusADU::CopyBase(adu, response);
        response.SetData(data);

        ns3::Ptr<ns3::Packet> p = response.ToPacket();
        sock->SendTo(p, 0, from);
    }
}

void
WriteCoilRequest::Execute(
    Socket sock,
    const ns3::Address& from,
    const ModbusADU& adu,
    PlcState& state) const
{
    uint16_t pos = CombineUint8(adu.GetDataByte(0), adu.GetDataByte(1));
    uint16_t value = CombineUint8(adu.GetDataByte(2), adu.GetDataByte(3));

    ns3::Ptr<ns3::Packet> p = adu.ToPacket();
    sock->SendTo(p, 0, from);
}

