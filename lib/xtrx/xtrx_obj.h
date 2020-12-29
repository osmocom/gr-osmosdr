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
#ifndef XTRX_OBJ_H
#define XTRX_OBJ_H

#include <boost/shared_ptr.hpp>
#include <xtrx_api.h>
#include <map>
#include <vector>
#include <boost/thread/mutex.hpp>

class xtrx_obj;

typedef std::shared_ptr<xtrx_obj> xtrx_obj_sptr;

class xtrx_obj
{
public:
  xtrx_obj(const std::string& path, unsigned loglevel, bool lmsreset);
  ~xtrx_obj();

  static std::vector<std::string> get_devices();

  static xtrx_obj_sptr get(const char* xtrx_dev,
                           unsigned loglevel,
                           bool lmsreset);
  static void clear_all();

  xtrx_dev* dev() { return _obj; }
  unsigned dev_count() { return _devices; }

  double set_smaplerate(double rate, double master, bool sink, unsigned flags);

  void set_vio(unsigned vio) { _vio = vio; }

  boost::mutex mtx;
protected:
  xtrx_dev* _obj;
  bool      _run;
  unsigned  _vio;

  double    _sink_rate;
  double    _sink_master;
  double    _source_rate;
  double    _source_master;

  unsigned  _flags;
  unsigned  _devices;
};

#endif // XTRX_OBJ_H
