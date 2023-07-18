### CURRENTLY WORKING ON

- Does move semantics work for LinkProcess? This would remove the additinal wrapper
  in python used to manage the object lifetime.

- Add a base class for scheduled updates, as in SCADA and IndustrialPlant

- Make industrial process again hidden in the PLC

- Find a way of setting a global simulation time, i.e. a time at which all simulations end.

- try out different simulation scenarios and leave them as separate examples

- README and Documentation. Add a list of supported and unsupported features:
    - SCADA to SCADA and PLC to PLC communications
    - Full Modbus implementation

- Add refresh rates

- Make Request Processors Static (no need to create new instances for each plc)

### PRIORITY

- Make address vector in scada efficient for find and inserts

- Set modifiable network speeds

### LOWER PRIORITY

- Consider changing the `scada.add_rtu(plc1.get_address())` to just use `scada.add_rtu(plc1)` instead. This allows us to get the address whenever we want. And we don't have to wait for the user to call build_network() before doing this.

- Expose operators for custom Var class for easier usage (take into consideration the VarType)

- Review, m_interval both for SCADA and PLC

- Update rate of process should be defined by the process being simulated. Since faster changing processes may need smaller update time.

- (??) Join PLC_in and PLC_out into a single data structure (using Coils, Input Registers and Digital Inputs) to work better with the modbus protocol

- What happens if a certain amount of data in the packet is not sended, can this happen?

- Timeout for read packet (can timeouts happen??)

### MQTT

- https://sourceforge.net/p/mqttforns3/wiki/Instructions/

