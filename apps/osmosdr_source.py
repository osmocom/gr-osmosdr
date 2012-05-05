#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: OsmoSDR Source
# Generated: Thu Nov 12 11:26:07 2009
##################################################

import osmosdr
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.gr import firdes
from gnuradio.wxgui import fftsink2
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import wx

class osmosdr_source_c(grc_wxgui.top_block_gui):

	def __init__(self):
		grc_wxgui.top_block_gui.__init__(self, title="OsmoSDR Source")

		self.src = osmosdr.source_c()

		self.src.set_sample_rate(1024000)
		self.src.set_center_freq(394.4e6)
		self.src.set_gain_mode(0)
		self.src.set_gain(9)

		##################################################
		# Variables
		##################################################
		self.sample_rate = self.src.get_sample_rate()
		self.center_freq = self.src.get_center_freq()

		##################################################
		# Blocks
		##################################################
		self.sink = fftsink2.fft_sink_c(
			self.GetWin(),
			fft_size=1024,
			baseband_freq=self.center_freq,
			sample_rate=self.sample_rate,
			ref_scale=2.0,
			ref_level=0,
			y_divs=10,
			fft_rate=10,
			average=False,
			avg_alpha=0.5
		)

		self.Add(self.sink.win)

		##################################################
		# Connections
		##################################################
		self.connect((self.src, 0), (self.sink, 0))


	def set_sample_rate(self, sample_rate):
		self.sample_rate = sample_rate
		self.sink.set_sample_rate(self.sample_rate)

	def set_center_freq(self, center_freq):
		self.center_freq = center_freq
		self.sink.set_center_freq(self.center_freq)

if __name__ == '__main__':
	parser = OptionParser(option_class=eng_option, usage="%prog: [options]")
	(options, args) = parser.parse_args()
	tb = osmosdr_source_c()
	tb.Run(True)

