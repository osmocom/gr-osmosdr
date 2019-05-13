# Copyright 2012 Free Software Foundation, Inc.
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
# Project setup
########################################################################
cmake_minimum_required(VERSION 3.8)
project(gr-osmosdr CXX C)
enable_testing()

#install to PyBOMBS target prefix if defined
if(DEFINED ENV{PYBOMBS_PREFIX})
    set(CMAKE_INSTALL_PREFIX $ENV{PYBOMBS_PREFIX})
    message(STATUS "PyBOMBS installed GNU Radio. Setting CMAKE_INSTALL_PREFIX to $ENV{PYBOMBS_PREFIX}")
endif()

# Get GNU Radio Installation Prefix
if(DEFINED CMAKE_INSTALL_PREFIX)
    set(GR_PREFIX ${CMAKE_INSTALL_PREFIX})
    message(STATUS "Installation is located at: ${GR_PREFIX}")
endif()

#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

#make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

set(ENABLE_NONFREE FALSE CACHE BOOL "Enable or disable nonfree components.")

# Set the version information here
set(VERSION_INFO_MAJOR_VERSION 0)
set(VERSION_INFO_API_COMPAT    1)
set(VERSION_INFO_MINOR_VERSION 5)
set(VERSION_INFO_MAINT_VERSION git)

cmake_policy(SET CMP0011 NEW)


