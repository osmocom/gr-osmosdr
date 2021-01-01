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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hackrf_common.h"

#include "arg_helpers.h"

int hackrf_common::_usage = 0;
std::mutex hackrf_common::_usage_mutex;

std::map<std::string, std::weak_ptr<hackrf_device>> hackrf_common::_devs;
std::mutex hackrf_common::_devs_mutex;

hackrf_common::hackrf_common(const std::string &args) :
  _dev(NULL),
  _sample_rate(0),
  _center_freq(0),
  _freq_corr(0),
  _auto_gain(false),
  _requested_bandwidth(0),
  _bandwidth(0),
  _bias(false),
  _started(false)
{
  int ret;
  hackrf_device *raw_dev;
  hackrf_device_list_t *list;
  int dev_index;
  std::string target_serial = "0";
  std::string final_serial = "";

  dict_t dict = params_to_dict(args);
  if (dict.count("hackrf") > 0 && dict["hackrf"].length() > 0) {
    target_serial = dict["hackrf"];
  }

  {
    std::lock_guard<std::mutex> guard(_usage_mutex);

    if (_usage == 0) {
      hackrf_init(); /* call only once before the first open */
    }

    _usage++;
  }

  list = hackrf_device_list();

  if (target_serial.length() > 1) {
    for (dev_index = 0; dev_index < list->devicecount; dev_index++) {
      if (list->serial_numbers[dev_index]) {
        std::string serial(list->serial_numbers[dev_index]);
        if (serial.compare(serial.length() - target_serial.length(),
                           target_serial.length(), target_serial) == 0) {
          break;
        }
      }
    }

    if (dev_index >= list->devicecount) {
      hackrf_device_list_free(list);
      throw std::runtime_error(
            "No device found with serial number '" + target_serial + "'");
    }
  } else {
    try {
      dev_index = std::stoi(target_serial);
    } catch (std::exception &ex) {
      hackrf_device_list_free(list);
      throw std::runtime_error(
            "Failed to use '" + target_serial + "' as HackRF device index number");
    }

    if (dev_index >= list->devicecount) {
      hackrf_device_list_free(list);
      throw std::runtime_error(
            "Failed to use '" + target_serial + "' as HackRF device index: not enough devices");
    }
  }

  if (list->serial_numbers[dev_index]) {
    final_serial = list->serial_numbers[dev_index];
  }

  {
    std::lock_guard<std::mutex> guard(_devs_mutex);

    if (_devs.count(final_serial) > 0 && !_devs[final_serial].expired()) {
      _dev = hackrf_sptr(_devs[final_serial]);
    } else {
      ret = hackrf_device_list_open(list, dev_index, &raw_dev);
      HACKRF_THROW_ON_ERROR(ret, "Failed to open HackRF device")
      _dev = hackrf_sptr(raw_dev, hackrf_common::close);
      _devs[final_serial] = static_cast<std::weak_ptr<struct hackrf_device>>(_dev);
    }
  }

  hackrf_device_list_free(list);

  uint8_t board_id;
  ret = hackrf_board_id_read(_dev.get(), &board_id);
  HACKRF_THROW_ON_ERROR(ret, "Failed to get HackRF board id")

  char version[40];
  memset(version, 0, sizeof(version));
  ret = hackrf_version_string_read(_dev.get(), version, sizeof(version));
  HACKRF_THROW_ON_ERROR(ret, "Failed to read version string")

  std::cerr << "Using " << hackrf_board_id_name(hackrf_board_id(board_id)) << " "
            << "with firmware " << version
            << std::endl;
}

void hackrf_common::close(void *dev)
{
  int ret = hackrf_close(static_cast<hackrf_device *>(dev));
  if (ret != HACKRF_SUCCESS)
  {
    std::cerr << HACKRF_FORMAT_ERROR(ret, "Failed to close HackRF") << std::endl;
  }

  {
    std::lock_guard<std::mutex> guard(_usage_mutex);

    _usage--;

    if (_usage == 0) {
      hackrf_exit(); /* call only once after last close */
    }
  }
}

