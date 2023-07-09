from icsim.python.bindings.industrial_networks import *

"""
Simple Water Tank Industrial Process, the process is as follows:

    - pump_flow: A pump to fill the tank
    - valve_flow: A valve to empty the tank
    - level_sensor: An analog level sensor to measure the height of the water
  
 pump  ___
           |
           |
        ___|___
       |       |
       |       |
       |       |
       |-------|
       |       |
       |       |
       |       |
       |_______|_____ valve
           |
           |___ level_sensor

NOTE: We use measurements in mm since receiving/sending float number
      information is still not supported
"""
class WaterTank(IndustrialProcess):
    tank_base_area = 1          # cross-sectional area of the tank in m^2
    pump_flow = 0.1             # 0.1 m^3/s
    valve_flow = 0.05           # 0.05 m^3/s

    def __init__(self):
        super().__init__()
        self._define_input_positions()

        # State Variables
        self.curr_height = AnalogSensor(0, 10)
        self.prev_time = 0

    """
    Update the state of the process. This is updating the relevant variables
    of the process and also reflecting those values on the PLC input, since the
    PLC is usually measuring some variable form the process.
    """
    def UpdateProcess(self, state, input) -> PlcState:
        current = get_current_time() # Current time

        pump_on = input.get_digital_state(self.PUMP)
        valve_on = input.get_digital_state(self.VALVE)

        if pump_on:
            self.curr_height += self.pump_flow * (current - self.prev_time) / self.tank_base_area

        if valve_on:
            self.curr_height -= self.valve_flow * (current - self.prev_time) / self.tank_base_area

        self.prev_time = current;

        # Set height sensors (send data in mm)
        state.set_analog_state(self.LEVEL_SENSOR, self.curr_height)

        return state


    """
    Define the position of the sensors and actuator as coils from the PLC
    """
    def _define_input_positions(self):
        self.LEVEL_SENSOR = 0   # Analog level sensor is in position 0

        self.PUMP = 0           # ON/OFF pump is on the output position 0
        self.VALVE = 1          # ON/OFF valve is on the output position 1
