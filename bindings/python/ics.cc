#include "industrial-process.h"
#include "industrial-network-builder.h"

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/pytypes.h>

namespace py = pybind11;

void
RunSimulationWrapper()
{
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
}


double
GetCurrentTime()
{
    return ns3::Simulator::Now().ToDouble(ns3::Time::S);
}

// This needs to be switched to receive an Industrial application instead
// so that PLC also supports it
void
SetApplicationLoopWrapper(ScadaApplication& scada, py::function loopFn)
{
    scada.SetScadaLoop([loopFn](std::map<std::string, Var>& values) {
        // Call the python function
        py::dict writeOut = loopFn(values);

        // Convert the Python dictionary to a new C++ map
        for (const auto& item : writeOut)
        {
            std::string varName = item.first.cast<std::string>();
            try
            {
                Var& variable = values.at(varName);
                if (variable.GetType() == VarType::Coil || variable.GetType() == VarType::LocalVariable)
                {
                    variable.SetValue(item.second.cast<uint16_t>());
                }
                else
                    NS_FATAL_ERROR("Variable '" << varName << "' is not of type Coil");
            }
            catch (std::out_of_range exception)
            {
                NS_FATAL_ERROR("Variable '" << varName << "' does not exist");
            }
        }
    });
}

PYBIND11_DECLARE_HOLDER_TYPE(T, ns3::Ptr<T>);

namespace PYBIND11_NAMESPACE
{
    namespace detail
    {
        template <typename T>
        struct holder_helper<ns3::Ptr<T>>
        { // <-- specialization
            static const T *get(const ns3::Ptr<T> &p) { return ns3::GetPointer(p); }
        };
    }
}

PYBIND11_MODULE(industrial_networks, m) {
    py::doc("Library for simulating industrial control systems networks");

    /**
     * SCADA and PLC
     */
    py::class_<IndustrialApplication, ns3::Ptr<IndustrialApplication>>(m, "IndustrialApplication");

    py::class_<PlcApplication, IndustrialApplication, ns3::Ptr<PlcApplication>>(m, "Plc")
        .def(py::init<const char*>())
        .def("link_process", &PlcApplication::LinkProcess)
        .def("get_address", &PlcApplication::GetAddress);

    py::class_<ScadaApplication, IndustrialApplication, ns3::Ptr<ScadaApplication>>(m, "Scada")
        .def(py::init<const char*>())
        .def(
            "add_variable",
            static_cast<void (ScadaApplication::*)(const std::string&, uint16_t)>(&ScadaApplication::AddVariable))
        .def(
            "add_variable",
            static_cast<void (ScadaApplication::*)(const ns3::Ptr<PlcApplication>&, const std::string&, VarType, uint8_t)>(&ScadaApplication::AddVariable))
        .def("add_rtu", py::overload_cast<ns3::Ipv4Address>(&ScadaApplication::AddRTU));

    py::enum_<VarType>(m, "VarType")
        .value("Coil", VarType::Coil)
        .value("DigitalInput", VarType::DigitalInput)
        .value("InputRegister", VarType::InputRegister)
        .value("LocalVariable", VarType::LocalVariable);

    py::class_<Var>(m, "Var")
        .def("get_value", &Var::GetValue)
        .def("set_value", &Var::SetValue);

    /**
     * Industrial Network Builder
     */
    py::class_<IndustrialNetworkBuilder>(m, "IndustrialNetworkBuilder")
        .def(py::init<ns3::Ipv4Address, ns3::Ipv4Mask>())
        .def("add_to_network", &IndustrialNetworkBuilder::AddToNetwork)
        .def("build_network", &IndustrialNetworkBuilder::BuildNetwork)
        .def("enable_pcap", &IndustrialNetworkBuilder::EnablePcap);

    //py::class_<ns3::Address>(m, "Address");

    py::class_<ns3::Ipv4Address>(m, "Ipv4Address")
        .def(py::init<const char*>());

    py::class_<ns3::Ipv4Mask>(m, "Ipv4Mask")
        .def(py::init<const char*>());

    /**
     * Industrial Process Type
     */
    py::enum_<IndustrialProcessType>(m, "IndustrialProcessType", "Enum Containing the Default industrial processes types")
        .value("WaterTank", IndustrialProcessType::WATER_TANK)
        .value("SemaphoreLight", IndustrialProcessType::SEMAPHORE_LIGHT)
        .export_values();

    /**
     * Wrappers
     */
    m.def("run_simulation", &RunSimulationWrapper);

    m.def("set_loop", &SetApplicationLoopWrapper);

    m.def("get_current_time", &GetCurrentTime);
}

