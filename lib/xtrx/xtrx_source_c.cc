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
#include <fstream>
#include <string>
#include <sstream>
#include <map>

#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/deinterleave.h>
#include <gnuradio/blocks/float_to_complex.h>

#include "xtrx_source_c.h"

#include "arg_helpers.h"

using namespace boost::assign;


xtrx_source_c_sptr make_xtrx_source_c(const std::string &args)
{
  return gnuradio::get_initial_sptr(new xtrx_source_c(args));
}

static size_t parse_nchan(const std::string &args)
{
  size_t nchan = 1;

  dict_t dict = params_to_dict(args);

  if (dict.count("nchan"))
    nchan = boost::lexical_cast< size_t >( dict["nchan"] );

  if (nchan < 1)
    nchan = 1;

  return nchan;
}

xtrx_source_c::xtrx_source_c(const std::string &args) :
  gr::sync_block("xtrx_source_c",
                 gr::io_signature::make(0, 0, 0),
                 gr::io_signature::make(parse_nchan(args),
                                        parse_nchan(args),
                                        sizeof(gr_complex))),
  _sample_flags(0),
  _rate(0),
  _master(0),
  _freq(0),
  _corr(0),
  _bandwidth(0),
  _auto_gain(false),
  _otw(XTRX_WF_16),
  _mimo_mode(false),
  _gain_lna(0),
  _gain_tia(0),
  _gain_pga(0),
  _channels(parse_nchan(args)),
  _swap_ab(false),
  _swap_iq(false),
  _loopback(false),
  _tdd(false),
  _fbctrl(false),
  _timekey(false),
  _dsp(0)
{
  _id = pmt::string_to_symbol(args);

  dict_t dict = params_to_dict(args);

  if (dict.count("otw_format")) {
    const std::string& otw = dict["otw_format"];
    if (otw == "sc16" || otw == "16") {
      _otw = XTRX_WF_16;
    } else if (otw == "sc12" || otw == "12") {
      _otw = XTRX_WF_12;
    } else if (otw == "sc8" || otw == "8") {
      _otw = XTRX_WF_8;
    } else {
      throw std::runtime_error("Parameter `otw_format` should be {sc16,sc12,sc8}");
    }
  }

  if (dict.count("master")) {
    _master = boost::lexical_cast< double >( dict["master"]);
  }

  std::cerr << args.c_str() << std::endl;

  int loglevel = 4;
  if (dict.count("loglevel")) {
    loglevel = boost::lexical_cast< int >( dict["loglevel"] );
  }

  bool lmsreset = 0;
  if (dict.count("lmsreset")) {
    lmsreset = boost::lexical_cast< bool >( dict["lmsreset"] );
  }

  if (dict.count("fbctrl")) {
	_fbctrl = boost::lexical_cast< bool >( dict["fbctrl"] );
  }

  if (dict.count("swap_ab")) {
    _swap_ab = true;
    std::cerr << "xtrx_source_c: swap AB channels";
  }

  if (dict.count("swap_iq")) {
    _swap_iq = true;
    std::cerr << "xtrx_source_c: swap IQ";
  }

  if (dict.count("sfl")) {
    _sample_flags = boost::lexical_cast< unsigned >( dict["sfl"] );
  }

  if (dict.count("loopback")) {
    _loopback = true;
    std::cerr << "xtrx_source_c: loopback";
  }

  if (dict.count("tdd")) {
    _tdd = true;
    std::cerr << "xtrx_source_c: TDD mode";
  }

  if (dict.count("dsp")) {
    _dsp = boost::lexical_cast< double >( dict["dsp"] );
    std::cerr << "xtrx_source_c: DSP:" << _dsp;
  }

  if (dict.count("dev")) {
    _dev =  dict["dev"];
    std::cerr << "xtrx_source_c: XTRX device: %s" << _dev.c_str();
  }

  _xtrx = xtrx_obj::get(_dev.c_str(), loglevel, lmsreset);
  if (_xtrx->dev_count() * 2 == _channels) {
    _mimo_mode = true;
  } else if (_xtrx->dev_count() != _channels) {
    throw std::runtime_error("Number of requested channels != number of devices");
  }

  if (dict.count("refclk")) {
    xtrx_set_ref_clk(_xtrx->dev(), boost::lexical_cast< unsigned >( dict["refclk"] ), XTRX_CLKSRC_INT);
  }
  if (dict.count("extclk")) {
    xtrx_set_ref_clk(_xtrx->dev(), boost::lexical_cast< unsigned >( dict["extclk"] ), XTRX_CLKSRC_EXT);
  }

  if (dict.count("vio")) {
    unsigned vio = boost::lexical_cast< unsigned >( dict["vio"] );
    _xtrx->set_vio(vio);
  }

  if (dict.count("dac")) {
    unsigned dac = boost::lexical_cast< unsigned >( dict["dac"] );
    xtrx_val_set(_xtrx->dev(), XTRX_TRX, XTRX_CH_ALL, XTRX_VCTCXO_DAC_VAL, dac);
  }

  if (dict.count("pmode")) {
    unsigned pmode = boost::lexical_cast< unsigned >( dict["pmode"] );
    xtrx_val_set(_xtrx->dev(), XTRX_TRX, XTRX_CH_ALL, XTRX_LMS7_PWR_MODE, pmode);
  }

  if (dict.count("timekey")) {
    _timekey = boost::lexical_cast< bool >( dict["timekey"] );
  }

  std::cerr << "xtrx_source_c::xtrx_source_c()" << std::endl;
  set_alignment(32);
  if (_otw == XTRX_WF_16) {
    if (_mimo_mode)
      set_output_multiple(4096);
    else
      set_output_multiple(8192);
  } else if (_otw == XTRX_WF_8) {
    if (_mimo_mode)
      set_output_multiple(8192);
    else
      set_output_multiple(16384);
  }
}

