#include "industrial-process.h"
#include "industrial-network-builder.h"

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/pytypes.h>

namespace py = pybind11;

class IndustrialProcessTrampoline : public IndustrialProcess
{
public:
    PlcState UpdateState(const PlcState& measured, PlcState plc_out) override
    {
        PYBIND11_OVERLOAD_PURE(
            PlcState,
            IndustrialProcess,
            UpdateState,
            measured, plc_out
        );
    }

    /// Updates the state of the Process 
    PlcState UpdateProcess(PlcState state, const PlcState& input) override
    {
        PYBIND11_OVERLOAD_PURE(
            PlcState,
            IndustrialProcess,
            UpdateProcess,
            state, input
        );
    }
};

class ScadaTrampoline : public ScadaApplication
{
public:
    ScadaTrampoline(const char* name) : ScadaApplication(name) {}

    void Update() override
    {
        PYBIND11_OVERLOAD(
            void,
            ScadaApplication,
            Update
        );
    }
};

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

PYBIND11_MODULE(industrial_networks, m)
{
    py::doc("Library for simulating industrial control systems networks");

    /**
     * SCADA and PLC
     */
    py::class_<IndustrialApplication, ns3::Ptr<IndustrialApplication>>(m, "IndustrialApplication");

    py::class_<PlcApplication, IndustrialApplication, ns3::Ptr<PlcApplication>>(m, "_PlcBase")
        .def(py::init<const char*>())
        .def(
            "_do_link_process",
            static_cast<void(PlcApplication::*)(IndustrialProcessType)>(&PlcApplication::LinkProcess))
        .def(
            "_do_link_process",
            static_cast<void(PlcApplication::*)(std::shared_ptr<IndustrialProcess>)>(&PlcApplication::LinkProcess)
        )
        .def("get_address", &PlcApplication::GetAddress);

    py::class_<ScadaApplication, IndustrialApplication, ScadaTrampoline, ns3::Ptr<ScadaApplication>>(m, "Scada")
        .def(py::init<const char*>())
        .def("add_variable", static_cast<void (ScadaApplication::*)(const ns3::Ptr<PlcApplication>&, const std::string&, VarType, uint8_t)>(&ScadaApplication::AddVariable))
        .def("add_rtu", py::overload_cast<ns3::Ipv4Address>(&ScadaApplication::AddRTU))
        .def("Update", &ScadaApplication::Update)
        .def("_write", &ScadaApplication::Write);

    py::enum_<VarType>(m, "VarType")
        .value("Coil", VarType::Coil)
        .value("DigitalInput", VarType::DigitalInput)
        .value("InputRegister", VarType::InputRegister);

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

    // TODO: Remove this. It will be implemented but Python and C++ will have their own versions
    //py::enum_<IndustrialProcessType>(m, "IndustrialProcessType", "Enum Containing the Default industrial processes types")
    //    .value("WaterTank", IndustrialProcessType::WATER_TANK)
    //    .value("SemaphoreLight", IndustrialProcessType::SEMAPHORE_LIGHT)
    //    .export_values();

    py::class_<IndustrialProcess, IndustrialProcessTrampoline, std::shared_ptr<IndustrialProcess>>(m, "IndustrialProcess")
        .def(py::init<>())
        .def("update_state", &IndustrialProcess::UpdateState)
        .def("update_process", &IndustrialProcess::UpdateProcess);
    
    /**
     * PlcState
     */
    py::class_<PlcState>(m, "PlcState")
        .def("get_digital_state", &PlcState::GetDigitalState)
        .def("set_digital_state", &PlcState::SetDigitalState);

    /**
     * Wrappers
     */
    m.def("run_simulation", &RunSimulationWrapper);

    m.def("get_current_time", &GetCurrentTime);
}

