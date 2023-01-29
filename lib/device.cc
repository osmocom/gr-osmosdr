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

#include <osmosdr/device.h>
#include <stdexcept>
#include <boost/format.hpp>
#include <algorithm>
#include <mutex>
#include <sstream>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ENABLE_FCD
#include <fcd_source_c.h>
#endif

#ifdef ENABLE_FILE
#include <file_source_c.h>
#endif

#ifdef ENABLE_RTL
#include <rtl_source_c.h>
#endif

#ifdef ENABLE_RTL_TCP
#include <rtl_tcp_source_c.h>
#endif

#ifdef ENABLE_UHD
#include <uhd_source_c.h>
#endif

#ifdef ENABLE_MIRI
#include <miri_source_c.h>
#endif

#ifdef ENABLE_SDRPLAY
#include <sdrplay_source_c.h>
#endif

#ifdef ENABLE_HACKRF
#include <hackrf_source_c.h>
#endif

#ifdef ENABLE_BLADERF
#include <bladerf_source_c.h>
#endif

#ifdef ENABLE_RFSPACE
#include <rfspace_source_c.h>
#endif

#ifdef ENABLE_AIRSPY
#include <airspy_source_c.h>
#endif

#ifdef ENABLE_AIRSPYHF
#include <airspyhf_source_c.h>
#endif

#ifdef ENABLE_SOAPY
#include <soapy_source_c.h>
#endif

#ifdef ENABLE_REDPITAYA
#include <redpitaya_source_c.h>
#endif

#ifdef ENABLE_FREESRP
#include <freesrp_source_c.h>
#endif

#include "arg_helpers.h"

using namespace osmosdr;

static const std::string args_delim = " ";
static const std::string pairs_delim = ",";
static const std::string pair_delim = "=";

static std::mutex _device_mutex;

device_t::device_t(const std::string &args)
{
  dict_t dict = params_to_dict(args);

  for (dict_t::value_type &entry : dict)
    (*this)[entry.first] = entry.second;
}

std::string device_t::to_pp_string(void) const
{
  if (this->size() == 0) return "Empty Device Address";

  std::stringstream ss;
  ss << "Device Address:" << std::endl;
  for (const device_t::value_type &entry : *this) {
    ss << boost::format("    %s: %s") % entry.first % entry.second << std::endl;
  }
  return ss.str();
}

std::string device_t::to_string(void) const
{
  std::stringstream ss;
  size_t count = 0;
  for (const device_t::value_type &entry : *this) {
    std::string value = entry.second;
    if (value.find(" ") != std::string::npos)
      value = "'" + value + "'";
    ss << ((count++) ? pairs_delim : "") + entry.first;
    if (value.length())
      ss << pair_delim + value;
  }
  return ss.str();
}

devices_t device::find(const device_t &hint)
{
  std::lock_guard<std::mutex> lock(_device_mutex);

  bool fake = true;

  if ( hint.count("nofake") )
    fake = false;

  devices_t devices;

#ifdef ENABLE_FCD
  for (std::string dev : fcd_source_c::get_devices())
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_RTL
  for (std::string dev : rtl_source_c::get_devices())
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_UHD
  for (std::string dev : uhd_source_c::get_devices())
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_MIRI
  for (std::string dev : miri_source_c::get_devices())
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_SDRPLAY
  for (std::string dev : sdrplay_source_c::get_devices())
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_BLADERF
  for (std::string dev : bladerf_source_c::get_devices())
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_HACKRF
  for (std::string dev : hackrf_source_c::get_devices())
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_RFSPACE
  for (std::string dev : rfspace_source_c::get_devices( fake ))
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_AIRSPY
  for (std::string dev : airspy_source_c::get_devices())
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_AIRSPYHF
  for (std::string dev : airspyhf_source_c::get_devices())
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_FREESRP
  for (std::string dev : freesrp_source_c::get_devices())
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_SOAPY
  for (std::string dev : soapy_source_c::get_devices())
    devices.push_back( device_t(dev) );
#endif

  /* software-only sources should be appended at the very end,
   * hopefully resulting in hardware sources to be shown first
   * in a graphical interface etc... */

#ifdef ENABLE_RTL_TCP
  for (std::string dev : rtl_tcp_source_c::get_devices( fake ))
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_REDPITAYA
  for (std::string dev : redpitaya_source_c::get_devices( fake ))
    devices.push_back( device_t(dev) );
#endif
#ifdef ENABLE_FILE
  for (std::string dev : file_source_c::get_devices( fake ))
    devices.push_back( device_t(dev) );
#endif

  return devices;
}
