#include <pybind11/pybind11.h>
#include <pybind11/operators.h>

namespace py = pybind11;

#include <osmosdr/time_spec.h>

void bind_time_spec(py::module& m)
{
    using time_spec_t = ::osmosdr::time_spec_t;

    py::class_<time_spec_t>(m, "time_spec_t")
        .def_static("get_system_time", &time_spec_t::get_system_time)
        .def(py::init<double>(), py::arg("secs") = 0)
        .def(py::init<time_t, double>(), py::arg("full_secs"), py::arg("frac_secs") = 0)
        .def(py::init<time_t, long, double>(), py::arg("full_secs"), py::arg("tick_count"), py::arg("tick_rate"))
        .def_static("from_ticks", &time_spec_t::from_ticks, py::arg("ticks"), py::arg("tick_rate"))
        .def("get_tick_count", &time_spec_t::get_tick_count, py::arg("tick_rate"))
        .def("to_ticks", &time_spec_t::to_ticks, py::arg("tick_rate"))
        .def("get_real_secs", &time_spec_t::get_real_secs)
        .def("get_full_secs", &time_spec_t::get_full_secs)
        .def("get_frac_secs", &time_spec_t::get_frac_secs)
        .def(py::self + py::self)
        .def(py::self += py::self)
        .def(py::self - py::self)
        .def(py::self -= py::self)
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def(py::self < py::self)
        .def(py::self > py::self)
        .def(py::self <= py::self)
        .def(py::self >= py::self);
}
