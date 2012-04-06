/* -*- c++ -*- */
/*
 * Copyright 2012 Free Software Foundation, Inc.
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
#ifndef INCLUDED_RTLSDR_SOURCE_C_H
#define INCLUDED_RTLSDR_SOURCE_C_H

#include <osmosdr_api.h>
#include <gr_sync_block.h>
#include <gruel/thread.h>
#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

class rtl_source_c;
typedef struct rtlsdr_dev rtlsdr_dev_t;

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
typedef boost::shared_ptr<rtl_source_c> rtl_source_c_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of rtl_source_c.
 *
 * To avoid accidental use of raw pointers, rtl_source_c's
 * constructor is private. make_rtl_source_c is the public
 * interface for creating new instances.
 */
OSMOSDR_API rtl_source_c_sptr make_rtl_source_c (const std::string & args = "");

/*!
 * \brief Provides a stream of complex samples.
 * \ingroup block
 *
 */
class OSMOSDR_API rtl_source_c : public gr_sync_block
{
private:
  // The friend declaration allows make_rtl_source_c to
  // access the private constructor.

  friend OSMOSDR_API rtl_source_c_sptr make_rtl_source_c (const std::string & args);

  /*!
   * \brief Provides a stream of complex samples.
   */
  rtl_source_c (const std::string & args);  	// private constructor

public:
  ~rtl_source_c ();	// public destructor

  int work( int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items );

  double set_center_freq( double freq );
  double set_sample_rate( double rate );
  double get_sample_rate();
  double set_gain( double gain );

private:
  static void _rtlsdr_callback(unsigned char *buf, uint32_t len, void *ctx);
  void rtlsdr_callback(unsigned char *buf, uint32_t len);
  static void _rtlsdr_wait(rtl_source_c *obj);
  void rtlsdr_wait();

  std::vector<gr_complex> _lut;

  rtlsdr_dev_t *_dev;
  gruel::thread _thread;
  boost::circular_buffer<unsigned char> _buf;
  boost::mutex _buf_mutex;
  boost::condition_variable _buf_cond;
  bool _has_i_sample;
};

#endif /* INCLUDED_RTLSDR_SOURCE_C_H */
