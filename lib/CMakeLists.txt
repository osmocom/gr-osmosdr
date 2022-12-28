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
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

########################################################################
# Setup target
########################################################################

list(APPEND gr_osmosdr_srcs
    source_impl.cc
    sink_impl.cc
    ranges.cc
    device.cc
    time_spec.cc
)

#-pthread Adds support for multithreading with the pthreads library.
#This option sets flags for both the preprocessor and linker. (man gcc)
if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND Boost_LIBRARIES -pthread)
endif()

#dirty macro to allow appending from subdirs
#this appends all unnamed implicit macro args!
MACRO (APPEND_LIB_LIST)
      SET (gr_osmosdr_libs "${gr_osmosdr_libs};${ARGN}" CACHE INTERNAL "lib list")
ENDMACRO (APPEND_LIB_LIST)

set(gr_osmosdr_libs "" CACHE INTERNAL "lib that accumulates link targets")

add_library(gnuradio-osmosdr SHARED)
APPEND_LIB_LIST(${Boost_LIBRARIES} gnuradio::gnuradio-runtime)
target_include_directories(gnuradio-osmosdr
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${Boost_INCLUDE_DIRS}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PUBLIC $<INSTALL_INTERFACE:include>
  )
set_target_properties(gnuradio-osmosdr PROPERTIES DEFINE_SYMBOL "gnuradio_osmosdr_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-osmosdr PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Setup defines for high resolution timing
########################################################################
message(STATUS "")
message(STATUS "Configuring high resolution timing...")
include(CheckCXXSourceCompiles)

set(CMAKE_REQUIRED_LIBRARIES -lrt)
CHECK_CXX_SOURCE_COMPILES("
    #include <ctime>
    int main(){
        timespec ts;
        return clock_gettime(CLOCK_MONOTONIC, &ts);
    }
    " HAVE_CLOCK_GETTIME
)
unset(CMAKE_REQUIRED_LIBRARIES)

include(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
    #include <mach/mach_time.h>
    int main(){
        mach_timebase_info_data_t info;
        mach_timebase_info(&info);
        mach_absolute_time();
        return 0;
    }
    " HAVE_MACH_ABSOLUTE_TIME
)

CHECK_CXX_SOURCE_COMPILES("
    #include <Windows.h>
    int main(){
        LARGE_INTEGER value;
        QueryPerformanceCounter(&value);
        QueryPerformanceFrequency(&value);
        return 0;
    }
    " HAVE_QUERY_PERFORMANCE_COUNTER
)

if(HAVE_CLOCK_GETTIME)
    message(STATUS "  High resolution timing supported through clock_gettime.")
    set(TIME_SPEC_DEFS HAVE_CLOCK_GETTIME)
    APPEND_LIB_LIST( "-lrt")
elseif(HAVE_MACH_ABSOLUTE_TIME)
    message(STATUS "  High resolution timing supported through mach_absolute_time.")
    set(TIME_SPEC_DEFS HAVE_MACH_ABSOLUTE_TIME)
elseif(HAVE_QUERY_PERFORMANCE_COUNTER)
    message(STATUS "  High resolution timing supported through QueryPerformanceCounter.")
    set(TIME_SPEC_DEFS HAVE_QUERY_PERFORMANCE_COUNTER)
else()
    message(STATUS "  High resolution timing supported through microsec_clock.")
    set(TIME_SPEC_DEFS HAVE_MICROSEC_CLOCK)
endif()

set_source_files_properties(
    time_spec.cc
    PROPERTIES COMPILE_DEFINITIONS "${TIME_SPEC_DEFS}"
)

########################################################################
# Setup IQBalance component
########################################################################
GR_REGISTER_COMPONENT("Osmocom IQ Imbalance Correction" ENABLE_IQBALANCE gnuradio-iqbalance_FOUND)
if(ENABLE_IQBALANCE)
    add_definitions(-DHAVE_IQBALANCE=1)
    target_include_directories(gnuradio-osmosdr PRIVATE ${gnuradio-iqbalance_INCLUDE_DIRS})
    APPEND_LIB_LIST( gnuradio::gnuradio-iqbalance)
endif(ENABLE_IQBALANCE)

########################################################################
# Setup FCD component
########################################################################
GR_REGISTER_COMPONENT("FUNcube Dongle" ENABLE_FCD GNURADIO_FUNCUBE_FOUND)
if(ENABLE_FCD)
    add_subdirectory(fcd)
endif(ENABLE_FCD)

########################################################################
# Setup File component
########################################################################
GR_REGISTER_COMPONENT("IQ File Source & Sink" ENABLE_FILE gnuradio-blocks_FOUND)
if(ENABLE_FILE)
    add_subdirectory(file)
endif(ENABLE_FILE)

########################################################################
# Setup RTL component
########################################################################
GR_REGISTER_COMPONENT("Osmocom RTLSDR" ENABLE_RTL LIBRTLSDR_FOUND)
if(ENABLE_RTL)
    add_subdirectory(rtl)
endif(ENABLE_RTL)

########################################################################
# Setup RTL_TCP component
########################################################################
GR_REGISTER_COMPONENT("RTLSDR TCP Client" ENABLE_RTL_TCP gnuradio-blocks_FOUND)
if(ENABLE_RTL_TCP)
    add_subdirectory(rtl_tcp)
endif(ENABLE_RTL_TCP)

########################################################################
# Setup UHD component
########################################################################
GR_REGISTER_COMPONENT("Ettus USRP Devices" ENABLE_UHD UHD_FOUND gnuradio-uhd_FOUND)
if(ENABLE_UHD)
    add_subdirectory(uhd)
endif(ENABLE_UHD)

########################################################################
# Setup MiriSDR component
########################################################################
GR_REGISTER_COMPONENT("Osmocom MiriSDR" ENABLE_MIRI LIBMIRISDR_FOUND)
if(ENABLE_MIRI)
    add_subdirectory(miri)
endif(ENABLE_MIRI)

########################################################################
# Setup SDRplay component
########################################################################
if(ENABLE_NONFREE)
GR_REGISTER_COMPONENT("SDRplay RSP (NONFREE)" ENABLE_SDRPLAY LIBSDRPLAY_FOUND)
if(ENABLE_SDRPLAY)
    add_subdirectory(sdrplay)
endif(ENABLE_SDRPLAY)
endif(ENABLE_NONFREE)

########################################################################
# Setup HackRF component
########################################################################
GR_REGISTER_COMPONENT("HackRF & rad1o Badge" ENABLE_HACKRF LIBHACKRF_FOUND)
if(ENABLE_HACKRF)
    add_subdirectory(hackrf)
endif(ENABLE_HACKRF)

########################################################################
# Setup bladeRF component
########################################################################
GR_REGISTER_COMPONENT("nuand bladeRF" ENABLE_BLADERF LIBBLADERF_FOUND)
if(ENABLE_BLADERF)
    add_subdirectory(bladerf)
endif(ENABLE_BLADERF)

########################################################################
# Setup RFSPACE component
########################################################################
GR_REGISTER_COMPONENT("RFSPACE Receivers" ENABLE_RFSPACE)
if(ENABLE_RFSPACE)
    add_subdirectory(rfspace)
endif(ENABLE_RFSPACE)

########################################################################
# Setup AIRSPY component
########################################################################
GR_REGISTER_COMPONENT("AIRSPY Receiver" ENABLE_AIRSPY LIBAIRSPY_FOUND)
if(ENABLE_AIRSPY)
    add_subdirectory(airspy)
endif(ENABLE_AIRSPY)

########################################################################
# Setup AIRSPYHF component
########################################################################
GR_REGISTER_COMPONENT("AIRSPY HF+ Receiver" ENABLE_AIRSPYHF LIBAIRSPYHF_FOUND)
if(ENABLE_AIRSPYHF)
    add_subdirectory(airspyhf)
endif(ENABLE_AIRSPYHF)

########################################################################
# Setup SoapySDR component
########################################################################
GR_REGISTER_COMPONENT("SoapySDR support" ENABLE_SOAPY SoapySDR_FOUND)
if(ENABLE_SOAPY)
    add_subdirectory(soapy)
endif(ENABLE_SOAPY)

########################################################################
# Setup Red Pitaya component
########################################################################
GR_REGISTER_COMPONENT("Red Pitaya SDR" ENABLE_REDPITAYA)
if(ENABLE_REDPITAYA)
    add_subdirectory(redpitaya)
endif(ENABLE_REDPITAYA)

########################################################################
# Setup FreeSRP component
########################################################################
GR_REGISTER_COMPONENT("FreeSRP support" ENABLE_FREESRP LIBFREESRP_FOUND)
if(ENABLE_FREESRP)
    add_subdirectory(freesrp)
endif(ENABLE_FREESRP)

########################################################################
# Setup XTRX component
########################################################################
GR_REGISTER_COMPONENT("XTRX SDR" ENABLE_XTRX LIBXTRX_FOUND)
if(ENABLE_XTRX)
    add_subdirectory(xtrx)
endif(ENABLE_XTRX)

########################################################################
# Setup configuration file
########################################################################
add_definitions(-DHAVE_CONFIG_H=1)
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
@ONLY)

########################################################################
# Finalize target
########################################################################
set_target_properties(gnuradio-osmosdr PROPERTIES SOURCES "${gr_osmosdr_srcs}")
target_link_libraries(gnuradio-osmosdr ${gr_osmosdr_libs})

########################################################################
# Install built library files
########################################################################
include(GrMiscUtils)
GR_LIBRARY_FOO(gnuradio-osmosdr)
