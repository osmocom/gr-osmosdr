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
#ifndef INCLUDED_OSMOSDR_SOURCE_IMPL_H
#define INCLUDED_OSMOSDR_SOURCE_IMPL_H

#include <osmosdr/source.h>

#ifdef HAVE_IQBALANCE
#include <gnuradio/iqbalance/optimize_c.h>
#include <gnuradio/iqbalance/fix_cc.h>
#endif

#include <source_iface.h>

#include <map>

class source_impl : public osmosdr::source
{
public:
  source_impl( const std::string & args );

  size_t get_num_channels( void );

  bool seek( long seek_point, int whence, size_t chan );

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

  void set_dc_offset_mode( int mode, size_t chan = 0 );
  void set_dc_offset( const std::complex<double> &offset, size_t chan = 0 );

  void set_iq_balance_mode( int mode, size_t chan = 0 );
  void set_iq_balance( const std::complex<double> &balance, size_t chan = 0 );

  double set_bandwidth( double bandwidth, size_t chan = 0 );
  double get_bandwidth( size_t chan = 0 );
  osmosdr::freq_range_t get_bandwidth_range( size_t chan = 0 );

  void set_time_source(const std::string &source, const size_t mboard = 0);
  std::string get_time_source(const size_t mboard);
  std::vector<std::string> get_time_sources(const size_t mboard);
  void set_clock_source(const std::string &source, const size_t mboard = 0);
  std::string get_clock_source(const size_t mboard);
  std::vector<std::string> get_clock_sources(const size_t mboard);
  double get_clock_rate(size_t mboard = 0);
  void set_clock_rate(double rate, size_t mboard = 0);
  ::osmosdr::time_spec_t get_time_now(size_t mboard = 0);
  ::osmosdr::time_spec_t get_time_last_pps(size_t mboard = 0);
  void set_time_now(const ::osmosdr::time_spec_t &time_spec, size_t mboard = 0);
  void set_time_next_pps(const ::osmosdr::time_spec_t &time_spec);
  void set_time_unknown_pps(const ::osmosdr::time_spec_t &time_spec);

private:
  std::vector< source_iface * > _devs;

  /* cache to prevent multiple device calls with the same value coming from grc */
  double _sample_rate;
  std::map< size_t, double > _center_freq;
  std::map< size_t, double > _freq_corr;
  std::map< size_t, bool > _gain_mode;
  std::map< size_t, double > _gain;
  std::map< size_t, double > _if_gain;
  std::map< size_t, double > _bb_gain;
  std::map< size_t, std::string > _antenna;
#ifdef HAVE_IQBALANCE
  gr::basic_block_sptr block;
  std::vector< gr::iqbalance::fix_cc::sptr > _iq_fix;
  std::vector< gr::iqbalance::optimize_c::sptr > _iq_opt;
  std::vector< int > _iq_mode;
  std::map< size_t, std::pair<float, float> > _vals;
#endif
  std::map< size_t, double > _bandwidth;
};

#endif /* INCLUDED_OSMOSDR_SOURCE_IMPL_H */
