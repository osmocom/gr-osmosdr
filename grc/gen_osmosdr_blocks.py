"""
Copyright 2012 Free Software Foundation, Inc.

This file is part of GNU Radio

GNU Radio Companion is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

GNU Radio Companion is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
"""

MAIN_TMPL = """\
<?xml version="1.0"?>
<block>
  <name>$(title) $sourk.title()</name>
  <key>$(prefix)_$(sourk)_c</key>
  <category>$($sourk.title())s</category>
  <throttle>1</throttle>
  <import>import osmosdr</import>
  <make>osmosdr.$(sourk)_c( args="nchan=" + str(\$nchan) + " " + \$args )
self.\$(id).set_sample_rate(\$sample_rate)
#for $n in range($max_nchan)
\#if \$nchan() > $n
self.\$(id).set_center_freq(\$freq$(n), $n)
self.\$(id).set_freq_corr(\$corr$(n), $n)
self.\$(id).set_iq_balance_mode(\$iq_balance_mode$(n), $n)
self.\$(id).set_gain_mode(\$gain_mode$(n), $n)
self.\$(id).set_gain(\$gain$(n), $n)
self.\$(id).set_if_gain(\$if_gain$(n), $n)
self.\$(id).set_bb_gain(\$bb_gain$(n), $n)
self.\$(id).set_antenna(\$ant$(n), $n)
self.\$(id).set_bandwidth(\$bw$(n), $n)
\#end if
#end for
  </make>
  <callback>set_sample_rate(\$sample_rate)</callback>
  #for $n in range($max_nchan)
  <callback>set_center_freq(\$freq$(n), $n)</callback>
  <callback>set_freq_corr(\$corr$(n), $n)</callback>
  <callback>set_iq_balance_mode(\$iq_balance_mode$(n), $n)</callback>
  <callback>set_gain_mode(\$gain_mode$(n), $n)</callback>
  <callback>set_gain(\$gain$(n), $n)</callback>
  <callback>set_if_gain(\$if_gain$(n), $n)</callback>
  <callback>set_bb_gain(\$bb_gain$(n), $n)</callback>
  <callback>set_antenna(\$ant$(n), $n)</callback>
  <callback>set_bandwidth(\$bw$(n), $n)</callback>
  #end for
  <param>
    <name>$(dir.title())put Type</name>
    <key>type</key>
    <type>enum</type>
    <option>
      <name>Complex float32</name>
      <key>fc32</key>
      <opt>type:fc32</opt>
    </option>
  </param>
  <param>
    <name>Device Arguments</name>
    <key>args</key>
    <value></value>
    <type>string</type>
    <hide>
      \#if \$args()
        none
      \#else
        part
      \#end if
    </hide>
  </param>
  <param>
    <name>Num Channels</name>
    <key>nchan</key>
    <value>1</value>
    <type>int</type>
    #for $n in range(1, $max_nchan+1)
    <option>
      <name>$(n)</name>
      <key>$n</key>
    </option>
    #end for
  </param>
  <param>
    <name>Sample Rate (sps)</name>
    <key>sample_rate</key>
    <value>samp_rate</value>
    <type>real</type>
  </param>
  $params
  <check>$max_nchan >= \$nchan</check>
  <check>\$nchan > 0</check>
  <$sourk>
    <name>$dir</name>
    <type>\$type.type</type>
    <nports>\$nchan</nports>
  </$sourk>
  <doc>
The osmocom block:

While primarily being developed for the OsmoSDR hardware, this block as well supports:

 * FunCube Dongle through libgnuradio-fcd
 * sysmocom OsmoSDR Devices through libosmosdr
 * Great Scott Gadgets HackRF through libhackrf
 * Ettus USRP Devices through Ettus UHD library
 * RTL2832U based DVB-T dongles through librtlsdr
 * RTL-TCP spectrum server (see librtlsdr project)
 * MSi2500 based DVB-T dongles through libmirisdr
 * gnuradio .cfile input through libgnuradio-core

By using the OsmoSDR block you can take advantage of a common software api in your application(s) independent of the underlying radio hardware.

Output Type:
This parameter controls the data type of the stream in gnuradio. Only complex float32 samples are supported at the moment.

Device Arguments:
The device argument is a comma delimited string used to locate devices on your system. Device arguments for multiple devices may be given by separating them with a space.
Use the device id or name/serial (if applicable) to specify a certain device or list of devices. If left blank, the first device found will be used.

Examples:

Optional arguments are placed into [] brackets, remove the brackets before using them! Specific variable values are separated with a |, choose one of them. Variable values containing spaces shall be enclosed in '' as demonstrated in examples section below.
Lines ending with ... mean it's possible to bind devices together by specifying multiple device arguments separated with a space.

Source Mode:
  fcd=0
  hackrf=0[,buffers=32]
  miri=0[,buffers=32] ...
  rtl=serial_number ...
  rtl=0[,rtl_xtal=28.8e6][,tuner_xtal=28.8e6] ...
  rtl=1[,buffers=32][,buflen=N*512] ...
  rtl=2[,direct_samp=0|1|2][,offset_tune=0|1] ...
  rtl_tcp=127.0.0.1:1234[,psize=16384][,direct_samp=0|1|2][,offset_tune=0|1] ...
  uhd[,serial=...][,lo_offset=0][,mcr=52e6][,nchan=2][,subdev='\\\\'B:0 A:0\\\\''] ...
  osmosdr=0[,buffers=32][,buflen=N*512] ...
  file='/path/to/your file',rate=1e6[,freq=100e6][,repeat=true][,throttle=true] ...

Sink Mode:
  hackrf=0[,buffers=32]
  uhd[,serial=...][,lo_offset=0][,mcr=52e6][,nchan=2][,subdev='\\\\'B:0 A:0\\\\''] ...

Num Channels:
Selects the total number of channels in this multi-device configuration. Required when specifying multiple device arguments.

Sample Rate:
The sample rate is the number of samples per second output by this block on each channel.

Frequency:
The center frequency is the frequency the RF chain is tuned to.

Freq. Corr.:
The frequency correction factor in parts per million (ppm). Set to 0 if unknown.

IQ Balance Mode:
Controls the behavior of software IQ imbalance corrrection.
  Off: Disable correction algorithm (pass through).
  Manual: Keep last estimated correction when switched from Automatic to Manual.
  Automatic: Periodicallly find the best solution to compensate for image signals.

This functionality depends on http://cgit.osmocom.org/cgit/gr-iqbal/

Gain Mode:
Chooses between the manual (default) and automatic gain mode where appropriate.
To allow manual control of RF/IF/BB gain stages, manual gain mode must be configured.
Currently, only RTL-SDR devices support automatic gain mode.

RF Gain:
Overall RF gain of the receiving device.

IF Gain:
Overall intermediate frequency gain of the receiving device.
This setting has only effect for RTL-SDR and OsmoSDR devices with E4000 tuners. Observations lead to a reasonable gain range from 15 to 30dB.

BB Gain:
Overall baseband gain of the receiving device.
This setting has only effect for HackRF Jawbreaker. Observations lead to a reasonable gain range from 15 to 30dB.

Antenna:
For devices with only one antenna, this may be left blank.
Otherwise, the user should specify one of the possible antenna choices.

Bandwidth:
Set the bandpass filter on the radio frontend. To use the default (automatic) bandwidth filter setting, this should be zero.

See the OsmoSDR project page for more detailed documentation:
http://sdr.osmocom.org/trac/
http://sdr.osmocom.org/trac/wiki/rtl-sdr
http://sdr.osmocom.org/trac/wiki/GrOsmoSDR
  </doc>
</block>
"""

