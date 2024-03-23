gr-osmosdr - generic gnuradio SDR I/O block
===========================================

While originally being developed for the
[OsmoSDR](https://osmocom.org/projects/osmosdr/wiki) hardware, this
block has become a generic SDR I/O block for a variety of SDR
hardware, including:

 * FUNcube Dongle / Pro+ through [gr-funcube](https://github.com/dl1ksv/gr-funcube)
 * RTL2832U based DVB-T dongles through [librtlsdr](https://osmocom.org/projects/rtl-sdr/wiki)
 * RTL-TCP spectrum server (see librtlsdr project)
 * MSi2500 based DVB-T dongles through [libmirisdr](https://gitea.osmocom.org/sdr/libmirisdr)
 * SDRplay RSP through SDRplay API library
 * gnuradio .cfile input through libgnuradio-blocks
 * RFSPACE SDR-IQ, SDR-IP, NetSDR (incl. X2 option), Cloud-IQ, and CloudSDR
 * AirSpy Wideband Receiver through [libairspy](https://github.com/airspy/airspyone_host)
 * CCCamp 2015 rad1o Badge through [libhackrf](https://github.com/greatscottgadgets/hackrf)
 * Great Scott Gadgets HackRF through [libhackrf](https://github.com/greatscottgadgets/hackrf)
 * Nuand LLC bladeRF through [libbladeRF library](https://www.nuand.com/libbladeRF-doc/)
 * Ettus USRP Devices through [Ettus UHD library](https://github.com/EttusResearch/uhd)
 * Fairwaves UmTRX through [Fairwaves' module for UHD](https://github.com/fairwaves/UHD-Fairwaves)
 * Fairwaves XTRX through [libxtrx](https://github.com/myriadrf/libxtrx)
 * Red Pitaya SDR transceiver <http://bazaar.redpitaya.com>
 * FreeSRP through [libfreesrp](https://github.com/myriadrf/libfreesrp)

By using the gr-osmosdr block you can take advantage of a common software API in
your application(s) independent of the underlying radio hardware.

Homepage + Documentation
------------------------

For installation and usage guidelines please read the documentation available
at <https://osmocom.org/projects/gr-osmosdr/wiki>

For the impatient :) a short excerpt:

The Gnu Radio block requires a recent gnuradio (>= v3.7) to be installed.

Before building the block you have to make sure that all the dependencies
(see list of supported devices above) you are intend to work with are
properly installed. The build system of gr-osmosdr will recognize them and
enable specific source/sink components thereafter.

Please note: prior pulling a new version from git and compiling it,
please do a `make uninstall` first to properly remove the previous version.

Building with cmake:
```
git clone https://gitea.osmocom.org/sdr/gr-osmosdr
cd gr-osmosdr/
mkdir build
cd build/
cmake ../
make
sudo make install
sudo ldconfig
```

NOTE: The osmocom blocks will appear under *Sources* and *Sinks* categories
in GRC menu.

Forum
-----

We welcome any gr-osmosdr related discussions in the
[SDR](https://discourse.osmocom.org/c/sdr/)
section of the osmocom discourse (web based Forum).

Mailing List
------------

Discussions related to libosmocore are happening on the
osmocom-sdr@lists.osmocom.org mailing list, please see
<https://lists.osmocom.org/mailman/listinfo/osmocom-sdr> for subscription
options and the list archive.

Please observe the [Osmocom Mailing List
Rules](https://osmocom.org/projects/cellular-infrastructure/wiki/Mailing_List_Rules)
when posting.


Issue tracker
-------------

We are using the Osmocom redmine at <https://osmocom.org/projects/gr-osmosdr/issues>

Contributing
------------

We maintain our source code in a self-hosted instance of gitea at
<https://gitea.osmocom.org/sdr/gr-osmosdr>. You can send pull requests there, or send
patches the old-fashioned way (git send-email) to the above-mentioned mailing list.
