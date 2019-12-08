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
# Rules for generating the source and sink yml wrappers
########################################################################
include(GrPython)

macro(GEN_BLOCK_YAML _generator _yaml_block)
    set(generator ${CMAKE_CURRENT_SOURCE_DIR}/${_generator})
    set(yaml_block ${CMAKE_CURRENT_BINARY_DIR}/${_yaml_block})
    list(APPEND yaml_blocks ${yaml_block})
    add_custom_command(
        DEPENDS ${generator} OUTPUT ${yaml_block}
        COMMAND ${PYTHON_EXECUTABLE} ${generator} ${yaml_block}
    )
endmacro(GEN_BLOCK_YAML)

GEN_BLOCK_YAML(gen_osmosdr_blocks.py    rtlsdr_source.block.yml)
GEN_BLOCK_YAML(gen_osmosdr_blocks.py    osmosdr_source.block.yml)
GEN_BLOCK_YAML(gen_osmosdr_blocks.py    osmosdr_sink.block.yml)

add_custom_target(osmosdr_grc_yaml_blocks ALL DEPENDS ${yaml_blocks})

install(FILES
    ${yaml_blocks}
    DESTINATION share/gnuradio/grc/blocks
)
