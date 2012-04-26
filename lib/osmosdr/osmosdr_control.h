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
#ifndef OSMOSDR_CONTROL_H
#define OSMOSDR_CONTROL_H

#include <string>
#include <complex>
#include <osmosdr_ranges.h>

/*!
 * session object to cdc-acm based control channel of the device
 */
class osmosdr_control
{
public:
    osmosdr_control(const std::string &args);
    virtual ~osmosdr_control();

    /*!
     * Discovers all devices connected to the host computer.
     * \return a vector of device addresses
     */
    static std::vector< std::string > find_devices();

protected:
    std::string audio_dev_name();
    std::string control_dev_name();
};

/*!
 * osmosdr_source class derives from this class to be able to control the device
 */
class osmosdr_rx_control : public osmosdr_control
{
public:
    osmosdr_rx_control(const std::string &args);
    virtual ~osmosdr_rx_control();
};

/*!
 * osmosdr_sink class derives from this class to be able to control the device
 */
class osmosdr_tx_control : public osmosdr_control
{
public:
    osmosdr_tx_control(const std::string &args);
    virtual ~osmosdr_tx_control();
};

#endif // OSMOSDR_CONTROL_H
