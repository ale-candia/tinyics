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

"""
Simple Water Tank Industrial Process, the process is as follows:

    - A pump to fill the tank at a given pump_flow
    - A valve to empty the tank at a given valve_flow
    - A level_up level sensor to identify when the tank is getting filled
    - A level_down level sensor to identify when the tank is getting emptied
  
 pump  ___
          |
       ___|___
      |       |
      |       |--- level_up
      |       |
      |-------|
      |       |
      |       |
      |       |--- level_down
      |_______|
          |
          |___ valve

"""
# TODO: Move this to its own module
class WaterTank(IndustrialProcess):
    tank_base_area = 1              # cross-sectional area of the tank in m^2
    pump_flow = 0.1             # 0.1 m^3/s
    valve_flow = 0.05           # 0.05 m^3/s
    level_down_height = 0.2;    # 0.2 m height
    level_up_height = 0.5;      # 0.5 m height

    def __init__(self):
        super().__init__()
        self._define_input_positions()

        # State Variables
        self.curr_height = 0
        self.prev_time = 0

    def UpdateState(self, measured, plc_out) -> PlcState:
        level_down_on = measured.get_digital_state(self.level_down)
        level_up_on = measured.get_digital_state(self.level_up)

        pump_on = plc_out.get_digital_state(self.pump)
        valve_on = plc_out.get_digital_state(self.valve)

        if level_up_on:
            # Turn pump off and valve on
            plc_out.set_digital_state(self.pump, False);
            plc_out.set_digital_state(self.valve, True);

        elif not level_down_on:
            plc_out.set_digital_state(self.pump, True);
            plc_out.set_digital_state(self.valve, False);
        
        return plc_out

    def UpdateProcess(self, state, input) -> PlcState:
        current = get_current_time() # Current time

        pump_on = input.get_digital_state(self.pump)
        valve_on = input.get_digital_state(self.valve)

        if pump_on:
            self.curr_height += self.pump_flow * (current - self.prev_time) / self.tank_base_area

        if valve_on:
            self.curr_height -= self.valve_flow * (current - self.prev_time) / self.tank_base_area

        self.prev_time = current;

        # Set level sensors
        state.set_digital_state(self.level_down, self.curr_height >= self.level_down_height)
        state.set_digital_state(self.level_up, self.curr_height >= self.level_up_height);

        return state

    """
    Define the position of the sensors and actuator as coils from the PLC
    """
    def _define_input_positions(self):
        self.level_down = 0 # lower level sensor 
        self.level_up = 1   # higher level sensor

        self.pump = 0
        self.valve = 1

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

