/* -*- c++ -*- */
/*
 * Copyright 2018 Jeff Long <willcode4@gmail.com>
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gnu Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sdrplay_source_c.h"
#include <gnuradio/io_signature.h>
#include "osmosdr/source.h"
#include "arg_helpers.h"

#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/chrono.hpp>

#include <iostream>
#include <mutex>

#define MAX_SUPPORTED_DEVICES   4

using namespace boost::assign;

// Index by mir_sdr_Bw_MHzT
static std::vector<double> bandwidths = {
  0,     // Dummy
  200e3,
  300e3,
  600e3,
  1536e3,
  5000e3,
  6000e3,
  7000e3,
  8000e3
};

// TODO - RSP1 lower freq is 10e3.
#define SDRPLAY_FREQ_MIN 1e3
#define SDRPLAY_FREQ_MAX 2000e6

static std::string hwName(int hwVer)
{
  if (hwVer == 1)
    return "RSP1";
  if (hwVer == 2)
    return "RSP2";
  if (hwVer ==255)
    return "RSP1A";
  return "UNK";
}

sdrplay_source_c_sptr
make_sdrplay_source_c (const std::string &args)
{
  return gnuradio::get_initial_sptr(new sdrplay_source_c (args));
}

// 0 inputs, 1 output
static const int MIN_IN = 0;
static const int MAX_IN = 0;
static const int MIN_OUT = 1;
static const int MAX_OUT = 1;

sdrplay_source_c::sdrplay_source_c (const std::string &args)
  : gr::sync_block ("sdrplay_source_c",
                    gr::io_signature::make(MIN_IN, MAX_IN, sizeof (gr_complex)),
                    gr::io_signature::make(MIN_OUT, MAX_OUT, sizeof (gr_complex))),
  _auto_gain(false),
  _gain(40),
  _gRdB(40),
  _lna(0),
  _bcastNotch(0),
  _dabNotch(0),
  _band(0),
  _fsHz(8e6),
  _decim(1),
  _rfHz(100e6),
  _bwType(mir_sdr_BW_6_000),
  _ifType(mir_sdr_IF_Zero),
  _dcMode(1),
  _buffer(NULL),
  _streaming(false),
  _flowgraphRunning(false),
  _reinit(false)
{
  dict_t dict = params_to_dict(args);
  if (dict.count("sdrplay")) {
    _devIndex = boost::lexical_cast<unsigned int>(dict["sdrplay"]);
  }
  else {
    _devIndex = 0;
  }

  mir_sdr_DebugEnable(1);

  unsigned int numDevices;
  mir_sdr_DeviceT mirDevices[MAX_SUPPORTED_DEVICES];
  mir_sdr_GetDevices(mirDevices, &numDevices, MAX_SUPPORTED_DEVICES);
  _hwVer = mirDevices[_devIndex].hwVer;

  if (_hwVer == 2) {
    _antenna = "A";
  }
  else {
    _antenna = "RX";
  }

  _biasT = 0;
  if ( dict.count("bias") ) {
    _biasT = boost::lexical_cast<int>( dict["bias"] );
  }
}

sdrplay_source_c::~sdrplay_source_c ()
{
  if (_streaming) {
    stopStreaming();
  }
}

bool sdrplay_source_c::start(void)
{
  _flowgraphRunning = true;
  return true;
}

bool sdrplay_source_c::stop(void)
{
  _flowgraphRunning = false;
  return true;
}

int sdrplay_source_c::work(int noutput_items,
                           gr_vector_const_void_star &input_items,
                           gr_vector_void_star &output_items)
{
  gr_complex *out = (gr_complex *)output_items[0];

  if (!_streaming)
    startStreaming();

  {
    boost::mutex::scoped_lock lock(_bufferMutex);
    _buffer = out;
    _bufferSpaceRemaining = noutput_items;
    _bufferOffset = 0;
    _bufferReady.notify_one();

    while (_buffer && _streaming) {
      _bufferReady.wait(lock);
    }
  }

  if (!_streaming) {
    return 0;
  }

  return noutput_items - _bufferSpaceRemaining;
}

// Called by sdrplay streamer thread when data is available
void sdrplay_source_c::streamCallback(short *xi, short *xq,
                                      unsigned int firstSampleNum,
                                      int grChanged, int rfChanged, int fsChanged,
                                      unsigned int numSamples, unsigned int reset,
                                      void *cbContext)
{
  unsigned int i = 0;
  _reinit = false;

  boost::mutex::scoped_lock lock(_bufferMutex);

  while (i < numSamples) {

    // Discard samples if not streaming, if flowgraph not running, or reinit needed.
    if (!_streaming || _reinit || !_flowgraphRunning)
      return;

    // If buffer is not ready for write, wait a short time. Discard samples on timeout.
    if (!_buffer)
      if (boost::cv_status::timeout ==
          _bufferReady.wait_for(lock, boost::chrono::milliseconds(250)))
        return;

    // Copy until out of samples or buffer is full
    while ((i < numSamples) && (_bufferSpaceRemaining > 0)) {
      _buffer[_bufferOffset] =
        gr_complex(float(xi[i]) / 32768.0, float(xq[i]) / 32768.0);

      i++;
      _bufferOffset++;
      _bufferSpaceRemaining--;
    }

    if (_bufferSpaceRemaining == 0) {
      _buffer = NULL;
      _bufferReady.notify_one();
    }
  }
}

// Callback wrapper
void sdrplay_source_c::streamCallbackWrap(short *xi, short *xq,
                                          unsigned int firstSampleNum,
                                          int grChanged, int rfChanged, int fsChanged,
                                          unsigned int numSamples, unsigned int reset,
                                          void *cbContext)
{
  sdrplay_source_c *obj = (sdrplay_source_c *)cbContext;
  obj->streamCallback(xi, xq,
                      firstSampleNum,
                      grChanged, rfChanged, fsChanged,
                      numSamples, reset,
                      cbContext);
}

// Called by strplay streamer thread when gain reduction is changed.
void sdrplay_source_c::gainChangeCallback(unsigned int gRdB,
                                          unsigned int lnaGRdB,
                                          void *cbContext)
{
  std::cerr << "GR change, BB+MIX -" << gRdB << "dB, LNA -" << lnaGRdB
            << "dB, band " << _band << std::endl;
}

// Callback wrapper
void sdrplay_source_c::gainChangeCallbackWrap(unsigned int gRdB,
                                              unsigned int lnaGRdB,
                                              void *cbContext)
{
  sdrplay_source_c *obj = (sdrplay_source_c *)cbContext;
  obj->gainChangeCallback(gRdB,
                          lnaGRdB,
                          cbContext);
}

void sdrplay_source_c::startStreaming(void)
{
  if (_streaming) {
    std::cerr << "startStreaming(): already streaming." << std::endl;
    return;
  }

  unsigned int numDevices;
  mir_sdr_DeviceT mirDevices[MAX_SUPPORTED_DEVICES];
  mir_sdr_ReleaseDeviceIdx();
  mir_sdr_GetDevices(mirDevices, &numDevices, MAX_SUPPORTED_DEVICES);
  mir_sdr_SetDeviceIdx(_devIndex);

  std::cerr << "Using SDRplay " << hwName(_hwVer) << " "
            << mirDevices[_devIndex].SerNo << std::endl;

  // Set bias voltage on/off (RSP1A/RSP2).
  std::cerr << "Bias voltage: " << _biasT << std::endl;
  if (_hwVer == 2)
    mir_sdr_RSPII_BiasTControl(_biasT);
  else if (_hwVer == 255)
    mir_sdr_rsp1a_BiasT(_biasT);

  _streaming = true;

  int gRdBsystem = 0;
  int gRdB = _gRdB;

  mir_sdr_StreamInit(&gRdB,
                     _fsHz / 1e6,
                     _rfHz / 1e6,
                     _bwType,
                     _ifType,
                     checkLNA(_lna),
                     &gRdBsystem,
                     mir_sdr_USE_RSP_SET_GR,
                     &_samplesPerPacket,
                     &streamCallbackWrap,
                     &gainChangeCallbackWrap,
                     this);

  // Set decimation with halfband filter
  mir_sdr_DecimateControl(_decim != 1, _decim, 1);

  // Note that gqrx never calls set_dc_offset_mode() if the IQ balance
  // module is available.
  set_dc_offset_mode(osmosdr::source::DCOffsetOff, 0);
  updateGains();

  // Model-specific initialization
  if (_hwVer == 2) {
    set_antenna(get_antenna(), 0);
    mir_sdr_RSPII_RfNotchEnable(_bcastNotch);
  }

  else if (_hwVer == 255) {
    mir_sdr_rsp1a_BroadcastNotch(_bcastNotch);
    mir_sdr_rsp1a_DabNotch(_dabNotch);
  }
}

void sdrplay_source_c::stopStreaming(void)
{
  if (!_streaming) {
    std::cerr << "stopStreaming(): already stopped." << std::endl;
    return;
  }

  _streaming = false;

  mir_sdr_StreamUninit();
  mir_sdr_ReleaseDeviceIdx();
}

void sdrplay_source_c::reinitDevice(int reason)
{
  // If no reason given, reinit everything
  if (reason == (int)mir_sdr_CHANGE_NONE)
    reason = (mir_sdr_CHANGE_GR |
              mir_sdr_CHANGE_FS_FREQ |
              mir_sdr_CHANGE_RF_FREQ |
              mir_sdr_CHANGE_BW_TYPE |
              mir_sdr_CHANGE_IF_TYPE |
              mir_sdr_CHANGE_LO_MODE |
              mir_sdr_CHANGE_AM_PORT);

  int gRdB;
  int gRdBsystem; // Returned overall system gain reduction

  updateGains();
  gRdB = _gRdB;

  // Tell stream CB to return
  _reinit = true;

  mir_sdr_Reinit(&gRdB,
                 _fsHz / 1e6,
                 _rfHz / 1e6,
                 _bwType,
                 _ifType,
                 mir_sdr_LO_Auto,
                 checkLNA(_lna),
                 &gRdBsystem,
                 mir_sdr_USE_RSP_SET_GR,
                 &_samplesPerPacket,
                 (mir_sdr_ReasonForReinitT)reason
                 );

  // Set decimation with halfband filter
  mir_sdr_DecimateControl(_decim != 1, _decim, 1);

  _bufferReady.notify_one();
}

std::vector<std::string> sdrplay_source_c::get_devices()
{
  unsigned int numDevices;
  mir_sdr_DeviceT mirDevices[MAX_SUPPORTED_DEVICES];
  std::vector<std::string> devices;

  mir_sdr_GetDevices(mirDevices, &numDevices, MAX_SUPPORTED_DEVICES);

  for (unsigned int i=0; i<numDevices; i++) {
    mir_sdr_DeviceT *dev = &mirDevices[i];
    if (!dev->devAvail)
      continue;
    std::string args = boost::str(boost::format("sdrplay=%d,label='SDRplay %s %s'")
                                  % i % hwName((int)dev->hwVer) % dev->SerNo );
    std::cerr << args << std::endl;
    devices.push_back( args );
  }

  return devices;
}

size_t sdrplay_source_c::get_num_channels()
{
  return 1;
}

osmosdr::meta_range_t sdrplay_source_c::get_sample_rates()
{
  osmosdr::meta_range_t range;
  range += osmosdr::range_t( 62.5e3, 10e6 );
  return range;
}

double sdrplay_source_c::set_sample_rate(double rate)
{
  rate = std::min( std::max(rate,62.5e3), 10e6 );
  _fsHz = rate;

  // Decimation is required for rates below 2MS/s
  _decim = 1;
  while (_fsHz < 2e6) {
    _decim *= 2;
    _fsHz *= 2;
  }

  if (_streaming)
    reinitDevice((int)mir_sdr_CHANGE_FS_FREQ);

  return get_sample_rate();
}

double sdrplay_source_c::get_sample_rate()
{
  return _fsHz/_decim;
}

osmosdr::freq_range_t sdrplay_source_c::get_freq_range(size_t chan)
{
  osmosdr::freq_range_t range;
  range += osmosdr::range_t(SDRPLAY_FREQ_MIN,  SDRPLAY_FREQ_MAX);
  return range;
}

double sdrplay_source_c::set_center_freq(double freq, size_t chan)
{
  _rfHz = freq;

  if (_streaming) {
    reinitDevice((int)mir_sdr_CHANGE_RF_FREQ);
  }

  return get_center_freq( chan );
}

double sdrplay_source_c::get_center_freq(size_t chan)
{
  return _rfHz;
}

double sdrplay_source_c::set_freq_corr(double ppm, size_t chan)
{
  return get_freq_corr( chan );
}

double sdrplay_source_c::get_freq_corr(size_t chan)
{
  return 0;
}

std::vector<std::string> sdrplay_source_c::get_gain_names(size_t chan)
{
  std::vector<std::string> gains;

  gains += "LNA_ATTEN_STEP";
  gains += "SYS_ATTEN_DB";

  // RSP1A and RSP2 have broadcast notch filters, and RSP1A has a DAB
  // notch filter. Show all controls for all models, mainly because
  // gqrx gets confused when switching between sources with different
  // sets of gains.
  gains += "BCAST_NOTCH";
  gains += "DAB_NOTCH";

  return gains;
}

osmosdr::gain_range_t sdrplay_source_c::get_gain_range(size_t chan)
{
  osmosdr::gain_range_t range;

  for (int i = 20; i <= 59; i++)
    range += osmosdr::range_t((float)i);

  return range;
}

osmosdr::gain_range_t sdrplay_source_c::get_gain_range(const std::string & name, size_t chan)
{
  osmosdr::gain_range_t range;
  int maxLnaState;

  if (name == "LNA_ATTEN_STEP") {
    if (_hwVer == 2)
      maxLnaState = 8;
    else if (_hwVer == 255)
      maxLnaState = 9;
    else
      maxLnaState = 3;
    for (int i = 0; i <= maxLnaState; i++)
      range += osmosdr::range_t((float)i);
  }
  // RSP1A, RSP2
  else if (name == "BCAST_NOTCH") {
    range += osmosdr::range_t((float)0);
    if (_hwVer == 2 || _hwVer == 255)
      range += osmosdr::range_t((float)1);
  }
  // RSP1A
  else if (name == "DAB_NOTCH") {
    range += osmosdr::range_t((float)0);
    if (_hwVer == 255)
      range += osmosdr::range_t((float)1);
  }
  else {
    for (int i = 20; i <= 59; i++)
      range += osmosdr::range_t((float)i);
  }

  return range;
}

bool sdrplay_source_c::set_gain_mode(bool automatic, size_t chan)
{
  _auto_gain = automatic;
  if (_streaming) {
    if (automatic) {
      mir_sdr_AgcControl(mir_sdr_AGC_5HZ, -30, 0, 0, 0, 0, 0);
    }
    else {
      mir_sdr_AgcControl(mir_sdr_AGC_DISABLE, 0, 0, 0, 0, 0, 0);
      set_gain(get_gain(0));
    }
  }

  return get_gain_mode(chan);
}

bool sdrplay_source_c::get_gain_mode(size_t chan)
{
  return _auto_gain;
}

int sdrplay_source_c::checkLNA(int lna)
{
  // Clip LNA reduction step. See table in API section 5.3.
  if (_hwVer == 1) {
    lna = std::min(3, lna);
  }
  else if (_hwVer == 255) {
    if (_rfHz < 60000000)
      lna = std::min(6, lna);
    else if (_rfHz >= 1000000000)
      lna = std::min(8, lna);
    else
      lna = std::min(9, lna);
  }
  else if (_hwVer == 2) {
    if (_rfHz >= 420000000)
      lna = std::min(5, lna);
    else if (_rfHz < 60000000 && _antenna == "HIGHZ")
      lna = std::min(4, lna);
    else
      lna = std::min(8, lna);
  }

  return lna;
}

void sdrplay_source_c::updateGains(void)
{
  int gRdBsystem = 0;
  _gRdB = _gain;
  int gRdB = _gRdB;
  int lna = checkLNA(_lna);

  mir_sdr_GetGrByFreq(_rfHz/1e6, (mir_sdr_BandT *)&_band, &gRdB, lna, &gRdBsystem,
                      mir_sdr_USE_RSP_SET_GR);
  if (_streaming)
    mir_sdr_RSP_SetGr(gRdB, lna, 1 /*absolute*/, 0 /*immediate*/);
}

