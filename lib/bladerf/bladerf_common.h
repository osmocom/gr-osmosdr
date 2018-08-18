/* -*- c++ -*- */
/*
 * Copyright 2013-2017 Nuand LLC
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
#ifndef INCLUDED_BLADERF_COMMON_H
#define INCLUDED_BLADERF_COMMON_H

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <libbladeRF.h>

#include "osmosdr/ranges.h"
#include "arg_helpers.h"

#include "bladerf_compat.h"

#ifdef _MSC_VER
#include <cstddef>
typedef ptrdiff_t ssize_t;
#endif //_MSC_VER

#define BLADERF_DEBUG_ENABLE

typedef std::shared_ptr<struct bladerf> bladerf_sptr;

/* Identification of the bladeRF hardware in use */
typedef enum {
  BOARD_TYPE_UNKNOWN,   /**< Board type is unknown */
  BOARD_TYPE_NONE,      /**< Uninitialized or no board present */
  BOARD_TYPE_BLADERF_1, /**< bladeRF 1 (LMS6002D-based, 1RX/1TX) */
  BOARD_TYPE_BLADERF_2, /**< bladeRF 2 (AD9361-based, 2RX/2TX) */
} bladerf_board_type;

/* Mapping of bladerf_channel to bool */
typedef std::map<bladerf_channel, bool> bladerf_channel_enable_map;

/* Mapping of bladerf_channel to gnuradio port/chan */
typedef std::map<bladerf_channel, int> bladerf_channel_map;

/* Convenience macros for throwing a runtime error */
#define BLADERF_THROW(message)                                              \
  {                                                                         \
    throw std::runtime_error(std::string(__FUNCTION__) + ": " + message);   \
  }

#define BLADERF_THROW_STATUS(status, message)                               \
  {                                                                         \
    BLADERF_THROW(boost::str(boost::format("%s: %s (%d)") % message         \
      % bladerf_strerror(status) % status));                                \
  }

/* Convenience macros for printing a warning message to stderr */
#define BLADERF_WARNING(message)                                            \
  {                                                                         \
    std::cerr << _pfx << __FUNCTION__ << ": " << message << std::endl;      \
  }

#define BLADERF_WARN_STATUS(status, message)                                \
  {                                                                         \
    BLADERF_WARNING(message << ": " << bladerf_strerror(status));           \
  }                                                                         \

/* Convenience macro for printing an informational message to stdout */
#define BLADERF_INFO(message)                                               \
  {                                                                         \
    std::cout << _pfx << __FUNCTION__ << ": " << message << std::endl;      \
  }

/* Convenience macro for printing a debug message to stdout */
#ifdef BLADERF_DEBUG_ENABLE
#define BLADERF_DEBUG(message) BLADERF_INFO("DEBUG: " << message)
#else
#define BLADERF_DEBUG(message)
#endif // BLADERF_DEBUG_ENABLE

/* Given a bladerf_channel_layout, calculate the number of streams */
size_t num_streams(bladerf_channel_layout layout);

/**
 * Common class for bladeRF interaction
 */
class bladerf_common
{
public:
  /*****************************************************************************
   * Public methods
   ****************************************************************************/
  bladerf_common();

protected:
  /*****************************************************************************
   * Protected methods
   ****************************************************************************/

  /**
   * Handle initialization and parameters common to both source & sink
   *
   * Specify arguments in key=value,key=value format, e.g.
   *    bladerf=0,buffers=512
   *
   * Recognized arguments:
   *  Key             Allowed values
   * ---------------------------------------------------------------------------
   * REQUIRED:
   *  bladerf         a valid instance or serial number
   * USB INTERFACE CONTROL:
   *  buffers         (default: NUM_BUFFERS)
   *  buflen          (default: NUM_SAMPLES_PER_BUFFER)
   *  stream_timeout  valid time in milliseconds (default: 3000)
   *  transfers       (default: NUM_TRANSFERS)
   * FPGA CONTROL:
   *  enable_metadata 1 to enable metadata
   *  fpga            a path to a valid .rbf file
   *  fpga-reload     1 to force reloading the FPGA unconditionally
   * RF CONTROL:
   *  agc             1 to enable, 0 to disable (default: hardware-dependent)
   *  agc_mode        default, manual, fast, slow, hybrid (default: default)
   *  loopback        bb_txlpf_rxvga2, bb_txlpf_rxlpf, bb_txvga1_rxvga2,
   *                  bb_txvga1_rxlpf, rf_lna1, rf_lna2, rf_lna3, firmware,
   *                  ad9361_bist, none (default: none)
   *                    ** Note: valid on receive channels only
   *  rxmux           baseband, 12bit, 32bit, digital (default: baseband)
   *                    ** Note: valid on receive channels only
   *  smb             a valid frequency
   *  tamer           internal, external_1pps, external (default: internal)
   *  xb200           auto, auto3db, 50M, 144M, 222M, custom (default: auto)
   * MISC:
   *  verbosity       verbose, debug, info, warning, error, critical, silent
   *                    (default: info)
   *                    ** Note: applies only to libbladeRF logging
   */
  void init(dict_t const &dict, bladerf_direction direction);

  /* Get a vector of available devices */
  static std::vector<std::string> devices();
  /* Get the type of the open bladeRF board */
  bladerf_board_type get_board_type();
  /* Get the maximum number of channels supported in a given direction */
  size_t get_max_channels(bladerf_direction direction);

  void set_channel_enable(bladerf_channel ch, bool enable);
  bool get_channel_enable(bladerf_channel ch);

  /* Set libbladeRF verbosity */
  void set_verbosity(std::string const &verbosity);

