/* -*- c++ -*- */
/*
 * Copyright 2013 Nuand LLC
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

#include <vector>
#include <string>

#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <gnuradio/gr_complex.h>

#include <libbladeRF.h>

#include "osmosdr/ranges.h"

/* We currently read/write 1024 samples (pairs of 16-bit signed ints) */
#define BLADERF_SAMPLE_BLOCK_SIZE     (1024)

/*
 * Default size of sample FIFO, in entries.
 * This can be overridden by the environment variable BLADERF_SAMPLE_FIFO_SIZE.
 */
#ifndef BLADERF_SAMPLE_FIFO_SIZE
#   define BLADERF_SAMPLE_FIFO_SIZE   (2 * 1024 * 1024)
#endif

#define BLADERF_SAMPLE_FIFO_MIN_SIZE  (3 * BLADERF_SAMPLE_BLOCK_SIZE)

class bladerf_common
{
public:
  bladerf_common();
  ~bladerf_common();

protected:
  void setup_device();

  osmosdr::freq_range_t freq_range();
  osmosdr::meta_range_t sample_rates();
  osmosdr::freq_range_t filter_bandwidths();

  static std::vector< std::string > devices();

  bool is_running();
  void set_running(bool is_running);

  bladerf *dev;

  int16_t *raw_sample_buf;
  boost::circular_buffer<gr_complex> *sample_fifo;
  boost::mutex sample_fifo_lock;
  boost::condition_variable samples_available;
private:
  bool running;
  boost::shared_mutex state_lock;
};

#endif
