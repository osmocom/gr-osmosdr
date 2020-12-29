/* -*- c++ -*- */
/*
 * Copyright 2016,2017 Sergey Kostanbaev <sergey.kostanbaev@fairwaves.co>
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
#ifndef XTRX_SOURCE_C_H
#define XTRX_SOURCE_C_H

#include <gnuradio/block.h>
#include <gnuradio/sync_block.h>

#include "source_iface.h"
#include "xtrx_obj.h"

static const pmt::pmt_t TIME_KEY = pmt::string_to_symbol("rx_time");
static const pmt::pmt_t RATE_KEY = pmt::string_to_symbol("rx_rate");
static const pmt::pmt_t FREQ_KEY = pmt::string_to_symbol("rx_freq");

class xtrx_source_c;

typedef std::shared_ptr< xtrx_source_c > xtrx_source_c_sptr;

xtrx_source_c_sptr make_xtrx_source_c( const std::string & args = "" );

class xtrx_source_c :
    public gr::sync_block,
    public source_iface
{
private:
  friend xtrx_source_c_sptr make_xtrx_source_c(const std::string &args);

  xtrx_source_c(const std::string &args);

public:
  ~xtrx_source_c();

  std::string name();

  static std::vector< std::string > get_devices( bool fake = false ) { return xtrx_obj::get_devices(); }

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

  std::vector< std::string > get_antennas( size_t chan = 0 );
  std::string set_antenna( const std::string & antenna, size_t chan = 0 );
  std::string get_antenna( size_t chan = 0 );

  double set_bandwidth( double bandwidth, size_t chan = 0 );
  double get_bandwidth( size_t chan = 0 );
  osmosdr::freq_range_t get_bandwidth_range( size_t chan = 0);

  int work (int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items);

  bool start();
  bool stop();

private:
  xtrx_obj_sptr _xtrx;
  pmt::pmt_t _id;

  unsigned _sample_flags;
  double _rate;
  double _master;
  double _freq;
  double _corr;
  double _bandwidth;
  bool _auto_gain;

  xtrx_wire_format_t _otw;
  bool _mimo_mode;

  int _gain_lna;
  int _gain_tia;
  int _gain_pga;

  unsigned _channels;
  xtrx_antenna_t _ant;

  bool     _swap_ab;
  bool     _swap_iq;
  bool     _loopback;
  bool     _tdd;
  bool     _fbctrl;
  bool     _timekey;

  double   _dsp;
  std::string _dev;
};

#endif // XTRX_SOURCE_C_H
