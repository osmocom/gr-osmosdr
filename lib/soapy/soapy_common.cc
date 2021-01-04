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

#include "soapy_common.h"
#include <SoapySDR/Version.hpp>

osmosdr::gain_range_t soapy_range_to_gain_range(const SoapySDR::Range &r)
{
    //default step size when unspecified
    double step = 1.0;

    //support the step size in 0.6 API and above
    //but do not allow unspecified steps
    //to avoid device by zero in some applications
    #ifdef SOAPY_SDR_API_HAS_RANGE_TYPE_STEP
    if (r.step() != 0.0) step = r.step();
    #endif

    return osmosdr::gain_range_t(r.minimum(), r.maximum(), step);
}

std::mutex &get_soapy_maker_mutex(void)
{
    static std::mutex m;
    return m;
}
