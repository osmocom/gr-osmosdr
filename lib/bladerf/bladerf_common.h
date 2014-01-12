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
#include <boost/assign.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <gr_complex.h>
#include <gruel/thread.h>

#include <libbladeRF.h>

#include "osmosdr/osmosdr_ranges.h"
#include "osmosdr_arg_helpers.h"

/* We currently read/write 1024 samples (pairs of 16-bit signed ints) */
#define BLADERF_SAMPLE_BLOCK_SIZE     (1024)

/*
 * BladeRF IQ correction defines
 */
#define BLADERF_RX_DC_RANGE     63
#define BLADERF_TX_DC_RANGE     127
#define BLADERF_GAIN_ZERO       4096
#define BLADERF_GAIN_RANGE      4096
#define BLADERF_PHASE_RANGE     2048

typedef boost::shared_ptr<struct bladerf> bladerf_sptr;

class bladerf_common
{
public:
  bladerf_common();
  virtual ~bladerf_common();

protected:
  /* Handle initialized and parameters common to both source & sink */
  void init(dict_t &dict, const char *type);

  double set_sample_rate(bladerf_module module, double rate);
  double get_sample_rate(bladerf_module module);

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
  size_t _samples_per_buffer;
  size_t _num_transfers;

  gruel::thread _thread;

  osmosdr::gain_range_t _vga1_range;
  osmosdr::gain_range_t _vga2_range;

  std::string _pfx;

private:
  bladerf_sptr open(const std::string &device_name);

  bool _is_running;
  boost::shared_mutex _state_lock;

  static boost::mutex _devs_mutex;
  static std::list<boost::weak_ptr<struct bladerf> > _devs;

  static bladerf_sptr get_cached_device(struct bladerf_devinfo devinfo);
  static void close(void *dev); /* called by shared_ptr */
};

#endif
