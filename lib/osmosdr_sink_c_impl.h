/* -*- c++ -*- */
/*
 * Copyright 2012 Dimitri Stolnikov <horiz0n@gmx.net>
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
#ifndef INCLUDED_OSMOSDR_SINK_C_IMPL_H
#define INCLUDED_OSMOSDR_SINK_C_IMPL_H

#include <osmosdr/osmosdr_sink_c.h>

class osmosdr_sink_c_impl : public osmosdr_sink_c
{
public:

private:
  osmosdr_sink_c_impl (const std::string & args);  	// private constructor

  // The friend declaration allows osmosdr_make_sink_c to
  // access the private constructor.
  friend osmosdr_sink_c_sptr osmosdr_make_sink_c (const std::string & args);
};

#endif /* INCLUDED_OSMOSDR_SINK_C_IMPL_H */
