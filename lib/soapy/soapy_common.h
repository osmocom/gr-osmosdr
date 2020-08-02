/* -*- c++ -*- */
/*
 * Copyright 2017 Josh Blum <josh@joshknows.com>
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

#ifndef INCLUDED_SOAPY_COMMON_H
#define INCLUDED_SOAPY_COMMON_H

#include <osmosdr/ranges.h>
#include <SoapySDR/Types.hpp>

#include <mutex>

/*!
 * Convert a soapy range to a gain range.
 * Careful to deal with the step size when zero.
 */
osmosdr::gain_range_t soapy_range_to_gain_range(const SoapySDR::Range &r);

/*!
 * Global mutex to protect factory routines.
 * (optional under 0.5 release above)
 */
std::mutex &get_soapy_maker_mutex(void);

#endif /* INCLUDED_SOAPY_COMMON_H */
