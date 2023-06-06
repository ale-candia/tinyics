#!/usr/bin/python3

from build.bindings.industrial_networks import *

CAPTURE_PACKETS = False

"""
This Wrapper should be used so that we control the lifetime of the linked
process, this lets us do link_process(WaterTank()).

(Can we get a copy or move ownership to C++?)
"""
#TODO: move this to its own module
class Plc(_PlcBase):
    def __init__(self, name):
        super().__init__(name)

    """
    Link the process and store a reference to it to control
    its lifetime. We also return self so that this can be
    used as Plc("name").link_proces(process)
    """
    def link_process(self, ip):
        self.__process = ip
        self._do_link_process(ip)

        return self

class MyScada(Scada):
    myvar = False

    def __init__(self, name):
        super().__init__(name)

    def Update(self):
        output = {}
        return output

# Define the automation stations (PLC and SCADA systems)
plc1 = Plc("plc1").link_process(WaterTank())
scada = MyScada("scada")

# Define and link the processes to be controlled to their PLCs

# Construct the industrial network
networkBuilder = IndustrialNetworkBuilder(Ipv4Address("192.168.1.0"), Ipv4Mask("255.255.255.0"))
networkBuilder.add_to_network(scada)
networkBuilder.add_to_network(plc1)
networkBuilder.build_network()

# Specify system connections
scada.add_rtu(plc1.get_address())

scada.add_variable(plc1, "Pump", VarType.Coil, 0)

if CAPTURE_PACKETS:
    networkBuilder.enable_pcap("sim")

# Run the simulation
run_simulation()

