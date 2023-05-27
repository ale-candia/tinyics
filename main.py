#!/usr/bin/python3

from build.bindings.industrial_networks import *

def scada_loop(vars):
    x0 = vars['X0'].get_value()
    x1 = vars['X1'].get_value()

    return {'X0': x0 + 1, 'X1': x1 + 2}

def say_hello():
    print("Hello World from python")


# Define the automation stations (PLC and SCADA systems)
plc1 = Plc("plc1")
plc2 = Plc("plc2")
scada = Scada("scada")
set_loop(scada, scada_loop)

# Define and link the processes to be controlled to their PLCs
plc1.link_process(IndustrialProcessType.WaterTank)
plc2.link_process(IndustrialProcessType.SemaphoreLight)

# Construct the industrial network
networkBuilder = IndustrialNetworkBuilder(Ipv4Address("192.168.1.0"), Ipv4Mask("255.255.255.0"))
networkBuilder.add_to_network(plc1)
networkBuilder.add_to_network(scada)
networkBuilder.add_to_network(plc2)
networkBuilder.build_network()

# Specify system connections
scada.add_rtu(plc1.get_address())
scada.add_rtu(plc2.get_address())

scada.add_variable(plc1, "X0", VarType.Coil, 0)
scada.add_variable(plc1, "X1", VarType.Coil, 1)

networkBuilder.enable_pcap("sim")

# Run the simulation
run_simulation()

