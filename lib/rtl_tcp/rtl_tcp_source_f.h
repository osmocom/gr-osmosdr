/* -*- c++ -*- */
/*
 * Copyright 2012 Hoernchen <la@tfc-server.de>
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

#ifndef RTL_TCP_SOURCE_F_H
#define RTL_TCP_SOURCE_F_H

#include <osmosdr_api.h>

#include <gr_sync_block.h>
#include <gruel/thread.h>

#if defined(_WIN32)
// if not posix, assume winsock
#pragma comment(lib, "ws2_32.lib")
#define USING_WINSOCK
#include <winsock2.h>
#include <ws2tcpip.h>
#define SHUT_RDWR 2
typedef char* optval_t;
#else
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
typedef void* optval_t;
#endif

#define ssize_t int

class rtl_tcp_source_f;
typedef boost::shared_ptr<rtl_tcp_source_f> rtl_tcp_source_f_sptr;

OSMOSDR_API rtl_tcp_source_f_sptr make_rtl_tcp_source_f (
      size_t itemsize,
      const char *host,
      unsigned short port,
      int payload_size,
      bool eof,
      bool wait);

class OSMOSDR_API rtl_tcp_source_f : public gr_sync_block
{
private:
  size_t        d_itemsize;
  int           d_payload_size;  // maximum transmission unit (packet length)
  bool          d_eof;           // zero-length packet is EOF
  bool          d_wait;          // wait if data if not immediately available
  int           d_socket;        // handle to socket
  unsigned char *d_temp_buff;    // hold buffer between calls
  size_t        d_temp_offset;   // point to temp buffer location offset
  float         *d_LUT;

 private:
  rtl_tcp_source_f(size_t itemsize, const char *host,
   unsigned short port, int payload_size, bool eof, bool wait);

  // The friend declaration allows make_source_c to
  // access the private constructor.
  friend OSMOSDR_API rtl_tcp_source_f_sptr make_rtl_tcp_source_f (
      size_t itemsize,
      const char *host,
      unsigned short port,
      int payload_size,
      bool eof,
      bool wait);

 public:
  ~rtl_tcp_source_f();

  int work(int noutput_items,
	   gr_vector_const_void_star &input_items,
	   gr_vector_void_star &output_items);

  void set_freq(int freq);
  void set_sample_rate(int sample_rate);
  void set_gain_mode(int manual);
  void set_gain(int gain);
  void set_freq_corr(int ppm);
};


#endif /* RTL_TCP_SOURCE_F_H */
