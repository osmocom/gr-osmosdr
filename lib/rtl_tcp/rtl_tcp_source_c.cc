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

#include <fstream>
#include <string>
#include <sstream>

#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>

#include <gr_io_signature.h>
#include <gr_deinterleave.h>
#include <gr_float_to_complex.h>

#include "rtl_tcp_source_c.h"

#include <osmosdr_arg_helpers.h>

using namespace boost::assign;

rtl_tcp_source_c_sptr make_rtl_tcp_source_c(const std::string &args)
{
  return gnuradio::get_initial_sptr(new rtl_tcp_source_c(args));
}

rtl_tcp_source_c::rtl_tcp_source_c(const std::string &args) :
  gr_hier_block2("rtl_tcp_source_c",
                 gr_make_io_signature (0, 0, 0),
                 gr_make_io_signature (1, 1, sizeof (gr_complex)))
{
  std::string host = "127.0.0.1";
  unsigned short port = 1234;
  int payload_size = 16384;

  _freq = 0;
  _rate = 0;
  _gain = 0;
  _corr = 0;
  _auto_gain = false;

  dict_t dict = params_to_dict(args);

  if (dict.count("rtl_tcp")) {
    std::vector< std::string > tokens;
    boost::algorithm::split( tokens, dict["rtl_tcp"], boost::is_any_of(":") );

    if ( tokens[0].length() && (tokens.size() == 1 || tokens.size() == 2 ) )
      host = tokens[0];

    if ( tokens.size() == 2 ) // port given
      port = boost::lexical_cast< unsigned short >( tokens[1] );
  }

  if (dict.count("psize"))
    payload_size = boost::lexical_cast< int >( dict["psize"] );

  if (!host.length())
    host = "127.0.0.1";

  if (0 == port)
    port = 1234;

  if (payload_size <= 0)
    payload_size = 16384;

  _src = make_rtl_tcp_source_f(sizeof(float), host.c_str(), port, payload_size,
                               false, false);

  set_gain_mode(false); // enable manual gain mode by default

  /* rtl tcp source provides a stream of interleaved IQ floats */
  gr_deinterleave_sptr deinterleave = gr_make_deinterleave(sizeof(float));

  /* block to convert deinterleaved floats to a complex stream */
  gr_float_to_complex_sptr f2c = gr_make_float_to_complex(1);

  connect(_src, 0, deinterleave, 0);
  connect(deinterleave, 0, f2c, 0); /* I */
  connect(deinterleave, 1, f2c, 1); /* Q */
  connect(f2c, 0, self(), 0);
}

rtl_tcp_source_c::~rtl_tcp_source_c()
{
}

gr_basic_block_sptr rtl_tcp_source_c::self()
{
  return gr_hier_block2::self();
}

std::string rtl_tcp_source_c::name()
{
  return "RTL TCP Client Source";
}

size_t rtl_tcp_source_c::get_num_channels( void )
{
  return 1;
}

osmosdr::meta_range_t rtl_tcp_source_c::get_sample_rates( void )
{
  osmosdr::meta_range_t range;

  range += osmosdr::range_t( 250000 ); // known to work
  range += osmosdr::range_t( 1000000 ); // known to work
  range += osmosdr::range_t( 1024000 ); // known to work
  range += osmosdr::range_t( 1800000 ); // known to work
  range += osmosdr::range_t( 1920000 ); // known to work
  range += osmosdr::range_t( 2048000 ); // known to work
  range += osmosdr::range_t( 2400000 ); // known to work
  range += osmosdr::range_t( 2600000 ); // may work
  range += osmosdr::range_t( 2800000 ); // may work
  range += osmosdr::range_t( 3000000 ); // may work
  range += osmosdr::range_t( 3200000 ); // max rate

  return range;
}

double rtl_tcp_source_c::set_sample_rate( double rate )
{
  _src->set_sample_rate( int(rate) );

  _rate = rate;

  return get_sample_rate();
}

