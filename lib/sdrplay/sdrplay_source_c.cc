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

#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/chrono.hpp>

#include <iostream>
#include <mutex>

#define MAX_SUPPORTED_DEVICES   4

using namespace boost::assign;

// Index by mir_sdr_BandT
static std::vector<Range<double>> rsp1bands = {
  {0, 12e6},        // AM Lo
  {12e6, 30e6},     // AM Mid
  {30e6, 60e6},     // AM Hi
  {60e6, 120e6},    // VHF
  {120e6, 250e6},   // Band 3
  {250e6, 420e6},   // Band X
  {420e6, 1000e6},  // Band 4/5
  {1000e6, 2000e6}  // Band L
};

// Indexed by mir_sdr_RSPII_BandT
static std::vector<Range<double>> rsp1abands = {
  {0, 0},           // Unknown
  {0, 2e6},        // AM Lo
  {2, 12e6},        // AM Lo
  {12e6, 30e6},     // AM Mid
  {30e6, 60e6},     // AM Hi
  {60e6, 120e6},    // VHF
  {120e6, 250e6},   // Band 3
  {250e6, 300e6},   // Band X Lo
  {300e6, 380e6},   // Band X Mid
  {380e6, 420e6},   // Band X Hi
  {420e6, 1000e6},  // Band 4/5
  {1000e6, 2000e6}  // Band L
};

// Indexed by mir_sdr_RSPII_BandT
static std::vector<Range<double>> rsp2bands = {
  {0, 0},           // Unknown
  {0, 12e6},        // AM Lo
  {12e6, 30e6},     // AM Mid
  {30e6, 60e6},     // AM Hi
  {60e6, 120e6},    // VHF
  {120e6, 250e6},   // Band 3
  {250e6, 300e6},   // Band X Lo
  {300e6, 380e6},   // Band X Mid
  {380e6, 420e6},   // Band X Hi
  {420e6, 1000e6},  // Band 4/5
  {1000e6, 2000e6}  // Band L
};

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

// TODO - switch for RSP1/2
#define SDRPLAY_FREQ_MIN 0
#define SDRPLAY_FREQ_MAX 2000e6

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
  _fsHz(2e6),
  _rfHz(100e6),
  _bwType(mir_sdr_BW_1_536),
  _ifType(mir_sdr_IF_Zero),
  _dcMode(1),
  _buffer(NULL),
  _running(false),
  _reinit(false)
{
  mir_sdr_DebugEnable(0);

  unsigned int numDevices;
  mir_sdr_DeviceT mirDevices[MAX_SUPPORTED_DEVICES];
  mir_sdr_GetDevices(mirDevices, &numDevices, MAX_SUPPORTED_DEVICES);

  // TODO: use selected device
  mir_sdr_SetDeviceIdx(0);
  _hwVer = mirDevices[0].hwVer;

  std::cerr << "Found SDRplay serial " << mirDevices[0].SerNo << " ";
  if (_hwVer == 2) {
    std::cerr << "RSP2" << std::endl;
    _bands = rsp2bands;
    _antenna = "A";
  }
  else if (_hwVer == 255) {
    std::cerr << "RSP1A" << std::endl;
    _bands = rsp1abands;
    _antenna = "RX";
  }
  else {
    std::cerr << "RSP1" << std::endl;
    _bands = rsp1bands;
    _antenna = "RX";
  }
}

sdrplay_source_c::~sdrplay_source_c ()
{
  if (_running) {
    stopDevice();
  }
  mir_sdr_ReleaseDeviceIdx();
}

