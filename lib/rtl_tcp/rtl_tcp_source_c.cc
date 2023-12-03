/* -*- mode: c++; c-basic-offset: 2 -*- */
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

#include <fstream>
#include <string>
#include <sstream>

#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>

#include <gnuradio/io_signature.h>

#include "rtl_tcp_source_c.h"
#include "arg_helpers.h"

#if defined(_WIN32)
// if not posix, assume winsock
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

#ifdef _MSC_VER
#include <cstddef>
typedef ptrdiff_t ssize_t;
#endif //_MSC_VER

#ifndef _WIN32
#include <netinet/in.h>
#else
#include <WinSock2.h>
#endif

#define BYTES_PER_SAMPLE  2 // rtl_tcp device delivers 8 bit unsigned IQ data

/* copied from rtl sdr code */
typedef struct { /* structure size must be multiple of 2 bytes */
  char magic[4];
  uint32_t tuner_type;
  uint32_t tuner_gain_count;
} dongle_info_t;

#ifdef _WIN32
#define __attribute__(x)
#pragma pack(push, 1)
#endif
struct command {
  unsigned char cmd;
  unsigned int param;
} __attribute__((packed));
#ifdef _WIN32
#pragma pack(pop)
#endif

#define USE_SELECT    1  // non-blocking receive on all platforms
#define USE_RCV_TIMEO 0  // non-blocking receive on all but Cygwin
#define SRC_VERBOSE 0
#define SNK_VERBOSE 0

static int is_error( int perr )
{
  // Compare error to posix error code; return nonzero if match.
#if defined(USING_WINSOCK)
#ifndef ENOPROTOOPT
#define ENOPROTOOPT 109
#endif
  // All codes to be checked for must be defined below
  int werr = WSAGetLastError();
  switch( werr ) {
  case WSAETIMEDOUT:
    return( perr == EAGAIN );
  case WSAENOPROTOOPT:
    return( perr == ENOPROTOOPT );
  default:
    fprintf(stderr,"rtl_tcp_source_c: unknown error %d WS err %d \n", perr, werr );
    throw std::runtime_error("internal error");
  }
  return 0;
#else
  return( perr == errno );
#endif
}

static void report_error( const char *msg1, const char *msg2 )
{
  // Deal with errors, both posix and winsock
#if defined(USING_WINSOCK)
  int werr = WSAGetLastError();
  fprintf(stderr, "%s: winsock error %d\n", msg1, werr );
#else
  perror(msg1);
#endif
  if( msg2 != NULL )
    throw std::runtime_error(msg2);
  return;
}

using namespace boost::assign;

const char * rtl_tcp_source_c::get_tuner_name(void)
{
  switch (d_tuner_type) {
  case RTLSDR_TUNER_E4000: return "E4000";
  case RTLSDR_TUNER_FC0012: return "FC0012";
  case RTLSDR_TUNER_FC0013: return "FC0013";
  case RTLSDR_TUNER_FC2580: return "FC2580";
  case RTLSDR_TUNER_R820T: return "R820T";
  case RTLSDR_TUNER_R828D: return "R828D";
  default: return "Unknown";
  }
}

rtl_tcp_source_c_sptr make_rtl_tcp_source_c(const std::string &args)
{
  return gnuradio::get_initial_sptr(new rtl_tcp_source_c(args));
}

