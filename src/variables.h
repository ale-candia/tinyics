#pragma once

#include "modbus.h"

#include "ns3/fatal-error.h"

#include <cstdint>

enum VarType
{
    Coil,
    DigitalInput,
    InputRegister,
    LocalVariable,
};

class Var
{
public:
    Var(VarType type, uint8_t pos, uint8_t uid)
        : m_Type(type), m_Pos(pos), m_Value(0), m_UID(uid), m_Changed(false)
    {
        if (uid == 0)
            NS_FATAL_ERROR("Unit Id cannot be 0 for RTU");
    }

    Var(VarType type, uint16_t value)
        : m_Type(type), m_Pos(0), m_Value(value), m_UID(0), m_Changed(false)
    {
        if (type != VarType::LocalVariable)
            NS_FATAL_ERROR("Only Local Variables can be defined without Remote Position");
    }

    VarType GetType() const { return m_Type; }
    
    uint16_t GetValue() const { return m_Value; }

    uint8_t GetPosition() const { return m_Pos; }

    uint8_t GetUID() const { return m_UID; }

    void SetValue(uint16_t val) {
        m_Value = val;
        m_Changed = true;
    }

    void SetValueUnchanged(uint16_t val) { m_Value = val; }
    void SetChanged(bool changed) { m_Changed = changed; }
    bool Changed() const { return m_Changed; }

    /*
     * Get the function code use to read this variable type
     */
    static MB_FunctionCode IntoFCRead(VarType type)
    {
        switch (type)
        {
            case VarType::Coil:
                return MB_FunctionCode::ReadCoils;
            case VarType::DigitalInput:
                return MB_FunctionCode::ReadDiscreteInputs;
            case VarType::InputRegister:
                return MB_FunctionCode::ReadInputRegisters;
            case VarType::LocalVariable:
                NS_FATAL_ERROR("");
        }
    }

    static VarType IntoVarType(MB_FunctionCode fc)
    {
        switch (fc)
        {
            case MB_FunctionCode::ReadCoils:
                return VarType::Coil;
            case MB_FunctionCode::ReadDiscreteInputs:
                return VarType::DigitalInput;
            case MB_FunctionCode::ReadInputRegisters:
                return VarType::InputRegister;
            default:
                return VarType::LocalVariable;
        }
    }
    
private:
    VarType m_Type;
    uint16_t m_Value;
    uint8_t m_Pos;
    uint8_t m_UID;
    bool m_Changed;
};

