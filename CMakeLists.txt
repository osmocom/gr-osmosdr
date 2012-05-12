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
cmake_minimum_required(VERSION 2.6)
project(gr-osmosdr CXX C)
enable_testing()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

#select the release build type by default to get optimization flags
if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release")
   message(STATUS "Build type not specified: defaulting to release.")
endif(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

# Set the version information here
set(VERSION_INFO_MAJOR_VERSION 0)
set(VERSION_INFO_API_COMPAT    0)
set(VERSION_INFO_MINOR_VERSION 1)
set(VERSION_INFO_MAINT_VERSION git)
include(GrVersion) #setup version info

########################################################################
# Compiler specific setup
########################################################################
IF(CMAKE_COMPILER_IS_GNUCXX)
    ADD_DEFINITIONS(-Wall)
    ADD_DEFINITIONS(-Wextra)
    ADD_DEFINITIONS(-Wno-unused)
    ADD_DEFINITIONS(-Wsign-compare)
    #ADD_DEFINITIONS(-Wconversion)
    #ADD_DEFINITIONS(-pedantic)
    #ADD_DEFINITIONS(-ansi)
    IF(NOT WIN32)
        #only export symbols that are declared to be part of the api (non dll platforms)
        ADD_DEFINITIONS(-fvisibility=hidden)
        ADD_DEFINITIONS(-fvisibility-inlines-hidden)
    ENDIF(NOT WIN32)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

########################################################################
# Setup boost
########################################################################
MESSAGE(STATUS "Configuring Boost C++ Libraries...")

SET(BOOST_REQUIRED_COMPONENTS
    thread
)

if(UNIX AND EXISTS "/usr/lib64")
    list(APPEND BOOST_LIBRARYDIR "/usr/lib64") #fedora 64-bit fix
endif(UNIX AND EXISTS "/usr/lib64")

set(Boost_ADDITIONAL_VERSIONS
    "1.35.0" "1.35" "1.36.0" "1.36" "1.37.0" "1.37" "1.38.0" "1.38" "1.39.0" "1.39"
    "1.40.0" "1.40" "1.41.0" "1.41" "1.42.0" "1.42" "1.43.0" "1.43" "1.44.0" "1.44"
    "1.45.0" "1.45" "1.46.0" "1.46" "1.47.0" "1.47" "1.48.0" "1.48" "1.49.0" "1.49"
    "1.50.0" "1.50" "1.51.0" "1.51" "1.52.0" "1.52" "1.53.0" "1.53" "1.54.0" "1.54"
    "1.55.0" "1.55" "1.56.0" "1.56" "1.57.0" "1.57" "1.58.0" "1.58" "1.59.0" "1.59"
    "1.60.0" "1.60" "1.61.0" "1.61" "1.62.0" "1.62" "1.63.0" "1.63" "1.64.0" "1.64"
    "1.65.0" "1.65" "1.66.0" "1.66" "1.67.0" "1.67" "1.68.0" "1.68" "1.69.0" "1.69"
)
find_package(Boost 1.35 COMPONENTS ${BOOST_REQUIRED_COMPONENTS})

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost required to compile osmosdr")
endif()

########################################################################
# Install directories
########################################################################
include(GrPlatform) #define LIB_SUFFIX
set(GR_RUNTIME_DIR      bin)
set(GR_LIBRARY_DIR      lib${LIB_SUFFIX})
set(GR_INCLUDE_DIR      include)
set(GR_DATA_DIR         share)
set(GR_PKG_DATA_DIR     ${GR_DATA_DIR}/${CMAKE_PROJECT_NAME})
set(GR_DOC_DIR          ${GR_DATA_DIR}/doc)
set(GR_PKG_DOC_DIR      ${GR_DOC_DIR}/${CMAKE_PROJECT_NAME})
set(GR_CONF_DIR         etc)
set(GR_PKG_CONF_DIR     ${GR_CONF_DIR}/${CMAKE_PROJECT_NAME}/conf.d)
set(GR_LIBEXEC_DIR      libexec)
set(GR_PKG_LIBEXEC_DIR  ${GR_LIBEXEC_DIR}/${CMAKE_PROJECT_NAME})
set(GRC_BLOCKS_DIR      ${GR_PKG_DATA_DIR}/grc/blocks)

########################################################################
# Find gnuradio build dependencies
########################################################################
find_package(Gruel)
find_package(GnuradioCore)
find_package(GnuradioAudio)
find_package(GnuradioUHD)
find_package(GnuradioFCD)
find_package(LibRTLSDR)

if(NOT GRUEL_FOUND)
    message(FATAL_ERROR "Gruel required to compile osmosdr")
endif()

if(NOT GNURADIO_CORE_FOUND)
    message(FATAL_ERROR "GnuRadio Core required to compile osmosdr")
endif()

########################################################################
# Setup the include and linker paths
########################################################################
include_directories(
    ${CMAKE_SOURCE_DIR}/include/osmosdr
    ${CMAKE_SOURCE_DIR}/lib
    ${Boost_INCLUDE_DIRS}
    ${GRUEL_INCLUDE_DIRS}
    ${GNURADIO_CORE_INCLUDE_DIRS}
)

link_directories(
    ${Boost_LIBRARY_DIRS}
    ${GRUEL_LIBRARY_DIRS}
    ${GNURADIO_CORE_LIBRARY_DIRS}
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
# Add subdirectories
########################################################################
add_subdirectory(include/osmosdr)
add_subdirectory(lib)
add_subdirectory(swig)
add_subdirectory(python)
add_subdirectory(grc)
add_subdirectory(apps)
add_subdirectory(docs)

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
MESSAGE(STATUS "Building for version: ${VERSION} / ${LIBVER}")
MESSAGE(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
