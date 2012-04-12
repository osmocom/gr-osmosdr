/* -*- c++ -*- */

#define OSMOSDR_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "osmosdr_swig_doc.i"

%{
#include "osmosdr_source_c.h"
//#include "osmosdr_sink_c.h"
%}

GR_SWIG_BLOCK_MAGIC(osmosdr,source_c);
%include "osmosdr_source_c.h"

//GR_SWIG_BLOCK_MAGIC(osmosdr,sink_c);
//%include "osmosdr_sink_c.h"

#if SWIGGUILE
%scheme %{
(load-extension-global "libguile-gnuradio-osmosdr_swig" "scm_init_gnuradio_osmosdr_swig_module")
%}

%goops %{
(use-modules (gnuradio gnuradio_core_runtime))
%}
#endif