########################################################################
# Compiler specific setup
########################################################################
IF(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|x86")
    SET(USE_SIMD "SSE2" CACHE STRING "Use SIMD instructions")
ELSE()
    SET(USE_SIMD "no" CACHE STRING "Use SIMD instructions")
ENDIF()
SET(USE_SIMD_VALUES "no" "SSE2" "AVX")
SET_PROPERTY(CACHE USE_SIMD PROPERTY STRINGS ${USE_SIMD_VALUES})
LIST(FIND USE_SIMD_VALUES ${USE_SIMD} USE_SIMD_INDEX)
IF(${USE_SIMD_INDEX} EQUAL -1)
    message(FATAL_ERROR "Option ${USE_SIMD} not supported, valid entries are ${USE_SIMD_VALUES}")
ENDIF()

IF(CMAKE_CXX_COMPILER MATCHES ".*clang")
    SET(CMAKE_COMPILER_IS_CLANGXX 1)
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
    ADD_DEFINITIONS(-Wall)
    ADD_DEFINITIONS(-Wextra)
    ADD_DEFINITIONS(-Wno-unused-parameter)
    ADD_DEFINITIONS(-Wsign-compare)
    #ADD_DEFINITIONS(-Wconversion)
    #ADD_DEFINITIONS(-pedantic)
    #ADD_DEFINITIONS(-ansi)
    IF(NOT WIN32)
        #only export symbols that are declared to be part of the api (non dll platforms)
        ADD_DEFINITIONS(-fvisibility=hidden)
        ADD_DEFINITIONS(-fvisibility-inlines-hidden)
    ENDIF(NOT WIN32)
    IF(USE_SIMD MATCHES SSE2)
        ADD_DEFINITIONS(-msse2)
        ADD_DEFINITIONS(-DUSE_SSE2)
    ENDIF()
    IF(USE_SIMD MATCHES AVX)
        ADD_DEFINITIONS(-march=native)
        ADD_DEFINITIONS(-DUSE_AVX)
    ENDIF()
ELSEIF(MSVC)
    IF(USE_SIMD MATCHES SSE2)
        ADD_DEFINITIONS(/arch:SSE2)
        ADD_DEFINITIONS(-DUSE_SSE2)
    ENDIF()
    IF(USE_SIMD MATCHES AVX)
        ADD_DEFINITIONS(/arch:AVX)
        ADD_DEFINITIONS(-DUSE_AVX)
    ENDIF()
ENDIF()


########################################################################
# Install directories
########################################################################
find_package(Gnuradio REQUIRED) # Side effect of MODULE_PATH getting prepended with system components
list(REVERSE CMAKE_MODULE_PATH)
set(MIN_GR_VERSION "3.8")
if("${Gnuradio_VERSION}" VERSION_LESS MIN_GR_VERSION)
    MESSAGE(FATAL_ERROR "GnuRadio version required: >=\"" ${MIN_GR_VERSION} "\" found: \"" ${Gnuradio_VERSION} "\"")
endif()

include(GrVersion)

include(GrPlatform) #define LIB_SUFFIX
if(NOT CMAKE_MODULES_DIR)
    set(CMAKE_MODULES_DIR lib${LIB_SUFFIX}/cmake)
endif(NOT CMAKE_MODULES_DIR)

set(GR_CMAKE_DIR        ${CMAKE_MODULES_DIR}/osmosdr)
set(GR_RUNTIME_DIR      bin)
set(GR_LIBRARY_DIR      lib${LIB_SUFFIX})
set(GR_INCLUDE_DIR      include)
set(GR_DATA_DIR         share)
set(GR_PKG_DATA_DIR     ${GR_DATA_DIR}/${CMAKE_PROJECT_NAME})
set(GR_DOC_DIR          ${GR_DATA_DIR}/doc)
if (NOT GR_PKG_DOC_DIR)
    set(GR_PKG_DOC_DIR      ${GR_DOC_DIR}/${CMAKE_PROJECT_NAME})
endif()
set(GR_CONF_DIR         etc)
set(GR_PKG_CONF_DIR     ${GR_CONF_DIR}/${CMAKE_PROJECT_NAME}/conf.d)
set(GR_LIBEXEC_DIR      libexec)
set(GR_PKG_LIBEXEC_DIR  ${GR_LIBEXEC_DIR}/${CMAKE_PROJECT_NAME})
set(GRC_BLOCKS_DIR      ${GR_PKG_DATA_DIR}/grc/blocks)

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
# Find build dependencies
########################################################################
set(GR_REQUIRED_COMPONENTS RUNTIME PMT BLOCKS)

find_package(gnuradio-blocks PATHS ${GR_PREFIX}/lib/cmake/gnuradio/)
message(STATUS "Found Block Block: ${gnuradio-blocks_FOUND}")

#[[find_package(gnuradio-pmt PATHS ${GR_PREFIX}/lib/cmake/gnuradio/)
message(STATUS "Found PMT Block: ${gnuradio-pmt_FOUND}")

find_package(gnuradio-runtime PATHS ${GR_PREFIX}/lib/cmake/gnuradio/)
message(STATUS "Found Runtime Block: ${gnuradio-runtime_FOUND}")

# Software Components part of GNU Radio - These should all be present for a default install
message(STATUS " Searching for IQ Balance...")
#find_package(Gnuradio COMPONENTS iqbalance REQUIRED)
find_package(gnuradio-iqbalance PATHS ${GR_PREFIX}/lib/cmake/iqbalance)
message (STATUS "Found IQ Balance: ${gnuradio-iqbalance_FOUND}")

message(STATUS " Searching for UHD Drivers...")
#find_package(Gnuradio COMPONENTS uhd REQUIRED)
find_package(UHD REQUIRED ${GR_PREFIX}/lib/cmake/uhd)
message (STATUS "Found UHD Driver: ${UHD_FOUND}")

message(STATUS " Searching for UHD Block...")
#find_package(Gnuradio COMPONENTS gnuradio-uhd REQUIRED)
find_package(gnuradio-uhd PATHS ${GR_PREFIX}/lib/cmake/gnuradio/)
message (STATUS "Found UHD Block: ${gnuradio-uhd_FOUND}")

message(STATUS " Searching for Volk...")
#find_package(Gnuradio COMPONENTS Volk REQUIRED)
find_package(volk PATHS ${GR_PREFIX}/lib/cmake/volk)
message (STATUS "Found Volk: ${volk_FOUND}") ]]#

find_package(GnuradioIQBalance)
find_package(UHD)
find_package(GnuradioUHD)
find_package(LibOsmoSDR)
find_package(LibRTLSDR)
find_package(LibMiriSDR)
if(ENABLE_NONFREE)
    find_package(LibSDRplay)
endif(ENABLE_NONFREE)
find_package(LibHackRF)
find_package(LibAIRSPY)
find_package(Volk)
find_package(LibbladeRF)
find_package(SoapySDR NO_MODULE)
find_package(LibFreeSRP)
find_package(Doxygen)

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