std::vector<std::string> hackrf_common::get_devices()
{
  std::vector<std::string> devices;
  std::string label;

  {
    std::lock_guard<std::mutex> guard(_usage_mutex);

    if (_usage == 0) {
      hackrf_init(); /* call only once before the first open */
    }

    _usage++;
  }

  hackrf_device_list_t *list = hackrf_device_list();

  for (int i = 0; i < list->devicecount; i++) {
    label = "HackRF ";
    label += hackrf_usb_board_id_name(list->usb_board_ids[i]);

    std::string args;
    if (list->serial_numbers[i]) {
      std::string serial(list->serial_numbers[i]);
      if (serial.length() > 6)
        serial = serial.substr(serial.length() - 6, 6);
      args = "hackrf=" + serial;
      if (serial.length() )
      label += " " + serial;
    } else {
      args = "hackrf"; /* will pick the first one, serial number is required for choosing a specific one */
    }

    args += ",label='" + label + "'";
    devices.push_back(args);
  }

  hackrf_device_list_free(list);

  {
    std::lock_guard<std::mutex> guard(_usage_mutex);

    _usage--;

    if (_usage == 0) {
      hackrf_exit(); /* call only once after last close */
    }
  }

  return devices;
}

osmosdr::meta_range_t hackrf_common::get_sample_rates()
{
  osmosdr::meta_range_t range;

  /* we only add integer rates here because of better phase noise performance.
   * the user is allowed to request arbitrary (fractional) rates within these
   * boundaries. */

  range.push_back(osmosdr::range_t( 8e6 ));
  range.push_back(osmosdr::range_t( 10e6 ));
  range.push_back(osmosdr::range_t( 12.5e6 ));
  range.push_back(osmosdr::range_t( 16e6 ));
  range.push_back(osmosdr::range_t( 20e6 )); /* confirmed to work on fast machines */

  return range;
}

double hackrf_common::set_sample_rate( double rate )
{
  int ret;

  if (_dev.get() && _started) {
    ret = hackrf_set_sample_rate( _dev.get(), rate );
    if ( HACKRF_SUCCESS != ret ) {
      HACKRF_THROW_ON_ERROR( ret, HACKRF_FUNC_STR( "hackrf_set_sample_rate", rate ) )
    }
  }

  _sample_rate = rate;
  return get_sample_rate();
}

double hackrf_common::get_sample_rate()
{
  return _sample_rate;
}

osmosdr::freq_range_t hackrf_common::get_freq_range( size_t chan )
{
  osmosdr::freq_range_t range;

  range.push_back(osmosdr::range_t( _sample_rate / 2, 7250e6 - _sample_rate / 2 ));

  return range;
}

double hackrf_common::set_center_freq( double freq, size_t chan )
{
  int ret;

  #define APPLY_PPM_CORR(val, ppm) ((val) * (1.0 + (ppm) * 0.000001))

  if (_dev.get() && _started) {
    double corr_freq = APPLY_PPM_CORR( freq, _freq_corr );
    ret = hackrf_set_freq( _dev.get(), uint64_t(corr_freq) );
    if ( HACKRF_SUCCESS != ret ) {
      HACKRF_THROW_ON_ERROR( ret, HACKRF_FUNC_STR( "hackrf_set_freq", corr_freq ) )
    }
  }

  _center_freq = freq;
  return get_center_freq( chan );
}

double hackrf_common::get_center_freq( size_t chan )
{
  return _center_freq;
}

double hackrf_common::set_freq_corr( double ppm, size_t chan )
{
  _freq_corr = ppm;

  set_center_freq( _center_freq );

  return get_freq_corr( chan );
}

double hackrf_common::get_freq_corr( size_t chan )
{
  return _freq_corr;
}

bool hackrf_common::set_gain_mode( bool automatic, size_t chan )
{
  _auto_gain = automatic;

  return get_gain_mode(chan);
}

bool hackrf_common::get_gain_mode( size_t chan )
{
  return _auto_gain;
}