rtl_tcp_source_c::rtl_tcp_source_c(const std::string &args) :
  gr::sync_block("rtl_tcp_source_c",
                 gr::io_signature::make(0, 0, 0),
                 gr::io_signature::make(1, 1, sizeof (gr_complex))),
  d_socket(-1),
  _no_tuner(false),
  _auto_gain(false),
  _if_gain(0)
{
  std::string host = "127.0.0.1";
  unsigned short port = 1234;
  int payload_size = 16384;
  unsigned int direct_samp = 0, offset_tune = 0;
  int bias_tee = 0;

  _freq = 0;
  _rate = 0;
  _gain = 0;
  _corr = 0;

  dict_t dict = params_to_dict(args);

  if (dict.count("rtl_tcp")) {
    std::vector< std::string > tokens;
    boost::algorithm::split( tokens, dict["rtl_tcp"], boost::is_any_of(":") );

    if ( tokens[0].length() && (tokens.size() == 1 || tokens.size() == 2 ) )
      host = tokens[0];

    if ( tokens.size() == 2 ) // port given
      port = boost::lexical_cast< unsigned short >( tokens[1] );
  }

  if (dict.count("psize"))
    payload_size = boost::lexical_cast< int >( dict["psize"] );

  if (dict.count("direct_samp"))
    direct_samp = boost::lexical_cast< unsigned int >( dict["direct_samp"] );

  if (dict.count("offset_tune"))
    offset_tune = boost::lexical_cast< unsigned int >( dict["offset_tune"] );

  if (dict.count("bias"))
    bias_tee = boost::lexical_cast<bool>( dict["bias"] );

  if (!host.length())
    host = "127.0.0.1";

  if (0 == port)
    port = 1234;

  if (payload_size <= 0)
    payload_size = 16384;

#if defined(USING_WINSOCK) // for Windows (with MinGW)
  // initialize winsock DLL
  WSADATA wsaData;
  int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
  if( iResult != NO_ERROR ) {
    report_error( "rtl_tcp_source_c WSAStartup", "can't open socket" );
  }
#endif

  // Set up the address stucture for the source address and port numbers
  // Get the source IP address from the host name
  struct addrinfo *ip_src;      // store the source IP address to use
  struct addrinfo hints;
  memset( (void*)&hints, 0, sizeof(hints) );
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;
  char port_str[12];
  sprintf( port_str, "%d", port );

  // FIXME leaks if report_error throws below
  int ret = getaddrinfo(host.c_str(), port_str, &hints, &ip_src);
  if (ret != 0)
    report_error("rtl_tcp_source_c/getaddrinfo",
                 "can't initialize source socket" );

  d_temp_buff = new unsigned char[payload_size];   // allow it to hold up to payload_size bytes
  d_LUT = new float[0x100];
  for (int i = 0; i < 0x100; ++i)
    d_LUT[i] = (((float)(i & 0xff)) - 127.4f) * (1.0f / 128.0f);

  // create socket
  d_socket = socket(ip_src->ai_family, ip_src->ai_socktype,
                    ip_src->ai_protocol);
  if (d_socket == -1)
    report_error("socket open","can't open socket");

  // Turn on reuse address
  int opt_val = 1;
  if (setsockopt(d_socket, SOL_SOCKET, SO_REUSEADDR, (optval_t)&opt_val, sizeof(int)) == -1)
    report_error("SO_REUSEADDR","can't set socket option SO_REUSEADDR");

  // Don't wait when shutting down
  linger lngr;
  lngr.l_onoff  = 1;
  lngr.l_linger = 0;
  if (setsockopt(d_socket, SOL_SOCKET, SO_LINGER, (optval_t)&lngr, sizeof(linger)) == -1)
    if (!is_error(ENOPROTOOPT)) // no SO_LINGER for SOCK_DGRAM on Windows
      report_error("SO_LINGER","can't set socket option SO_LINGER");

#if USE_RCV_TIMEO
  // Set a timeout on the receive function to not block indefinitely
  // This value can (and probably should) be changed
  // Ignored on Cygwin
#if defined(USING_WINSOCK)
  DWORD timeout = 1000;  // milliseconds
#else
  timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
#endif
  if (setsockopt(d_socket, SOL_SOCKET, SO_RCVTIMEO, (optval_t)&timeout, sizeof(timeout)) == -1)
    report_error("SO_RCVTIMEO","can't set socket option SO_RCVTIMEO");
#endif // USE_RCV_TIMEO

  if (::connect(d_socket, ip_src->ai_addr, ip_src->ai_addrlen) != 0)
    report_error("rtl_tcp_source_c/connect","can't open TCP connection");
  freeaddrinfo(ip_src);

  int flag = 1;
  setsockopt(d_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag,sizeof(flag));

  dongle_info_t dongle_info;
  ret = recv(d_socket, (char*)&dongle_info, sizeof(dongle_info), 0);
  if (sizeof(dongle_info) != ret)
    fprintf(stderr,"failed to read dongle info\n");

  d_tuner_type = RTLSDR_TUNER_UNKNOWN;
  d_tuner_gain_count = 0;
  d_tuner_if_gain_count = 0;

  if (memcmp(dongle_info.magic, "RTL0", 4) == 0) {
    d_tuner_type = rtlsdr_tuner(ntohl(dongle_info.tuner_type));
    d_tuner_gain_count = ntohl(dongle_info.tuner_gain_count);
    if (RTLSDR_TUNER_E4000 == d_tuner_type)
      d_tuner_if_gain_count = 53;
  }

  if (d_tuner_type != RTLSDR_TUNER_UNKNOWN) {
    std::cerr << "The RTL TCP server reports a "
              << get_tuner_name()
              << " tuner with "
              << d_tuner_gain_count << " RF and "
              << d_tuner_if_gain_count << " IF gains."
              << std::endl;
  }

  set_gain_mode(false); /* enable manual gain mode by default */

  // set direct sampling
  struct command cmd;

  cmd = { 0x09, htonl(direct_samp) };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);
  if (direct_samp)
    _no_tuner = true;

  // set offset tuning
  cmd = { 0x0a, htonl(offset_tune) };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);

  // set bias tee
  cmd = { 0x0e, htonl(bias_tee) };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);
}