########################################################################
# Setup doxygen option
########################################################################
if(DOXYGEN_FOUND)
    option(ENABLE_DOXYGEN "Build docs using Doxygen" ON)
else(DOXYGEN_FOUND)
    option(ENABLE_DOXYGEN "Build docs using Doxygen" OFF)
endif(DOXYGEN_FOUND)

########################################################################
# Setup the include and linker paths
########################################################################
include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/lib
        ${Boost_INCLUDE_DIRS}
        ${GNURADIO_ALL_INCLUDE_DIRS}
)

link_directories(
        ${Boost_LIBRARY_DIRS}
        ${GNURADIO_ALL_LIBRARY_DIRS}
)

# Set component parameters
set(GR_OSMOSDR_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "" FORCE)
set(GR_OSMOSDR_SWIG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/swig CACHE INTERNAL "" FORCE)

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
# Enable python component
########################################################################
find_package(PythonLibs)
find_package(SWIG)

if(SWIG_FOUND)
    message(STATUS "Minimum SWIG version required is 1.3.31")
    set(SWIG_VERSION_CHECK FALSE)
    if("${SWIG_VERSION}" VERSION_GREATER "1.3.30")
        set(SWIG_VERSION_CHECK TRUE)
    endif()
endif(SWIG_FOUND)

include(GrComponent)
GR_REGISTER_COMPONENT("Python support" ENABLE_PYTHON
        PYTHONLIBS_FOUND
        SWIG_FOUND
        SWIG_VERSION_CHECK
        )


########################################################################
# Add subdirectories
########################################################################
add_subdirectory(include/osmosdr)
add_subdirectory(lib)
if(ENABLE_PYTHON)
    add_subdirectory(swig)
    add_subdirectory(python)
    add_subdirectory(grc)
    add_subdirectory(apps)
endif(ENABLE_PYTHON)
add_subdirectory(docs)

########################################################################
# Install cmake search helper for this library
########################################################################

install(FILES cmake/Modules/osmosdrConfig.cmake
    DESTINATION ${CMAKE_MODULES_DIR}/osmosdr
)

########################################################################
# Create Pkg Config File
########################################################################
FOREACH(inc ${Boost_INCLUDE_DIRS})
    LIST(APPEND GR_OSMOSDR_PC_CFLAGS "-I${inc}")
ENDFOREACH(inc)

FOREACH(lib ${Boost_LIBRARY_DIRS})
    LIST(APPEND GR_OSMOSDR_PC_LIBS "-L${lib}")
ENDFOREACH(lib)

# use space-separation format for the pc file
STRING(REPLACE ";" " " GR_OSMOSDR_PC_REQUIRES "${GR_OSMOSDR_PC_REQUIRES}")
STRING(REPLACE ";" " " GR_OSMOSDR_PC_CFLAGS "${GR_OSMOSDR_PC_CFLAGS}")
STRING(REPLACE ";" " " GR_OSMOSDR_PC_LIBS "${GR_OSMOSDR_PC_LIBS}")

# unset these vars to avoid hard-coded paths to cross environment
IF(CMAKE_CROSSCOMPILING)
    UNSET(GR_OSMOSDR_PC_CFLAGS)
    UNSET(GR_OSMOSDR_PC_LIBS)
ENDIF(CMAKE_CROSSCOMPILING)

# fake gnuradio cpack behavior as long as we don't use it directly
set(CPACK_PACKAGE_NAME "gnuradio-osmosdr")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GNU Radio block for various radio hardware")
set(CPACK_PACKAGE_VERSION ${VERSION})

CONFIGURE_FILE(
        ${CMAKE_CURRENT_SOURCE_DIR}/gnuradio-osmosdr.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/gnuradio-osmosdr.pc
        @ONLY)

INSTALL(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/gnuradio-osmosdr.pc
        DESTINATION ${GR_LIBRARY_DIR}/pkgconfig
)

########################################################################
# Print Summary
########################################################################
GR_PRINT_COMPONENT_SUMMARY()
if(ENABLE_NONFREE)
    MESSAGE(STATUS
            "NONFREE components have been enabled. The resulting
   binaries cannot be distributed under GPL terms.
  "
            )
endif(ENABLE_NONFREE)

MESSAGE(STATUS "Building for version: ${VERSION} / ${LIBVER}")
MESSAGE(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