double hackrf_common::set_gain( double gain, size_t chan )
{
  int ret;
  double clip_gain = (gain >= 14.0) ? 14.0 : 0.0;

  if (_dev.get() && _started) {
    uint8_t value = (clip_gain == 14.0) ? 1 : 0;

    ret = hackrf_set_amp_enable( _dev.get(), value );
    if ( HACKRF_SUCCESS != ret ) {
      HACKRF_THROW_ON_ERROR( ret, HACKRF_FUNC_STR( "hackrf_set_amp_enable", value ) )
    }
  }

  _amp_gain = clip_gain;
  return hackrf_common::get_gain(chan);
}

double hackrf_common::get_gain( size_t chan )
{
  return _amp_gain;
}

std::vector< std::string > hackrf_common::get_antennas( size_t chan )
{
  return { get_antenna( chan ) };
}

std::string hackrf_common::set_antenna( const std::string & antenna, size_t chan )
{
  return get_antenna( chan );
}

std::string hackrf_common::get_antenna( size_t chan )
{
  return "TX/RX";
}

double hackrf_common::set_bandwidth( double bandwidth, size_t chan )
{
  int ret;
//  osmosdr::freq_range_t bandwidths = get_bandwidth_range( chan );

  _requested_bandwidth = bandwidth;
  if ( bandwidth == 0.0 ) /* bandwidth of 0 means automatic filter selection */
    bandwidth = _sample_rate * 0.75; /* select narrower filters to prevent aliasing */

  /* compute best default value depending on sample rate (auto filter) */
  uint32_t bw = hackrf_compute_baseband_filter_bw( uint32_t(bandwidth) );

  if (_dev.get() && _started) {
    ret = hackrf_set_baseband_filter_bandwidth( _dev.get(), bw );
    if (HACKRF_SUCCESS != ret) {
      HACKRF_THROW_ON_ERROR( ret, HACKRF_FUNC_STR( "hackrf_set_baseband_filter_bandwidth", bw ) )
    }
  }

  _bandwidth = bw;
  return get_bandwidth(chan);
}

double hackrf_common::get_bandwidth( size_t chan )
{
  return _bandwidth;
}

osmosdr::freq_range_t hackrf_common::get_bandwidth_range( size_t chan )
{
  osmosdr::freq_range_t bandwidths;

  // TODO: read out from libhackrf when an API is available

  bandwidths.push_back(osmosdr::range_t( 1750000 ));
  bandwidths.push_back(osmosdr::range_t( 2500000 ));
  bandwidths.push_back(osmosdr::range_t( 3500000 ));
  bandwidths.push_back(osmosdr::range_t( 5000000 ));
  bandwidths.push_back(osmosdr::range_t( 5500000 ));
  bandwidths.push_back(osmosdr::range_t( 6000000 ));
  bandwidths.push_back(osmosdr::range_t( 7000000 ));
  bandwidths.push_back(osmosdr::range_t( 8000000 ));
  bandwidths.push_back(osmosdr::range_t( 9000000 ));
  bandwidths.push_back(osmosdr::range_t( 10000000 ));
  bandwidths.push_back(osmosdr::range_t( 12000000 ));
  bandwidths.push_back(osmosdr::range_t( 14000000 ));
  bandwidths.push_back(osmosdr::range_t( 15000000 ));
  bandwidths.push_back(osmosdr::range_t( 20000000 ));
  bandwidths.push_back(osmosdr::range_t( 24000000 ));
  bandwidths.push_back(osmosdr::range_t( 28000000 ));

  return bandwidths;
}

bool hackrf_common::set_bias( bool bias )
{
  int ret;

  if (_dev.get() && _started) {
    ret = hackrf_set_antenna_enable(_dev.get(), static_cast<uint8_t>(bias));
    if (ret != HACKRF_SUCCESS)
    {
      std::cerr << "Failed to apply antenna bias voltage state: " << bias << HACKRF_FORMAT_ERROR(ret, "") << std::endl;
    }
  }

  _bias = bias;
  return get_bias();
}

bool hackrf_common::get_bias()
{
  return _bias;
}

void hackrf_common::start()
{
  _started = true;
  set_center_freq(get_center_freq());
  set_sample_rate(get_sample_rate());
  if (_requested_bandwidth != 0)
    set_bandwidth(get_bandwidth());
  set_gain(get_gain());
  set_bias(get_bias());
}

void hackrf_common::stop()
{
  _started = false;
}
