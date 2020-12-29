/* -*- c++ -*- */
/*
 * Copyright 2015 Josh Blum <josh@joshknows.com>
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
#ifndef INCLUDED_SOAPY_SINK_C_H
#define INCLUDED_SOAPY_SINK_C_H

#include <gnuradio/block.h>
#include <gnuradio/sync_block.h>

#include "osmosdr/ranges.h"
#include "sink_iface.h"

class soapy_sink_c;

namespace SoapySDR
{
    class Device;
    class Stream;
}

/*
 * We use std::shared_ptr's instead of raw pointers for all access
 * to gr_blocks (and many other data structures).  The shared_ptr gets
 * us transparent reference counting, which greatly simplifies storage
 * management issues.  This is especially helpful in our hybrid
 * C++ / Python system.
 *
 * See http://www.boost.org/libs/smart_ptr/smart_ptr.htm
 *
 * As a convention, the _sptr suffix indicates a std::shared_ptr
 */
typedef std::shared_ptr<soapy_sink_c> soapy_sink_c_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of soapy_sink_c.
 *
 * To avoid accidental use of raw pointers, soapy_sink_c's
 * constructor is private.  make_soapy_sink_c is the public
 * interface for creating new instances.
 */
soapy_sink_c_sptr make_soapy_sink_c (const std::string & args = "");

class soapy_sink_c :
    public gr::sync_block,
    public sink_iface
{
private:
  // The friend declaration allows soapy_make_sink_c to
  // access the private constructor.
  friend soapy_sink_c_sptr make_soapy_sink_c (const std::string & args);

  soapy_sink_c (const std::string & args);  	// private constructor

  ~soapy_sink_c(void);

public:
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
osmosdr::freq_range_t get_freq_range( size_t chan);
double set_center_freq( double freq, size_t chan);
double get_center_freq( size_t chan);
double set_freq_corr( double ppm, size_t chan);
double get_freq_corr( size_t chan);
std::vector<std::string> get_gain_names( size_t chan);
osmosdr::gain_range_t get_gain_range( size_t chan);
osmosdr::gain_range_t get_gain_range( const std::string & name,
                                                size_t chan);
bool set_gain_mode( bool automatic, size_t chan);
bool get_gain_mode( size_t chan);
double set_gain( double gain, size_t chan);
double set_gain( double gain,
                           const std::string & name,
                           size_t chan);
double get_gain( size_t chan);
double get_gain( const std::string & name, size_t chan);
double set_if_gain( double gain, size_t chan);
double set_bb_gain( double gain, size_t chan);
std::vector< std::string > get_antennas( size_t chan);
std::string set_antenna( const std::string & antenna,
                                   size_t chan);
std::string get_antenna( size_t chan);
void set_dc_offset( const std::complex<double> &offset, size_t chan);
void set_iq_balance( const std::complex<double> &balance, size_t chan);
double set_bandwidth( double bandwidth, size_t chan);
double get_bandwidth( size_t chan);
osmosdr::freq_range_t get_bandwidth_range( size_t chan);
void set_time_source(const std::string &source,
                               const size_t mboard);
std::string get_time_source(const size_t mboard);
std::vector<std::string> get_time_sources(const size_t mboard);
void set_clock_source(const std::string &source,
                                const size_t mboard);
std::string get_clock_source(const size_t mboard);
std::vector<std::string> get_clock_sources(const size_t mboard);
double get_clock_rate(size_t mboard);
void set_clock_rate(double rate, size_t mboard);
::osmosdr::time_spec_t get_time_now(size_t mboard);
::osmosdr::time_spec_t get_time_last_pps(size_t mboard);
void set_time_now(const ::osmosdr::time_spec_t &time_spec,
                            size_t mboard);
void set_time_next_pps(const ::osmosdr::time_spec_t &time_spec);
void set_time_unknown_pps(const ::osmosdr::time_spec_t &time_spec);

private:
    SoapySDR::Device *_device;
    SoapySDR::Stream *_stream;
    size_t _nchan;
};

#endif /* INCLUDED_SOAPY_SINK_C_H */
