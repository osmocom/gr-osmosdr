/* -*- c++ -*- */

#define OSMOSDR_API

// suppress Warning 319: No access specifier given for base class 'boost::noncopyable' (ignored).
#pragma SWIG nowarn=319

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "osmosdr_swig_doc.i"

%{
#include "osmosdr/osmosdr_device.h"
#include "osmosdr/osmosdr_source_c.h"
#include "osmosdr/osmosdr_sink_c.h"
%}

// Workaround for a SWIG 2.0.4 bug with templates. Probably needs to be looked in to.
%{
#if PY_VERSION_HEX >= 0x03020000
# define SWIGPY_SLICE_ARG(obj) ((PyObject*) (obj))
#else
# define SWIGPY_SLICE_ARG(obj) ((PySliceObject*) (obj))
#endif
%}

%template(string_vector_t) std::vector<std::string>;

//%template(size_vector_t) std::vector<size_t>;

%include <osmosdr/osmosdr_pimpl.h>

%ignore osmosdr::device_t::operator[]; //ignore warnings about %extend

%template(string_string_dict_t) std::map<std::string, std::string>; //define before device
%template(devices_t) std::vector<osmosdr::device_t>;
%include <osmosdr/osmosdr_device.h>

%template(range_vector_t) std::vector<osmosdr::range_t>; //define before range
%include <osmosdr/osmosdr_ranges.h>

GR_SWIG_BLOCK_MAGIC(osmosdr,source_c);
%include "osmosdr/osmosdr_source_c.h"

GR_SWIG_BLOCK_MAGIC(osmosdr,sink_c);
%include "osmosdr/osmosdr_sink_c.h"

