"""
This is a simple example of a distributed control system scenario.
We have a water tank process controlled by the plc_wt, a semaphore
connected to the plc_s and a centralized SCADA system that can
reads/write data to the PLCs using Modbus.

The plc_s doesn't actually contain any logic but the SCADA is used
to write to the PLC's output coils. With this we can simulate a distributed
scenario with the SCADA as a centralized unit that reads data from a PLC
controlling a process and updates the state of some other process that's
not connected to the first one.
"""
from icsim import *

GENERATE_PCAP_FILE = False
GENERATE_PLOT = True

pump_light = []
valve_light = []
t_s = []
"""
Define a semaphore Industrial Process.

This is basically two light bulbs each indicating the state of the
pump and valve respectively. When ON it means the pump/valve is working.
"""
class Semaphore(IndustrialProcess):
    PUMP_LIGHT_POS = 0
    VALVE_LIGHT_POS = 1

    def __init__(self):
        super().__init__()

    def UpdateProcess(self, state, input) -> PlcState:

        pump_light.append(input.get_digital_state(self.PUMP_LIGHT_POS))
        valve_light.append(input.get_digital_state(self.VALVE_LIGHT_POS))
        t_s.append(get_current_time())

        return state

    def UpdateState(self, measured, plc_out) -> PlcState:
        # We don't update the PLC output in this process
        return plc_out


tank_height = []
t_wt = []

class MyScada(Scada):
    def __init__(self, name):
        super().__init__(name)

    def Update(self, vars):
        output = {}

        # Process Logic
        # If the pump/valve is working then turn on the light in the semaphore
        output["pump_light"] = 1 if vars["pump"].get_value() > 0 else 0
        output["valve_light"] = 1 if vars["valve"].get_value() > 0 else 0

        # Gather Values for graph
        tank_height.append(scale_word_to_range(vars["tank_height"].get_value(), 0, 10))
        t_wt.append(get_current_time())

        self._write(output)


water_tank = process.WaterTank()
scada = MyScada("scada")

# Define and link the processes to be controlled to their PLCs
plc_wt = Plc("water_control").link_process(water_tank)
plc_s = Plc("semaphore").link_process(Semaphore())

# Construct the industrial network
networkBuilder = IndustrialNetworkBuilder(Ipv4Address("192.168.1.0"), Ipv4Mask("255.255.255.0"))
networkBuilder.add_to_network(scada)
networkBuilder.add_to_network(plc_wt)
networkBuilder.add_to_network(plc_s)
networkBuilder.build_network()

# Specify system connections
scada.add_rtu(plc_wt.get_address())
scada.add_rtu(plc_s.get_address())

scada.add_variable(plc_wt, "pump", VarType.Coil, water_tank.PUMP)
scada.add_variable(plc_wt, "valve", VarType.Coil, water_tank.VALVE)
scada.add_variable(plc_wt, "tank_height", VarType.InputRegister, water_tank.LEVEL_SENSOR)

scada.add_variable(plc_s, "pump_light", VarType.Coil, Semaphore.PUMP_LIGHT_POS)
scada.add_variable(plc_s, "valve_light", VarType.Coil, Semaphore.VALVE_LIGHT_POS)

if GENERATE_PCAP_FILE:
    networkBuilder.enable_pcap("sim")

# Run the simulation
run_simulation()

if GENERATE_PLOT:
    import matplotlib.pyplot as plt

    fig, axs = plt.subplots(3)
    fig.suptitle('Simulated Processes')

    # Wether the water tank is on or not (from the scada)
    axs[0].plot(t_wt, tank_height)
    axs[0].set_title('Water Tank Height')
    axs[0].grid()

    axs[1].plot(t_s, pump_light, 'tab:orange', drawstyle='steps')
    axs[1].set_title('Pump Light ON/OFF')
    axs[1].grid()

    axs[2].plot(t_s, valve_light, 'tab:green', drawstyle='steps')
    axs[2].set_title('Valve Light ON/OFF')
    axs[2].grid()

    plt.show()

