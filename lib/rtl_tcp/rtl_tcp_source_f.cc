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
//#define HAVE_WINDOWS_H


#include <rtl_tcp_source_f.h>
#include <gr_io_signature.h>
#include <stdexcept>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define USE_SELECT    1  // non-blocking receive on all platforms
#define USE_RCV_TIMEO 0  // non-blocking receive on all but Cygwin
#define SRC_VERBOSE 0
#define SNK_VERBOSE 0

static int is_error( int perr )
{
	// Compare error to posix error code; return nonzero if match.
#if defined(USING_WINSOCK)
#define ENOPROTOOPT 109
	// All codes to be checked for must be defined below
	int werr = WSAGetLastError();
	switch( werr ) {
	case WSAETIMEDOUT:
		return( perr == EAGAIN );
	case WSAENOPROTOOPT:
		return( perr == ENOPROTOOPT );
	default:
		fprintf(stderr,"rtl_tcp_source_f: unknown error %d WS err %d \n", perr, werr );
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

rtl_tcp_source_f::rtl_tcp_source_f(size_t itemsize,
                                   const char *host,
                                   unsigned short port,
                                   int payload_size,
                                   bool eof,
                                   bool wait)
	: gr_sync_block ("rtl_tcp_source_f",
                   gr_make_io_signature(0, 0, 0),
                   gr_make_io_signature(1, 1, sizeof(float))),
	d_itemsize(itemsize),
  d_payload_size(payload_size),
	d_eof(eof),
  d_wait(wait),
  d_socket(-1),
  d_temp_offset(0)
{
	int ret = 0;
	struct sockaddr_in cliaddr;
	socklen_t clilen;
#if defined(USING_WINSOCK) // for Windows (with MinGW)
	// initialize winsock DLL
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if( iResult != NO_ERROR ) {
		report_error( "rtl_tcp_source_f WSAStartup", "can't open socket" );
	}
#endif

	// Set up the address stucture for the source address and port numbers
	// Get the source IP address from the host name
	struct addrinfo *ip_src;      // store the source IP address to use
	struct addrinfo hints;
	memset( (void*)&hints, 0, sizeof(hints) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	char port_str[12];
	sprintf( port_str, "%d", port );

	// FIXME leaks if report_error throws below
	ret = getaddrinfo( host, port_str, &hints, &ip_src );
	if( ret != 0 )
		report_error("rtl_tcp_source_f/getaddrinfo",
		"can't initialize source socket" );

	// FIXME leaks if report_error throws below
	d_temp_buff = new unsigned char[d_payload_size];   // allow it to hold up to payload_size bytes
	d_LUT= new float[0xff+1];
	for(int i=0; i <=(0xff);++i){
		d_LUT[i] = (((float)(i&0xff))-127.5f)*(1.0f/128.0f);
	}
	// create socket
	d_socket = socket(ip_src->ai_family, ip_src->ai_socktype,
		ip_src->ai_protocol);
	if(d_socket == -1) {
		report_error("socket open","can't open socket");
	}

	// Turn on reuse address
	int opt_val = 1;
	if(setsockopt(d_socket, SOL_SOCKET, SO_REUSEADDR, (optval_t)&opt_val, sizeof(int)) == -1) {
		report_error("SO_REUSEADDR","can't set socket option SO_REUSEADDR");
	}

	// Don't wait when shutting down
	linger lngr;
	lngr.l_onoff  = 1;
	lngr.l_linger = 0;
	if(setsockopt(d_socket, SOL_SOCKET, SO_LINGER, (optval_t)&lngr, sizeof(linger)) == -1) {
		if( !is_error(ENOPROTOOPT) ) {  // no SO_LINGER for SOCK_DGRAM on Windows
			report_error("SO_LINGER","can't set socket option SO_LINGER");
		}
	}

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
	if(setsockopt(d_socket, SOL_SOCKET, SO_RCVTIMEO, (optval_t)&timeout, sizeof(timeout)) == -1) {
		report_error("SO_RCVTIMEO","can't set socket option SO_RCVTIMEO");
	}
#endif // USE_RCV_TIMEO


	while(connect(d_socket, ip_src->ai_addr, ip_src->ai_addrlen) != 0);
	freeaddrinfo(ip_src);	
	
	int flag = 1;
	setsockopt(d_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag,sizeof(flag));
}

rtl_tcp_source_f_sptr make_rtl_tcp_source_f (size_t itemsize,
    const char *ipaddr,
    unsigned short port,
    int payload_size,
    bool eof,
    bool wait)
{
	return gnuradio::get_initial_sptr(new rtl_tcp_source_f (
                                      itemsize,
                                      ipaddr,
                                      port,
                                      payload_size,
                                      eof,
                                      wait));
}

rtl_tcp_source_f::~rtl_tcp_source_f ()
{
	delete [] d_temp_buff;

	if (d_socket != -1){
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

int rtl_tcp_source_f::work (int noutput_items,
                            gr_vector_const_void_star &input_items,
                            gr_vector_void_star &output_items)
{
	float *out = (float *) output_items[0];
	ssize_t r=0, nbytes=0, bytes_received=0;
	ssize_t total_bytes = (ssize_t)(d_itemsize*noutput_items);

	int bytesleft = noutput_items;
	int index = 0;
	int receivedbytes = 0;
	while(bytesleft > 0) {
		receivedbytes = recv(d_socket, (char*)&d_temp_buff[index], bytesleft, 0);

		if(receivedbytes == -1 && !is_error(EAGAIN)){
			fprintf(stderr, "socket error\n");
			return -1;
		}
		bytesleft -= receivedbytes;
		index += receivedbytes;
	}
	r = noutput_items;
	
	for(int i=0; i<r; ++i)
		out[i]=d_LUT[*(d_temp_buff+d_temp_offset+i)];

	return r;
}

#ifdef _WIN32
#define __attribute__(x)
#pragma pack(push, 1)
#endif
struct command{
	unsigned char cmd;
	unsigned int param;
}__attribute__((packed));
#ifdef _WIN32
#pragma pack(pop)
#endif

void rtl_tcp_source_f::set_freq(int freq)
{
  struct command cmd = { 0x01, freq };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);
}

void rtl_tcp_source_f::set_sample_rate(int sample_rate)
{
  struct command cmd = { 0x02, sample_rate };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);
}

void rtl_tcp_source_f::set_gain_mode(int manual)
{
  struct command cmd = { 0x03, manual };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);
}

void rtl_tcp_source_f::set_gain(int gain)
{
  struct command cmd = { 0x04, gain };
  send(d_socket, (const char*)&cmd, sizeof(cmd), 0);
}
