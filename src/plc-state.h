#pragma once

#include <cstdint>

struct PlcState
{
    uint8_t digitalPorts;
    uint16_t analogPorts[2];
};