double sdrplay_source_c::set_gain(double gain, size_t chan)
{
  _gain = (int)gain;

  if (_streaming)
    updateGains();

  return gain;
}

double sdrplay_source_c::set_gain(double gain, const std::string & name, size_t chan)
{
  int bcastNotchChanged = 0;
  int dabNotchChanged = 0;

  if (name == "LNA_ATTEN_STEP") {
    _lna = int(gain);
  }
  // RSP1A, RSP2
  else if (name == "BCAST_NOTCH" && (_hwVer == 2 || _hwVer == 255)) {
    if (int(gain) != _bcastNotch)
      bcastNotchChanged = 1;
    _bcastNotch = int(gain);
  }
  // RSP1A
  else if (name == "DAB_NOTCH" && _hwVer == 255) {
    if (int(gain) != _dabNotch)
      dabNotchChanged = 1;
    _dabNotch = int(gain);
  }
  else {
    _gain = int(gain);
  }

  if (_streaming) {
    updateGains();

    if (bcastNotchChanged) {
      if (_hwVer == 255 ) {
        mir_sdr_rsp1a_BroadcastNotch(_bcastNotch);
      }
      else if (_hwVer == 2) {
        mir_sdr_RSPII_RfNotchEnable(_bcastNotch);
      }
    }

    if (dabNotchChanged) {
      mir_sdr_rsp1a_DabNotch(_dabNotch);
    }
  }

  return gain;
}

