/* -*- mode: c++; c-basic-offset: 2 -*- */
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
#ifndef RTL_TCP_SOURCE_C_H
#define RTL_TCP_SOURCE_C_H

#include <gnuradio/sync_block.h>

#include "source_iface.h"

class rtl_tcp_source_c;

typedef std::shared_ptr< rtl_tcp_source_c > rtl_tcp_source_c_sptr;

rtl_tcp_source_c_sptr make_rtl_tcp_source_c( const std::string & args = "" );

class rtl_tcp_source_c :
    public gr::sync_block,
    public source_iface
{
private:
  /* copied from rtl sdr */
  enum rtlsdr_tuner {
    RTLSDR_TUNER_UNKNOWN = 0,
    RTLSDR_TUNER_E4000,
    RTLSDR_TUNER_FC0012,
    RTLSDR_TUNER_FC0013,
    RTLSDR_TUNER_FC2580,
    RTLSDR_TUNER_R820T,
    RTLSDR_TUNER_R828D
  };

  friend rtl_tcp_source_c_sptr make_rtl_tcp_source_c(const std::string &args);

  rtl_tcp_source_c(const std::string &args);
  const char * get_tuner_name(void);

public:
  ~rtl_tcp_source_c();

  int work(int noutput_items,
	   gr_vector_const_void_star &input_items,
	   gr_vector_void_star &output_items);

  std::string name();

  static std::vector< std::string > get_devices( bool fake = false );

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

private:
  int d_socket;		  // handle to socket
  double _freq, _rate, _gain, _corr;
  bool _no_tuner;
  bool _auto_gain;
  double _if_gain;

  enum rtlsdr_tuner d_tuner_type;
  unsigned int d_tuner_gain_count;
  unsigned int d_tuner_if_gain_count;
  unsigned char *d_temp_buff; // hold buffer between calls
  float *d_LUT;
};

#endif // RTL_TCP_SOURCE_C_H