xtrx_source_c::~xtrx_source_c()
{
  std::cerr << "xtrx_source_c::~xtrx_source_c()" << std::endl;
}

std::string xtrx_source_c::name()
{
  return "GrLibXTRX";
}

size_t xtrx_source_c::get_num_channels( void )
{
  return output_signature()->max_streams();
}

osmosdr::meta_range_t xtrx_source_c::get_sample_rates( void )
{
  osmosdr::meta_range_t range;
  range += osmosdr::range_t( 200000, 160000000, 1 );
  return range;
}

double xtrx_source_c::set_sample_rate( double rate )
{
  std::cerr << "Set sample rate " << rate << std::endl;
  _rate = _xtrx->set_smaplerate(rate, _master, false, _sample_flags);
  return get_sample_rate();
}

double xtrx_source_c::get_sample_rate( void )
{
  return _rate;
}

osmosdr::freq_range_t xtrx_source_c::get_freq_range( size_t chan )
{
  osmosdr::freq_range_t range;
  range += osmosdr::range_t( double(0.03e9), double(3.8e9), 1); // as far as we know
  return range;
}

double xtrx_source_c::set_center_freq( double freq, size_t chan )
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);

  _freq = freq;
  double corr_freq = (freq)*(1.0 + (_corr) * 0.000001);

  if (_tdd)
    return get_center_freq(chan);

  xtrx_channel_t xchan = (xtrx_channel_t)(XTRX_CH_A << chan);

  std::cerr << "Set freq " << freq << std::endl;

  int res = xtrx_tune_ex(_xtrx->dev(), XTRX_TUNE_RX_FDD, xchan, corr_freq - _dsp, &_freq);
  if (res) {
    std::cerr << "Unable to deliver frequency " << corr_freq << std::endl;
  }

  res = xtrx_tune_ex(_xtrx->dev(), XTRX_TUNE_BB_RX, xchan, _dsp, NULL);

  return get_center_freq(chan);
}

double xtrx_source_c::get_center_freq( size_t chan )
{
  return _freq;
}

double xtrx_source_c::set_freq_corr( double ppm, size_t chan )
{
  _corr = ppm;

  set_center_freq(_freq, chan);

  return get_freq_corr( chan );
}

double xtrx_source_c::get_freq_corr( size_t chan )
{
  return _corr;
}

static const std::map<std::string, xtrx_gain_type_t> s_lna_map = boost::assign::map_list_of
    ("LNA", XTRX_RX_LNA_GAIN)
    ("TIA", XTRX_RX_TIA_GAIN)
    ("PGA", XTRX_RX_PGA_GAIN)
    ("LB", XTRX_RX_LB_GAIN)
    ;

static xtrx_gain_type_t get_gain_type(const std::string& name)
{
  std::map<std::string, xtrx_gain_type_t>::const_iterator it;

  it = s_lna_map.find(name);
  if (it != s_lna_map.end()) {
	return it->second;
  }

  return XTRX_RX_LNA_GAIN;
}

static const std::vector<std::string> s_lna_list = boost::assign::list_of
  ("LNA")("TIA")("PGA")("LB")
    ;

