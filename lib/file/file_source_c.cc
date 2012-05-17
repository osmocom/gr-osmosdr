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

#include <gr_io_signature.h>
#include <gr_file_source.h>
#include <gr_throttle.h>

#include "file_source_c.h"

#include <osmosdr_arg_helpers.h>

using namespace boost::assign;

file_source_c_sptr make_file_source_c(const std::string &args)
{
  return gnuradio::get_initial_sptr(new file_source_c(args));
}

file_source_c::file_source_c(const std::string &args) :
  gr_hier_block2("file_source_c",
                 gr_make_io_signature (0, 0, 0),
                 gr_make_io_signature (1, 1, sizeof (gr_complex)))
{
  std::string filename;
  bool repeat = false;
  bool throttle = false;
  _freq = 0;
  _rate = 0;

  dict_t dict = params_to_dict(args);

  if (dict.count("file"))
    filename = dict["file"];

  if (dict.count("freq"))
    _freq = boost::lexical_cast< double >( dict["freq"] );

  if (dict.count("rate"))
    _rate = boost::lexical_cast< double >( dict["rate"] );

  if (dict.count("repeat"))
    repeat = "true" == dict["repeat"] ? true : false;

  if (dict.count("throttle"))
    throttle = "true" == dict["throttle"] ? true : false;

  if (!filename.length())
    throw std::runtime_error("No file name specified.");

  if (_freq < 0)
    throw std::runtime_error("Parameter 'freq' may not be negative.");

  if (0 == _rate)
    throw std::runtime_error("Parameter 'rate' is missing in arguments.");

  gr_file_source_sptr src = gr_make_file_source( sizeof(gr_complex),
                                                 filename.c_str(),
                                                 repeat );

  if (throttle) {
    gr_throttle::sptr throttle = gr_make_throttle( sizeof(gr_complex), _rate );

    connect( src, 0, throttle, 0 );
    connect( throttle, 0, self(), 0 );
  } else {
    connect( src, 0, self(), 0 );
  }
}

file_source_c::~file_source_c()
{
}

gr_basic_block_sptr file_source_c::self()
{
  return gr_hier_block2::self();
}

std::string file_source_c::name()
{
  return "File Source";
}

size_t file_source_c::get_num_channels( void )
{
  return 1;
}

osmosdr::meta_range_t file_source_c::get_sample_rates( void )
{
  osmosdr::meta_range_t range;

  range += osmosdr::range_t( get_sample_rate() );

  return range;
}

double file_source_c::set_sample_rate( double rate )
{
  return get_sample_rate();
}

double file_source_c::get_sample_rate( void )
{
  return _rate;
}

osmosdr::freq_range_t file_source_c::get_freq_range( size_t chan )
{
  osmosdr::freq_range_t range(_freq, _freq);

  return range;
}

double file_source_c::set_center_freq( double freq, size_t chan )
{
  return get_center_freq(chan);
}

double file_source_c::get_center_freq( size_t chan )
{
  return _freq;
}

double file_source_c::set_freq_corr( double ppm, size_t chan )
{
  return get_freq_corr( chan );
}

double file_source_c::get_freq_corr( size_t chan )
{
  return 0;
}

std::vector<std::string> file_source_c::get_gain_names( size_t chan )
{
  return std::vector< std::string >();
}

osmosdr::gain_range_t file_source_c::get_gain_range( size_t chan )
{
  osmosdr::gain_range_t range(0, 0);

  return range;
}

osmosdr::gain_range_t file_source_c::get_gain_range( const std::string & name, size_t chan )
{
  return get_gain_range( chan );
}

bool file_source_c::set_gain_mode( bool automatic, size_t chan )
{
  return get_gain_mode(chan);
}

bool file_source_c::get_gain_mode( size_t chan )
{
  return true;
}

double file_source_c::set_gain( double gain, size_t chan )
{
  return get_gain(chan);
}

double file_source_c::set_gain( double gain, const std::string & name, size_t chan )
{
  return set_gain(chan);
}

double file_source_c::get_gain( size_t chan )
{
  return 0;
}

double file_source_c::get_gain( const std::string & name, size_t chan )
{
  return get_gain(chan);
}

std::vector< std::string > file_source_c::get_antennas( size_t chan )
{
  std::vector< std::string > antennas;

  antennas += get_antenna(chan);

  return antennas;
}

std::string file_source_c::set_antenna( const std::string & antenna, size_t chan )
{
  return get_antenna(chan);
}

std::string file_source_c::get_antenna( size_t chan )
{
  return "ANT";
}
