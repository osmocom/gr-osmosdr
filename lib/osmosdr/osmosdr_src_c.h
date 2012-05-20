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
#ifndef INCLUDED_OSMOSDR_SRC_C_H
#define INCLUDED_OSMOSDR_SRC_C_H

#include <osmosdr_api.h>
#include <osmosdr_control.h>
#include <gr_hier_block2.h>

#include "osmosdr_src_iface.h"

class osmosdr_src_c;

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
typedef boost::shared_ptr<osmosdr_src_c> osmosdr_src_c_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of osmosdr_src_c.
 *
 * To avoid accidental use of raw pointers, osmosdr_src_c's
 * constructor is private.  osmosdr_make_src_c is the public
 * interface for creating new instances.
 */
OSMOSDR_API osmosdr_src_c_sptr osmosdr_make_src_c (const std::string & args = "");

/*!
 * \brief Provides a stream of complex samples.
 * \ingroup block
 *
 * \sa osmosdr_sink_c for a version that subclasses gr_hier_block2.
 */
class OSMOSDR_API osmosdr_src_c :
    public gr_hier_block2,
    public osmosdr_rx_control,
    public osmosdr_src_iface
{
private:
  // The friend declaration allows osmosdr_make_src_c to
  // access the private constructor.

  friend OSMOSDR_API osmosdr_src_c_sptr osmosdr_make_src_c (const std::string & args);

  /*!
   * \brief Provides a stream of complex samples.
   */
  osmosdr_src_c (const std::string & args);  	// private constructor

 public:
  ~osmosdr_src_c ();	// public destructor

  static std::vector< std::string > get_devices();

  size_t get_num_channels( void );

  osmosdr::meta_range_t get_sample_rates( void );
  double set_sample_rate( double rate );
  double get_sample_rate( void );

  osmosdr::freq_range_t get_freq_range( size_t chan = 0 );
  double set_center_freq( double freq, size_t chan = 0 );
  double get_center_freq( size_t chan = 0 );
  double set_freq_corr( double ppm, size_t chan = 0 );
  double get_freq_corr( size_t chan = 0 );

  std::vector<std::string> get_gain_names( size_t chan = 0 );
  osmosdr::gain_range_t get_gain_range( size_t chan = 0 );
  osmosdr::gain_range_t get_gain_range( const std::string & name, size_t chan = 0 );
  bool set_gain_mode( bool automatic, size_t chan = 0 );
  bool get_gain_mode( size_t chan = 0 );
  double set_gain( double gain, size_t chan = 0 );
  double set_gain( double gain, const std::string & name, size_t chan = 0 );
  double get_gain( size_t chan = 0 );
  double get_gain( const std::string & name, size_t chan = 0 );

  std::vector< std::string > get_antennas( size_t chan = 0 );
  std::string set_antenna( const std::string & antenna, size_t chan = 0 );
  std::string get_antenna( size_t chan = 0 );

private:
  bool _auto_gain;
};

#endif /* INCLUDED_OSMOSDR_SRC_C_H */
