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

#include <fstream>
#include <string>
#include <sstream>

#include <boost/assign.hpp>

#include "osmosdr_control.h"

using namespace boost::assign;

osmosdr_control::osmosdr_control(const std::string &args)
{
  /* lookup acm control channel device name for a given alsa device name */

  /*
  if (args.empty())
    pick first available device or throw an exception();
  */
}

osmosdr_control::~osmosdr_control()
{
}

/*
  1 [OsmoSDR        ]: USB-Audio - OsmoSDR
                       sysmocom OsmoSDR at usb-0000:00:06.1-2, high speed
*/
std::vector< std::string > osmosdr_control::find_devices()
{
  std::vector< std::string > devices;

  std::string line;
  std::ifstream cards( "/proc/asound/cards" );
  if ( cards.is_open() )
  {
    while ( cards.good() )
    {
      getline (cards, line);

      if ( line.find( "USB-Audio - OsmoSDR" ) != std::string::npos )
      {
        int id;
        std::istringstream( line ) >> id;

        std::ostringstream hw_id;
        hw_id << "hw:" << id; // build alsa identifier

        devices += hw_id.str();
      }
    }

    cards.close();
  }

  return devices;
}

std::string osmosdr_control::audio_dev_name()
{
  return "hw:1";
}

std::string osmosdr_control::control_dev_name()
{
  return "/dev/ttyUSB0";
}

osmosdr_rx_control::osmosdr_rx_control(const std::string &args) :
  osmosdr_control(args)
{
}

osmosdr_rx_control::~osmosdr_rx_control()
{
}

osmosdr_tx_control::osmosdr_tx_control(const std::string &args) :
  osmosdr_control(args)
{
}

osmosdr_tx_control::~osmosdr_tx_control()
{
}
