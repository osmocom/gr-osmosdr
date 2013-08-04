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
# Install public header files
########################################################################
install(FILES
    gnuradio/blocks/api.h
    gnuradio/blocks/null_sink.h
    gnuradio/blocks/null_source.h
    gnuradio/blocks/throttle.h
    DESTINATION include/osmosdr/runtime/gnuradio/blocks
)

install(FILES
    gnuradio/thread/thread.h
    gnuradio/thread/thread_body_wrapper.h
    gnuradio/thread/thread_group.h
    DESTINATION include/osmosdr/runtime/gnuradio/thread
)

install(FILES
    gnuradio/uhd/api.h
    gnuradio/uhd/usrp_sink.h
    gnuradio/uhd/usrp_source.h
    DESTINATION include/osmosdr/runtime/gnuradio/uhd
)
install(FILES
    gnuradio/api.h
    gnuradio/attributes.h
    gnuradio/basic_block.h
    gnuradio/block.h
    gnuradio/block_detail.h
    gnuradio/block_gateway.h
    gnuradio/block_registry.h
    gnuradio/buffer.h
    gnuradio/constants.h
    gnuradio/endianness.h
    gnuradio/expj.h
    gnuradio/feval.h
    gnuradio/flowgraph.h
    gnuradio/fxpt.h
    gnuradio/fxpt_nco.h
    gnuradio/fxpt_vco.h
    gnuradio/gr_complex.h
    gnuradio/hier_block2.h
    gnuradio/high_res_timer.h
    gnuradio/io_signature.h
    gnuradio/logger.h
    gnuradio/math.h
    gnuradio/misc.h
    gnuradio/prefs.h
    gnuradio/random.h
    gnuradio/realtime.h
    gnuradio/realtime_impl.h
    gnuradio/runtime_types.h
    gnuradio/sincos.h
    gnuradio/sptr_magic.h
    gnuradio/sync_block.h
    gnuradio/sync_decimator.h
    gnuradio/sync_interpolator.h
    gnuradio/sys_paths.h
    gnuradio/top_block.h
    gnuradio/tpb_detail.h
    gnuradio/types.h
    DESTINATION include/osmosdr/runtime/gnuradio
)
