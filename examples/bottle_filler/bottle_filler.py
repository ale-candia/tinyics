# file -> bottle_filler.py
import tinyics
from physics import *

class BottleFiller(tinyics.IndustrialProcess):
    BOTTLE_HEIGHT = 20      # height of the bottle is [cm]
    BOTTLE_BASE_AREA = 100  # base area of the water bottle [cm2]

    CONVEYOR_SPEED = 0.05   # how fast the conveyor belt moves when activated [m/s]
    BOTTLE_DISTANCE = 0.2   # the distance between two consecutive bottles [m]

    """
    Initialize values

    Note that the bottle_distance_to_tap is not an AnalogSensor but the bottle_water_level is.
    This is because we're not 'measuring' the bottle_distance_to_tap, this is only a local
    variable we use to compute the physics of the process, similar to the prev_time (previous
    time step).

    bottle_distance_to_tap is used to compute how much distance has the previous bottle travelled.
    We assume that the bottles are equally spaced from each other so using how much the previous
    bottle has travelled let us compute if there is a new bottle below the tap to stop the
    conyevor.
    """
    def __init__(self):
        super().__init__()

        self.prev_time = 0
        self.bottle_distance_to_tap = 0

        # an analog sensor that measures height of the water bottle
        self.bottle_water_level = tinyics.AnalogSensor(0, 20)

    def UpdateProcess(self, measurements, input):
        current = tinyics.get_current_time()
        elapsed_time = current - self.prev_time

        conveyor_moving = input.get_digital_state(PlcBottle.CONVEYOR_POS)
        valve_on = input.get_digital_state(PlcBottle.VALVE_POS)

        # update position of the water bottle in the conveyor
        if conveyor_moving:
            self.bottle_distance_to_tap += elapsed_time * self.CONVEYOR_SPEED

        bottle_in_place = self.bottle_distance_to_tap >= self.BOTTLE_DISTANCE or self.bottle_distance_to_tap == 0

        # transform from cm3 to L
        amount = self.bottle_water_level.get_value() * self.BOTTLE_BASE_AREA / 1000

        # update level on the water bottle
        if bottle_in_place:
            self.bottle_distance_to_tap = 0

        if valve_on:
            amount += GlobalParams.AmountDrained

            if not bottle_in_place:
                print("[Warning] Water is being wasted")
        else:
            amount = 0

        self.prev_time = current
        self.bottle_water_level.set_value(amount * 1000 / self.BOTTLE_BASE_AREA)

        # update measurements
        measurements.set_digital_state(PlcBottle.BOTTLE_DETECTED_POS, bottle_in_place)
        measurements.set_analog_state(PlcBottle.BOTTLE_LEVEL_POS, self.bottle_water_level)

# Defines the logic for controlling the bottle filling process
class PlcBottle(tinyics.Plc):
    # position of the sensors (inputs)
    BOTTLE_LEVEL_POS = 0        # position 0 among analog inputs
    BOTTLE_DETECTED_POS = 0     # position 0 among digital inputs

    # position of the actuators (outputs)
    CONVEYOR_POS = 0            # position 0 among the digital outputs (coils)
    VALVE_POS = 1               # position 1 among the digital outputs (coils)

    # constants
    MAX_BOTTLE_LEVEL = 0.75

    def __init__(self, name):
        # initialize the class with a unique name
        super().__init__(name)
        self.link_process(BottleFiller(), 25) # link the process to the bottle fill process

        self.t = []
        self.h = []

    def Update(self, measured, plc_out):
        bottle_level = tinyics.scale_word_to_range(measured.get_analog_state(self.BOTTLE_LEVEL_POS), 0, 20)

        self._report(bottle_level)

        if measured.get_digital_state(self.BOTTLE_DETECTED_POS):
            # if the bottle is in place and still not completelly filled, then open
            # the valve to pour water and stop the conveyor belt
            if bottle_level >= 0 and bottle_level < self.MAX_BOTTLE_LEVEL:
                plc_out.set_digital_state(self.VALVE_POS, True)
                plc_out.set_digital_state(self.CONVEYOR_POS, False)

                GlobalParams.OUTPUT_VALVE_OPEN = True

            # if the bottle is in position but completely filled then close the valve
            # and move the conveyor
            else:
                plc_out.set_digital_state(self.VALVE_POS, False)
                plc_out.set_digital_state(self.CONVEYOR_POS, True)

                GlobalParams.OUTPUT_VALVE_OPEN = False

    def _report(self, level):
        self.t.append(tinyics.get_current_time())
        self.h.append(level)

    def get_t(self):
        return self.t

    def get_h(self):
        return self.h
