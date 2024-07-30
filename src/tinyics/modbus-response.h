#pragma once

#include <map>

#include "modbus.h"
#include "utils.h"
#include "variables.h"

class ModbusResponseProcessor
{
public:
    ~ModbusResponseProcessor() = default;

    /**
     * Processes the incoming response and sends the response using the provided socket.
     */
    static void Execute(MB_FunctionCode fc,
                        const ModbusADU &adu,
                        const std::vector<Var *> &vars,
                        uint16_t start);

private:
    static void DigitalReadResponse(const ModbusADU &adu,
                                    const std::vector<Var *> &vars,
                                    uint16_t start);

    static void RegisterReadResponse(const ModbusADU &adu,
                                     const std::vector<Var *> &vars,
                                     uint16_t start);

    static void WriteCoilResponse(const ModbusADU &adu,
                                  const std::vector<Var *> &vars,
                                  uint16_t start);
};
