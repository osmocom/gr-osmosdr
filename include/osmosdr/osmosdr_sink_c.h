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
#ifndef INCLUDED_OSMOSDR_SINK_C_H
#define INCLUDED_OSMOSDR_SINK_C_H

#include <osmosdr_api.h>
#include <gr_hier_block2.h>

class osmosdr_sink_c;

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
typedef boost::shared_ptr<osmosdr_sink_c> osmosdr_sink_c_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of osmosdr_sink_c.
 *
 * To avoid accidental use of raw pointers, osmosdr_sink_c's
 * constructor is private.  osmosdr_make_sink_c is the public
 * interface for creating new instances.
 */
OSMOSDR_API osmosdr_sink_c_sptr osmosdr_make_sink_c ( const std::string & args = "" );

/*!
 * \brief Takes a stream of complex samples.
 * \ingroup block
 *
 * This uses the preferred technique: subclassing gr_hier_block2.
 */
class OSMOSDR_API osmosdr_sink_c : virtual public gr_hier_block2
{
public:

};

#endif /* INCLUDED_OSMOSDR_SINK_C_H */
