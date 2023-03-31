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
    Ptr<PlcApplication> plc1 = CreateObject<PlcApplication>();
    Ptr<PlcApplication> plc2 = CreateObject<PlcApplication>();
    Ptr<ScadaApplication> scada = CreateObject<ScadaApplication>();

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
    scada->SetRemote(plc1->GetAddress());
    scada->SetRemote(plc2->GetAddress());

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();
}
