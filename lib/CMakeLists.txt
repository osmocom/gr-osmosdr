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
INCLUDE(GrComponent)

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
    source_impl.cc
    sink_impl.cc
    ranges.cc
    device.cc
)

GR_OSMOSDR_APPEND_LIBS(
    ${Boost_LIBRARIES}
    ${GNURADIO_RUNTIME_LIBRARIES}
    ${GNURADIO_PMT_LIBRARIES}
    ${GNURADIO_BLOCKS_LIBRARIES}
)

########################################################################
# Set up built-in GNU Radio runtime component
########################################################################
GR_REGISTER_COMPONENT("Built-in GNU Radio runtime" ENABLE_RUNTIME RUNTIME_MODE)
if(ENABLE_RUNTIME)

message(STATUS "")
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/ConfigChecks.cmake)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR}/runtime)

GR_OSMOSDR_APPEND_SRCS(
    runtime/blocks/throttle_impl.cc
    runtime/blocks/null_sink_impl.cc
    runtime/blocks/null_source_impl.cc

    runtime/math/fast_atan2f.cc
    runtime/math//fxpt.cc
    runtime/math/random.cc
    runtime/math/sincos.cc

    runtime/thread/thread.cc
    runtime/thread/thread_body_wrapper.cc
    runtime/thread/thread_group.cc

    runtime/basic_block.cc
    runtime/block.cc
    runtime/block_detail.cc
    runtime/block_executor.cc
    runtime/block_gateway_impl.cc
    runtime/block_registry.cc
    runtime/buffer.cc
    runtime/circular_file.cc
    runtime/feval.cc
    runtime/flat_flowgraph.cc
    runtime/flowgraph.cc
    runtime/hier_block2.cc
    runtime/hier_block2_detail.cc
    runtime/high_res_timer.cc
    runtime/io_signature.cc
    runtime/local_sighandler.cc
    runtime/logger.cc
    runtime/misc.cc
    runtime/pagesize.cc
    runtime/prefs.cc
    runtime/realtime.cc
    runtime/realtime_impl.cc
    runtime/scheduler.cc
    runtime/scheduler_sts.cc
    runtime/scheduler_tpb.cc
    runtime/single_threaded_scheduler.cc
    runtime/sptr_magic.cc
    runtime/sync_block.cc
    runtime/sys_paths.cc
    runtime/top_block.cc
    runtime/top_block_impl.cc
    runtime/tpb_detail.cc
    runtime/tpb_thread_body.cc
    runtime/vmcircbuf.cc
    runtime/vmcircbuf_createfilemapping.cc
    runtime/vmcircbuf_mmap_shm_open.cc
    runtime/vmcircbuf_mmap_tmpfile.cc
    runtime/vmcircbuf_prefs.cc
    runtime/vmcircbuf_sysv_shm.cc
)

########################################################################
# Handle the generated constants
########################################################################
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
    "import time;print time.strftime('%a, %d %b %Y %H:%M:%S', time.gmtime())"
    OUTPUT_VARIABLE BUILD_DATE OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(GR_VERSION "built-in")

message(STATUS "Loading prefix ${prefix} into constants...")
message(STATUS "Loading SYSCONFDIR ${SYSCONFDIR} into constants...")
message(STATUS "Loading GR_PREFSDIR ${GR_PREFSDIR} into constants...")
message(STATUS "Loading build date ${BUILD_DATE} into constants...")
message(STATUS "Loading version ${GR_VERSION} into constants...")

#double escape for windows backslash path separators
string(REPLACE "\\" "\\\\" prefix ${prefix})
string(REPLACE "\\" "\\\\" SYSCONFDIR ${SYSCONFDIR})
string(REPLACE "\\" "\\\\" GR_PREFSDIR ${GR_PREFSDIR})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime/constants.cc.in
    ${CMAKE_CURRENT_BINARY_DIR}/constants.cc
@ONLY)

