# Copyright 2012 Free Software Foundation, Inc.
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
# Project setup
########################################################################
cmake_minimum_required(VERSION 3.8)
project(gr-osmosdr CXX C)
include(GNUInstallDirs)
enable_testing()

#policy setup
cmake_policy(SET CMP0011 NEW)

#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

########################################################################
# GNURadio setup
########################################################################
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

# Find GNURadio (pmt and runtime are core, always included)
include(FindPkgConfig)
find_package(Gnuradio "3.9" REQUIRED COMPONENTS blocks fft filter)

# Set the version information here
set(VERSION_MAJOR 0)
set(VERSION_API   2)
set(VERSION_ABI   0)
set(VERSION_PATCH 0)
include(GrVersion) #setup version info

########################################################################
# Compiler specific setup
########################################################################

if((CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    AND NOT WIN32)
    #http://gcc.gnu.org/wiki/Visibility
    add_definitions(-fvisibility=hidden)
    add_definitions(-fvisibility-inlines-hidden)
endif()

set(CMAKE_CXX_STANDARD 11)

# Misc options
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_definitions(-Wall)
    add_definitions(-Wextra)
    add_definitions(-Wno-unused-parameter)
    add_definitions(-Wsign-compare)
    #add_definitions(-Wconversion)
    #add_definitions(-pedantic)
    #add_definitions(-ansi)
endif()

# SIMD
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|x86")
    set(USE_SIMD "SSE2" CACHE STRING "Use SIMD instructions")
else()
    set(USE_SIMD "no" CACHE STRING "Use SIMD instructions")
endif()
set(USE_SIMD_VALUES "no" "SSE2" "AVX")
set_property(CACHE USE_SIMD PROPERTY STRINGS ${USE_SIMD_VALUES})
list(FIND USE_SIMD_VALUES ${USE_SIMD} USE_SIMD_INDEX)
if(${USE_SIMD_INDEX} EQUAL -1)
    message(FATAL_ERROR "Option ${USE_SIMD} not supported, valid entries are ${USE_SIMD_VALUES}")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(USE_SIMD MATCHES SSE2)
        add_definitions(-msse2)
        add_definitions(-DUSE_SSE2)
    endif()
    if(USE_SIMD MATCHES AVX)
        add_definitions(-march=native)
        add_definitions(-DUSE_AVX)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(USE_SIMD MATCHES SSE2)
        add_definitions(/arch:SSE2)
        add_definitions(-DUSE_SSE2)
    endif()
    if(USE_SIMD MATCHES AVX)
        add_definitions(/arch:AVX)
        add_definitions(-DUSE_AVX)
    endif()
    # boost feels like using lib pragmas to link to libs,
    # but the boost libs might not even be in the (default) lib search path
    add_definitions(-DBOOST_ALL_NO_LIB)
    # macro turns std::min into errors...
    add_definitions(-DNOMINMAX)
endif()

########################################################################
# Find boost
########################################################################
find_package(Boost "1.65" REQUIRED chrono thread system)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost required to compile osmosdr")
endif()

########################################################################
# Find build dependencies and setup options
########################################################################

include(GrComponent)

set(ENABLE_NONFREE FALSE CACHE BOOL "Enable or disable nonfree components.")


    # GNURadio components & OOTs
    ##############################


    # Note this is not supposed to be lique that for GR components
    # but ATM GR's handling of components is broken
message(STATUS "Searching for GNURadio-Blocks...")
find_package(gnuradio-blocks PATHS ${Gnuradio_DIR})
message(STATUS " Found GNURadio-Blocks: ${gnuradio-blocks_FOUND}")

message(STATUS "Searching for IQ Balance...")
find_package(gnuradio-iqbalance PATHS ${Gnuradio_DIR})
message (STATUS " Found IQ Balance: ${gnuradio-iqbalance_FOUND}")

message(STATUS "Searching for UHD Drivers...")
find_package(UHD)
message (STATUS " Found UHD Driver: ${UHD_FOUND}")

message(STATUS "Searching for UHD Block...")
find_package(gnuradio-uhd PATHS ${Gnuradio_DIR})
message (STATUS " Found UHD Block: ${gnuradio-uhd_FOUND}")

message(STATUS "Searching for Volk...")
find_package(Volk REQUIRED)
message (STATUS " Found Volk: ${Volk_FOUND}")

    # Hardware drivers
    ####################

find_package(LibRTLSDR)
find_package(LibMiriSDR)
if(ENABLE_NONFREE)
    find_package(LibSDRplay)
endif(ENABLE_NONFREE)
find_package(LibHackRF)
find_package(LibAIRSPY)
find_package(LibAIRSPYHF)
find_package(LibbladeRF)
find_package(GnuradioFuncube)
find_package(SoapySDR NO_MODULE)
find_package(LibFreeSRP)
find_package(LibXTRX)
find_package(Doxygen)

    # Python
    ##########

find_package(PythonLibs 3)
find_package(pybind11)

GR_REGISTER_COMPONENT("Python support" ENABLE_PYTHON
    PYTHONLIBS_FOUND
    pybind11_FOUND
)

########################################################################
# Install directories
########################################################################
include(GrPlatform) #define LIB_SUFFIX

if(NOT CMAKE_MODULES_DIR)
    set(CMAKE_MODULES_DIR lib${LIB_SUFFIX}/cmake)
endif(NOT CMAKE_MODULES_DIR)

set(GR_INCLUDE_DIR      include)
set(GR_CMAKE_DIR        ${CMAKE_MODULES_DIR}/osmosdr)
set(GR_PKG_DATA_DIR     ${GR_DATA_DIR}/${CMAKE_PROJECT_NAME})
set(GR_PKG_DOC_DIR      ${GR_DOC_DIR}/${CMAKE_PROJECT_NAME})
set(GR_PKG_CONF_DIR     ${GR_CONF_DIR}/${CMAKE_PROJECT_NAME}/conf.d)
set(GR_PKG_LIBEXEC_DIR  ${GR_LIBEXEC_DIR}/${CMAKE_PROJECT_NAME})

########################################################################
# On Apple only, set install name and use rpath correctly, if not already set
########################################################################
if(APPLE)
    if(NOT CMAKE_INSTALL_NAME_DIR)
        set(CMAKE_INSTALL_NAME_DIR
            ${CMAKE_INSTALL_PREFIX}/${GR_LIBRARY_DIR} CACHE
            PATH "Library Install Name Destination Directory" FORCE)
    endif(NOT CMAKE_INSTALL_NAME_DIR)
    if(NOT CMAKE_INSTALL_RPATH)
        set(CMAKE_INSTALL_RPATH
            ${CMAKE_INSTALL_PREFIX}/${GR_LIBRARY_DIR} CACHE
            PATH "Library Install RPath" FORCE)
    endif(NOT CMAKE_INSTALL_RPATH)
    if(NOT CMAKE_BUILD_WITH_INSTALL_RPATH)
        set(CMAKE_BUILD_WITH_INSTALL_RPATH ON CACHE
            BOOL "Do Build Using Library Install RPath" FORCE)
    endif(NOT CMAKE_BUILD_WITH_INSTALL_RPATH)
endif(APPLE)

########################################################################
# Setup doxygen option
########################################################################
find_package(Doxygen)

if(DOXYGEN_FOUND)
    option(ENABLE_DOXYGEN "Build docs using Doxygen" ON)
else(DOXYGEN_FOUND)
    option(ENABLE_DOXYGEN "Build docs using Doxygen" OFF)
endif(DOXYGEN_FOUND)

########################################################################
# Create uninstall target
########################################################################
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
@ONLY)

add_custom_target(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    )

########################################################################
# Add subdirectories
########################################################################
add_subdirectory(include/osmosdr)
add_subdirectory(lib)
if(ENABLE_PYTHON)
    add_subdirectory(python)
    add_subdirectory(grc)
    add_subdirectory(apps)
endif(ENABLE_PYTHON)
add_subdirectory(docs)

########################################################################
# Print Summary
########################################################################
GR_PRINT_COMPONENT_SUMMARY()
if(ENABLE_NONFREE)
    MESSAGE(STATUS
            "NONFREE components have been enabled. The resulting
   binaries cannot be distributed under GPL terms.
  ")
endif(ENABLE_NONFREE)

MESSAGE(STATUS "Building for version: ${VERSION} / ${LIBVER}")
MESSAGE(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
