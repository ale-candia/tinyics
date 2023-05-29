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
    Var(VarType type, uint8_t pos)
        : m_type(type), m_pos(pos), m_value(0) {}

    Var(VarType type, uint8_t pos, uint16_t value)
        : m_type(type), m_pos(pos), m_value(value) {}

    VarType GetType() const { return m_type; }
    
    uint16_t GetValue() const { return m_value; }

    uint8_t GetPosition() const { return m_pos; }

    void SetValue(uint16_t val) { m_value = val; }

    static MB_FunctionCode IntoFunctionCode(VarType type)
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
    VarType m_type;
    uint16_t m_value;
    uint8_t m_pos;
};

