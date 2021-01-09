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

#include "xtrx_sink_c.h"

#include "arg_helpers.h"

static const int max_burstsz = 4096;
using namespace boost::assign;

xtrx_sink_c_sptr make_xtrx_sink_c(const std::string &args)
{
  return gnuradio::get_initial_sptr(new xtrx_sink_c(args));
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

xtrx_sink_c::xtrx_sink_c(const std::string &args) :
  gr::sync_block("xtrx_sink_c",
                 gr::io_signature::make(parse_nchan(args),
                                        parse_nchan(args),
                                        sizeof(gr_complex)),
                 gr::io_signature::make(0, 0, 0)),
  _sample_flags(0),
  _rate(0),
  _master(0),
  _freq(0),
  _corr(0),
  _bandwidth(0),
  _dsp(0),
  _auto_gain(false),
  _otw(XTRX_WF_16),
  _mimo_mode(false),
  _gain_tx(0),
  _channels(parse_nchan(args)),
  _ts(8192),
  _swap_ab(false),
  _swap_iq(false),
  _tdd(false),
  _allow_dis(false),
  _dev("")
{

  dict_t dict = params_to_dict(args);

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

  if (dict.count("txdelay")) {
	_ts += 8192 * boost::lexical_cast< int >( dict["txdelay"] );
  }

  if (dict.count("allowdis")) {
	_allow_dis = boost::lexical_cast< bool >( dict["allowdis"] );
  }

  if (dict.count("swap_ab")) {
    _swap_ab = true;
    std::cerr << "xtrx_sink_c: swap AB channels";
  }

  if (dict.count("swap_iq")) {
    _swap_iq = true;
    std::cerr << "xtrx_sink_c: swap IQ";
  }

  if (dict.count("sfl")) {
    _sample_flags = boost::lexical_cast< unsigned >( dict["sfl"] );
  }

  if (dict.count("tdd")) {
    _tdd = true;
    std::cerr << "xtrx_sink_c: TDD mode";
  }

  if (dict.count("dsp")) {
      _dsp = boost::lexical_cast< double >( dict["dsp"] );
      std::cerr << "xtrx_sink_c: DSP:" << _dsp;
  }

  if (dict.count("dev")) {
      _dev =  dict["dev"];
      std::cerr << "xtrx_sink_c: XTRX device: %s" << _dev.c_str();
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

  std::cerr << "xtrx_sink_c::xtrx_sink_c()" << std::endl;
  set_alignment(32);
  set_output_multiple(max_burstsz);
}

xtrx_sink_c::~xtrx_sink_c()
{
  std::cerr << "xtrx_sink_c::~xtrx_sink_c()" << std::endl;
}

std::string xtrx_sink_c::name()
{
  return "GrLibXTRX";
}

size_t xtrx_sink_c::get_num_channels( void )
{
  return input_signature()->max_streams();
}

osmosdr::meta_range_t xtrx_sink_c::get_sample_rates( void )
{
  osmosdr::meta_range_t range;
  range += osmosdr::range_t( 1000000, 160000000, 1 );
  return range;
}

double xtrx_sink_c::set_sample_rate( double rate )
{
  std::cerr << "Set sample rate " << rate << std::endl;
  _rate = _xtrx->set_smaplerate(rate, _master, true, _sample_flags);
  return get_sample_rate();
}

double xtrx_sink_c::get_sample_rate( void )
{
  return _rate;
}

osmosdr::freq_range_t xtrx_sink_c::get_freq_range( size_t chan )
{
  osmosdr::freq_range_t range;
  range += osmosdr::range_t( double(0.03e9), double(3.8e9), 1); // as far as we know
  return range;
}

double xtrx_sink_c::set_center_freq( double freq, size_t chan )
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);

  _freq = freq;
  double corr_freq = (freq)*(1.0 + (_corr) * 0.000001);

  std::cerr << "TX Set freq " << freq << std::endl;
  xtrx_channel_t xchan = (xtrx_channel_t)(XTRX_CH_A << chan);

  int res = xtrx_tune_ex(_xtrx->dev(), (_tdd) ? XTRX_TUNE_TX_AND_RX_TDD : XTRX_TUNE_TX_FDD, xchan, corr_freq - _dsp, &_freq);
  if (res) {
    std::cerr << "Unable to deliver frequency " << corr_freq << std::endl;
  }

  res = xtrx_tune_ex(_xtrx->dev(), XTRX_TUNE_BB_TX, xchan, _dsp, NULL);
  return get_center_freq(chan);
}

