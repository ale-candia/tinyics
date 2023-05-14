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

int
main(int argc, char* argv[])
{
    // Define the automation stations (PLC and SCADA systems)
    Ptr<PlcApplication> plc1 = CreateObject<PlcApplication>("plc1");
    Ptr<PlcApplication> plc2 = CreateObject<PlcApplication>("plc2");
    Ptr<ScadaApplication> scada = CreateObject<ScadaApplication>("scada");

    // Define and link the processes to be controlled to their PLCs
    plc1 -> LinkProcess(IndustrialProcessType::WATER_TANK);
    plc2 -> LinkProcess(IndustrialProcessType::SEMAPHORE_LIGHT);

    // Construct the industrial network
    IndustrialNetworkBuilder networkBuilder("192.168.1.0", "255.255.255.0");
    networkBuilder.AddToNetwork(plc1);
    networkBuilder.AddToNetwork(scada);
    networkBuilder.AddToNetwork(plc2);
    networkBuilder.BuildNetwork();

    // Specify system connections
    scada->AddRTU(plc1->GetAddress());
    scada->AddRTU(plc2->GetAddress());

    scada->SetReadConfigForPlc(
        plc1,
        std::make_tuple<uint16_t, uint16_t>(0, 5),
        std::make_tuple<uint16_t, uint16_t>(0, 0),
        std::make_tuple<uint16_t, uint16_t>(0, 1)
    );

    networkBuilder.EnablePcap("sim");

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();
}

