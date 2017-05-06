/* -*- c++ -*- */
/*
 * Copyright 2013 Dimitri Stolnikov <horiz0n@gmx.net>
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
#ifndef INCLUDED_HACKRF_OPEN_H
#define INCLUDED_HACKRF_OPEN_H

#include <gnuradio/thread/thread.h>
#include <gnuradio/sync_block.h>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include <libhackrf/hackrf.h>

#include "source_iface.h"

int hackrf_device_open(std::string * hackrf_serial, hackrf_device** _dev);

#endif /* INCLUDED_HACKRF_OPEN_H */