double xtrx_sink_c::get_center_freq( size_t chan )
{
  return _freq + _dsp;
}

double xtrx_sink_c::set_freq_corr( double ppm, size_t chan )
{
  _corr = ppm;

  set_center_freq(_freq, chan);

  return get_freq_corr( chan );
}

double xtrx_sink_c::get_freq_corr( size_t chan )
{
  return _corr;
}


static const std::vector<std::string> s_lna_list = boost::assign::list_of("TX");

std::vector<std::string> xtrx_sink_c::get_gain_names( size_t chan )
{
  return s_lna_list;
}

osmosdr::gain_range_t xtrx_sink_c::get_gain_range( size_t chan )
{
  return get_gain_range("TX", chan);
}

osmosdr::gain_range_t xtrx_sink_c::get_gain_range( const std::string & name, size_t chan )
{
  osmosdr::gain_range_t range;
  range += osmosdr::range_t( -31, 0, 1 );
  return range;
}

bool xtrx_sink_c::set_gain_mode( bool automatic, size_t chan )
{
  _auto_gain = automatic;
  return get_gain_mode(chan);
}

bool xtrx_sink_c::get_gain_mode( size_t chan )
{
  return _auto_gain;
}

double xtrx_sink_c::set_gain( double gain, size_t chan )
{
  return set_gain(gain, "TX", chan);
}

double xtrx_sink_c::set_gain( double igain, const std::string & name, size_t chan )
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);

  osmosdr::gain_range_t gains = xtrx_sink_c::get_gain_range( name, chan );
  double gain = gains.clip(igain);
  double actual_gain;

  std::cerr << "Set TX gain: " << igain << std::endl;

  int res = xtrx_set_gain(_xtrx->dev(), (xtrx_channel_t)(XTRX_CH_A << chan),
                          XTRX_TX_PAD_GAIN, gain, &actual_gain);
  if (res) {
    std::cerr << "Unable to set gain `" << name.c_str() << "`; err=" << res << std::endl;
  }

  _gain_tx = actual_gain;
  return actual_gain;
}

double xtrx_sink_c::get_gain( size_t chan )
{
  return get_gain("TX");
}

double xtrx_sink_c::get_gain( const std::string & name, size_t chan )
{
  return _gain_tx;
}

double xtrx_sink_c::set_bandwidth( double bandwidth, size_t chan )
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);
  std::cerr << "Set bandwidth " << bandwidth << " chan " << chan << std::endl;

  if (bandwidth <= 0.0) {
    bandwidth = get_sample_rate() * 0.75;
    if (bandwidth < 0.5e6) {
      bandwidth = 0.5e6;
    }
  }

  int res = xtrx_tune_tx_bandwidth(_xtrx->dev(),
                                   (xtrx_channel_t)(XTRX_CH_A << chan),
                                   bandwidth, &_bandwidth);
  if (res) {
    std::cerr << "Can't set bandwidth: " << res << std::endl;
  }
  return get_bandwidth(chan);
}

double xtrx_sink_c::get_bandwidth( size_t chan )
{
  return _bandwidth;
}


static const std::map<std::string, xtrx_antenna_t> s_ant_map = boost::assign::map_list_of
  ("AUTO", XTRX_TX_AUTO)
  ("B1", XTRX_TX_H)
  ("B2", XTRX_TX_W)
  ("TXH", XTRX_TX_H)
  ("TXW", XTRX_TX_W)
  ;
static const std::map<xtrx_antenna_t, std::string> s_ant_map_r = boost::assign::map_list_of
  (XTRX_TX_H, "TXH")
  (XTRX_TX_W, "TXW")
  (XTRX_TX_AUTO, "AUTO")
  ;

static xtrx_antenna_t get_ant_type(const std::string& name)
{
  std::map<std::string, xtrx_antenna_t>::const_iterator it;

  it = s_ant_map.find(name);
  if (it != s_ant_map.end()) {
    return it->second;
  }

  return XTRX_TX_AUTO;
}

static const std::vector<std::string> s_ant_list = boost::assign::list_of
  ("AUTO")("TXH")("TXW")
  ;


std::vector< std::string > xtrx_sink_c::get_antennas( size_t chan )
{
  return s_ant_list;
}

