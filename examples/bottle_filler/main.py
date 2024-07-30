import tinyics
from bottle_filler import PlcBottle 
from water_tank import PlcWaterTank 

# variables for the plot
tank_height = []    # level of water in the tank
bottle_level = []   # height of the bottle being filled
t = []              # time steps

"""
Function used to create the plots at the end of the simulation
"""
def generate_plot():
    import matplotlib.pyplot as plt

    fig, axs = plt.subplots(2)
    fig.suptitle('Simulated Processes')

    axs[0].plot(t, tank_height)
    axs[0].set_title('Water Tank Height')
    axs[0].set_ylabel('Height [cm]')
    axs[0].set_xlabel('Time [s]')
    axs[0].grid()

    axs[1].plot(t, bottle_level, 'tab:orange')
    axs[1].set_title('Bottle Level')
    axs[1].set_ylabel('Height [cm]')
    axs[1].set_xlabel('Time [s]')
    axs[1].grid()

    plt.show()


"""
SCADA we are going to use a SCADA system to gather process data and plot
graphs after the simulation
"""
class MyScada(tinyics.Scada):
    def __init__(self, name, rate):
        super().__init__(name, rate)

    def Update(self, vars):
        # use defined variables to make the plot
        tank_height.append(tinyics.scale_word_to_range(vars["tank_height"].get_value(), 0, 10))
        bottle_level.append(tinyics.scale_word_to_range(vars["bottle_level"].get_value(), 0, 20))
        t.append(tinyics.get_current_time())

# Define the control system components
plc_wt = PlcWaterTank("wt")
plc_bf = PlcBottle("bf")
#scada = MyScada("scada") # set a refresh rate of 500ms <=> 0.5 s
#scada = MyScada("scada", 1000) # set a refresh rate of 500ms <=> 0.5 s
scada = MyScada("scada", 100) # set a refresh rate of 500ms <=> 0.5 s

# Construct the industrial network
networkBuilder = tinyics.IndustrialNetworkBuilder(
        tinyics.Ipv4Address("192.168.1.0"),
        tinyics.Ipv4Mask("255.255.255.0"))
networkBuilder.add_to_network(scada)
networkBuilder.add_to_network(plc_wt)
networkBuilder.add_to_network(plc_bf)
networkBuilder.build_network()

#networkBuilder.enable_pcap("sim")

# Specify system connections
scada.add_rtu(plc_wt.get_address())
scada.add_rtu(plc_bf.get_address())

scada.add_variable(plc_wt, "tank_height", tinyics.VarType.InputRegister, plc_wt.LEVEL_SENSOR_POS)
scada.add_variable(plc_bf, "bottle_level", tinyics.VarType.InputRegister, plc_bf.BOTTLE_LEVEL_POS)

# Run the simulation
tinyics.run_simulation(100)
generate_plot()
