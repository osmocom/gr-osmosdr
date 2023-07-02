/* -*- c++ -*- */
/*
 * Copyright 2017 Sergey Kostanbaev <sergey.kostanbaev@fairwaves.co>
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
#include "xtrx_obj.h"
#include <iostream>
#include <sstream>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

static std::map<std::string, xtrx_obj_sptr> s_objects;

xtrx_obj_sptr xtrx_obj::get(const char* xtrx_dev,
                            unsigned loglevel,
                            bool lmsreset)
{
  std::map<std::string, xtrx_obj_sptr>::iterator i;
  std::string name(xtrx_dev);

  i = s_objects.find(name);
  if (i == s_objects.end()) {
    // No such object
    s_objects[name].reset(new xtrx_obj(name, loglevel, lmsreset));
  }

  return s_objects[name];
}

void xtrx_obj::clear_all()
{
  s_objects.clear();
}

std::vector<std::string> xtrx_obj::get_devices()
{
  std::vector<std::string> devices;
  // TODO
  devices.push_back("/dev/xtrx0");
  return devices;
}


xtrx_obj::xtrx_obj(const std::string &path, unsigned loglevel, bool lmsreset)
  : _run(false)
  , _vio(0)
  , _sink_rate(0)
  , _sink_master(0)
  , _source_rate(0)
  , _source_master(0)
  , _flags(0)
{
  unsigned xtrxflag = (loglevel & XTRX_O_LOGLVL_MASK) | ((lmsreset) ? XTRX_O_RESET : 0);
  std::cerr << "xtrx_obj::xtrx_obj = " << xtrxflag << std::endl;

  xtrx_log_setlevel(loglevel, NULL);

  int res = xtrx_open_string(path.c_str(), &_obj);
  if (res < 0) {
    std::stringstream message;
    message << "Couldn't open "  ": Error: " << -res;

    throw std::runtime_error( message.str() );
  }

  _devices = res;
}

double xtrx_obj::set_smaplerate(double rate, double master, bool sink, unsigned flags)
{
  boost::mutex::scoped_lock lock(mtx);

  if (sink) {
    _sink_rate = rate;
    _sink_master = master;
  } else {
    _source_rate = rate;
    _source_master = master;
  }
  _flags |= flags | XTRX_SAMPLERATE_FORCE_UPDATE;

  if (_sink_master != 0 && _source_master != 0 && _sink_master != _source_master) {
    std::stringstream message;
    message << "Can't operate on diferrent master settings for XTRX sink and source"
               " sink_master " << _sink_master << " source_master" << _source_master;

    throw std::runtime_error( message.str() );
  }

  double rxrate = 0, txrate = 0;
  double actmaster = (_source_master > 0) ? _source_master : _sink_master;
  int res = xtrx_set_samplerate(_obj,
                                actmaster,
                                _source_rate,
                                _sink_rate,
                                _flags,
                                NULL,
                                &rxrate,
                                &txrate);
  if (res) {
    std::cerr << "Unable to set samplerate, error=" << res << std::endl;
  if (sink)
    return _sink_rate;
  return _source_rate;
  }

  if (_vio) {
    xtrx_val_set(_obj, XTRX_TRX, XTRX_CH_AB, XTRX_LMS7_VIO, _vio);
  }

  if (sink)
    return txrate;
  return rxrate;
}

xtrx_obj::~xtrx_obj()
{
  if (_obj) {
    if (_run) {
      //boost::mutex::scoped_lock lock(mtx);
      xtrx_stop(_obj, XTRX_TRX);
    }
    xtrx_close(_obj);
  }
}
