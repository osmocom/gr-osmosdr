# Copyright 2011 Free Software Foundation, Inc.
#
# This file is part of gr-osmosdr
#
# gr-osmosdr is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# gr-osmosdr is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with gr-osmosdr; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Include swig generation macros
########################################################################
find_package(SWIG)
find_package(PythonLibs 3)
if(NOT SWIG_FOUND OR NOT PYTHONLIBS_FOUND)
    return()
endif()
include(GrSwig)
include(GrPython)

########################################################################
# Setup swig generation
########################################################################
set(GR_SWIG_INCLUDE_DIRS $<TARGET_PROPERTY:gnuradio::runtime_swig,INTERFACE_INCLUDE_DIRECTORIES>)
set(GR_SWIG_TARGET_DEPS gnuradio::runtime_swig)

set(GR_SWIG_LIBRARIES gnuradio-osmosdr)

set(GR_SWIG_DOC_FILE ${CMAKE_CURRENT_BINARY_DIR}/osmosdr_swig_doc.i)
set(GR_SWIG_DOC_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../include/osmosdr)

GR_SWIG_MAKE(osmosdr_swig osmosdr_swig.i)

########################################################################
# Install the build swig module
########################################################################
GR_SWIG_INSTALL(TARGETS osmosdr_swig DESTINATION ${GR_PYTHON_DIR}/osmosdr)

########################################################################
# Install swig .i files for development
########################################################################
install(
    FILES
    osmosdr_swig.i
    ${CMAKE_CURRENT_BINARY_DIR}/osmosdr_swig_doc.i
    DESTINATION ${GR_INCLUDE_DIR}/osmosdr/swig
)
