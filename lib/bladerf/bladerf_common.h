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

#include <gr_complex.h>
#include <gruel/thread.h>

#include <libbladeRF.h>

#include "osmosdr/osmosdr_ranges.h"

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

typedef boost::shared_ptr<struct bladerf> bladerf_sptr;

class bladerf_common
{
public:
  bladerf_common();
  virtual ~bladerf_common();

protected:
  bladerf_sptr open(const std::string &device_name);

  osmosdr::freq_range_t freq_range();
  osmosdr::meta_range_t sample_rates();
  osmosdr::freq_range_t filter_bandwidths();

  static std::vector< std::string > devices();

  bool is_running();
  void set_running(bool is_running);

  bladerf_sptr _dev;

  void **_buffers;
  struct bladerf_stream *_stream;
  size_t _num_buffers;
  size_t _buf_index;

  gruel::thread _thread;

  boost::circular_buffer<gr_complex> *_fifo;
  boost::mutex _fifo_lock;
  boost::condition_variable _samp_avail;

  osmosdr::gain_range_t _vga1_range;
  osmosdr::gain_range_t _vga2_range;
private:
  bool _is_running;
  boost::shared_mutex _state_lock;

  static boost::mutex _devs_mutex;
  static std::list<boost::weak_ptr<struct bladerf> > _devs;

  static bladerf_sptr get_cached_device(struct bladerf_devinfo devinfo);
  static void close(void *dev); /* called by shared_ptr */
};

#endif
