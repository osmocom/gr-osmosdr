# Copyright 2011 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Setup library
########################################################################
INCLUDE(GrPlatform) #define LIB_SUFFIX
INCLUDE(GrMiscUtils)

########################################################################
# Helpful Macros
########################################################################
MACRO(GR_OSMOSDR_APPEND_SRCS)
    LIST(APPEND gr_osmosdr_srcs ${ARGV})
ENDMACRO(GR_OSMOSDR_APPEND_SRCS)

MACRO(GR_OSMOSDR_APPEND_LIBS)
    LIST(APPEND gr_osmosdr_libs ${ARGV})
ENDMACRO(GR_OSMOSDR_APPEND_LIBS)

GR_OSMOSDR_APPEND_SRCS(
    osmosdr_source_c_impl.cc
    osmosdr_sink_c_impl.cc
    osmosdr_ranges.cc
)

GR_OSMOSDR_APPEND_LIBS(
    ${Boost_LIBRARIES}
    ${GRUEL_LIBRARIES}
    ${GNURADIO_CORE_LIBRARIES}
)

########################################################################
# Setup OsmoSDR component
########################################################################
include(GrComponent)
GR_REGISTER_COMPONENT("Sysmocom OsmoSDR" ENABLE_OSMOSDR)

if(ENABLE_OSMOSDR)
GR_INCLUDE_SUBDIRECTORY(osmosdr)
endif(ENABLE_OSMOSDR)

########################################################################
# Setup FCD component
########################################################################
include(GrComponent)
GR_REGISTER_COMPONENT("FunCube Dongle" ENABLE_FCD GNURADIO_FCD_FOUND)

if(ENABLE_FCD)
GR_INCLUDE_SUBDIRECTORY(fcd)
endif(ENABLE_FCD)

########################################################################
# Setup File component
########################################################################
include(GrComponent)
GR_REGISTER_COMPONENT("File Source" ENABLE_FILE GNURADIO_CORE_FOUND)

if(ENABLE_FILE)
GR_INCLUDE_SUBDIRECTORY(file)
endif(ENABLE_FILE)

########################################################################
# Setup RTL component
########################################################################
include(GrComponent)
GR_REGISTER_COMPONENT("Osmocom RTLSDR" ENABLE_RTL LIBRTLSDR_FOUND)

if(ENABLE_RTL)
GR_INCLUDE_SUBDIRECTORY(rtl)
endif(ENABLE_RTL)

########################################################################
# Setup RTL_TCP component
########################################################################
include(GrComponent)
GR_REGISTER_COMPONENT("Osmocom RTLSDR TCP Client" ENABLE_RTL_TCP GNURADIO_CORE_FOUND)

if(ENABLE_RTL_TCP)
GR_INCLUDE_SUBDIRECTORY(rtl_tcp)
endif(ENABLE_RTL_TCP)

########################################################################
# Setup UHD component
########################################################################
include(GrComponent)
GR_REGISTER_COMPONENT("Ettus UHD" ENABLE_UHD GNURADIO_UHD_FOUND)

if(ENABLE_UHD)
GR_INCLUDE_SUBDIRECTORY(uhd)
endif(ENABLE_UHD)

########################################################################
# Setup configuration file
########################################################################
ADD_DEFINITIONS(-DHAVE_CONFIG_H=1)
include_directories(APPEND ${CMAKE_CURRENT_BINARY_DIR})
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
@ONLY)

########################################################################
# Setup libgnuradio-osmosdr library
########################################################################
ADD_LIBRARY(gnuradio-osmosdr SHARED ${gr_osmosdr_srcs})
TARGET_LINK_LIBRARIES(gnuradio-osmosdr ${gr_osmosdr_libs})
SET_TARGET_PROPERTIES(gnuradio-osmosdr PROPERTIES DEFINE_SYMBOL "gnuradio_osmosdr_EXPORTS")
SET_TARGET_PROPERTIES(gnuradio-osmosdr PROPERTIES SOVERSION "${MAJOR_VERSION}")
SET_TARGET_PROPERTIES(gnuradio-osmosdr PROPERTIES VERSION "${MAJOR_VERSION}.${MINOR_VERSION}")
IF(DEFINED GR_OSMOSDR_OUTPUT_NAME)
    SET_TARGET_PROPERTIES(uhd PROPERTIES OUTPUT_NAME ${GR_OSMOSDR_OUTPUT_NAME})
ENDIF(DEFINED GR_OSMOSDR_OUTPUT_NAME)

########################################################################
# Install libgnuradio-osmosdr library
########################################################################
INSTALL(TARGETS gnuradio-osmosdr
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)