std::vector<std::string> xtrx_source_c::get_gain_names( size_t chan )
{
  return s_lna_list;
}

osmosdr::gain_range_t xtrx_source_c::get_gain_range( size_t chan )
{
  return get_gain_range("LNA", chan);
}

osmosdr::gain_range_t xtrx_source_c::get_gain_range( const std::string & name, size_t chan )
{
  osmosdr::gain_range_t range;

  if (name == "LNA") {
    range += osmosdr::range_t( 0, 24,  3 );
    range += osmosdr::range_t( 25, 30, 1 );
  } else if (name == "TIA") {
    range += osmosdr::range_t( 0 );
    range += osmosdr::range_t( 9 );
    range += osmosdr::range_t( 12 );
  } else if (name == "PGA") {
    range += osmosdr::range_t( -12.5, 12.5, 1 );
  } else if (name == "LB") {
    range += osmosdr::range_t( -40, 0, 1 );
  }

  return range;
}

bool xtrx_source_c::set_gain_mode( bool automatic, size_t chan )
{
  _auto_gain = automatic;
  return get_gain_mode(chan);
}

bool xtrx_source_c::get_gain_mode( size_t chan )
{
  return _auto_gain;
}

double xtrx_source_c::set_gain( double gain, size_t chan )
{
  return set_gain(gain, "LNA", chan);
}

double xtrx_source_c::set_gain( double igain, const std::string & name, size_t chan )
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);

  osmosdr::gain_range_t gains = xtrx_source_c::get_gain_range( name, chan );
  double gain = gains.clip(igain);
  double actual_gain;
  xtrx_gain_type_t gt = get_gain_type(name);

  std::cerr << "Set gain " << name << " (" << gt << "): " << igain << std::endl;

  int res = xtrx_set_gain(_xtrx->dev(), (xtrx_channel_t)(XTRX_CH_A << chan),
                          gt, gain, &actual_gain);
  if (res) {
    std::cerr << "Unable to set gain `" << name.c_str() << "`; err=" << res << std::endl;
  }

  switch (gt) {
  case XTRX_RX_LNA_GAIN: _gain_lna = actual_gain; break;
  case XTRX_RX_TIA_GAIN: _gain_tia = actual_gain; break;
  case XTRX_RX_PGA_GAIN: _gain_pga = actual_gain; break;
  default: break;
  }

  return actual_gain;
}

double xtrx_source_c::get_gain( size_t chan )
{
  return get_gain("LNA");
}

double xtrx_source_c::get_gain( const std::string & name, size_t chan )
{
  xtrx_gain_type_t gt = get_gain_type(name);
  switch (gt) {
  case XTRX_RX_LNA_GAIN: return _gain_lna;
  case XTRX_RX_TIA_GAIN: return _gain_tia;
  case XTRX_RX_PGA_GAIN: return _gain_pga;
  default: return 0;
  }
}

double xtrx_source_c::set_if_gain(double gain, size_t chan)
{
  return set_gain(gain, "PGA", chan);
}

double xtrx_source_c::set_bandwidth( double bandwidth, size_t chan )
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);
  std::cerr << "Set bandwidth " << bandwidth << " chan " << chan << std::endl;

  if (bandwidth <= 0.0) {
    bandwidth = get_sample_rate() * 0.75;
    if (bandwidth < 0.5e6) {
      bandwidth = 0.5e6;
    }
  }

  int res = xtrx_tune_rx_bandwidth(_xtrx->dev(), (xtrx_channel_t)(XTRX_CH_A << chan),
                                   bandwidth, &_bandwidth);
  if (res) {
    std::cerr << "Can't set bandwidth: " << res << std::endl;
  }
  return get_bandwidth(chan);
}

double xtrx_source_c::get_bandwidth( size_t chan )
{
  return _bandwidth;
}

osmosdr::freq_range_t xtrx_source_c::get_bandwidth_range( size_t chan )
{
  return osmosdr::freq_range_t(500e3, 140e6, 0);
}


static const std::map<std::string, xtrx_antenna_t> s_ant_map = boost::assign::map_list_of
    ("AUTO", XTRX_RX_AUTO)
    ("RXL", XTRX_RX_L)
    ("RXH", XTRX_RX_H)
    ("RXW", XTRX_RX_W)
    ("RXL_LB", XTRX_RX_L_LB)
    ("RXW_LB", XTRX_RX_W_LB)
    ;
