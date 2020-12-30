#include <pybind11/pybind11.h>

namespace py = pybind11;

#include <osmosdr/ranges.h>

void bind_ranges(py::module& m)
{
    m.attr("ALL_MBOARDS") = ::osmosdr::ALL_MBOARDS;
    m.attr("ALL_CHANS") = ::osmosdr::ALL_CHANS;


    using range_t = ::osmosdr::range_t;

    py::class_<range_t>(m, "range_t")
        .def(py::init<double>(), py::arg("value") = 0)
        .def(py::init<double, double, double>(), py::arg("start"), py::arg("stop"), py::arg("step") = 0)
        .def("start", &range_t::start)
        .def("stop", &range_t::stop)
        .def("step", &range_t::step)
        .def("to_pp_string", &range_t::to_pp_string);


    using meta_range_t = ::osmosdr::meta_range_t;

    py::class_<meta_range_t>(m, "meta_range_t")
        .def(py::init())
        .def(py::init<double, double, double>(), py::arg("start"), py::arg("stop"), py::arg("step") = 0)
        .def("start", &meta_range_t::start)
        .def("stop", &meta_range_t::stop)
        .def("step", &meta_range_t::step)
        .def("clip", &meta_range_t::clip, py::arg("value"), py::arg("clip_step") = false)
        .def("values", &meta_range_t::values)
        .def("to_pp_string", &meta_range_t::to_pp_string);
}
