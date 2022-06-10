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

#ifndef OSMOSDR_SINK_IFACE_H
#define OSMOSDR_SINK_IFACE_H

#include <osmosdr/ranges.h>
#include <osmosdr/time_spec.h>
#include <gnuradio/basic_block.h>

/*!
 * TODO: document
 *
 */
class sink_iface
{
public:
  virtual ~sink_iface() = default;

  /*!
   * Get the number of channels the underlying radio hardware offers.
   * \return the number of available channels
   */
  virtual size_t get_num_channels( void ) = 0;

  /*!
   * Get the possible sample rates for the underlying radio hardware.
   * \return a range of rates in Sps
   */
  virtual osmosdr::meta_range_t get_sample_rates( void ) = 0;

  /*!
   * Set the sample rate for the underlying radio hardware.
   * This also will select the appropriate IF bandpass, if applicable.
   * \param rate a new rate in Sps
   */
  virtual double set_sample_rate( double rate ) = 0;

  /*!
   * Get the sample rate for the underlying radio hardware.
   * This is the actual sample rate and may differ from the rate set.
   * \return the actual rate in Sps
   */
  virtual double get_sample_rate( void ) = 0;

  /*!
   * Get the tunable frequency range for the underlying radio hardware.
   * \param chan the channel index 0 to N-1
   * \return the frequency range in Hz
   */
  virtual osmosdr::freq_range_t get_freq_range( size_t chan = 0 ) = 0;

  /*!
   * Tune the underlying radio hardware to the desired center frequency.
   * This also will select the appropriate RF bandpass.
   * \param freq the desired frequency in Hz
   * \param chan the channel index 0 to N-1
   * \return the actual frequency in Hz
   */
  virtual double set_center_freq( double freq, size_t chan = 0 ) = 0;

  /*!
   * Get the center frequency the underlying radio hardware is tuned to.
   * This is the actual frequency and may differ from the frequency set.
   * \param chan the channel index 0 to N-1
   * \return the frequency in Hz
   */
  virtual double get_center_freq( size_t chan = 0 ) = 0;

  /*!
   * Set the frequency correction value in parts per million.
   * \param ppm the desired correction value in parts per million
   * \param chan the channel index 0 to N-1
   * \return correction value in parts per million
   */
  virtual double set_freq_corr( double ppm, size_t chan = 0 ) = 0;

  /*!
   * Get the frequency correction value.
   * \param chan the channel index 0 to N-1
   * \return correction value in parts per million
   */
  virtual double get_freq_corr( size_t chan = 0 ) = 0;

  /*!
   * Get the gain stage names of the underlying radio hardware.
   * \param chan the channel index 0 to N-1
   * \return a vector of strings containing the names of gain stages
   */
  virtual std::vector<std::string> get_gain_names( size_t chan = 0 ) = 0;

  /*!
   * Get the settable overall gain range for the underlying radio hardware.
   * \param chan the channel index 0 to N-1
   * \return the gain range in dB
   */
  virtual osmosdr::gain_range_t get_gain_range( size_t chan = 0 ) = 0;

  /*!
   * Get the settable gain range for a specific gain stage.
   * \param name the name of the gain stage
   * \param chan the channel index 0 to N-1
   * \return the gain range in dB
   */
  virtual osmosdr::gain_range_t get_gain_range( const std::string & name,
                                                size_t chan = 0 ) = 0;

  /*!
   * Set the gain mode for the underlying radio hardware.
   * This might be supported only for certain hardware types.
   * \param automatic the gain mode (true means automatic gain mode)
   * \param chan the channel index 0 to N-1
   * \return the actual gain mode
   */
  virtual bool set_gain_mode( bool automatic, size_t chan = 0 ) { return false; }

  /*!
   * Get the gain mode selected for the underlying radio hardware.
   * \param chan the channel index 0 to N-1
   * \return the actual gain mode (true means automatic gain mode)
   */
  virtual bool get_gain_mode( size_t chan = 0 ) { return false; }

  /*!
   * Set the gain for the underlying radio hardware.
   * This function will automatically distribute the desired gain value over
   * available gain stages in an appropriate way and return the actual value.
   * \param gain the gain in dB
   * \param chan the channel index 0 to N-1
   * \return the actual gain in dB
   */
  virtual double set_gain( double gain, size_t chan = 0 ) = 0;

  /*!
   * Set the named gain on the underlying radio hardware.
   * \param gain the gain in dB
   * \param name the name of the gain stage
   * \param chan the channel index 0 to N-1
   * \return the actual gain in dB
   */
  virtual double set_gain( double gain,
                           const std::string & name,
                           size_t chan = 0 ) = 0;

  /*!
   * Get the actual gain setting of the underlying radio hardware.
   * \param chan the channel index 0 to N-1
   * \return the actual gain in dB
   */
  virtual double get_gain( size_t chan = 0 ) = 0;

  /*!
   * Get the actual gain setting of a named stage.
   * \param name the name of the gain stage
   * \param chan the channel index 0 to N-1
   * \return the actual gain in dB
   */
  virtual double get_gain( const std::string & name, size_t chan = 0 ) = 0;

  /*!
   * Set the IF gain for the underlying radio hardware.
   * This function will automatically distribute the desired gain value over
   * available IF gain stages in an appropriate way and return the actual value.
   * \param gain the gain in dB
   * \param chan the channel index 0 to N-1
   * \return the actual gain in dB
   */
  virtual double set_if_gain( double gain, size_t chan = 0 ) { return 0; }

  /*!
   * Set the BB gain for the underlying radio hardware.
   * This function will automatically distribute the desired gain value over
   * available BB gain stages in an appropriate way and return the actual value.
   * \param gain the gain in dB
   * \param chan the channel index 0 to N-1
   * \return the actual gain in dB
   */
  virtual double set_bb_gain( double gain, size_t chan = 0 ) { return 0; }

