#include "modbus.h"
#include "utils.h"

ModbusADU::ModbusADU()
{
    m_Size = MB_BASE_SZ;
    m_Bytes = new uint8_t[m_Size];
    SetInitialValues();
}

ModbusADU::ModbusADU(const uint8_t *buff, uint16_t start, uint16_t finish)
{
    if (finish < start)
    {
        NS_FATAL_ERROR_NO_MSG();
    }
    m_Size = finish - start + 1;

    if (m_Size < MB_BASE_SZ)
    {
        NS_FATAL_ERROR("Modbus Application Data Unit should be at least" << MB_BASE_SZ << "Bytes");
    }

    m_Bytes = new uint8_t[m_Size];

    memcpy(m_Bytes, buff + start, m_Size);
}

ModbusADU::ModbusADU(ModbusADU&& other) noexcept
{
    m_Size = other.m_Size;
    m_Bytes = other.m_Bytes;

    other.m_Size = 0;
    other.m_Bytes = nullptr;
}

ModbusADU::~ModbusADU()
{
    delete m_Bytes;
}

std::vector<ModbusADU>
ModbusADU::GetModbusADUs(const ns3::Ptr<ns3::Packet>& packet)
{
    if (packet->GetSize() < MB_BASE_SZ)
    {
        NS_FATAL_ERROR("Data was not recognize as a Modbus ADU packet");
    }

    std::vector<ModbusADU> adus;

    uint32_t dataStreamSize = packet->GetSize();
    uint8_t *dataStream = new uint8_t[dataStreamSize];
    packet->CopyData(dataStream, dataStreamSize);

    // Start and finish of a single Modbus ADU
    uint16_t start=0, finish;

    // While the data stream is large enough to read the lenght field
    while (start + LENGTH_FIELD_POS + 1 < dataStreamSize)
    {
        uint16_t lenght = CombineUint8(
            dataStream[start + LENGTH_FIELD_POS],
            dataStream[start + LENGTH_FIELD_POS + 1]
        );

        finish = (start + MB_BASE_SZ + lenght - 2) - 1;

        if (finish < dataStreamSize)
        {
            // Create Modbus ADU and add it to the vector
            adus.push_back(std::move(ModbusADU(dataStream, start, finish)));

            start = finish + 1;
        }

        else
        {
            break;
        }
    }
    delete[] dataStream;

    return adus;
}

ModbusADU& ModbusADU::operator=(ModbusADU source)
{
    if (m_Size != source.m_Size)
    {
        // Free old data
        delete m_Bytes;

        // Copy the data from the source Modbus ADU
        m_Size = source.m_Size;
        m_Bytes = new uint8_t[m_Size];
    }

    memcpy(m_Bytes, source.m_Bytes, m_Size);

    return *this;
}

void
ModbusADU::CopyBase(const ModbusADU& source, ModbusADU& dest)
{
    if (source.m_Size < MB_BASE_SZ)
    {
        NS_FATAL_ERROR("Trying to copy invalid ADU");
    }

    if (dest.m_Size != MB_BASE_SZ)
    {
        delete dest.m_Bytes;

        dest.m_Size = MB_BASE_SZ;
        dest.m_Bytes = new uint8_t[dest.m_Size];
    }

    memcpy(dest.m_Bytes, source.m_Bytes, dest.m_Size);
}

void
ModbusADU::SetTransactionID(uint16_t tid)
{
    if (!m_Bytes)
    {
        NS_FATAL_ERROR("Called SetTransactionId with an empty byte buffer");
    }

    // Load the Transaction Identifier
    auto [higher, lower] = SplitUint16(tid);

    m_Bytes[TRANSACTION_ID_POS] = higher;
    m_Bytes[TRANSACTION_ID_POS + 1] = lower;
}

void
ModbusADU::SetLengthField(uint16_t length)
{
    if (!m_Bytes)
    {
        NS_FATAL_ERROR("Called SetTransactionId with an empty byte buffer");
    }

    // Load the Lenght Field
    auto [higher, lower] = SplitUint16(length);

    m_Bytes[LENGTH_FIELD_POS] = higher;
    m_Bytes[LENGTH_FIELD_POS + 1] = lower;
}

void
ModbusADU::SetUnitID(uint8_t uid)
{
    if (!m_Bytes)
    {
        NS_FATAL_ERROR("Called SetUnitID with an empty byte buffer");
    }

    m_Bytes[UNIT_ID_POS] = uid;
}

void
ModbusADU::SetFunctionCode(MB_FunctionCode fc)
{
    if (!m_Bytes)
    {
        NS_FATAL_ERROR("Called SetUnitID with an empty byte buffer");
    }

    m_Bytes[FUNCTION_CODE_POS] = (uint8_t)fc;
}

uint16_t
ModbusADU::GetTransactionID() const
{
    return CombineUint8(
        m_Bytes[TRANSACTION_ID_POS],
        m_Bytes[TRANSACTION_ID_POS + 1]
    );
};

uint16_t
ModbusADU::GetLengthField() const
{
    return CombineUint8(
        m_Bytes[LENGTH_FIELD_POS],
        m_Bytes[LENGTH_FIELD_POS + 1]
    );
}

uint8_t
ModbusADU::GetUnitID() const
{
    return m_Bytes[UNIT_ID_POS];
}

MB_FunctionCode
ModbusADU::GetFunctionCode() const
{
    return static_cast<MB_FunctionCode>(m_Bytes[FUNCTION_CODE_POS]);
}

uint32_t
ModbusADU::GetBufferSize() const
{
    return m_Size;
}

ns3::Ptr<ns3::Packet>
ModbusADU::ToPacket() const
{
    return ns3::Create<ns3::Packet>(m_Bytes, m_Size);
}

void
ModbusADU::SetInitialValues()
{
    if (!m_Bytes)
    {
        NS_FATAL_ERROR("Called SetInitialValues with an empty byte buffer");
    }

    // Set protocol identifier to 0 for Modbus TCP
    m_Bytes[PROTOCOL_ID_POS] = 0;
    m_Bytes[PROTOCOL_ID_POS + 1] = 0;

    // Set Length Field to (no data)
    SetLengthField(0);
}

uint8_t
ModbusADU::GetDataByte(uint8_t idx) const
{
    if (idx >= GetLengthField() - 2)
    {
        std::cout << "buffer size: " << (int)m_Size << std::endl;
        NS_FATAL_ERROR("Index out of range for data bytes in Modbus ADU");
    }

    return m_Bytes[MB_BASE_SZ + idx];
}

