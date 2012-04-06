/* -*- c++ -*- */
/*
 * Copyright 2004 Free Software Foundation, Inc.
 * Copyright 2012 Dimitri Stolnikov <horiz0n@gmx.net>
 *
 * This file is part of GNU Radio
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
#ifndef INCLUDED_OSMOSDR_SOURCE_C_H
#define INCLUDED_OSMOSDR_SOURCE_C_H

#include <osmosdr_api.h>
#include <osmosdr_ranges.h>
#include <gr_hier_block2.h>

class osmosdr_source_c;

/*
 * We use boost::shared_ptr's instead of raw pointers for all access
 * to gr_blocks (and many other data structures).  The shared_ptr gets
 * us transparent reference counting, which greatly simplifies storage
 * management issues.  This is especially helpful in our hybrid
 * C++ / Python system.
 *
 * See http://www.boost.org/libs/smart_ptr/smart_ptr.htm
 *
 * As a convention, the _sptr suffix indicates a boost::shared_ptr
 */
typedef boost::shared_ptr<osmosdr_source_c> osmosdr_source_c_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of osmosdr_source_c.
 *
 * To avoid accidental use of raw pointers, osmosdr_source_c's
 * constructor is private.  osmosdr_make_source_c is the public
 * interface for creating new instances.
 */
OSMOSDR_API osmosdr_source_c_sptr osmosdr_make_source_c (const std::string & args = "");

/*!
 * \brief Provides a stream of complex samples.
 * \ingroup block
 *
 * \sa osmosdr_sink_c for a version that subclasses gr_hier_block2.
 */
class OSMOSDR_API osmosdr_source_c : virtual public gr_hier_block2
{
public:
  /*!
   * Get the possible sample rates for the OsmoSDR device.
   * \return a range of rates in Sps
   */
  virtual osmosdr::meta_range_t get_samp_rates( void ) = 0;

  /*!
   * Set the sample rate for the OsmoSDR device.
   * This also will select the appropriate IF bandpass, if applicable.
   * \param rate a new rate in Sps
   */
  virtual double set_samp_rate( double rate ) = 0;

  /*!
   * Get the sample rate for the OsmoSDR device.
   * This is the actual sample rate and may differ from the rate set.
   * \return the actual rate in Sps
   */
  virtual double get_samp_rate( void ) = 0;

  /*!
   * Get the tunable frequency range.
   * \return the frequency range in Hz
   */
  virtual osmosdr::freq_range_t get_freq_range( size_t chan = 0 ) = 0;

  /*!
   * Tune the OsmoSDR device to the desired center frequency.
   * This also will select the appropriate RF bandpass.
   * \param freq the desired frequency in Hz
   * \return a tune result with the actual frequencies
   */
  virtual double set_center_freq( double freq, size_t chan = 0 ) = 0;

  /*!
   * Get the center frequency.
   * \return the frequency in Hz
   */
  virtual double get_center_freq( size_t chan = 0 ) = 0;

  /*!
   * Set the frequency correction value in parts per million.
   * \return correction value in parts per million
   */
  virtual double set_freq_correction( double ppm, size_t chan = 0 ) = 0;

  /*!
   * Get the frequency correction value.
   * \return correction value in parts per million
   */
  virtual double get_freq_correction( size_t chan = 0 ) = 0;

  /*!
   * Get the actual OsmoSDR gain setting of named stage.
   * \return the actual gain in dB
   */
  virtual std::vector<std::string> get_gain_names( size_t chan = 0 ) = 0;

  /*!
   * Get the settable gain range.
   * \param name the name of the gain stage
   * \return the gain range in dB
   */
  virtual osmosdr::gain_range_t get_gain_range( const std::string & name = "", size_t chan = 0 ) = 0;

  /*!
   * Set the gain for the OsmoSDR.
   * \param gain the gain in dB
   */
  virtual void set_gain( double gain, size_t chan = 0 ) = 0;

  /*!
   * Set the named gain on the OsmoSDR.
   * \param gain the gain in dB
   * \param name the name of the gain stage
   */
  virtual void set_gain( double gain, const std::string & name = "", size_t chan = 0 ) = 0;

  /*!
   * Get the actual OsmoSDR gain setting of a named stage.
   * \param name the name of the gain stage
   * \return the actual gain in dB
   */
  virtual double get_gain( const std::string & name = "", size_t chan = 0 ) = 0;

#if 0

  virtual std::vector< std::string > get_antennas( size_t chan = 0 ) = 0;
  virtual std::string set_antenna( const std::string & antenna, size_t chan = 0 ) = 0;
  virtual std::string get_antenna( size_t chan = 0 ) = 0;

  /*!
   * Enable/disable the automatic DC offset correction.
   * The automatic correction subtracts out the long-run average.
   *
   * When disabled, the averaging option operation is halted.
   * Once halted, the average value will be held constant
   * until the user re-enables the automatic correction
   * or overrides the value by manually setting the offset.
   *
   * \param enb true to enable automatic DC offset correction
   */
  virtual void set_dc_offset(const bool enb) = 0;

  /*!
   * Set a constant DC offset value.
   * The value is complex to control both I and Q.
   * Only set this when automatic correction is disabled.
   * \param offset the dc offset (1.0 is full-scale)
   */
  virtual void set_dc_offset(const std::complex<double> &offset) = 0;

  /*!
   * Set the RX frontend IQ imbalance correction.
   * Use this to adjust the magnitude and phase of I and Q.
   *
   * \param correction the complex correction value
   */
  virtual void set_iq_balance(const std::complex<double> &correction) = 0;

  /*!
   * Get the master clock rate.
   * \return the clock rate in Hz
   */
  virtual double get_clock_rate() = 0;

  /*!
   * Set the master clock rate.
   * \param rate the new rate in Hz
   */
  virtual void set_clock_rate(double rate) = 0;

  /*!
   * Get automatic gain control status
   * \return the clock rate in Hz
   */
  virtual bool get_agc() = 0;

  /*!
   * Enable/disable the automatic gain control.
   * \param enb true to enable automatic gain control
   * \param attack attack time of the AGC circuit
   * \param decay decay time of the AGC circuit
   */
  virtual void set_agc(bool enb, double attack, double decay) = 0;

  /*! TODO: add more */
#endif
};

#endif /* INCLUDED_OSMOSDR_SOURCE_C_H */
