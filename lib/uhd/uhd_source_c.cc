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

#include <boost/foreach.hpp>
#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>

//#include <uhd/property_tree.hpp>

#include <gr_io_signature.h>

#include <osmosdr_arg_helpers.h>

#include "uhd_source_c.h"

using namespace boost::assign;

uhd_source_c_sptr make_uhd_source_c(const std::string &args)
{
  return gnuradio::get_initial_sptr(new uhd_source_c(args));
}

uhd_source_c::uhd_source_c(const std::string &args) :
    gr_hier_block2("uhd_source_c",
                   gr_make_io_signature (0, 0, 0),
                   args_to_io_signature(args)),
    _lo_offset(0.0f)
{
  size_t nchan = 1;
  dict_t dict = params_to_dict(args);

  if (dict.count("nchan"))
    nchan = boost::lexical_cast< size_t >( dict["nchan"] );

  if (0 == nchan)
    nchan = 1;

  if (dict.count("lo_offset"))
    _lo_offset = boost::lexical_cast< double >( dict["lo_offset"] );

  std::string arguments; // rebuild argument string without internal arguments
  BOOST_FOREACH( dict_t::value_type &entry, dict ) {
    if ( "uhd" != entry.first &&
         "nchan" != entry.first &&
         "subdev" != entry.first &&
         "lo_offset" != entry.first ) {
      arguments += entry.first + "=" + entry.second + ",";
    }
  }

  _src = uhd_make_usrp_source( arguments,
                               uhd::io_type_t::COMPLEX_FLOAT32,
                               nchan );

  if (dict.count("subdev")) {
    _src->set_subdev_spec( dict["subdev"] );

    std::cerr << "-- Using subdev spec '" << _src->get_subdev_spec() << "'."
              << std::endl;
  }

  if (0.0 != _lo_offset)
    std::cerr << "-- Using lo offset of " << _lo_offset << " Hz." << std::endl;

  for ( size_t i = 0; i < nchan; i++ )
    connect( _src, i, self(), i );
}

uhd_source_c::~uhd_source_c()
{
}

std::vector< std::string > uhd_source_c::get_devices()
{
  std::vector< std::string > devices;

  uhd::device_addr_t hint;
  BOOST_FOREACH(const uhd::device_addr_t &dev, uhd::device::find(hint)) {
    std::string args = "uhd,subdev='A:0'," + dev.to_string();

    std::string label = "Ettus";

    std::string type = dev.cast< std::string >("type", "USRP");
    std::string name = dev.cast< std::string >("name", "");
    std::string serial = dev.cast< std::string >("serial", "");

    if (type.length()) {
      boost::to_upper(type);
      label += " " + type;
    }

    if (name.length())
      label += " " + name;

    if (serial.length())
      label += " " + serial;

    args += ",label='" + label +  + "'";

    devices.push_back( args );
  }

  //devices.clear();
  //devices.push_back( "uhd,type=usrp1,label='Ettus USRP'" );

  return devices;
}

gr_basic_block_sptr uhd_source_c::self()
{
  return gr_hier_block2::self();
}

std::string uhd_source_c::name()
{
//  uhd::property_tree::sptr prop_tree = _src->get_device()->get_device()->get_tree();
//  std::string dev_name = prop_tree->access<std::string>("/name").get();
  std::string mboard_name = _src->get_device()->get_mboard_name();

//  std::cerr << "'" << dev_name << "' '" << mboard_name << "'" << std::endl;
//  'USRP1 Device' 'USRP1 (Classic)'
//  'B-Series Device' 'B100 (B-Hundo)'

  return mboard_name;
}

size_t uhd_source_c::get_num_channels()
{
  return _src->get_device()->get_rx_num_channels();
}

osmosdr::meta_range_t uhd_source_c::get_sample_rates( void )
{
  osmosdr::meta_range_t rates;

  BOOST_FOREACH( uhd::range_t rate, _src->get_samp_rates() )
      rates += osmosdr::range_t( rate.start(), rate.stop(), rate.step() );

  return rates;
}

double uhd_source_c::set_sample_rate( double rate )
{
  _src->set_samp_rate( rate );
  return get_sample_rate();
}

double uhd_source_c::get_sample_rate( void )
{
  return _src->get_samp_rate();
}

osmosdr::freq_range_t uhd_source_c::get_freq_range( size_t chan )
{
  osmosdr::freq_range_t range;

  BOOST_FOREACH( uhd::range_t freq, _src->get_freq_range(chan) )
      range += osmosdr::range_t( freq.start(), freq.stop(), freq.step() );

  return range;
}

double uhd_source_c::set_center_freq( double freq, size_t chan )
{
  //_src->set_center_freq(freq, chan);

  // advanced tuning with tune_request_t
  uhd::tune_request_t tune_req(freq, _lo_offset);

  _src->set_center_freq(tune_req, chan);

  return get_center_freq(chan);
}

double uhd_source_c::get_center_freq( size_t chan )
{
  return _src->get_center_freq(chan);
}

double uhd_source_c::set_freq_corr( double ppm, size_t chan )
{
  return get_freq_corr(chan);
}

double uhd_source_c::get_freq_corr( size_t chan )
{
  return 0; // frequency correction is not supported with UHD
}

std::vector<std::string> uhd_source_c::get_gain_names( size_t chan )
{
  return _src->get_gain_names( chan );
}

osmosdr::gain_range_t uhd_source_c::get_gain_range( size_t chan )
{
  osmosdr::gain_range_t range;

  BOOST_FOREACH( uhd::range_t gain, _src->get_gain_range(chan) )
      range += osmosdr::range_t( gain.start(), gain.stop(), gain.step() );

  return range;
}

osmosdr::gain_range_t uhd_source_c::get_gain_range( const std::string & name, size_t chan )
{
  osmosdr::gain_range_t range;

  BOOST_FOREACH( uhd::range_t gain, _src->get_gain_range(name, chan) )
      range += osmosdr::range_t( gain.start(), gain.stop(), gain.step() );

  return range;
}

bool uhd_source_c::set_gain_mode( bool automatic, size_t chan )
{
  return get_gain_mode(chan);
}

bool uhd_source_c::get_gain_mode( size_t chan )
{
  return true;
}

double uhd_source_c::set_gain( double gain, size_t chan )
{
  _src->set_gain(gain, chan);

  return get_gain(chan);
}

double uhd_source_c::set_gain( double gain, const std::string & name, size_t chan )
{
  _src->set_gain(gain, name, chan);

  return get_gain(name, chan);
}

double uhd_source_c::get_gain( size_t chan )
{
  return _src->get_gain(chan);
}

double uhd_source_c::get_gain( const std::string & name, size_t chan )
{
  return _src->get_gain(name, chan);
}

std::vector< std::string > uhd_source_c::get_antennas( size_t chan )
{
  return _src->get_antennas(chan);
}

std::string uhd_source_c::set_antenna( const std::string & antenna, size_t chan )
{
  _src->set_antenna(antenna, chan);

  return _src->get_antenna(chan);
}

std::string uhd_source_c::get_antenna( size_t chan )
{
  return _src->get_antenna(chan);
}