double sdrplay_source_c::get_gain(size_t chan)
{
  return _gain;
}

double sdrplay_source_c::get_gain(const std::string & name, size_t chan)
{
  if (name == "LNA_ATTEN_STEP")
    return _lna;
  else if (name == "BCAST_NOTCH")
    return _bcastNotch;
  else if (name == "DAB_NOTCH")
    return _dabNotch;
  else
    return _gain;
}

std::vector<std::string> sdrplay_source_c::get_antennas(size_t chan)
{
  std::vector<std::string> antennas;

  if (_hwVer == 2) {
    antennas += "A";
    antennas += "B";
    antennas += "HIGHZ";
  }
  else {
    antennas += "RX";
  }

  return antennas;
}

std::string sdrplay_source_c::set_antenna(const std::string & antenna, size_t chan)
{
  _antenna = antenna;

  if (_streaming) {
    if (_hwVer == 2) {
      // HIGHZ is ANTENNA_B with AmPortSelect
      if (antenna == "HIGHZ") {
        mir_sdr_RSPII_AntennaControl(mir_sdr_RSPII_ANTENNA_B);
        mir_sdr_AmPortSelect(1);
      }
      else {
        if (antenna == "A")
          mir_sdr_RSPII_AntennaControl(mir_sdr_RSPII_ANTENNA_A);
        else
          mir_sdr_RSPII_AntennaControl(mir_sdr_RSPII_ANTENNA_B);
        mir_sdr_AmPortSelect(0);
      }

      reinitDevice((int)mir_sdr_CHANGE_AM_PORT);
    }
  }

  return antenna;
}