std::string xtrx_sink_c::set_antenna( const std::string & antenna, size_t chan )
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);
  _ant = get_ant_type(antenna);

  std::cerr << "Set antenna " << antenna << std::endl;

  int res = xtrx_set_antenna_ex(_xtrx->dev(),
                                (xtrx_channel_t)(XTRX_CH_A << chan),
                                _ant);
  if (res) {
    std::cerr << "Can't set antenna: " << antenna << std::endl;
  }
  return get_antenna( chan );
}

std::string xtrx_sink_c::get_antenna( size_t chan )
{
  return s_ant_map_r.find(_ant)->second;
}

void xtrx_sink_c::tag_process(int ninput_items)
{
  std::sort(_tags.begin(), _tags.end(), gr::tag_t::offset_compare);

  const uint64_t samp0_count = this->nitems_read(0);
  uint64_t max_count = samp0_count + ninput_items;

  bool found_time_tag = false;
  for (const gr::tag_t &my_tag : _tags) {
    const uint64_t my_tag_count = my_tag.offset;
    const pmt::pmt_t &key = my_tag.key;
    const pmt::pmt_t &value = my_tag.value;

    if (my_tag_count >= max_count) {
      break;
    } else if(pmt::equal(key, TIME_KEY)) {
      //if (my_tag_count != samp0_count) {
      //    max_count = my_tag_count;
      //    break;
      //}
      found_time_tag = true;
      //_metadata.has_time_spec = true;
      //_metadata.time_spec = ::uhd::time_spec_t
      //      (pmt::to_uint64(pmt::tuple_ref(value, 0)),
      //       pmt::to_double(pmt::tuple_ref(value, 1)));
      uint64_t seconds = pmt::to_uint64(pmt::tuple_ref(value, 0));
      double fractional = pmt::to_double(pmt::tuple_ref(value, 1));

      std::cerr << "TX_TIME: " << seconds << ":" << fractional << std::endl;
    }
  } // end for

  if (found_time_tag) {
    //_metadata.has_time_spec = true;
  }
}

int xtrx_sink_c::work (int noutput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
{
  int ninput_items = noutput_items;
  const uint64_t samp0_count = nitems_read(0);
  get_tags_in_range(_tags, 0, samp0_count, samp0_count + ninput_items);
  if (!_tags.empty())
    tag_process(ninput_items);

  xtrx_send_ex_info_t nfo;
  nfo.samples = noutput_items;
  nfo.buffer_count = input_items.size();
  nfo.buffers = &input_items[0];
  nfo.flags = XTRX_TX_DONT_BUFFER;
  if (!_allow_dis)
    nfo.flags |= XTRX_TX_NO_DISCARD;
  nfo.ts = _ts;
  nfo.timeout = 0;

  int res = xtrx_send_sync_ex(_xtrx->dev(), &nfo);
  if (res) {
    std::cerr << "Err: " << res << std::endl;

    std::stringstream message;
    message << "xtrx_send_burst_sync error: " << -res;
    throw std::runtime_error( message.str() );
  }

  _ts += noutput_items;
  for (unsigned i = 0; i < input_items.size(); i++) {
    consume(i, noutput_items);
  }
  return 0;
}

bool xtrx_sink_c::start()
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);

  xtrx_run_params_t params;
  xtrx_run_params_init(&params);

  params.dir = XTRX_TX;
  if (!_mimo_mode)
    params.tx.flags |= XTRX_RSP_SISO_MODE;

  if (_swap_ab)
    params.tx.flags |= XTRX_RSP_SWAP_AB;

  if (_swap_iq)
    params.tx.flags |= XTRX_RSP_SWAP_IQ;

  params.tx.hfmt = XTRX_IQ_FLOAT32;
  params.tx.wfmt = _otw;
  params.tx.chs = XTRX_CH_AB;
  params.tx.paketsize = 0;
  params.rx_stream_start = 256*1024;

  int res = xtrx_run_ex(_xtrx->dev(), &params);
  if (res) {
    std::cerr << "Got error: " << res << std::endl;
  }

  return res == 0;
}

bool xtrx_sink_c::stop()
{
  boost::mutex::scoped_lock lock(_xtrx->mtx);

  //TODO:
  std::cerr << "xtrx_sink_c::stop()" << std::endl;
  int res = xtrx_stop(_xtrx->dev(), XTRX_TX);
  if (res) {
    std::cerr << "Got error: " << res << std::endl;
  }

  return res == 0;
}
