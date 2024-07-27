#include "industrial-process.h"
#include "industrial-network-builder.h"
#include "industrial-plant.h"
#include "scada-application.h"

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/pytypes.h>

namespace py = pybind11;

class IndustrialProcessTrampoline : public IndustrialProcess
{
public:
    /// Updates the state of the Process
    void UpdateProcess(PlcState *state, const PlcState *input) override
    {
        PYBIND11_OVERLOAD_PURE(
            void,
            IndustrialProcess,
            UpdateProcess,
            state, input
        );
    }
};

class ScadaTrampoline : public ScadaApplication
{
public:
    ScadaTrampoline(const char *name, uint64_t rate = 500) : ScadaApplication(name, rate) {}

    void Update(const std::map<std::string, Var>& vars) override
    {
        PYBIND11_OVERLOAD(
            void,
            ScadaApplication,
            Update,
            vars
        );
    }
};

class PlcTrampoline : public PlcApplication
{
public:
    PlcTrampoline(const char *name) : PlcApplication(name) {}

    void Update(const PlcState *measured, PlcState *plc_out) override
    {
        PYBIND11_OVERLOAD(
            void,
            PlcApplication,
            Update,
            measured, plc_out
        );
    }
};

void
RunSimulationWrapper(double time = 20.0)
{
    ns3::Simulator::Stop(ns3::Seconds(time));
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

    py::class_<PlcApplication, IndustrialApplication, PlcTrampoline, ns3::Ptr<PlcApplication>>(m, "_PlcBase")
        .def(py::init<const char*>())
        .def(
            "_do_link_process",
            static_cast<void(PlcApplication::*)(std::shared_ptr<IndustrialProcess>, uint8_t)>(&PlcApplication::LinkProcess)
        )
        .def("Update", &PlcApplication::Update)
        .def("get_address", &PlcApplication::GetAddress);

    py::class_<ScadaApplication, IndustrialApplication, ScadaTrampoline, ns3::Ptr<ScadaApplication>>(m, "Scada")
        .def(py::init<const char*>())
        .def(py::init<const char*, uint64_t>())
        .def("add_variable", static_cast<void (ScadaApplication::*)(const ns3::Ptr<PlcApplication>&, const std::string&, VarType, uint8_t)>(&ScadaApplication::AddVariable))
        .def("add_rtu", py::overload_cast<ns3::Ipv4Address>(&ScadaApplication::AddRTU))
        .def("Update", &ScadaApplication::Update)
        .def("_write", &ScadaApplication::Write)
        .def("set_refresh_rate", &ScadaApplication::SetRefreshRate);

    py::enum_<VarType>(m, "VarType")
        .value("Coil", VarType::Coil)
        .value("DigitalInput", VarType::DigitalInput)
        .value("InputRegister", VarType::InputRegister);

    py::class_<Var>(m, "Var")
        .def("get_value", &Var::GetValue)
        .def("set_value", &Var::SetValue);

    py::class_<IndustrialNetworkBuilder>(m, "IndustrialNetworkBuilder")
        .def(py::init<ns3::Ipv4Address, ns3::Ipv4Mask>())
        .def("add_to_network", &IndustrialNetworkBuilder::AddToNetwork)
        .def("build_network", &IndustrialNetworkBuilder::BuildNetwork)
        .def("enable_pcap", &IndustrialNetworkBuilder::EnablePcap);

    py::class_<ns3::Ipv4Address>(m, "Ipv4Address")
        .def(py::init<const char*>());

    py::class_<ns3::Ipv4Mask>(m, "Ipv4Mask")
        .def(py::init<const char*>());

    py::class_<IndustrialProcess, IndustrialProcessTrampoline, std::shared_ptr<IndustrialProcess>>(m, "IndustrialProcess")
        .def(py::init<>())
        .def("update_process", &IndustrialProcess::UpdateProcess);

    py::class_<PlcState>(m, "PlcState")
        .def(py::init<>())
        .def("get_digital_state", &PlcState::GetDigitalState)
        .def("set_digital_state", &PlcState::SetDigitalState)
        .def("get_analog_state", &PlcState::GetAnalogState)
        .def("set_analog_state", py::overload_cast<uint8_t, const AnalogSensor&>(&PlcState::SetAnalogState))
        .def("set_analog_state", py::overload_cast<uint8_t, double>(&PlcState::SetAnalogState));

    py::class_<AnalogSensor>(m, "AnalogSensor")
        .def(py::init<>())
        .def(py::init<double, double>())
        .def(py::init<double, double, double>())
        .def("get_value", &AnalogSensor::GetValue)
        .def("set_value", &AnalogSensor::SetValue)
        .def("__iadd__", &AnalogSensor::operator+=)
        .def("__isub__", &AnalogSensor::operator-=)
        .def("__imul__", &AnalogSensor::operator*=)
        .def("__eq__", &AnalogSensor::operator==)
        .def("__ne__", &AnalogSensor::operator!=)
        .def("__lt__", &AnalogSensor::operator<)
        .def("__gt__", &AnalogSensor::operator>)
        .def("__le__", &AnalogSensor::operator<=)
        .def("__ge__", &AnalogSensor::operator>=);

    py::class_<IndustrialPlant>(m, "IndustrialPlant")
        .def("set_refresh_rate", &IndustrialPlant::SetRefreshRate);

    // Functions
    m.def("run_simulation", &RunSimulationWrapper, py::arg("time") = 20.0);

    m.def("get_current_time", &GetCurrentTime);

    // TODO: Can this be done internally when calling get_analog_state() ?
    m.def("scale_word_to_range", &DenormalizeU16InRange);
}