GR_OSMOSDR_APPEND_SRCS(${CMAKE_CURRENT_BINARY_DIR}/constants.cc)

# PowerPC workaround for posix_memalign
# Might not be needed, but we'll keep it for now.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)")
    GR_OSMOSDR_APPEND_SRCS(runtime/posix_memalign.cc)
endif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(powerpc|ppc)")

########################################################################
# Will set TRY_SHM_VMCIRCBUF to 1 by default except on Windows machines.
# Can manually set with -DTRY_SHM_VMCIRCBUF=0|1
########################################################################
if(WIN32)
    OPTION(TRY_SHM_VMCIRCBUF "Try SHM VMCIRCBUF" OFF)
else(WIN32)
    OPTION(TRY_SHM_VMCIRCBUF "Try SHM VMCIRCBUF" ON)
endif(WIN32)

message(STATUS "TRY_SHM_VMCIRCBUF set to ${TRY_SHM_VMCIRCBUF}.")

if(TRY_SHM_VMCIRCBUF)
    ADD_DEFINITIONS( -DTRY_SHM_VMCIRCBUF )
endif(TRY_SHM_VMCIRCBUF)

########################################################################
# Add libraries for winsock2.h on Windows
########################################################################
CHECK_INCLUDE_FILE_CXX(windows.h HAVE_WINDOWS_H)
if(HAVE_WINDOWS_H)
    include_directories(${CMAKE_SOURCE_DIR}/cmake/msvc)
    ADD_DEFINITIONS(-DHAVE_WINDOWS_H -DUSING_WINSOCK -DWIN32_LEAN_AND_MEAN)
    ADD_DEFINITIONS(-DNOMINMAX -D_USE_MATH_DEFINES)
    ADD_DEFINITIONS(/MP -D_CRT_SECURE_NO_WARNINGS -DD_CRT_SECURE_NO_DEPRECATE)
    MESSAGE(STATUS "Adding windows libs to gnuradio osmosdr lib...")
    GR_OSMOSDR_APPEND_LIBS(WS2_32.lib WSock32.lib)
endif(HAVE_WINDOWS_H)

########################################################################
# Need to link with librt for shm_*
########################################################################
if(LINUX)
    GR_OSMOSDR_APPEND_LIBS(rt)
endif()

endif(ENABLE_RUNTIME)

########################################################################
# Setup IQBalance component
########################################################################
GR_REGISTER_COMPONENT("Osmocom IQ Imbalance Correction" ENABLE_IQBALANCE GNURADIO_IQBALANCE_FOUND)
if(ENABLE_IQBALANCE)
add_definitions(-DHAVE_IQBALANCE=1)
include_directories(${GNURADIO_IQBALANCE_INCLUDE_DIRS})
GR_OSMOSDR_APPEND_LIBS(${GNURADIO_IQBALANCE_LIBRARIES})
endif(ENABLE_IQBALANCE)

########################################################################
# Setup OsmoSDR component
########################################################################
GR_REGISTER_COMPONENT("sysmocom OsmoSDR" ENABLE_OSMOSDR LIBOSMOSDR_FOUND)
if(ENABLE_OSMOSDR)
GR_INCLUDE_SUBDIRECTORY(osmosdr)
endif(ENABLE_OSMOSDR)

########################################################################
# Setup FCD component
########################################################################
GR_REGISTER_COMPONENT("FUNcube Dongle" ENABLE_FCD GNURADIO_FCD_FOUND)
GR_REGISTER_COMPONENT("FUNcube Dongle Pro+" ENABLE_FCDPP GNURADIO_FCDPP_FOUND)
if(ENABLE_FCD)
add_definitions(-DHAVE_FCD=1)
endif(ENABLE_FCD)
if(ENABLE_FCDPP)
add_definitions(-DHAVE_FCDPP=1)
endif(ENABLE_FCDPP)
if(ENABLE_FCD OR ENABLE_FCDPP)
GR_INCLUDE_SUBDIRECTORY(fcd)
endif(ENABLE_FCD OR ENABLE_FCDPP)

