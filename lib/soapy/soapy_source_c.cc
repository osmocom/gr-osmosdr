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

/*
 * config.h is generated by configure.  It contains the results
 * of probing for features, options etc.  It should be the first
 * file included in your .cc file.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>

#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <gnuradio/io_signature.h>

#include "arg_helpers.h"
#include "soapy_source_c.h"
#include "osmosdr/source.h"
#include <SoapySDR/Device.hpp>

using namespace boost::assign;

boost::mutex &get_soapy_maker_mutex(void);

/*
 * Create a new instance of soapy_source_c and return
 * a boost shared_ptr.  This is effectively the public constructor.
 */
soapy_source_c_sptr make_soapy_source_c (const std::string &args)
{
  return gnuradio::get_initial_sptr(new soapy_source_c (args));
}

/*
 * The private constructor
 */
soapy_source_c::soapy_source_c (const std::string &args)
  : gr::sync_block ("soapy_source_c",
                    gr::io_signature::make (0, 0, 0),
                    args_to_io_signature(args))
{
    {
        boost::mutex::scoped_lock l(get_soapy_maker_mutex());
        _device = SoapySDR::Device::make(params_to_dict(args));
    }
    _nchan = std::max(1, args_to_io_signature(args)->max_streams());
    std::vector<size_t> channels;
    for (size_t i = 0; i < _nchan; i++) channels.push_back(i);
    _stream = _device->setupStream(SOAPY_SDR_RX, "CF32", channels);
}

soapy_source_c::~soapy_source_c(void)
{
    _device->closeStream(_stream);
    boost::mutex::scoped_lock l(get_soapy_maker_mutex());
    SoapySDR::Device::unmake(_device);
}

bool soapy_source_c::start()
{
    return _device->activateStream(_stream) == 0;
}

bool soapy_source_c::stop()
{
    return _device->deactivateStream(_stream) == 0;
}

int soapy_source_c::work( int noutput_items,
                            gr_vector_const_void_star &input_items,
                            gr_vector_void_star &output_items )
{
    int flags = 0;
    long long timeNs = 0;
    int ret = _device->readStream(
        _stream, &output_items[0],
        noutput_items, flags, timeNs);

    if (ret < 0) return 0; //call again
    return ret;
}

std::vector<std::string> soapy_source_c::get_devices()
{
    std::vector<std::string> result;
    int i = 0;
    BOOST_FOREACH(SoapySDR::Kwargs kw, SoapySDR::Device::enumerate())
    {
        kw["soapy"] = boost::lexical_cast<std::string>(i++);
        result.push_back(dict_to_args_string(kw));
    }
    return result;
}

size_t soapy_source_c::get_num_channels( void )
{
    return _nchan;
}

osmosdr::meta_range_t soapy_source_c::get_sample_rates( void )
{
    osmosdr::meta_range_t result;
    BOOST_FOREACH(const double rate, _device->listSampleRates(SOAPY_SDR_RX, 0))
    {
        result.push_back(osmosdr::range_t(rate));
    }
    return result;
}

double soapy_source_c::set_sample_rate( double rate )
{
    _device->setSampleRate(SOAPY_SDR_RX, 0, rate);
    return this->get_sample_rate();
}

double soapy_source_c::get_sample_rate( void )
{
    return _device->getSampleRate(SOAPY_SDR_RX, 0);
}

osmosdr::freq_range_t soapy_source_c::get_freq_range( size_t chan )
{
    osmosdr::meta_range_t result;
    BOOST_FOREACH(const SoapySDR::Range r, _device->getFrequencyRange(SOAPY_SDR_RX, 0))
    {
        result.push_back(osmosdr::range_t(r.minimum(), r.maximum()));
    }
    return result;
}

double soapy_source_c::set_center_freq( double freq, size_t chan )
{
    _device->setFrequency(SOAPY_SDR_RX, chan, freq);
    return this->get_center_freq(chan);
}

double soapy_source_c::get_center_freq( size_t chan )
{
    return _device->getFrequency(SOAPY_SDR_RX, chan);
}

double soapy_source_c::set_freq_corr( double ppm, size_t chan )
{
    _device->setFrequency(SOAPY_SDR_RX, chan, "CORR", ppm);
    return this->get_freq_corr(chan);
}

double soapy_source_c::get_freq_corr( size_t chan )
{
    return _device->getFrequency(SOAPY_SDR_RX, chan, "CORR");
}

std::vector<std::string> soapy_source_c::get_gain_names( size_t chan )
{
    return _device->listGains(SOAPY_SDR_RX, chan);
}

osmosdr::gain_range_t soapy_source_c::get_gain_range( size_t chan )
{
    SoapySDR::Range r = _device->getGainRange(SOAPY_SDR_RX, chan);
    return osmosdr::gain_range_t(r.minimum(), r.maximum());
}

osmosdr::gain_range_t soapy_source_c::get_gain_range( const std::string & name,
                                                size_t chan )
{
    SoapySDR::Range r = _device->getGainRange(SOAPY_SDR_RX, chan, name);
    return osmosdr::gain_range_t(r.minimum(), r.maximum());
}

bool soapy_source_c::set_gain_mode( bool automatic, size_t chan )
{
    _device->setGainMode(SOAPY_SDR_RX, chan, automatic);
    return this->get_gain_mode(chan);
}

bool soapy_source_c::get_gain_mode( size_t chan )
{
    return _device->getGainMode(SOAPY_SDR_RX, chan);
}

