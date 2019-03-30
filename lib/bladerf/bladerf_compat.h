/* -*- c++ -*- */
/*
 * Copyright 2017 Nuand LLC
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
#ifndef INCLUDED_BLADERF_COMPAT_H
#define INCLUDED_BLADERF_COMPAT_H

#if defined(LIBBLADERF_API_VERSION) && (LIBBLADERF_API_VERSION < 0x01080100)
    #warning Old libbladeRF detected: using compatibility workarounds.

    #define BLADERF_COMPATIBILITY

    /* New libbladeRF supports multiple channels, via various enums. */
    typedef bladerf_module bladerf_channel;
    #define BLADERF_CHANNEL_RX(ch) BLADERF_MODULE_RX
    #define BLADERF_CHANNEL_TX(ch) BLADERF_MODULE_TX
    #define BLADERF_CHANNEL_INVALID BLADERF_MODULE_INVALID

    typedef bladerf_module bladerf_channel_layout;
    #define BLADERF_RX_X1 BLADERF_MODULE_RX
    #define BLADERF_TX_X1 BLADERF_MODULE_TX
    #define BLADERF_RX_X2 BLADERF_MODULE_INVALID
    #define BLADERF_TX_X2 BLADERF_MODULE_INVALID

    typedef bladerf_module bladerf_direction;
    #define BLADERF_RX BLADERF_MODULE_RX
    #define BLADERF_TX BLADERF_MODULE_TX
    #define BLADERF_DIRECTION_MASK (0x1)

    /* Changed API calls */
    static
    int bladerf_get_frequency(struct bladerf *dev,
                              bladerf_channel ch,
                              uint64_t *freq) // was unsigned int *frequency
    {
        unsigned int f32 = 0;
        int status = bladerf_get_frequency(dev, ch, &f32);
        *freq = static_cast<uint64_t>(f32);
        return status;
    }

    static
    int bladerf_sync_tx(struct bladerf *dev,
                        void const *samples,  // was void *samples
                        unsigned int num_samples,
                        struct bladerf_metadata *metadata,
                        unsigned int timeout_ms)
    {
        void *s = const_cast<void *>(samples);
        return bladerf_sync_tx(dev, s, num_samples, metadata, timeout_ms);
    }

    /* Changed enums/defines */
    #define BLADERF_GAIN_DEFAULT BLADERF_GAIN_MANUAL
    #define BLADERF_GAIN_MGC BLADERF_GAIN_MANUAL
    #define BLADERF_RX_MUX_BASEBAND BLADERF_RX_MUX_BASEBAND_LMS

    /* New functionality with no equivalent */
    #define BLADERF_LB_AD9361_BIST BLADERF_LB_NONE
    #define bladerf_get_board_name(name) "bladerf1"

#endif // libbladeRF < 1.8.1
#endif // INCLUDED_BLADERF_COMPAT_H
