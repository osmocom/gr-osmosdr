#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <osmosdr/device.h>

void bind_device(py::module& m)
{
    using device_t = ::osmosdr::device_t;

    py::class_<device_t>(m, "device_t")
        .def(py::init<std::string&>(), py::arg("args") = "")
        .def("to_pp_string", &device_t::to_pp_string)
        .def("to_string", &device_t::to_string);


    using devices_t = ::osmosdr::devices_t;

    py::class_<devices_t>(m, "devices_t");


    using device = ::osmosdr::device;

    py::class_<device>(m, "device")
        .def_static("find", &device::find, py::arg("hint") = device_t());
}
