"""
This is a simple example of a distributed control system scenario.
We have a water tank process controlled by the plc_wt, a semaphore
connected to the plc_s and a centralized SCADA system that can
reads/write data to the PLCs using Modbus.

The plc_s(semaphore) doesn't actually contain any logic, instead the SCADA
is used to write to the PLC's output coils. With this we can simulate a
distributed scenario with the SCADA as a centralized unit that reads data
from a PLC controlling a process and updates the state of some other process
that's not connected to the first one.

         (SCADA)
            |
            |
       +----+----+
       |         |
     (PLC)     (PLC)
       |         |
       |         |
  Semaphore  Water Tank
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

"""
Define the logic for processes controlling the semaphore and the water tank
"""
class PlcS(Plc):
    """
    PLC controlling the semaphore
    """
    def __init__(self, name):
        super().__init__(name)
        self.link_process(Semaphore())

    def Update(self, measured, plc_out) -> PlcState:
        # For this example, the PLC itself is not updating the coil and
        # turning on the lights. We use a remote command from the SCADA
        # to update these, so there's no change in the PLC output done here.
        return plc_out

class PlcWT(Plc):
    """
    PLC controlling the water tank
    """
    level_down_height = 0.2;    # 0.2m minimum water level in the tank 
    level_up_height = 0.5;      # 0.5m maximum water level in the tank

    def __init__(self, name):
        super().__init__(name)
        self.link_process(process.WaterTank())

    """
    We get some measurements of the process and, according to those measurements,
    we update the output of the PLC to some desired value
    """
    def Update(self, measured, plc_out) -> PlcState:
        height = scale_word_to_range(measured.get_analog_state(self.process.LEVEL_SENSOR), 0, 10)

        pump_on = plc_out.get_digital_state(self.process.PUMP)
        valve_on = plc_out.get_digital_state(self.process.VALVE)

        if height >= self.level_up_height:
            # Turn pump off and valve on
            plc_out.set_digital_state(self.process.PUMP, False);
            plc_out.set_digital_state(self.process.VALVE, True);

        elif not height >= self.level_down_height:
            plc_out.set_digital_state(self.process.PUMP, True);
            plc_out.set_digital_state(self.process.VALVE, False);
        
        return plc_out


tank_height = []
t_wt = []

"""
SCADA logic
"""
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

# Define the control system components
plc_wt = PlcWT("water_control")
plc_s = PlcS("semaphore")
scada = MyScada("scada")

# Construct the industrial network
networkBuilder = IndustrialNetworkBuilder(Ipv4Address("192.168.1.0"), Ipv4Mask("255.255.255.0"))
networkBuilder.add_to_network(scada)
networkBuilder.add_to_network(plc_wt)
networkBuilder.add_to_network(plc_s)
networkBuilder.build_network()

# Specify system connections
scada.add_rtu(plc_wt.get_address())
scada.add_rtu(plc_s.get_address())

scada.add_variable(plc_wt, "pump", VarType.Coil, plc_wt.process.PUMP)
scada.add_variable(plc_wt, "valve", VarType.Coil, plc_wt.process.VALVE)
scada.add_variable(plc_wt, "tank_height", VarType.InputRegister, plc_wt.process.LEVEL_SENSOR)

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