  /*!
   * Get the available antennas of the underlying radio hardware.
   * \param chan the channel index 0 to N-1
   * \return a vector of strings containing the names of available antennas
   */
  virtual std::vector< std::string > get_antennas( size_t chan = 0 ) = 0;

  /*!
   * Select the active antenna of the underlying radio hardware.
   * \param antenna the antenna name
   * \param chan the channel index 0 to N-1
   * \return the actual antenna's name
   */
  virtual std::string set_antenna( const std::string & antenna,
                                   size_t chan = 0 ) = 0;

  /*!
   * Get the actual underlying radio hardware antenna setting.
   * \param chan the channel index 0 to N-1
   * \return the actual antenna's name
   */
  virtual std::string get_antenna( size_t chan = 0 ) = 0;

  /*!
   * Set the TX frontend DC offset value.
   * The value is complex to control both I and Q.
   *
   * \param offset the dc offset (1.0 is full-scale)
   * \param chan the channel index 0 to N-1
   */
  virtual void set_dc_offset( const std::complex<double> &offset, size_t chan = 0 ) { }

  /*!
   * Set the TX frontend IQ balance correction.
   * Use this to adjust the magnitude and phase of I and Q.
   *
   * \param balance the complex correction value
   * \param chan the channel index 0 to N-1
   */
  virtual void set_iq_balance( const std::complex<double> &balance, size_t chan = 0 ) { }

  /*!
   * Set the bandpass filter on the radio frontend.
   * \param bandwidth the filter bandwidth in Hz, set to 0 for automatic selection
   * \param chan the channel index 0 to N-1
   * \return the actual filter bandwidth in Hz
   */
  virtual double set_bandwidth( double bandwidth, size_t chan = 0 ) { return 0; }

  /*!
   * Get the actual bandpass filter setting on the radio frontend.
   * \param chan the channel index 0 to N-1
   * \return the actual filter bandwidth in Hz
   */
  virtual double get_bandwidth( size_t chan = 0 ) { return 0; }

  /*!
   * Get the possible bandpass filter settings on the radio frontend.
   * \param chan the channel index 0 to N-1
   * \return a range of bandwidths in Hz
   */
  virtual osmosdr::freq_range_t get_bandwidth_range( size_t chan = 0 )
    { return osmosdr::freq_range_t(); }

  /*!
   * Set the time source for the device.
   * This sets the method of time synchronization,
   * typically a pulse per second or an encoded time.
   * Typical options for source: external, MIMO.
   * \param source a string representing the time source
   * \param mboard which motherboard to set the config
   */
  virtual void set_time_source(const std::string &source,
                               const size_t mboard = 0) { }

  /*!
   * Get the currently set time source.
   * \param mboard which motherboard to get the config
   * \return the string representing the time source
   */
  virtual std::string get_time_source(const size_t mboard) { return ""; }

  /*!
   * Get a list of possible time sources.
   * \param mboard which motherboard to get the list
   * \return a vector of strings for possible settings
   */
  virtual std::vector<std::string> get_time_sources(const size_t mboard)
  {
    return std::vector<std::string>();
  }

  /*!
   * Set the clock source for the device.
   * This sets the source for a 10 Mhz reference clock.
   * Typical options for source: internal, external, MIMO.
   * \param source a string representing the clock source
   * \param mboard which motherboard to set the config
   */
  virtual void set_clock_source(const std::string &source,
                                const size_t mboard = 0) { }

  /*!
   * Get the currently set clock source.
   * \param mboard which motherboard to get the config
   * \return the string representing the clock source
   */
  virtual std::string get_clock_source(const size_t mboard) { return ""; }

  /*!
   * Get a list of possible clock sources.
   * \param mboard which motherboard to get the list
   * \return a vector of strings for possible settings
   */
  virtual std::vector<std::string> get_clock_sources(const size_t mboard)
  {
    return std::vector<std::string>();
  }

  /*!
   * Get the master clock rate.
   * \param mboard the motherboard index 0 to M-1
   * \return the clock rate in Hz
   */
  virtual double get_clock_rate(size_t mboard = 0) { return 0; }

  /*!
   * Set the master clock rate.
   * \param rate the new rate in Hz
   * \param mboard the motherboard index 0 to M-1
   */
  virtual void set_clock_rate(double rate, size_t mboard = 0) { }

  /*!
   * Get the current time registers.
   * \param mboard the motherboard index 0 to M-1
   * \return the current device time
   */
  virtual ::osmosdr::time_spec_t get_time_now(size_t mboard = 0)
  {
    return ::osmosdr::time_spec_t::get_system_time();
  }

  /*!
   * Get the time when the last pps pulse occured.
   * \param mboard the motherboard index 0 to M-1
   * \return the current device time
   */
  virtual ::osmosdr::time_spec_t get_time_last_pps(size_t mboard = 0)
  {
    return ::osmosdr::time_spec_t::get_system_time();
  }

  /*!
   * Sets the time registers immediately.
   * \param time_spec the new time
   * \param mboard the motherboard index 0 to M-1
   */
  virtual void set_time_now(const ::osmosdr::time_spec_t &time_spec,
                            size_t mboard = 0) { }

  /*!
   * Set the time registers at the next pps.
   * \param time_spec the new time
   */
  virtual void set_time_next_pps(const ::osmosdr::time_spec_t &time_spec) { }

  /*!
   * Sync the time registers with an unknown pps edge.
   * \param time_spec the new time
   */
  virtual void set_time_unknown_pps(const ::osmosdr::time_spec_t &time_spec) { }
};

#endif // OSMOSDR_SINK_IFACE_H