int sdrplay_source_c::work(int noutput_items,
                           gr_vector_const_void_star &input_items,
                           gr_vector_void_star &output_items)
{
  gr_complex *out = (gr_complex *)output_items[0];

  if (!_running)
    startDevice();

  {
    boost::mutex::scoped_lock lock(_bufferMutex);
    _buffer = out;
    _bufferSpaceRemaining = noutput_items;
    _bufferOffset = 0;
    _bufferReady.notify_one();

    while (_buffer && _running) {
      _bufferReady.wait_for(lock, boost::chrono::milliseconds(100));
    }
  }

  if (!_running) {
    //return WORK_DONE;
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
  double bitScale;
  _reinit = false;

  if (_hwVer == 16)
    if (_fsHz >= 9216000)
      bitScale = 1.0f / 256;
    else if (_fsHz >= 8064000)
      bitScale = 1.0f / 1024;
    else if (_fsHz >= 6048000)
      bitScale = 1.0f / 2048;
    else
      bitScale = 1.0f / 8192;
  else
    bitScale = 1.0f / 2048;

  while (i < numSamples) {
    // Wait for work() to make buffer ready
    {
      boost::mutex::scoped_lock lock(_bufferMutex);
      while (!_buffer && _running && !_reinit) {
        _bufferReady.wait_for(lock, boost::chrono::milliseconds(100));
      }
    }

    if (!_running || _reinit) {
      return;
    }

    // Copy until out of samples or buffer is full
    while ((i < numSamples) && (_bufferSpaceRemaining > 0)) {
      boost::mutex::scoped_lock lock(_bufferMutex);
      _buffer[_bufferOffset] =
        // TODO - handle 14-bit and 16-bit samples for RSP1A
        gr_complex(float(xi[i]) * bitScale, float(xq[i]) * bitScale);

      i++;
      _bufferOffset++;
      _bufferSpaceRemaining--;
    }

    if (_bufferSpaceRemaining == 0) {
      boost::mutex::scoped_lock lock(_bufferMutex);
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
  std::cerr << "GR change, BB+MIX -" << gRdB << "dB, LNA -" << lnaGRdB << "dB" << std::endl;
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

void sdrplay_source_c::startDevice(void)
{
  int gRdBsystem = 0;
  int gRdB = _gRdB;

  if (_running) {
    std::cerr << "Already running." << std::endl;
    return;
  }
  _running = true;

  mir_sdr_StreamInit(&gRdB,
                     _fsHz / 1e6,
                     _rfHz / 1e6,
                     _bwType,
                     _ifType,
                     _lna,
                     &gRdBsystem,
                     mir_sdr_USE_RSP_SET_GR,
                     &_samplesPerPacket,
                     &streamCallbackWrap,
                     &gainChangeCallbackWrap,
                     this);

  // Note that gqrx never calls set_dc_offset_mode() if the IQ balance
  // module is available.
  set_dc_offset_mode(osmosdr::source::DCOffsetOff, 0);

  if (_hwVer == 2)
    set_antenna(get_antenna(), 0);
}

void sdrplay_source_c::stopDevice(void)
{
  if (!_running) {
    std::cerr << "Already stopped." << std::endl;
    return;
  }

  _running = false;

  mir_sdr_StreamUninit();
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
                 _lna,
                 &gRdBsystem,
                 mir_sdr_USE_RSP_SET_GR,
                 &_samplesPerPacket,
                 (mir_sdr_ReasonForReinitT)reason
                 );

  _bufferReady.notify_one();
}

std::vector<std::string> sdrplay_source_c::get_devices()
{
  unsigned int numDevices;
  mir_sdr_DeviceT mirDevices[MAX_SUPPORTED_DEVICES];
  std::vector<std::string> devices;

  mir_sdr_GetDevices(mirDevices, &numDevices, MAX_SUPPORTED_DEVICES);

  for (unsigned int i=0; i<numDevices; i++) {
    std::string args = boost::str(boost::format("sdrplay=%d,label='SDRplay RSP'") % (i+1) );
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

  // TODO - different for RSP1?
  // TODO - gqrx doesn't pay any attention, has own sdrplay values (wrong)
  range += osmosdr::range_t( 2000e3, 10000e3 );

  return range;
}

double sdrplay_source_c::set_sample_rate(double rate)
{
  rate = std::min( std::max(rate,2e6), 10e6 );
  _fsHz = rate;
  if (_running)
    reinitDevice((int)mir_sdr_CHANGE_FS_FREQ);

  return get_sample_rate();
}

double sdrplay_source_c::get_sample_rate()
{
  return _fsHz;
}

osmosdr::freq_range_t sdrplay_source_c::get_freq_range(size_t chan)
{
  osmosdr::freq_range_t range;
  // TODO - different for RSP1/1A?
  range += osmosdr::range_t(SDRPLAY_FREQ_MIN,  SDRPLAY_FREQ_MAX);
  return range;
}

double sdrplay_source_c::set_center_freq(double freq, size_t chan)
{
  int oldBand;

  _rfHz = freq;

  if (_running) {
    oldBand = _band;
    updateGains();
    // reinitDevice() is required only if band changes
    // mir_sdr_SetRf() is faster if band has not changed
    if (_band == oldBand)
      mir_sdr_SetRf(_rfHz, 1 /*absolute*/, 0 /*immediate*/);
    else
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
  std::vector< std::string > gains;

  gains += "LNA_REDUCTION";
  gains += "SYS_REDUCTION";

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

  if (name == "LNA_REDUCTION") {
    if (_hwVer == 2)
      maxLnaState = 8;
    else if (_hwVer == 255)
      maxLnaState = 9;
    else
      maxLnaState = 4;
    for (int i = 0; i <= maxLnaState; i++)
      range += osmosdr::range_t((float)i);
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
  if (_running) {
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

void sdrplay_source_c::updateGains(void)
{
  //mir_sdr_BandT band;
  int gRdBsystem = 0;
  _gRdB = _gain;
  int gRdB = _gRdB;

  // TODO - clip _lna
  mir_sdr_GetGrByFreq(_rfHz/1e6, (mir_sdr_BandT *)&_band, &gRdB, _lna, &gRdBsystem,
                      mir_sdr_USE_RSP_SET_GR);
  if (_running)
    mir_sdr_RSP_SetGr(gRdB, _lna, 1 /*absolute*/, 0 /*immediate*/);
}

double sdrplay_source_c::set_gain(double gain, size_t chan)
{
  _gain = (int)gain;

  if (_running)
    updateGains();

  return gain;
}

double sdrplay_source_c::set_gain(double gain, const std::string & name, size_t chan)
{
  if (name == "LNA_REDUCTION")
    _lna = int(gain);
  else
    _gain = int(gain);

  if (_running)
    updateGains();

  return gain;
}

double sdrplay_source_c::get_gain(size_t chan)
{
  return _gain;
}

double sdrplay_source_c::get_gain(const std::string & name, size_t chan)
{
  if (name == "LNA_REDUCTION")
    return _lna;
  else
    return _gain;
}

std::vector< std::string > sdrplay_source_c::get_antennas(size_t chan)
{
  std::vector< std::string > antennas;

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

  if (_running) {
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
    if (_running) {
      mir_sdr_SetDcMode(0, 0);
      mir_sdr_DCoffsetIQimbalanceControl(0, 0);
    }
  }
  else if (osmosdr::source::DCOffsetManual == mode) {
    _dcMode = 0;
    if (_running) {
      mir_sdr_SetDcMode(0, 1);
      mir_sdr_DCoffsetIQimbalanceControl(0, 1);
    }
  }
  else if (osmosdr::source::DCOffsetAutomatic == mode) {
    _dcMode = 1;
    if (_running) {
      mir_sdr_SetDcMode(4, 1);
      mir_sdr_DCoffsetIQimbalanceControl(1, 1);
      mir_sdr_SetDcTrackTime(63);
    }
  }
}

void sdrplay_source_c::set_dc_offset(const std::complex<double> &offset, size_t chan)
{
  std::cerr << "Manual DC correction mode is not implemented." << std::endl;
}

double sdrplay_source_c::set_bandwidth(double bandwidth, size_t chan)
{
  _bwType = mir_sdr_BW_8_000;

  for (double bw : bandwidths) {
    if (bandwidth <= bw) {
      _bwType = (mir_sdr_Bw_MHzT)(bw/1e3);
      break;
    }
  }

  int actual = get_bandwidth(chan);
  std::cerr << "set_bandwidth requested=" << bandwidth
            << " actual=" << actual << std::endl;

  if (_running) {
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
