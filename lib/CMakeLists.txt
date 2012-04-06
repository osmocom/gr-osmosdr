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
include(GrPlatform) #define LIB_SUFFIX

add_library(gnuradio-osmosdr SHARED
    osmosdr_ranges.cc
    osmosdr_sink_c_impl.cc
    osmosdr_source_c_impl.cc
    rtl_source_c.cc
)

target_link_libraries(gnuradio-osmosdr
    ${Boost_LIBRARIES}
    ${GRUEL_LIBRARIES}
    ${GNURADIO_CORE_LIBRARIES}
#    ${GNURADIO_AUDIO_LIBRARIES}
    ${LIBRTLSDR_LIBRARIES}
)

set_target_properties(gnuradio-osmosdr PROPERTIES DEFINE_SYMBOL "gnuradio_osmosdr_EXPORTS")

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-osmosdr
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Add subdirectories
########################################################################
#add_subdirectory(fcd)
#add_subdirectory(osmosdr)
#add_subdirectory(rtl)
#add_subdirectory(uhd)