rtl_tcp_source_c::~rtl_tcp_source_c()
{
  delete [] d_LUT;
  delete [] d_temp_buff;

  if (d_socket != -1) {
    shutdown(d_socket, SHUT_RDWR);
#if defined(USING_WINSOCK)
    closesocket(d_socket);
#else
    ::close(d_socket);
#endif
    d_socket = -1;
  }

#if defined(USING_WINSOCK) // for Windows (with MinGW)
  // free winsock resources
  WSACleanup();
#endif
}


int rtl_tcp_source_c::work(int noutput_items,
			   gr_vector_const_void_star &input_items,
			   gr_vector_void_star &output_items)
{
  gr_complex *out = (gr_complex *)output_items[0];
  int bytesleft = noutput_items * BYTES_PER_SAMPLE;
  int index = 0;
  int receivedbytes = 0;
  while (bytesleft > 0) {
    receivedbytes = recv(d_socket, (char*)&d_temp_buff[index], bytesleft, 0);

    if (receivedbytes == -1 && !is_error(EAGAIN)) {
      fprintf(stderr, "socket error\n");
      return -1;
    }
    bytesleft -= receivedbytes;
    index += receivedbytes;
  }

  for (int i = 0; i < noutput_items; i++)
    out[i] = gr_complex(d_LUT[d_temp_buff[i * 2]], d_LUT[d_temp_buff[i * 2 + 1]]);

  return noutput_items;
}

std::string rtl_tcp_source_c::name()
{
  return "RTL TCP Client";
}

std::vector<std::string> rtl_tcp_source_c::get_devices( bool fake )
{
  std::vector<std::string> devices;

  if ( fake )
  {
    std::string args = "rtl_tcp=localhost:1234";
    args += ",label='RTL-SDR Spectrum Server'";
    devices.push_back( args );
  }

  return devices;
}

size_t rtl_tcp_source_c::get_num_channels( void )
{
  return 1;
}

osmosdr::meta_range_t rtl_tcp_source_c::get_sample_rates( void )
{
  osmosdr::meta_range_t range;

  range += osmosdr::range_t( 250000 ); // known to work
  range += osmosdr::range_t( 1000000 ); // known to work
  range += osmosdr::range_t( 1024000 ); // known to work
  range += osmosdr::range_t( 1800000 ); // known to work
  range += osmosdr::range_t( 1920000 ); // known to work
  range += osmosdr::range_t( 2000000 ); // known to work
  range += osmosdr::range_t( 2048000 ); // known to work
  range += osmosdr::range_t( 2400000 ); // known to work
  range += osmosdr::range_t( 2560000 ); // known to work
//  range += osmosdr::range_t( 2600000 ); // may work
//  range += osmosdr::range_t( 2800000 ); // may work
//  range += osmosdr::range_t( 3000000 ); // may work
//  range += osmosdr::range_t( 3200000 ); // max rate

  return range;
}

double rtl_tcp_source_c::set_sample_rate( double rate )
{
  struct command cmd = { 0x02, htonl(rate) };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);

  _rate = rate;

  return get_sample_rate();
}

double rtl_tcp_source_c::get_sample_rate( void )
{
  return _rate;
}

