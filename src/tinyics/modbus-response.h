#pragma once

#include "modbus.h"
#include "utils.h"
#include "variables.h"

#include <map>

class ResponseProcessor
{
  public:
    ~ResponseProcessor() = default;

    /**
     * Processes and sends the response using the provided socket
     */
    virtual void Execute(const ModbusADU& adu, const std::vector<Var*>& vars, uint16_t start) const = 0;
};

class DigitalReadResponse : public ResponseProcessor
{
  public:
    void Execute(const ModbusADU& adu, const std::vector<Var*>& vars, uint16_t start) const override;
};

class RegisterReadResponse : public ResponseProcessor
{
  public:
    void Execute(const ModbusADU& adu, const std::vector<Var*>& vars, uint16_t start) const override;
};

class WriteCoilResponse : public ResponseProcessor
{
  public:
    void Execute(const ModbusADU& adu, const std::vector<Var*>& vars, uint16_t start) const override;
};
