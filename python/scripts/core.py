try:
    # Import private objects to be used here
    from ..bindings.industrial_networks import _PlcBase

except ImportError:
    raise ImportError("Error importings icsim, verify it has been built correctly")

"""
This Wrapper should be used so that we control the lifetime of the linked
process, this lets us do link_process(WaterTank()).

(Can we get a copy or move ownership to C++?)
"""
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

