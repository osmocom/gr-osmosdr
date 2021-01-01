/* -*- c++ -*- */
/*
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
#ifndef INCLUDED_HACKRF_COMMON_H
#define INCLUDED_HACKRF_COMMON_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <boost/format.hpp>

#include <osmosdr/ranges.h>
#include <libhackrf/hackrf.h>

#define BUF_LEN  (16 * 32 * 512) /* must be multiple of 512 */
#define BUF_NUM   15

#define BYTES_PER_SAMPLE  2 /* HackRF device produces/consumes 8 bit signed IQ data */

#define HACKRF_FORMAT_ERROR(ret, msg) \
  boost::str( boost::format(msg " (%1%) %2%") \
    % ret % hackrf_error_name((enum hackrf_error)ret) )

#define HACKRF_THROW_ON_ERROR(ret, msg) \
  if ( ret != HACKRF_SUCCESS ) \
  { \
    throw std::runtime_error( HACKRF_FORMAT_ERROR(ret, msg) ); \
  }

#define HACKRF_FUNC_STR(func, arg) \
  boost::str(boost::format(func "(%1%)") % arg) + " has failed"

typedef std::shared_ptr<hackrf_device> hackrf_sptr;

class hackrf_common
{
public:
  hackrf_common(const std::string &args);

protected:
  static std::vector< std::string > get_devices();

  osmosdr::meta_range_t get_sample_rates( void );
  double set_sample_rate( double rate );
  double get_sample_rate( void );

  osmosdr::freq_range_t get_freq_range( size_t chan = 0 );
  double set_center_freq( double freq, size_t chan = 0 );
  double get_center_freq( size_t chan = 0 );
  double set_freq_corr( double ppm, size_t chan = 0 );
  double get_freq_corr( size_t chan = 0 );

  bool set_gain_mode( bool automatic, size_t chan = 0 );
  bool get_gain_mode( size_t chan = 0 );
  double set_gain( double gain, size_t chan = 0 );
  double get_gain( size_t chan = 0 );

  std::vector< std::string > get_antennas( size_t chan = 0 );
  std::string set_antenna( const std::string & antenna, size_t chan = 0 );
  std::string get_antenna( size_t chan = 0 );

  double set_bandwidth( double bandwidth, size_t chan = 0 );
  double get_bandwidth( size_t chan = 0 );
  osmosdr::freq_range_t get_bandwidth_range( size_t chan = 0 );

  bool set_bias( bool bias );
  bool get_bias();

  void start();
  void stop();

  hackrf_sptr _dev;

private:
  static void close(void *dev);

  static int _usage;
  static std::mutex _usage_mutex;

  static std::map<std::string, std::weak_ptr<hackrf_device>> _devs;
  static std::mutex _devs_mutex;

  double _sample_rate;
  double _center_freq;
  double _freq_corr;
  bool _auto_gain;
  double _amp_gain;
  double _requested_bandwidth;
  double _bandwidth;
  bool _bias;
  bool _started;
};

#endif /* INCLUDED_HACKRF_COMMON_H */
