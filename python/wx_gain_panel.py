#!/usr/bin/env python
# -*- coding: utf-8 -*-

import wx

from gnuradio.gr.pubsub import pubsub
from gnuradio.wxgui import forms


GAIN_KEY = lambda x: 'gain:'+x


class wx_gain_panel(forms.static_box_sizer, pubsub):

	def __init__(self, parent, cobj, chan=0):
		# Super init
		pubsub.__init__(self)
		forms.static_box_sizer.__init__(self,
			parent	= parent,
			label	= "Gain Settings",
			orient	= wx.VERTICAL,
			bold	= True,
		)

		# Save params
		self.parent = parent
		self.cobj = cobj
		self.chan = 0

		# Init gui
		self._init_gui()

	def _init_gui(self):
		# Top spacer
		self.AddSpacer(3);

		# Space each gain
		for g_name in self.cobj.get_gain_names(self.chan):
			# Get range
			g_range = self.cobj.get_gain_range(g_name, self.chan)

			# Skip invalid/non-configurable ones
			if g_range.stop() <= g_range.start():
				continue

			# Current value
			self[GAIN_KEY(g_name)] = self.cobj.get_gain(g_name, self.chan)

			# Subscribe
			self.subscribe(
				GAIN_KEY(g_name),
				lambda gain,name=g_name,self=self: self.set_named_gain(name, gain)
			)

			# Create UI
			g_hbox = wx.BoxSizer(wx.HORIZONTAL)
			self.Add(g_hbox, 0, wx.EXPAND)
			self.AddSpacer(3)

			g_hbox.AddSpacer(3)
			forms.text_box(
				parent		= self.parent,
				sizer		= g_hbox,
				proportion	= 0,
				converter	= forms.float_converter(),
				ps			= self,
				key			= GAIN_KEY(g_name),
				label		= g_name + " Gain (dB)",
			)
			g_hbox.AddSpacer(3)
			forms.slider(
				parent		= self.parent,
				sizer		= g_hbox,
				proportion	= 1,
				ps			= self,
				key			= GAIN_KEY(g_name),
				minimum		= g_range.start(),
				maximum		= g_range.stop(),
				step_size	= g_range.step() or (g_range.stop() - g_range.start()) / 10.0,
			)
			g_hbox.AddSpacer(3)

	def set_named_gain(self, name, gain):
		if gain is None:
			g_range = self.cobj.get_gain_range(name, self.chan)
			self[GAIN_KEY(name)] = (g_range.start() + g_range.stop()) / 2.0
		else:
			cur_gain = self.cobj.get_gain(name, self.chan)
			if cur_gain != gain:
				print "%s %f %f" % (name, cur_gain, gain)
				self[GAIN_KEY(name)] = self.cobj.set_gain(gain, name, self.chan)

