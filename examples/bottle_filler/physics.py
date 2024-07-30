"""
Variables used for process interconnection. These are usually set by a single
class but used in other places.
"""
class GlobalParams:
    # rate at which water leaves the tank
    AmountDrained = 0

    # whether the draining valve is on (set by the PLC controlling
    # the bottle filler)
    OUTPUT_VALVE_OPEN = False
