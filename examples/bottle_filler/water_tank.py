import tinyics
from physics import *

class WaterTank(tinyics.IndustrialProcess):
    TANK_BASE_AREA = 300        # base area of the water tank [cm2]
    INPUT_VALVE_FLOW = 0.2      # rate at which water enters the tank [L/s]
    OUTPUT_VALVE_FLOW = 0.03    # rate at which water exits the tank [L/s]

    def __init__(self):
        super().__init__()

        self.prev_time = 0

        self.curr_height = tinyics.AnalogSensor(0, 10)


    """
    Compute the height of the water tank and update the measurements. Also update the
    tank output flow.
    """
    def UpdateProcess(self, measurements, input):
        current = tinyics.get_current_time()
        elapsed_time = current - self.prev_time

        input_valve_on = input.get_digital_state(PlcWaterTank.INPUT_VALVE_POS)
        
        # transform from cm3 to L
        amount = self.curr_height.get_value() * self.TANK_BASE_AREA / 1000
        if input_valve_on:
            amount += self.INPUT_VALVE_FLOW * elapsed_time

        if GlobalParams.OUTPUT_VALVE_OPEN:
            drained = self.OUTPUT_VALVE_FLOW * elapsed_time

            if amount - drained >= 0:
                amount -= drained
                GlobalParams.AmountDrained = drained

            else:
                amount = 0
                GlobalParams.AmountDrained = drained - abs(amount - drained)

        self.prev_time = current;
        self.curr_height.set_value(amount * 1000 / self.TANK_BASE_AREA)

        # Set height sensors (send data in mm)
        measurements.set_analog_state(PlcWaterTank.LEVEL_SENSOR_POS, self.curr_height)

class PlcWaterTank(tinyics.Plc):
    # position of the sensors (inputs)
    LEVEL_SENSOR_POS = 0   # position 0 among analog inputs

    # position of the actuators (outputs)
    INPUT_VALVE_POS = 0    # position 0 among digital outputs (coils)

    # constants
    MAX_HEIGHT = 5.5    # maximum height at which the tank will be filled [cm]
    MIN_HEIGHT = 4      # minimum height at which the tank will be filled [cm]

    def __init__(self, name):
        # initialize the class with a unique name
        super().__init__(name)
        self.link_process(WaterTank(), 1) # link with priority 1

    """
    The PLC will maintain the level of water in the range [MIN_HEIGHT, MAX_HEIGHT].
    """
    def Update(self, measured, plc_out):

        height = tinyics.scale_word_to_range(measured.get_analog_state(self.LEVEL_SENSOR_POS), 0, 10)

        if height >= self.MAX_HEIGHT:
            plc_out.set_digital_state(self.INPUT_VALVE_POS, False)

        elif height <= self.MIN_HEIGHT:
            plc_out.set_digital_state(self.INPUT_VALVE_POS, True)
