/**
 *  The main goal is to have an API that use ns-3 as a backend to take care of
 *  the networking aspect of the process.
 * 
 *  The user should be able to specify what the elements Industrial Network is
 *  composed of and how they are related to each other.
 * 
 *           (SCADA)
 *              |
 *              |
 *         +----+----+
 *         |         |
 *       (PLC)     (PLC)
 */

#include "industrial-process.h"
#include "industrial-network-builder.h"

class Semaphore : public IndustrialProcess
{
public:
    Semaphore() {}

    PlcState UpdateProcess(PlcState measured, const PlcState& plcOut) override
    {
        return measured;
    }
    
    PlcState UpdateState(const PlcState& measured, PlcState plcOut) override
    {
        return plcOut;
    }

    static constexpr uint8_t PUMP_LIGHT_POS = 0;
    static constexpr uint8_t VALVE_LIGHT_POS = 1;

private:
    bool m_PumpPrevState = false;
    bool m_ValvePrevState = false;
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
    ns3::Ptr<PlcApplication> plc1 = ns3::CreateObject<PlcApplication>("plc1");
    ns3::Ptr<PlcApplication> plc2 = ns3::CreateObject<PlcApplication>("plc2");
    ns3::Ptr<ScadaApplication> scada = ns3::CreateObject<MyScada>("scada");

    // Define and link the processes to be controlled to their PLCs
    plc1 -> LinkProcess(IndustrialProcessType::WATER_TANK);
    plc2 -> LinkProcess(std::make_shared<Semaphore>());

    // Construct the industrial network
    IndustrialNetworkBuilder networkBuilder("192.168.1.0", "255.255.255.0");
    networkBuilder.AddToNetwork(plc1);
    networkBuilder.AddToNetwork(scada);
    networkBuilder.AddToNetwork(plc2);
    networkBuilder.BuildNetwork();

    // Specify system connections
    scada->AddRTU(plc1->GetAddress());
    scada->AddRTU(plc2->GetAddress());

    scada->AddVariable(plc1, "pump", VarType::Coil, WT_PUMP);
    scada->AddVariable(plc1, "valve", VarType::Coil, WT_VALVE);

    scada->AddVariable(plc2, "pump_light", VarType::Coil, Semaphore::PUMP_LIGHT_POS);
    scada->AddVariable(plc2, "valve_light", VarType::Coil, Semaphore::VALVE_LIGHT_POS);

    networkBuilder.EnablePcap("sim");

    // Run the simulation
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
}