static const std::map<xtrx_antenna_t, std::string> s_ant_map_r = boost::assign::map_list_of
    (XTRX_RX_AUTO, "AUTO")
    (XTRX_RX_L, "RXL")
    (XTRX_RX_H, "RXH")
    (XTRX_RX_W, "RXW")
    (XTRX_RX_L_LB, "RXL_LB")
    (XTRX_RX_W_LB, "RXW_LB")
    ;

static xtrx_antenna_t get_ant_type(const std::string& name)
{
  std::map<std::string, xtrx_antenna_t>::const_iterator it;

  it = s_ant_map.find(name);
  if (it != s_ant_map.end()) {
    return it->second;
  }

  return XTRX_RX_AUTO;
}

static const std::vector<std::string> s_ant_list = boost::assign::list_of
    ("AUTO")("RXL")("RXH")("RXW")
    ;


std::vector< std::string > xtrx_source_c::get_antennas( size_t chan )
{
  return s_ant_list;
}

std::string xtrx_source_c::set_antenna( const std::string & antenna, size_t chan )
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);
  _ant = get_ant_type(antenna);

  std::cerr << "Set antenna " << antenna << " type:" << _ant << std::endl;

  int res = xtrx_set_antenna_ex(_xtrx->dev(), (xtrx_channel_t)(XTRX_CH_A << chan),
                                _ant);
  if (res) {
    std::cerr << "Can't set antenna: " << antenna << std::endl;
  }
  return get_antenna( chan );
}

std::string xtrx_source_c::get_antenna( size_t chan )
{
  return s_ant_map_r.find(_ant)->second;
}

int xtrx_source_c::work (int noutput_items,
                         gr_vector_const_void_star &input_items,
                         gr_vector_void_star &output_items)
{
  xtrx_recv_ex_info_t ri;
  ri.samples = noutput_items;
  ri.buffer_count = output_items.size();
  ri.buffers = &output_items[0];
  ri.flags = RCVEX_DONT_INSER_ZEROS | RCVEX_DROP_OLD_ON_OVERFLOW;
  ri.timeout = 1000;

  int res = xtrx_recv_sync_ex(_xtrx->dev(), &ri);
  if (res) {
    std::stringstream message;
    message << "xtrx_recv_sync error: " << -res;
    throw std::runtime_error( message.str() );
  }

  if (_timekey) {
    uint64_t seconds = (ri.out_first_sample / _rate);
    double fractional = (ri.out_first_sample - (uint64_t)(_rate * seconds)) / _rate;

    //std::cerr << "Time " << seconds << ":" << fractional << std::endl;
    const pmt::pmt_t val = pmt::make_tuple
        (pmt::from_uint64(seconds),
         pmt::from_double(fractional));
    for(size_t i = 0; i < output_items.size(); i++) {
      this->add_item_tag(i, nitems_written(0), TIME_KEY,
                         val, _id);
      this->add_item_tag(i, nitems_written(0), RATE_KEY,
                         pmt::from_double(_rate), _id);
      this->add_item_tag(i, nitems_written(0), FREQ_KEY,
                         pmt::from_double(this->get_center_freq(i)), _id);
    }
  }
  return ri.out_samples;
}

bool xtrx_source_c::start()
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);

  xtrx_run_params_t params;
  xtrx_run_params_init(&params);

  params.dir = XTRX_RX;
  if (!_mimo_mode)
    params.rx.flags |= XTRX_RSP_SISO_MODE;

  if (_swap_ab)
    params.rx.flags |= XTRX_RSP_SWAP_AB;

  if (_swap_iq)
    params.rx.flags |= XTRX_RSP_SWAP_IQ;

  params.rx.hfmt = XTRX_IQ_FLOAT32;
  params.rx.wfmt = _otw;
  params.rx.chs = XTRX_CH_AB;
  params.rx.paketsize = 0;
  params.rx_stream_start = 256*1024;

  params.nflags = (_loopback) ? XTRX_RUN_DIGLOOPBACK : 0;

  int res = xtrx_run_ex(_xtrx->dev(), &params);
  if (res) {
    std::cerr << "Got error: " << res << std::endl;
  }

  res = xtrx_tune_ex(_xtrx->dev(), XTRX_TUNE_BB_RX, XTRX_CH_ALL, _dsp, NULL);

  return res == 0;
}

bool xtrx_source_c::stop()
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);
  //TODO:
  std::cerr << "xtrx_source_c::stop()" << std::endl;
  int res = xtrx_stop(_xtrx->dev(), XTRX_RX);
  if (res) {
    std::cerr << "Got error: " << res << std::endl;
  }

  return res == 0;
}