########################################################################
# Setup File component
########################################################################
GR_REGISTER_COMPONENT("IQ File Source" ENABLE_FILE GNURADIO_BLOCKS_FOUND)
if(ENABLE_FILE)
GR_INCLUDE_SUBDIRECTORY(file)
endif(ENABLE_FILE)

########################################################################
# Setup RTL component
########################################################################
GR_REGISTER_COMPONENT("Osmocom RTLSDR" ENABLE_RTL LIBRTLSDR_FOUND)
if(ENABLE_RTL)
GR_INCLUDE_SUBDIRECTORY(rtl)
endif(ENABLE_RTL)

########################################################################
# Setup RTL_TCP component
########################################################################
GR_REGISTER_COMPONENT("RTLSDR TCP Client" ENABLE_RTL_TCP GNURADIO_BLOCKS_FOUND)
if(ENABLE_RTL_TCP)
GR_INCLUDE_SUBDIRECTORY(rtl_tcp)
endif(ENABLE_RTL_TCP)

########################################################################
# Setup UHD component
########################################################################
GR_REGISTER_COMPONENT("Ettus USRP Devices" ENABLE_UHD UHD_FOUND)
if(ENABLE_UHD)
GR_INCLUDE_SUBDIRECTORY(uhd)
endif(ENABLE_UHD)

########################################################################
# Setup MiriSDR component
########################################################################
GR_REGISTER_COMPONENT("Osmocom MiriSDR" ENABLE_MIRI LIBMIRISDR_FOUND)
if(ENABLE_MIRI)
GR_INCLUDE_SUBDIRECTORY(miri)
endif(ENABLE_MIRI)

########################################################################
# Setup HackRF component
########################################################################
GR_REGISTER_COMPONENT("HackRF Jawbreaker" ENABLE_HACKRF LIBHACKRF_FOUND)
if(ENABLE_HACKRF)
GR_INCLUDE_SUBDIRECTORY(hackrf)
endif(ENABLE_HACKRF)

########################################################################
# Setup bladeRF component
########################################################################
GR_REGISTER_COMPONENT("nuand bladeRF" ENABLE_BLADERF LIBBLADERF_FOUND)
if(ENABLE_BLADERF)
GR_INCLUDE_SUBDIRECTORY(bladerf)
endif(ENABLE_BLADERF)

########################################################################
# Setup configuration file
########################################################################
ADD_DEFINITIONS(-DHAVE_CONFIG_H=1)
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR})
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
@ONLY)

########################################################################
# Set up Windows DLL resource files
########################################################################
IF(MSVC)
    include(${CMAKE_SOURCE_DIR}/cmake/Modules/GrVersion.cmake)

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/gnuradio-osmosdr.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/gnuradio-osmosdr.rc
    @ONLY)

    GR_OSMOSDR_APPEND_SRCS(${CMAKE_CURRENT_BINARY_DIR}/gnuradio-osmosdr.rc)
ENDIF(MSVC)

########################################################################
# Setup libgnuradio-osmosdr library
########################################################################
ADD_LIBRARY(gnuradio-osmosdr SHARED ${gr_osmosdr_srcs})
TARGET_LINK_LIBRARIES(gnuradio-osmosdr ${gr_osmosdr_libs})
SET_TARGET_PROPERTIES(gnuradio-osmosdr PROPERTIES DEFINE_SYMBOL "gnuradio_osmosdr_EXPORTS")
GR_LIBRARY_FOO(gnuradio-osmosdr)

if(ENABLE_RUNTIME)
    SET_TARGET_PROPERTIES(gnuradio-osmosdr PROPERTIES DEFINE_SYMBOL "gnuradio_runtime_EXPORTS")
    SET_TARGET_PROPERTIES(gnuradio-osmosdr PROPERTIES DEFINE_SYMBOL "gnuradio_blocks_EXPORTS")
endif(ENABLE_RUNTIME)