double soapy_source_c::set_gain( double gain, size_t chan )
{
    _device->setGain(SOAPY_SDR_RX, chan, gain);
    return this->get_gain(chan);
}

double soapy_source_c::set_gain( double gain,
                           const std::string & name,
                           size_t chan )
{
    _device->setGain(SOAPY_SDR_RX, chan, name, gain);
    return this->get_gain(name, chan);
}

double soapy_source_c::get_gain( size_t chan )
{
    return _device->getGain(SOAPY_SDR_RX, chan);
}

double soapy_source_c::get_gain( const std::string & name, size_t chan )
{
    return _device->getGain(SOAPY_SDR_RX, chan, name);
}

double soapy_source_c::set_if_gain( double gain, size_t chan )
{
    //docs specify RF gain is the first element
    const std::string name = this->get_gain_names(chan).front();
    return this->set_gain(gain, name, chan);
}

double soapy_source_c::set_bb_gain( double gain, size_t chan )
{
    //docs specify baseband gain is the last element
    const std::string name = this->get_gain_names(chan).back();
    return this->set_gain(gain, name, chan);
}

std::vector< std::string > soapy_source_c::get_antennas( size_t chan )
{
    return _device->listAntennas(SOAPY_SDR_RX, chan);
}

std::string soapy_source_c::set_antenna( const std::string & antenna,
                                   size_t chan )
{
    _device->setAntenna(SOAPY_SDR_RX, chan, antenna);
    return this->get_antenna(chan);
}

std::string soapy_source_c::get_antenna( size_t chan )
{
    return _device->getAntenna(SOAPY_SDR_RX, chan);
}

void soapy_source_c::set_dc_offset_mode( int mode, size_t chan )
{
    switch (mode)
    {
    case osmosdr::source::DCOffsetOff:
        _device->setDCOffsetMode(SOAPY_SDR_RX, chan, false);
        this->set_dc_offset(0.0, chan); //reset
        break;
    case osmosdr::source::DCOffsetManual:
        _device->setDCOffsetMode(SOAPY_SDR_RX, chan, false);
        break;
    case osmosdr::source::DCOffsetAutomatic:
        _device->setDCOffsetMode(SOAPY_SDR_RX, chan, true);
        break;
    }
}

void soapy_source_c::set_dc_offset( const std::complex<double> &offset, size_t chan )
{
    _device->setDCOffset(SOAPY_SDR_RX, chan, offset);
}

void soapy_source_c::set_iq_balance_mode( int mode, size_t chan )
{
    if (mode != 0)
        throw std::runtime_error("soapy_source_c::set_iq_balance_mode() not supported");
}

void soapy_source_c::set_iq_balance( const std::complex<double> &balance, size_t chan )
{
    _device->setIQBalance(SOAPY_SDR_RX, chan, balance);
}

double soapy_source_c::set_bandwidth( double bandwidth, size_t chan )
{
    _device->setBandwidth(SOAPY_SDR_RX, chan, bandwidth);
    return this->get_bandwidth(chan);
}

double soapy_source_c::get_bandwidth( size_t chan ) 
{
    return _device->getBandwidth(SOAPY_SDR_RX, chan);
}

osmosdr::freq_range_t soapy_source_c::get_bandwidth_range( size_t chan )
{
    osmosdr::meta_range_t result;
    BOOST_FOREACH(const double bw, _device->listBandwidths(SOAPY_SDR_RX, 0))
    {
        result.push_back(osmosdr::range_t(bw));
    }
    return result;
}

void soapy_source_c::set_time_source(const std::string &source,
                               const size_t)
{
    _device->setTimeSource(source);
}

std::string soapy_source_c::get_time_source(const size_t)
{
    return _device->getTimeSource();
}

std::vector<std::string> soapy_source_c::get_time_sources(const size_t)
{
    return _device->listTimeSources();
}

void soapy_source_c::set_clock_source(const std::string &source,
                                const size_t)
{
    _device->setClockSource(source);
}

std::string soapy_source_c::get_clock_source(const size_t)
{
    return _device->getClockSource();
}

std::vector<std::string> soapy_source_c::get_clock_sources(const size_t)
{
    return _device->listClockSources();
}

double soapy_source_c::get_clock_rate(size_t)
{
    return _device->getMasterClockRate();
}

void soapy_source_c::set_clock_rate(double rate, size_t)
{
    _device->setMasterClockRate(rate);
}

::osmosdr::time_spec_t soapy_source_c::get_time_now(size_t)
{
    return ::osmosdr::time_spec_t::from_ticks(_device->getHardwareTime(), 1e6);
}

::osmosdr::time_spec_t soapy_source_c::get_time_last_pps(size_t)
{
    return ::osmosdr::time_spec_t::from_ticks(_device->getHardwareTime("PPS"), 1e6);
}

void soapy_source_c::set_time_now(const ::osmosdr::time_spec_t &time_spec,
                            size_t)
{
    _device->setHardwareTime(time_spec.to_ticks(1e9));
}

void soapy_source_c::set_time_next_pps(const ::osmosdr::time_spec_t &time_spec)
{
    _device->setHardwareTime(time_spec.to_ticks(1e9), "PPS");
}

void soapy_source_c::set_time_unknown_pps(const ::osmosdr::time_spec_t &time_spec)
{
    _device->setHardwareTime(time_spec.to_ticks(1e9), "UNKNOWN_PPS");
}
