#!/usr/bin/python3

from build.bindings.industrial_networks import *

PLOT_GRAPHS = False
CAPTURE_PACKETS = True

pump = []
valve = []
level1 = []
level2 = []
t = []

def gather_values(vars):
    pump.append(vars['Pump'].get_value())
    valve.append(vars['Valve'].get_value())
    level1.append(vars['L1'].get_value())
    level2.append(vars['L2'].get_value())

    t.append(get_current_time())

def plot_vars():
    import matplotlib.pyplot as plt

    fig, (ax0, ax1) = plt.subplots(nrows=1, ncols=2, sharex=True)

    ax0.set_title('Digital Output')
    ax0.errorbar(t, pump)
    ax0.errorbar(t, valve)

    ax1.set_title('Sensors (Digital Inputs)')
    ax1.errorbar(t, level1)
    ax1.errorbar(t, level2)

    fig.suptitle('Water Tank Readings')
    plt.show()

def scada_loop(vars):
    output = {}
    a1 = vars["A1"].get_value()
    test_value = vars["TestVar"].get_value()

    if a1 % 3 == 0:
        # Invert the value of TestVar each 3 iterations
        output["TestVar"] = not (True if test_value > 0 else False)

    output["A1"] = a1 + 1

    if PLOT_GRAPHS:
        gather_values(vars)

    return output

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

#scada.add_variable(plc1, "Pump", VarType.Coil, 0)
#scada.add_variable(plc1, "Valve", VarType.Coil, 1)

scada.add_variable(plc2, "TestVar", VarType.Coil, 0)
scada.add_variable("A1", 0)

if CAPTURE_PACKETS:
    networkBuilder.enable_pcap("sim")

# Run the simulation
run_simulation()

if PLOT_GRAPHS:
    plot_vars()