double rtl_tcp_source_c::get_sample_rate( void )
{
  return _rate;
}

osmosdr::freq_range_t rtl_tcp_source_c::get_freq_range( size_t chan )
{
  // FIXME: assumption on E4000 tuner

  /* there is a (temperature dependent) gap between 1100 to 1250 MHz */
  osmosdr::freq_range_t range( 50e6, 2.2e6 );

  return range;
}

double rtl_tcp_source_c::set_center_freq( double freq, size_t chan )
{
  _src->set_freq( int(freq) );

  _freq = freq;

  return get_center_freq(chan);
}

double rtl_tcp_source_c::get_center_freq( size_t chan )
{
  return _freq;
}

double rtl_tcp_source_c::set_freq_corr( double ppm, size_t chan )
{
  _src->set_freq_corr( int(ppm) );

  _corr = ppm;

  return get_freq_corr( chan );
}

double rtl_tcp_source_c::get_freq_corr( size_t chan )
{
  return _corr;
}

std::vector<std::string> rtl_tcp_source_c::get_gain_names( size_t chan )
{
  std::vector< std::string > names;

  names += "LNA";

  return names;
}

osmosdr::gain_range_t rtl_tcp_source_c::get_gain_range( size_t chan )
{
  osmosdr::gain_range_t range;

  // FIXME: assumption on E4000 tuner

  range += osmosdr::range_t( -1.0 );
  range += osmosdr::range_t( 1.5 );
  range += osmosdr::range_t( 4.0 );
  range += osmosdr::range_t( 6.5 );
  range += osmosdr::range_t( 9.0 );
  range += osmosdr::range_t( 11.5 );
  range += osmosdr::range_t( 14.0 );
  range += osmosdr::range_t( 16.5 );
  range += osmosdr::range_t( 19.0 );
  range += osmosdr::range_t( 21.5 );
  range += osmosdr::range_t( 24.0 );
  range += osmosdr::range_t( 29.0 );
  range += osmosdr::range_t( 34.0 );
  range += osmosdr::range_t( 42.0 );
  range += osmosdr::range_t( 43.0 );
  range += osmosdr::range_t( 45.0 );
  range += osmosdr::range_t( 47.0 );
  range += osmosdr::range_t( 49.0 );

  return range;
}

osmosdr::gain_range_t rtl_tcp_source_c::get_gain_range( const std::string & name, size_t chan )
{
  return get_gain_range( chan );
}

bool rtl_tcp_source_c::set_gain_mode( bool automatic, size_t chan )
{
  _src->set_gain_mode(int(!automatic));

  _auto_gain = automatic;

  return get_gain_mode(chan);
}

bool rtl_tcp_source_c::get_gain_mode( size_t chan )
{
  return _auto_gain;
}

double rtl_tcp_source_c::set_gain( double gain, size_t chan )
{
  osmosdr::gain_range_t gains = rtl_tcp_source_c::get_gain_range( chan );

  _src->set_gain( int(gains.clip(gain) * 10.0) );

  _gain = gain;

  return get_gain(chan);
}

double rtl_tcp_source_c::set_gain( double gain, const std::string & name, size_t chan )
{
  return set_gain(chan);
}

double rtl_tcp_source_c::get_gain( size_t chan )
{
  return 0;
}

double rtl_tcp_source_c::get_gain( const std::string & name, size_t chan )
{
  return get_gain(chan);
}

std::vector< std::string > rtl_tcp_source_c::get_antennas( size_t chan )
{
  std::vector< std::string > antennas;

  antennas += get_antenna(chan);

  return antennas;
}

std::string rtl_tcp_source_c::set_antenna( const std::string & antenna, size_t chan )
{
  return get_antenna(chan);
}

std::string rtl_tcp_source_c::get_antenna( size_t chan )
{
  return "ANT";
}