osmosdr::freq_range_t rtl_tcp_source_c::get_freq_range( size_t chan )
{
  osmosdr::freq_range_t range;

  if (_no_tuner) {
    range += osmosdr::range_t( 0, double(28.8e6) ); // as far as we know
    return range;
  }

  switch (d_tuner_type) {
  case RTLSDR_TUNER_FC0012:
    range += osmosdr::range_t( 22e6, 948e6 );
    break;
  case RTLSDR_TUNER_FC0013:
    range += osmosdr::range_t( 22e6, 1.1e9 );
    break;
  case RTLSDR_TUNER_FC2580:
    range += osmosdr::range_t( 146e6, 308e6 );
    range += osmosdr::range_t( 438e6, 924e6 );
    break;
  case RTLSDR_TUNER_R820T:
    range += osmosdr::range_t( 24e6, 1766e6 );
    break;
  case RTLSDR_TUNER_R828D:
    range += osmosdr::range_t( 24e6, 1766e6 );
    break;
  default:			// assume E4000 tuner
    /* there is a (temperature dependent) gap between 1100 to 1250 MHz */
    range += osmosdr::range_t( 52e6, 2.2e9 );
  }

  return range;
}

double rtl_tcp_source_c::set_center_freq( double freq, size_t chan )
{
  struct command cmd = { 0x01, htonl(freq) };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);

  _freq = freq;

  return get_center_freq(chan);
}

double rtl_tcp_source_c::get_center_freq( size_t chan )
{
  return _freq;
}

double rtl_tcp_source_c::set_freq_corr( double ppm, size_t chan )
{
  struct command cmd = { 0x05, htonl(ppm) };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);

  _corr = ppm;

  return get_freq_corr( chan );
}

double rtl_tcp_source_c::get_freq_corr( size_t chan )
{
  return _corr;
}

std::vector<std::string> rtl_tcp_source_c::get_gain_names( size_t chan )
{
  std::vector< std::string > names;

  names += "LNA";

  if (d_tuner_type == RTLSDR_TUNER_E4000)
    names += "IF";

  return names;
}

osmosdr::gain_range_t rtl_tcp_source_c::get_gain_range( size_t chan )
{
  osmosdr::gain_range_t range;

  /* the following gain values have been copied from librtlsdr */

  /* all gain values are expressed in tenths of a dB */
  const int e4k_gains[] = { -10, 15, 40, 65, 90, 115, 140, 165, 190, 215,
                            240, 290, 340, 420 };
  const int fc0012_gains[] = { -99, -40, 71, 179, 192 };
  const int fc0013_gains[] = { -99, -73, -65, -63, -60, -58, -54, 58, 61,
                               63, 65, 67, 68, 70, 71, 179, 181, 182,
                               184, 186, 188, 191, 197 };
  const int fc2580_gains[] = { 0 /* no gain values */ };
  const int r820t_gains[] = { 0, 9, 14, 27, 37, 77, 87, 125, 144, 157,
                              166, 197, 207, 229, 254, 280, 297, 328,
                              338, 364, 372, 386, 402, 421, 434, 439,
                              445, 480, 496 };
  const int unknown_gains[] = { 0 /* no gain values */ };

  const int *ptr = NULL;
  int len = 0;

  switch (d_tuner_type) {
  case RTLSDR_TUNER_E4000:
    ptr = e4k_gains; len = sizeof(e4k_gains);
    break;
  case RTLSDR_TUNER_FC0012:
    ptr = fc0012_gains; len = sizeof(fc0012_gains);
    break;
  case RTLSDR_TUNER_FC0013:
    ptr = fc0013_gains; len = sizeof(fc0013_gains);
    break;
  case RTLSDR_TUNER_FC2580:
    ptr = fc2580_gains; len = sizeof(fc2580_gains);
    break;
  case RTLSDR_TUNER_R820T:
    ptr = r820t_gains; len = sizeof(r820t_gains);
    break;
  default:
    ptr = unknown_gains; len = sizeof(unknown_gains);
    break;
  }

  if ( ptr != NULL && len > 0 )
  {
    for (int i = 0; i < int(len / sizeof(int)); i++)
      range += osmosdr::range_t( ptr[i] / 10.0f );
  }

  return range;
}

