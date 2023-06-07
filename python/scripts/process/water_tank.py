from icsim.python.bindings.industrial_networks import IndustrialProcess, PlcState, get_current_time

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
class WaterTank(IndustrialProcess):
    tank_base_area = 1          # cross-sectional area of the tank in m^2
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

        # Set level sensors
        state.set_digital_state(self.LEVEL_DOWN, self.curr_height >= self.level_down_height)
        state.set_digital_state(self.LEVEL_UP, self.curr_height >= self.level_up_height);

        return state

    """
    Update the outupt states of the PLC, this is basically the logic of the PLC.
    We get some measurements of the process and, according to those measurements,
    we update the output of the PLC to some desired value
    """
    def UpdateState(self, measured, plc_out) -> PlcState:
        level_down_on = measured.get_digital_state(self.LEVEL_DOWN)
        level_up_on = measured.get_digital_state(self.LEVEL_UP)

        pump_on = plc_out.get_digital_state(self.PUMP)
        valve_on = plc_out.get_digital_state(self.VALVE)

        if level_up_on:
            # Turn pump off and valve on
            plc_out.set_digital_state(self.PUMP, False);
            plc_out.set_digital_state(self.VALVE, True);

        elif not level_down_on:
            plc_out.set_digital_state(self.PUMP, True);
            plc_out.set_digital_state(self.VALVE, False);
        
        return plc_out


    """
    Define the position of the sensors and actuator as coils from the PLC
    """
    def _define_input_positions(self):
        self.LEVEL_DOWN = 0 # lower level sensor 
        self.LEVEL_UP = 1   # higher level sensor

        self.PUMP = 0
        self.VALVE = 1

