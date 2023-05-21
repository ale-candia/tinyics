try:
    from .build.bindings.ics import *
except ImportError:
    raise ImportError("Error importings ics, verify it has been built correctly")
