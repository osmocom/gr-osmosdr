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
	<name>OsmoSDR $sourk.title()</name>
	<key>osmosdr_$(sourk)_c</key>
	<throttle>1</throttle>
	<import>import osmosdr</import>
	<make>osmosdr.$(sourk)_c( args="nchan=" + str(\$nchan) + " " + \$args  )
self.\$(id).set_sample_rate(\$sample_rate)
#for $n in range($max_nchan)
\#if \$nchan() > $n
self.\$(id).set_center_freq(\$freq$(n), $n)
self.\$(id).set_freq_corr(\$corr$(n), $n)
self.\$(id).set_gain_mode(\$gain_mode$(n), $n)
self.\$(id).set_gain(\$gain$(n), $n)
\#if \$ant$(n)()
self.\$(id).set_antenna(\$ant$(n), $n)
\#end if
\#end if
#end for
</make>
	<callback>set_sample_rate(\$sample_rate)</callback>
	#for $n in range($max_nchan)
	<callback>set_center_freq(\$freq$(n), $n)</callback>
	<callback>set_freq_corr(\$corr$(n), $n)</callback>
	<callback>set_gain_mode(\$gain_mode$(n), $n)</callback>
	<callback>set_gain(\$gain$(n), $n)</callback>
	<callback>set_antenna(\$ant$(n), $n)</callback>
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
The OsmoSDR $sourk.title() block:

Output Type:
This parameter controls the data type of the stream in gnuradio.

Device Arguments:
The device argument is a delimited string used to locate devices on your system.
Use the device id or name (if applicable) to specify a specific device or list
of devices. If left blank, the first device found will be used.

Examples:
  fcd=0 fcd=1 fcd=2 ...
  rtl=0 rtl=1 rtl=2,rtl_xtal=28.80001e6,tuner_xtal=26e6 ...
  uhd=0|name,mcr=52e6,nchan=2,subdev='\\\\'B:0 A:0'\\\\' ...
  osmosdr=0|name,mcr=64e6,nchan=5,port=/dev/ttyUSB0 ...
  file=/path/to/file.ext,freq=428e6,rate=1e6,repeat=true,throttle=true

Num Channels:
Selects the total number of channels in this multi-device configuration.

Sample Rate:
The sample rate is the number of samples per second output by this block.

Frequency:
The center frequency is the overall frequency of the RF chain.

Freq. Corr.:
The frequency correction factor in parts per million (ppm). Leave 0 if unknown.

Gain:
Overall gain of the device's signal path. For the gain setting to apply the
manual gain mode must be enabled first for some devices (namely rtlsdr).

Gain Mode:
Chooses between the manual (default) and automatc gain mode where appropriate.
Currently, only rtlsdr devices support automatic gain mode.

Antenna:
For devices with only one antenna, this may be left blank.
Otherwise, the user should specify one of the possible antenna choices.

See the OsmoSDR manual for more detailed documentation:
http://sdr.osmocom.org/trac/
	</doc>
</block>
"""

PARAMS_TMPL = """
	<param>
		<name>Ch$(n): Frequency (Hz)</name>
		<key>freq$(n)</key>
		<value>0</value>
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
		<name>Ch$(n): Gain (dB)</name>
		<key>gain$(n)</key>
		<value>0</value>
		<type>real</type>
		<hide>\#if \$nchan() > $n then 'none' else 'all'#</hide>
	</param>
	<param>
		<name>Ch$(n): Gain Mode</name>
		<key>gain_mode$(n)</key>
		<value></value>
		<type>enum</type>
		<hide>\#if \$nchan() > $n then 'none' else 'all'#</hide>
		<option>
			<name>Manual</name>
			<key>1</key>
		</option>
		<option>
			<name>Auto</name>
			<key>0</key>
		</option>
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
"""

def parse_tmpl(_tmpl, **kwargs):
	from Cheetah import Template
	return str(Template.Template(_tmpl, kwargs))

max_num_channels = 5

if __name__ == '__main__':
	import sys
	for file in sys.argv[1:]:
		if file.endswith ('source_c.xml'):
			sourk = 'source'
			dir = 'out'
		elif file.endswith ('sink_c.xml'):
			sourk = 'sink'
			dir = 'in'
		else: raise Exception, 'is %s a source or sink?'%file

		params = ''.join([parse_tmpl(PARAMS_TMPL, n=n) for n in range(max_num_channels)])
		open(file, 'w').write(parse_tmpl(MAIN_TMPL,
			max_nchan=max_num_channels,
			params=params,
			sourk=sourk,
			dir=dir,
		))
