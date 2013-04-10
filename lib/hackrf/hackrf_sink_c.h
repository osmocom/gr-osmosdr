/* -*- c++ -*- */
/*
 * Copyright 2013 Dimitri Stolnikov <horiz0n@gmx.net>
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
#ifndef INCLUDED_HACKRF_SINK_C_H
#define INCLUDED_HACKRF_SINK_C_H

#include <boost/shared_ptr.hpp>

class hackrf_sink_c;

/*
 * We use boost::shared_ptr's instead of raw pointers for all access
 * to gr_blocks (and many other data structures).  The shared_ptr gets
 * us transparent reference counting, which greatly simplifies storage
 * management issues.  This is especially helpful in our hybrid
 * C++ / Python system.
 *
 * See http://www.boost.org/libs/smart_ptr/smart_ptr.htm
 *
 * As a convention, the _sptr suffix indicates a boost::shared_ptr
 */
typedef boost::shared_ptr<hackrf_sink_c> hackrf_sink_c_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of hackrf_sink_c.
 *
 * To avoid accidental use of raw pointers, hackrf_sink_c's
 * constructor is private.  make_hackrf_sink_c is the public
 * interface for creating new instances.
 */
hackrf_sink_c_sptr make_hackrf_sink_c (const std::string & args = "");

class hackrf_sink_c
{
private:
  // The friend declaration allows hackrf_make_sink_c to
  // access the private constructor.
  friend hackrf_sink_c_sptr make_hackrf_sink_c (const std::string & args);

  hackrf_sink_c (const std::string & args);  	// private constructor

public:
  ~hackrf_source_c (); 	// public destructor
};

#endif /* INCLUDED_HACKRF_SINK_C_H */
