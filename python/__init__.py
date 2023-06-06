try:
    # Import public objects from C++
    from .bindings.industrial_networks import *

except ImportError:
    raise ImportError("Error importings icsim, verify it has been built correctly")

from .scripts.core import *
from .scripts import process

