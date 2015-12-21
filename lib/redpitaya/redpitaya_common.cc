/* -*- c++ -*- */
/*
 * Copyright 2015 Pavel Demin
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
#include <stdexcept>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "redpitaya_common.h"

#if defined(__APPLE__) || defined(__MACH__)
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif
#endif

void redpitaya_send_command( int socket, uint32_t command )
{
  ssize_t size;
  std::stringstream message;

#if defined(_WIN32)
  size = ::send( socket, (char *)&command, sizeof(command), 0 );
#else
  size = ::send( socket, &command, sizeof(command), MSG_NOSIGNAL );
#endif

  if ( size != sizeof(command) )
  {
    message << "Sending command failed: " << std::hex << command;
    throw std::runtime_error( message.str() );
  }
}