osmosdr::gain_range_t rtl_tcp_source_c::get_gain_range( const std::string & name, size_t chan )
{
  if ( "IF" == name ) {
    if (d_tuner_type == RTLSDR_TUNER_E4000)
      return osmosdr::gain_range_t(3, 56, 1);
    else
      return osmosdr::gain_range_t();
  }

  return get_gain_range( chan );
}

bool rtl_tcp_source_c::set_gain_mode( bool automatic, size_t chan )
{
  // gain mode
  struct command cmd = { 0x03, htonl(!automatic) };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);

  // AGC mode
  cmd = { 0x08, htonl(automatic) };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);

  _auto_gain = automatic;

  return get_gain_mode(chan);
}

bool rtl_tcp_source_c::get_gain_mode( size_t chan )
{
  return _auto_gain;
}

double rtl_tcp_source_c::set_gain( double gain, size_t chan )
{
  osmosdr::gain_range_t gains = rtl_tcp_source_c::get_gain_range( chan );

  struct command cmd = { 0x04, htonl(int(gains.clip(gain) * 10.0)) };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);

  _gain = gain;

  return get_gain(chan);
}

double rtl_tcp_source_c::set_gain( double gain, const std::string & name, size_t chan )
{
  if ( "IF" == name ) {
    return set_if_gain( gain, chan );
  }

  return set_gain( gain, chan );
}

double rtl_tcp_source_c::get_gain( size_t chan )
{
  return 0;
}

double rtl_tcp_source_c::get_gain( const std::string & name, size_t chan )
{
  if ( "IF" == name ) {
    return _if_gain;
  }

  return get_gain( chan );
}

double rtl_tcp_source_c::set_if_gain(double gain, size_t chan)
{
  if (d_tuner_type != RTLSDR_TUNER_E4000) {
    _if_gain = 0;
    return _if_gain;
  }

  std::vector< osmosdr::gain_range_t > if_gains;

  if_gains += osmosdr::gain_range_t(-3, 6, 9);
  if_gains += osmosdr::gain_range_t(0, 9, 3);
  if_gains += osmosdr::gain_range_t(0, 9, 3);
  if_gains += osmosdr::gain_range_t(0, 2, 1);
  if_gains += osmosdr::gain_range_t(3, 15, 3);
  if_gains += osmosdr::gain_range_t(3, 15, 3);

  std::map< int, double > gains;

  /* initialize with min gains */
  for (unsigned int i = 0; i < if_gains.size(); i++) {
    gains[ i + 1 ] = if_gains[ i ].start();
  }

  for (int i = if_gains.size() - 1; i >= 0; i--) {
    osmosdr::gain_range_t range = if_gains[ i ];

    double error = gain;

    for( double g = range.start(); g <= range.stop(); g += range.step() ) {

      double sum = 0;
      for (int j = 0; j < int(gains.size()); j++) {
        if ( i == j )
          sum += g;
        else
          sum += gains[ j + 1 ];
      }

      double err = std::abs(gain - sum);
      if (err < error) {
        error = err;
        gains[ i + 1 ] = g;
      }
    }
  }
#if 0
  std::cerr << gain << " => "; double sum = 0;
  for (unsigned int i = 0; i < gains.size(); i++) {
    sum += gains[ i + 1 ];
    std::cerr << gains[ i + 1 ] << " ";
  }
  std::cerr << " = " << sum << std::endl;
#endif
  for (unsigned int stage = 1; stage <= gains.size(); stage++) {
    int gain_i = int(gains[stage] * 10.0);
    uint32_t params = stage << 16 | (gain_i & 0xffff);
    struct command cmd = { 0x06, htonl(params) };
    send(d_socket, (const char*)&cmd, sizeof(cmd), 0);
  }

  _if_gain = gain;
  return gain;
}

std::vector< std::string > rtl_tcp_source_c::get_antennas( size_t chan )
{
  std::vector< std::string > antennas;

  antennas += get_antenna(chan);

  return antennas;
}

std::string rtl_tcp_source_c::set_antenna( const std::string & antenna, size_t chan )
{
  return get_antenna(chan);
}

std::string rtl_tcp_source_c::get_antenna( size_t chan )
{
  return "RX";
}
