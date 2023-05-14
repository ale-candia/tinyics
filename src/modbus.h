#pragma once

#include <_types/_uint8_t.h>
#include <_types/_uint16_t.h>
#include <_types/_uint32_t.h>

#include "ns3/ptr.h"
#include "ns3/packet.h"

#include "utils.h"

// Size of a Modbus ADU without data field
#define MB_BASE_SZ 8

enum MB_FunctionCode
{
    ReadCoils = 1,
    ReadDiscreteInputs = 2,
    //ReadHoldingRegisters = 3, won't be supported yet
    ReadInputRegisters = 4,
    //WriteSingleCoil = 5, won't be supported yet
    //WriteSingleRegister = 6, won't be supported yet
};

/**
 * \brief A Modbus Application Data Unit
 *
 * The ADU is composed from:
 *   - Transaction Identifier: Set by the client to specify a transaction.
 *   - Protocol Identifier: 0 for Modbus so it is static.
 *   - Length field: The Remaining Length of the ADU.
 *   - Unit Identifier: Set by the client to identify the machine we're talking to.
 *   - Function Code: Tells the server what kind of action to perform.
 *   - Data: Data as response or commands.
 */
class ModbusADU {
public:
    ModbusADU();
    ModbusADU(ModbusADU&& other) noexcept;
    ~ModbusADU();

    ModbusADU& operator=(ModbusADU other);  // copy assignment

    static void CopyBase(const ModbusADU& source, ModbusADU& dest);

    void SetTransactionID(uint16_t tid);
    void SetUnitID(uint8_t uid);
    void SetFunctionCode(MB_FunctionCode fc);

    template<typename T>
    void SetData(const std::vector<T>& data);

    uint16_t GetTransactionID() const;
    uint16_t GetLengthField() const;
    uint8_t GetUnitID() const;
    uint8_t GetFunctionCode() const;
    uint8_t GetDataByte(uint8_t idx) const;
    uint32_t GetBufferSize() const;

    static std::vector<ModbusADU> GetModbusADUs(const ns3::Ptr<ns3::Packet>& packet);

    /**
     * Build a ns3::Packet from the byte buffer, this is safer than
     * passing the pointer around.
     */
    ns3::Ptr<ns3::Packet> ToPacket() const;


private:
    ModbusADU(const uint8_t* buff, uint16_t start, uint16_t finish);
    /// Position in the ADU byte buffer
    enum FieldStartPosition
    {
        TRANSACTION_ID_POS = 0,
        PROTOCOL_ID_POS    = 2,
        LENGTH_FIELD_POS   = 4,
        UNIT_ID_POS        = 6,
        FUNCTION_CODE_POS  = 7,
        DATA_POS           = 8,
    };
    void SetLengthField(uint16_t length);

    void SetInitialValues();
    
    // Elements of the Modbus ADU
    uint8_t *m_ADUBytes;
    uint8_t m_BufferSize;
};

template<typename T>
void
ModbusADU::SetData(const std::vector<T>& data)
{
    uint8_t dataSize;
    if (std::is_same<T, uint8_t>::value)
    {
        dataSize = data.size();
    }
    else if(std::is_same<T, uint16_t>::value)
    {
        dataSize = 2 * data.size();
    }
    else
    {
        NS_FATAL_ERROR("Can only load uint8_t or uint16_t into Modbus ADU");
    }

    if (m_BufferSize != dataSize + MB_BASE_SZ)
    {
        m_BufferSize = dataSize + MB_BASE_SZ;

        // Allocate memory for the new data
        uint8_t *buffer = new uint8_t[m_BufferSize];

        // Copy old data into new buffer
        memcpy(buffer, m_ADUBytes, MB_BASE_SZ);

        // Free old data, we don't need it anymore
        delete m_ADUBytes;

        // Use newly created buffer
        m_ADUBytes = buffer;

        SetLengthField(dataSize + 2);
    }


    uint8_t i = 0;
    if (std::is_same<T, uint16_t>::value)
    {
        for (auto byte : data)
        {
            auto [higher, lower] = SplitUint16(byte);

            m_ADUBytes[MB_BASE_SZ + i] = higher;
            m_ADUBytes[MB_BASE_SZ + i + 1] = lower;

            i+=2;
        }
    }

    if (std::is_same<T, uint8_t>::value)
    {
        for (auto byte : data)
        {
            m_ADUBytes[MB_BASE_SZ + i] = byte;
            i++;
        }
    }

}

