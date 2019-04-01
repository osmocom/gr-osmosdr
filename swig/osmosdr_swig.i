/* -*- c++ -*- */

#define OSMOSDR_API

%include "gnuradio.i"           // the common stuff

//load generated python docstrings
%include "osmosdr_swig_doc.i"

%{
#include "osmosdr/source.h"
#include "osmosdr/sink.h"
%}

%include "osmosdr/source.h"
GR_SWIG_BLOCK_MAGIC2(osmosdr, source);
%include "osmosdr/sink.h"
GR_SWIG_BLOCK_MAGIC2(osmosdr, sink);