  /* Convert an antenna/channel name (e.g. "RX2") to a bladerf_channel */
  bladerf_channel str2channel(std::string const &ch);
  /* Convert a bladerf_channel to an antenna/channel name (e.g. "RX2") */
  std::string channel2str(bladerf_channel ch);
  /* Convert a bladerf_channel to a hardware port identifier */
  int channel2rfport(bladerf_channel ch);

  /* Using the channel map, get the bladerf_channel for a gnuradio chan */
  bladerf_channel chan2channel(bladerf_direction direction, size_t chan = 0);

  /* Get range of supported sampling rates for channel ch */
  osmosdr::meta_range_t sample_rates(bladerf_channel ch);
  /* Set sampling rate on channel ch to rate */
  double set_sample_rate(double rate, bladerf_channel ch);
  /* Get the current sampling rate on channel ch */
  double get_sample_rate(bladerf_channel ch);

  /* Get range of supported RF frequencies for channel ch */
  osmosdr::freq_range_t freq_range(bladerf_channel ch);
  /* Set center RF frequency of channel ch to freq */
  double set_center_freq(double freq, bladerf_channel ch);
  /* Get the center RF frequency of channel ch */
  double get_center_freq(bladerf_channel ch);

  /* Get range of supported bandwidths for channel ch */
  osmosdr::freq_range_t filter_bandwidths(bladerf_channel ch);
  /* Set the bandwidth on channel ch to bandwidth */
  double set_bandwidth(double bandwidth, bladerf_channel ch);
  /* Get the current bandwidth of channel ch */
  double get_bandwidth(bladerf_channel ch);

  /* Get the names of gain stages on channel ch */
  std::vector<std::string> get_gain_names(bladerf_channel ch);
  /* Get range of supported overall gain values on channel ch */
  osmosdr::gain_range_t get_gain_range(bladerf_channel ch);
  /* Get range of supported gain values for gain stage 'name' on channel ch */
  osmosdr::gain_range_t get_gain_range(std::string const &name,
                                       bladerf_channel ch);

  /* Enable or disable the automatic gain control on channel ch */
  bool set_gain_mode(bool automatic, bladerf_channel ch,
                     bladerf_gain_mode agc_mode = BLADERF_GAIN_DEFAULT);
  /* Get the current automatic gain control status on channel ch */
  bool get_gain_mode(bladerf_channel ch);

  /* Set the overall gain value on channel ch */
  double set_gain(double gain, bladerf_channel ch);
  /* Set the gain of stage 'name' on channel ch */
  double set_gain(double gain, std::string const &name, bladerf_channel ch);
  /* Get the overall gain value on channel ch */
  double get_gain(bladerf_channel ch);
  /* Get the gain of stage 'name' on channel ch */
  double get_gain(std::string const &name, bladerf_channel ch);

  /* Get the list of antennas supported by a channel */
  std::vector<std::string> get_antennas(bladerf_direction dir);
  bool set_antenna(bladerf_direction dir, size_t chan, const std::string &antenna);

  /* Set the DC offset on channel ch */
  int set_dc_offset(std::complex<double> const &offset, bladerf_channel ch);
  /* Set the IQ balance on channel ch */
  int set_iq_balance(std::complex<double> const &balance, bladerf_channel ch);

  /* Get the list of supported clock sources */
  std::vector<std::string> get_clock_sources(size_t mboard = 0);
  /* Set the clock source to */
  void set_clock_source(std::string const &source, size_t mboard = 0);
  /* Get the name of the current clock source */
  std::string get_clock_source(size_t mboard = 0);

  /* Set the SMB frequency */
  void set_smb_frequency(double frequency);
  /* Get the current SMB frequency */
  double get_smb_frequency();

  /*****************************************************************************
   * Protected members
   ****************************************************************************/
  bladerf_sptr _dev;            /**< shared pointer for the active device */
  std::string _pfx;             /**< prefix for console messages */
  unsigned int _failures;       /**< counter for consecutive rx/tx failures */

  size_t _num_buffers;          /**< number of buffers to allocate */
  size_t _samples_per_buffer;   /**< how many samples per buffer */
  size_t _num_transfers;        /**< number of active backend transfers */
  unsigned int _stream_timeout; /**< timeout for backend transfers */

  bladerf_format _format;       /**< sample format to use */

  bladerf_channel_map _chanmap; /**< map of antennas to channels */
  bladerf_channel_enable_map _enables;  /**< enabled channels */

  /*****************************************************************************
   * Protected constants
   ****************************************************************************/
  /* Maximum bladerf_sync_{rx,tx} failures to allow before giving up */
  static const unsigned int MAX_CONSECUTIVE_FAILURES = 3;

  /* BladeRF IQ correction parameters */
  static const int16_t DCOFF_SCALE = 2048;
  static const int16_t GAIN_SCALE = 4096;
  static const int16_t PHASE_SCALE = 4096;

private:
  /*****************************************************************************
   * Private methods
   ****************************************************************************/
  /* Open the bladeRF described by device_name. Returns a sptr if successful */
  bladerf_sptr open(const std::string &device_name);
  /* Called by shared_ptr when a bladerf_sptr hits a refcount of 0 */
  static void close(void *dev);
  /* If a device described by devinfo is open, this returns a sptr to it */
  static bladerf_sptr get_cached_device(struct bladerf_devinfo devinfo);
  /* Prints a summary of device information */
  void print_device_info();

  bool is_antenna_valid(bladerf_direction dir, const std::string &antenna);

  /*****************************************************************************
   * Private members
   ****************************************************************************/
  static std::mutex _devs_mutex;  /**< mutex for access to _devs */
  static std::list<std::weak_ptr<struct bladerf> > _devs;  /**< dev cache */
};

#endif
