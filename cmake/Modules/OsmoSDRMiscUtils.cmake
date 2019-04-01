# Copyright 2010-2011,2014 Free Software Foundation, Inc.
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

if(DEFINED __INCLUDED_OSMOSDR_MISC_UTILS_CMAKE)
    return()
endif()
set(__INCLUDED_OSMOSDR_MISC_UTILS_CMAKE TRUE)

########################################################################
# This macro includes subdirectories rather than adding them
# so that the subdirectory can affect variables in the level above.
# This provides a work-around for the lack of convenience libraries.
# This way a subdirectory can append to the list of library sources.
########################################################################
macro(GR_INCLUDE_SUBDIRECTORY subdir)
    #insert the current directories on the front of the list
    list(INSERT _cmake_source_dirs 0 ${CMAKE_CURRENT_SOURCE_DIR})
    list(INSERT _cmake_binary_dirs 0 ${CMAKE_CURRENT_BINARY_DIR})

    #set the current directories to the names of the subdirs
    set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
    set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${subdir})

    #include the subdirectory CMakeLists to run it
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

    #reset the value of the current directories
    list(GET _cmake_source_dirs 0 CMAKE_CURRENT_SOURCE_DIR)
    list(GET _cmake_binary_dirs 0 CMAKE_CURRENT_BINARY_DIR)

    #pop the subdir names of the front of the list
    list(REMOVE_AT _cmake_source_dirs 0)
    list(REMOVE_AT _cmake_binary_dirs 0)
endmacro(GR_INCLUDE_SUBDIRECTORY)


########################################################################
# Generates the .la libtool file
# This appears to generate libtool files that cannot be used by auto*.
# Usage GR_LIBTOOL(TARGET [target] DESTINATION [dest])
# Notice: there is not COMPONENT option, these will not get distributed.
########################################################################
function(GR_LIBTOOL)
    if(NOT DEFINED GENERATE_LIBTOOL)
        set(GENERATE_LIBTOOL OFF) #disabled by default
    endif()

    if(GENERATE_LIBTOOL)
        include(CMakeParseArgumentsCopy)
        CMAKE_PARSE_ARGUMENTS(GR_LIBTOOL "" "TARGET;DESTINATION" "" ${ARGN})

        find_program(LIBTOOL libtool)
        if(LIBTOOL)
            include(CMakeMacroLibtoolFile)
            CREATE_LIBTOOL_FILE(${GR_LIBTOOL_TARGET} /${GR_LIBTOOL_DESTINATION})
        endif(LIBTOOL)
    endif(GENERATE_LIBTOOL)

endfunction(GR_LIBTOOL)

########################################################################
# Do standard things to the library target
# - set target properties
# - make install rules
# Also handle gnuradio custom naming conventions w/ extras mode.
########################################################################
function(GR_LIBRARY_FOO_V2 target)
    #parse the arguments for component names
    include(CMakeParseArgumentsCopy)
    CMAKE_PARSE_ARGUMENTS(GR_LIBRARY "" "RUNTIME_COMPONENT;DEVEL_COMPONENT" "" ${ARGN})

    #set additional target properties
    set_target_properties(${target} PROPERTIES SOVERSION ${LIBVER})

    #install the generated files like so...
    install(TARGETS ${target}
            LIBRARY DESTINATION ${GR_LIBRARY_DIR} COMPONENT ${GR_LIBRARY_RUNTIME_COMPONENT} # .so/.dylib file
            ARCHIVE DESTINATION ${GR_LIBRARY_DIR} COMPONENT ${GR_LIBRARY_DEVEL_COMPONENT}   # .lib file
            RUNTIME DESTINATION ${GR_RUNTIME_DIR} COMPONENT ${GR_LIBRARY_RUNTIME_COMPONENT} # .dll file
            )

    #extras mode enabled automatically on linux
    if(NOT DEFINED LIBRARY_EXTRAS)
        set(LIBRARY_EXTRAS ${LINUX})
    endif()

    #special extras mode to enable alternative naming conventions
    if(LIBRARY_EXTRAS)

        #create .la file before changing props
        GR_LIBTOOL(TARGET ${target} DESTINATION ${GR_LIBRARY_DIR})

        #give the library a special name with ultra-zero soversion
        set_target_properties(${target} PROPERTIES OUTPUT_NAME ${target}-${LIBVER} SOVERSION "0.0.0")
        set(target_name lib${target}-${LIBVER}.so.0.0.0)

        #custom command to generate symlinks
        add_custom_command(
                TARGET ${target}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E create_symlink ${target_name} ${CMAKE_CURRENT_BINARY_DIR}/lib${target}.so
                COMMAND ${CMAKE_COMMAND} -E create_symlink ${target_name} ${CMAKE_CURRENT_BINARY_DIR}/lib${target}-${LIBVER}.so.0
                COMMAND ${CMAKE_COMMAND} -E touch ${target_name} #so the symlinks point to something valid so cmake 2.6 will install
        )

        #and install the extra symlinks
        install(
                FILES
                ${CMAKE_CURRENT_BINARY_DIR}/lib${target}.so
                ${CMAKE_CURRENT_BINARY_DIR}/lib${target}-${LIBVER}.so.0
                DESTINATION ${GR_LIBRARY_DIR} COMPONENT ${GR_LIBRARY_RUNTIME_COMPONENT}
        )

    endif(LIBRARY_EXTRAS)
endfunction(GR_LIBRARY_FOO_V2)
