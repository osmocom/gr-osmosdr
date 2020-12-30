# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND osmosdr_python_files
    device_python.cc
    sink_python.cc
    source_python.cc
    ranges_python.cc
    time_spec_python.cc
    python_bindings.cc)

GR_PYBIND_MAKE_OOT(osmosdr 
   ../..
   gr::osmosdr
   "${osmosdr_python_files}")

install(TARGETS osmosdr_python DESTINATION ${GR_PYTHON_DIR}/osmosdr COMPONENT pythonapi)
