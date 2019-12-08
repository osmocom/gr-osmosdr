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
# This file included, use CMake directory variables
########################################################################

target_include_directories(gnuradio-osmosdr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(ENABLE_FCD)
    target_include_directories(gnuradio-osmosdr PRIVATE
        ${Gnuradio-fcd_INCLUDE_DIRS}
    )
    target_link_libraries(gnuradio-osmosdr PRIVATE
        ${Gnuradio-fcd_LIBRARIES}
    )
endif(ENABLE_FCD)

if(ENABLE_FCDPP)
    target_include_directories(gnuradio-osmosdr PRIVATE
        ${Gnuradio-fcdpp_INCLUDE_DIRS}
    )
    target_link_libraries(gnuradio-osmosdr PRIVATE
        ${Gnuradio-fcdpp_LIBRARIES}
    )
endif(ENABLE_FCDPP)

list(APPEND gr_osmosdr_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/fcd_source_c.cc
)
set(gr_osmosdr_srcs ${gr_osmosdr_srcs} PARENT_SCOPE)
