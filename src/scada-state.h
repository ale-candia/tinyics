#include <array>
#include <cstdint>

class ScadaReadingsConfig
{
public:
    uint16_t startCoil, numCoil;
    uint16_t startDiscreteIn, numDiscreteIn;
    uint16_t startInRegs, numInRegs;
};

class ScadaReadings
{
public:
    ScadaReadings() = default;
    ScadaReadings(ScadaReadingsConfig configs) : m_readings(configs)
    {
        m_pendingCoils = false;
        m_pendingInReg = false;
        m_pendingDiscreteIn = false;
    }
    ~ScadaReadings() = default;

private:
    ScadaReadingsConfig m_readings;

    /*
     * Whether we have a packet pending
     *  0 -> coil
     *  1 -> discrete inputs
     *  2 -> input registers
     */
    //std::array<bool, 3> m_pending;
    bool m_pendingCoils;
    bool m_pendingInReg;
    bool m_pendingDiscreteIn;

friend class ScadaApplication;
};