PARAMS_TMPL = """
  <param>
    <name>Ch$(n): Frequency (Hz)</name>
    <key>freq$(n)</key>
    <value>100e6</value>
    <type>real</type>
    <hide>\#if \$nchan() > $n then 'none' else 'all'#</hide>
  </param>
  <param>
  <name>Ch$(n): Freq. Corr. (ppm)</name>
    <key>corr$(n)</key>
    <value>0</value>
    <type>real</type>
    <hide>\#if \$nchan() > $n then 'none' else 'all'#</hide>
  </param>
  <param>
    <name>Ch$(n): IQ Balance Mode</name>
    <key>iq_balance_mode$(n)</key>
    <value>0</value>
    <type>int</type>
    <hide>\#if \$nchan() > $n then 'none' else 'all'#</hide>
    <option>
      <name>Off</name>
      <key>0</key>
    </option>
    <option>
      <name>Manual</name>
      <key>1</key>
    </option>
    <option>
      <name>Automatic</name>
      <key>2</key>
    </option>
  </param>
  <param>
    <name>Ch$(n): Gain Mode</name>
    <key>gain_mode$(n)</key>
    <value>0</value>
    <type>int</type>
    <hide>\#if \$nchan() > $n then 'none' else 'all'#</hide>
    <option>
      <name>Manual</name>
      <key>0</key>
    </option>
    <option>
      <name>Automatic</name>
      <key>1</key>
    </option>
  </param>
  <param>
    <name>Ch$(n): RF Gain (dB)</name>
    <key>gain$(n)</key>
    <value>10</value>
    <type>real</type>
    <hide>\#if \$nchan() > $n then 'none' else 'all'#</hide>
  </param>
  <param>
    <name>Ch$(n): IF Gain (dB)</name>
    <key>if_gain$(n)</key>
    <value>20</value>
    <type>real</type>
    <hide>\#if \$nchan() > $n then 'none' else 'all'#</hide>
  </param>
  <param>
    <name>Ch$(n): BB Gain (dB)</name>
    <key>bb_gain$(n)</key>
    <value>20</value>
    <type>real</type>
    <hide>\#if \$nchan() > $n then 'none' else 'all'#</hide>
  </param>
  <param>
    <name>Ch$(n): Antenna</name>
    <key>ant$(n)</key>
    <value></value>
    <type>string</type>
    <hide>
      \#if not \$nchan() > $n
        all
      \#elif \$ant$(n)()
        none
      \#else
        part
      \#end if
    </hide>
  </param>
  <param>
    <name>Ch$(n): Bandwidth (Hz)</name>
    <key>bw$(n)</key>
    <value>0</value>
    <type>real</type>
    <hide>
      \#if not \$nchan() > $n
        all
      \#elif \$bw$(n)()
        none
      \#else
        part
      \#end if
    </hide>
  </param>
"""

def parse_tmpl(_tmpl, **kwargs):
  from Cheetah import Template
  return str(Template.Template(_tmpl, kwargs))

max_num_channels = 5

import os.path

if __name__ == '__main__':
  import sys
  for file in sys.argv[1:]:
    head, tail = os.path.split(file)

    if tail.startswith('rtlsdr'):
      title = 'RTL-SDR'
      prefix = 'rtlsdr'
    elif tail.startswith('osmosdr'):
      title = 'osmocom'
      prefix = 'osmosdr'
    else: raise Exception, 'file %s has wrong syntax!'%tail

    if tail.endswith ('source_c.xml'):
      sourk = 'source'
      dir = 'out'
    elif tail.endswith ('sink_c.xml'):
      sourk = 'sink'
      dir = 'in'
    else: raise Exception, 'is %s a source or sink?'%file

    params = ''.join([parse_tmpl(PARAMS_TMPL, n=n) for n in range(max_num_channels)])
    open(file, 'w').write(parse_tmpl(MAIN_TMPL,
      max_nchan=max_num_channels,
      params=params,
      title=title,
      prefix=prefix,
      sourk=sourk,
      dir=dir,
    ))
