/* -*- c++ -*- */
/*
 * Copyright 2006,2012-2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/basic_block.h>
#include <gnuradio/block_registry.h>
#include <stdexcept>
#include <sstream>
#include <iostream>

namespace gr {

  static long s_next_id = 0;
  static long s_ncurrently_allocated = 0;

  long
  basic_block_ncurrently_allocated()
  {
    return s_ncurrently_allocated;
  }

  basic_block::basic_block(const std::string &name,
                           io_signature::sptr input_signature,
                           io_signature::sptr output_signature)
    : d_name(name),
      d_input_signature(input_signature),
      d_output_signature(output_signature),
      d_unique_id(s_next_id++),
      d_symbolic_id(global_block_registry.block_register(this)),
      d_symbol_name(global_block_registry.register_symbolic_name(this)),
      d_color(WHITE),
      d_rpc_set(false)
  {
    s_ncurrently_allocated++;
  }

  basic_block::~basic_block()
  {
    s_ncurrently_allocated--;
    global_block_registry.block_unregister(this);
  }

  basic_block_sptr
  basic_block::to_basic_block()
  {
    return shared_from_this();
  }

  void
  basic_block::set_block_alias(std::string name)
  { 
    global_block_registry.register_symbolic_name(this, name); 
  }

} /* namespace gr */