std::string sdrplay_source_c::get_antenna(size_t chan)
{
  return _antenna.c_str();
}

// NOTE: DC offset controlled here, IQ balance always on.
void sdrplay_source_c::set_dc_offset_mode(int mode, size_t chan)
{
  if (osmosdr::source::DCOffsetOff == mode) {
    _dcMode = 0;
    if (_streaming) {
      mir_sdr_SetDcMode(0, 0);
      mir_sdr_DCoffsetIQimbalanceControl(0, 0);
    }
  }
  else if (osmosdr::source::DCOffsetManual == mode) {
    _dcMode = 0;
    if (_streaming) {
      mir_sdr_SetDcMode(0, 1);
      mir_sdr_DCoffsetIQimbalanceControl(0, 1);
    }
  }
  else if (osmosdr::source::DCOffsetAutomatic == mode) {
    _dcMode = 1;
    if (_streaming) {
      mir_sdr_SetDcMode(4, 1);
      mir_sdr_DCoffsetIQimbalanceControl(1, 1);
      mir_sdr_SetDcTrackTime(63);
    }
  }
}

void sdrplay_source_c::set_dc_offset(const std::complex<double> &offset, size_t chan)
{
  std::cerr << "set_dc_offset(): not implemented" << std::endl;
}

double sdrplay_source_c::set_bandwidth(double bandwidth, size_t chan)
{
  _bwType = mir_sdr_BW_8_000;

  for (double bw : bandwidths) {
    // Skip dummy value at index 0
    if (bw == 0)
      continue;
    if (bandwidth <= bw) {
      _bwType = (mir_sdr_Bw_MHzT)(bw/1e3);
      break;
    }
  }

  int actual = get_bandwidth(chan);
  std::cerr << "SDRplay bandwidth requested=" << bandwidth
            << " actual=" << actual << std::endl;

  if (_streaming) {
    reinitDevice((int)mir_sdr_CHANGE_BW_TYPE);
  }

  return actual;
}

double sdrplay_source_c::get_bandwidth(size_t chan)
{
  return (double)_bwType * 1e3;
}

osmosdr::freq_range_t sdrplay_source_c::get_bandwidth_range(size_t chan)
{
  osmosdr::freq_range_t range;

  // bandwidths[0] is a dummy
  for (unsigned int i=1; i<bandwidths.size(); i++)
    range += osmosdr::range_t(bandwidths[i]);

  return range;
}
