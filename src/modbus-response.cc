#include "modbus-response.h"

void
DigitalReadResponse::Execute(const ModbusADU& adu, const std::vector<Var*>& vars, uint16_t start) const
{
    // Amount of data bytes (including the byte count)
    uint16_t buff_size = adu.GetLengthField() - 2;

    if (buff_size == 0)
        return;

    // Last position requested on ADU

    uint8_t byte_count = adu.GetDataByte(0);

    for (auto var : vars)
    {
        uint8_t pos = var->GetPosition() - start;

        // The byte where the coil is and the position of the coil within that byte
        uint8_t byte = std::trunc(pos / 8.0) + 1;
        uint8_t bit = pos % 8;

        // If the bit is turned on, then set the value to 1 (true) otherwise set the value
        // to 0
        uint8_t value = (GetBitsInRangeBE(bit, 1, adu.GetDataByte(byte)) > 0) ? 1 : 0;

        var->SetValueUnchanged(value);
    }
}

void
RegisterReadResponse::Execute(const ModbusADU& adu, const std::vector<Var*>& vars, uint16_t start) const
{
    // Amount of data bytes (including the byte count)
    uint16_t buff_size = adu.GetLengthField() - 2;

    if (buff_size == 0)
        return;

    uint8_t byte_count = adu.GetDataByte(0);

    for (auto var : vars)
    {
        // Indicates the place of the Hight byte for the variable
        uint8_t pos = var->GetPosition() - start;

        if (pos > byte_count / 2) return;

        // We should add 1 here, to taking into consideration the byte_count
        uint8_t high = adu.GetDataByte(2 * pos + 1);
        uint8_t low = adu.GetDataByte(2 * pos + 2);

        // If the bit is turned on, then set the value to 1 (true) otherwise set the value
        // to 0
        uint16_t value = CombineUint8(high, low);

        var->SetValueUnchanged(value);
    }

}

