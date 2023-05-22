try:
    from .build.bindings.industrial_networks import *
except ImportError:
    raise ImportError("Error importings icsim, verify it has been built correctly")
