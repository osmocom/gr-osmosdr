/* -*- c++ -*- */
/*
 * Copyright 2013 Dimitri Stolnikov <horiz0n@gmx.net>
 * Copyright 2020 Clayton Smith <argilo@gmail.com>
 *
 * gr-osmosdr is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * gr-osmosdr is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gr-osmosdr; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#ifndef INCLUDED_HACKRF_SOURCE_C_H
#define INCLUDED_HACKRF_SOURCE_C_H

#include <gnuradio/sync_block.h>

#include <condition_variable>
#include <mutex>

#include <libhackrf/hackrf.h>

#include "source_iface.h"
#include "hackrf_common.h"

class hackrf_source_c;

/*
 * We use boost::shared_ptr's instead of raw pointers for all access
 * to gr::blocks (and many other data structures).  The shared_ptr gets
 * us transparent reference counting, which greatly simplifies storage
 * management issues.  This is especially helpful in our hybrid
 * C++ / Python system.
 *
 * See http://www.boost.org/libs/smart_ptr/smart_ptr.htm
 *
 * As a convention, the _sptr suffix indicates a boost::shared_ptr
 */
typedef boost::shared_ptr<hackrf_source_c> hackrf_source_c_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of hackrf_source_c.
 *
 * To avoid accidental use of raw pointers, hackrf_source_c's
 * constructor is private.  make_hackrf_source_c is the public
 * interface for creating new instances.
 */
hackrf_source_c_sptr make_hackrf_source_c (const std::string & args = "");

/*!
 * \brief Provides a stream of complex samples.
 * \ingroup block
 */
class hackrf_source_c :
    public gr::sync_block,
    public source_iface,
    protected hackrf_common
{
private:
  // The friend declaration allows make_hackrf_source_c to
  // access the private constructor.
  friend hackrf_source_c_sptr make_hackrf_source_c (const std::string & args);

  /*!
   * \brief Provides a stream of complex samples.
   */
  hackrf_source_c (const std::string & args);  	// private constructor

public:
  ~hackrf_source_c (); 	// public destructor

  bool start();
  bool stop();

  int work( int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items );

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

  double set_if_gain( double gain, size_t chan = 0 );
  double set_bb_gain( double gain, size_t chan = 0 );

  std::vector< std::string > get_antennas( size_t chan = 0 );
  std::string set_antenna( const std::string & antenna, size_t chan = 0 );
  std::string get_antenna( size_t chan = 0 );

  double set_bandwidth( double bandwidth, size_t chan = 0 );
  double get_bandwidth( size_t chan = 0 );
  osmosdr::freq_range_t get_bandwidth_range( size_t chan = 0 );

private:
  static int _hackrf_rx_callback(hackrf_transfer* transfer);
  int hackrf_rx_callback(unsigned char *buf, uint32_t len);

  std::vector<float> _lut;

  unsigned char **_buf;
  unsigned int _buf_num;
  unsigned int _buf_len;
  unsigned int _buf_head;
  unsigned int _buf_used;
  std::mutex _buf_mutex;
  std::condition_variable _buf_cond;

  unsigned int _buf_offset;
  int _samp_avail;

  double _lna_gain;
  double _vga_gain;
};

#endif /* INCLUDED_HACKRF_SOURCE_C_H */
