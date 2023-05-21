#include "industrial-process.h"
#include "industrial-network-builder.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

void
RunSimulationWrapper()
{
    ns3::Simulator::Run();
    ns3::Simulator::Destroy();
}

namespace py = pybind11;

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
            static_cast<void (ScadaApplication::*)(const std::string&, VarType)>(&ScadaApplication::AddVariable))
        .def(
            "add_variable",
            static_cast<void (ScadaApplication::*)(const ns3::Ptr<PlcApplication>&, const std::string&, VarType, uint8_t)>(&ScadaApplication::AddVariable))
        .def("add_rtu", py::overload_cast<ns3::Ipv4Address>(&ScadaApplication::AddRTU));

    py::enum_<VarType>(m, "VarType")
        .value("Coil", VarType::Coil)
        .value("DigitalInput", VarType::DigitalInput)
        .value("InputRegister", VarType::InputRegister)
        .value("LocalVariable", VarType::LocalVariable);

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
     * Run the simulation
     */
    m.def("run_simulation", &RunSimulationWrapper);
}

