from build.bindings.industrial_networks import *

# Define the automation stations (PLC and SCADA systems)
plc1 = Plc("plc1")
plc2 = Plc("plc2")
scada = Scada("scada");

# Define and link the processes to be controlled to their PLCs
plc1.link_process(IndustrialProcessType.WaterTank)
plc2.link_process(IndustrialProcessType.SemaphoreLight)

# Construct the industrial network
networkBuilder = IndustrialNetworkBuilder(Ipv4Address("192.168.1.0"), Ipv4Mask("255.255.255.0"))
networkBuilder.add_to_network(plc1);
networkBuilder.add_to_network(scada);
networkBuilder.add_to_network(plc2);
networkBuilder.build_network();

# Specify system connections
scada.add_rtu(plc1.get_address());
scada.add_rtu(plc2.get_address());

scada.add_variable(plc1, "X0", VarType.Coil, 0);
scada.add_variable(plc1, "X1", VarType.Coil, 1);

networkBuilder.enable_pcap("sim");

print("Before the Simulation")
# Run the simulation
run_simulation()
print("After the simulation")

