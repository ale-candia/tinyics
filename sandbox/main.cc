/**
 *  The main goal is to have an API that use ns-3 as a backend to take care of
 *  the networking aspect of the process.
 * 
 *  The user should be able to specify what the elements Industrial Network is
 *  composed of and how they are related to each other.
 *
 *  This is a simple example of a distributed control system scenario.
 * We have a water tank process controlled by the plc_wt, a semaphore
 * connected to the plc_s and a centralized SCADA system that can
 * reads/write data to the PLCs using Modbus.
 * 
 * The plc_s(semaphore) doesn't actually contain any logic, instead the SCADA
 * is used to write to the PLC's output coils. With this we can simulate a
 * distributed scenario with the SCADA as a centralized unit that reads data
 * from a PLC controlling a process and updates the state of some other process
 * that's not connected to the first one.
 * 
 *           (SCADA)
 *              |
 *              |
 *         +----+----+
 *         |         |
 *       (PLC)     (PLC)
 *         |         |
 *         |         |
 *     Semaphore   Water
 *                 Tank
 */

#include "industrial-process.h"
#include "industrial-network-builder.h"

/**
 * A Water Tank system
 *
 * It contains two level sensors, a pump and a valve
 */
class WaterTank : public IndustrialProcess
{
public:
    // SENSORS
    static constexpr double LEVEL_SENSOR_POS = 0;

    // ACTUATORS
    static constexpr uint8_t PUMP_POS = 0;
    static constexpr uint8_t VALVE_POS = 1;

    WaterTank()
    {
        m_currHeight = AnalogSensor(0, 10);
    }
    ~WaterTank() = default;

    void UpdateProcess(PlcState* state, const PlcState* input) override
    {
        auto current = ns3::Simulator::Now().ToDouble(ns3::Time::S);

        bool pupmOn = input->GetDigitalState(PUMP_POS);
        bool valveOn = input->GetDigitalState(VALVE_POS);

        if (pupmOn)
        {
            m_currHeight += s_pumpFlow * (current - m_prevTime) / s_tankWidth;
        }
        if (valveOn)
        {
            m_currHeight -= s_valveFlow * (current - m_prevTime) / s_tankWidth;
        }

        m_prevTime = current;

        // update level sensor
        state->SetAnalogState(LEVEL_SENSOR_POS, m_currHeight);
    }

private:
    static constexpr float s_tankWidth = 1;    // cross-sectional area of the tank
    static constexpr float s_pumpFlow = 0.1;   // 0.1 m/s = 10cm/s
    static constexpr float s_valveFlow = 0.05; // 0.05 m/s = 5cm/s

    double m_prevTime;
    AnalogSensor m_currHeight;
};


class Semaphore : public IndustrialProcess
{
public:
    Semaphore() {}

    void UpdateProcess(PlcState* state, const PlcState* input) override {}

    static constexpr uint8_t PUMP_LIGHT_POS = 0;
    static constexpr uint8_t VALVE_LIGHT_POS = 1;

private:
    bool m_PumpPrevState = false;
    bool m_ValvePrevState = false;
};

class PlcSemaphore : public PlcApplication
{
public:
    PlcSemaphore(const char* name) : PlcApplication(name)
    {
        this->LinkProcess(std::make_shared<Semaphore>());
    }
};

class PlcWaterTank : public PlcApplication
{
public:
    PlcWaterTank(const char* name) : PlcApplication(name)
    {
        this->LinkProcess(std::make_shared<WaterTank>());
    }

    void Update(const PlcState *measured, PlcState *plcOut) override
    {
        // The value returned by the PLC is a 16-bit value (called word) that 
        // has to be denormalized into the actual physical value
        double height = DenormalizeU16InRange(measured->GetAnalogState(WaterTank::LEVEL_SENSOR_POS), 0, 10);

        bool pumpOn = plcOut->GetDigitalState(WaterTank::PUMP_POS);
        bool valveOn = plcOut->GetDigitalState(WaterTank::VALVE_POS);

        if (height > s_level_up)
        {
            // Turn pump off and valve on
            plcOut->SetDigitalState(WaterTank::PUMP_POS, false);
            plcOut->SetDigitalState(WaterTank::VALVE_POS, true);
        }
        else if (height < s_level_down)
        {
            // Turn pump on and valve off
            plcOut->SetDigitalState(WaterTank::PUMP_POS, true);
            plcOut->SetDigitalState(WaterTank::VALVE_POS, false);
        }
    }

private:
    static constexpr float s_level_down = 0.2;  // minimum water level
    static constexpr float s_level_up = 0.5;    // maximum water level
};

class MyScada : public ScadaApplication
{
public:
    MyScada(const char* name) : ScadaApplication(name) {}

    void Update(const std::map<std::string, Var>& vars) override
    {
        std::map<std::string, uint16_t> output;

        output["pump_light"] = vars.at("pump").GetValue() > 0;
        output["valve_light"] = vars.at("valve").GetValue() > 0;

        this->Write(output);
    }
};

int
main(int argc, char* argv[])
{
    // Define the automation stations (PLC and SCADA systems)
    ns3::Ptr<PlcApplication> plc1 = ns3::CreateObject<PlcWaterTank>("wt");
    ns3::Ptr<PlcApplication> plc2 = ns3::CreateObject<PlcSemaphore>("sema");
    ns3::Ptr<ScadaApplication> scada = ns3::CreateObject<MyScada>("scada");

    // Construct the industrial network
    IndustrialNetworkBuilder networkBuilder("192.168.1.0", "255.255.255.0");
    networkBuilder.AddToNetwork(plc1);
    networkBuilder.AddToNetwork(scada);
    networkBuilder.AddToNetwork(plc2);
    networkBuilder.BuildNetwork();

    // Specify system connections
    scada->AddRTU(plc1->GetAddress());
    scada->AddRTU(plc2->GetAddress());

    scada->AddVariable(plc1, "pump", VarType::Coil, WaterTank::PUMP_POS);
    scada->AddVariable(plc1, "valve", VarType::Coil, WaterTank::VALVE_POS);
    scada->AddVariable(plc1, "level_sensor", VarType::InputRegister, WaterTank::LEVEL_SENSOR_POS);

    scada->AddVariable(plc2, "pump_light", VarType::Coil, Semaphore::PUMP_LIGHT_POS);
    scada->AddVariable(plc2, "valve_light", VarType::Coil, Semaphore::VALVE_LIGHT_POS);

    networkBuilder.EnablePcap("sim");

    // Run the simulation
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
}

