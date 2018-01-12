/* -*- c++ -*- */
/*
 * Copyright 2018 Jeff Long <willcode4@gmail.com>
 * Copyright 2015 SDRplay Ltd <support@sdrplay.com>
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
#ifndef INCLUDED_SDRPLAY_SOURCE_C_H
#define INCLUDED_SDRPLAY_SOURCE_C_H

#include <gnuradio/sync_block.h>
#include <gnuradio/thread/thread.h>

#include "osmosdr/ranges.h"

#include "source_iface.h"

#include <mirsdrapi-rsp.h>

class sdrplay_source_c;

template <typename T>
struct Range {
  T min;
  T max;
};

/*
 * We use boost::shared_ptr's instead of raw pointers for all access
 * to gr::blocks (and many other data structures).  The shared_ptr gets
 * us transparent reference counting, which greatly simplifies storage
 * management issues.  This is especially helpful in our hybrid
 * C++ / Python system.
 *
 * See http://www.boost.org/libs/smart_ptr/smart_ptr.htm
 *
 * As a convention, the _sptr suffix indicates a boost::shared_ptr
 */
typedef boost::shared_ptr<sdrplay_source_c> sdrplay_source_c_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of sdrplay_source_c.
 *
 * To avoid accidental use of raw pointers, sdrplay_source_c's
 * constructor is private.  make_sdrplay_source_c is the public
 * interface for creating new instances.
 */
sdrplay_source_c_sptr make_sdrplay_source_c (const std::string & args = "");

/*!
 * \brief Provides a stream of complex samples.
 * \ingroup block
 */
class sdrplay_source_c :
    public gr::sync_block,
    public source_iface
{
private:
   // The friend declaration allows make_sdrplay_source_c to
   // access the private constructor.

   friend sdrplay_source_c_sptr make_sdrplay_source_c (const std::string & args);

   /*!
    * \brief Provides a stream of complex samples.
    */
   sdrplay_source_c (const std::string & args);  	// private constructor

public:
   ~sdrplay_source_c ();	// public destructor

   int work( int noutput_items,
            gr_vector_const_void_star &input_items,
            gr_vector_void_star &output_items );

   bool start( void );
   bool stop( void );

   static std::vector< std::string > get_devices();

   size_t get_num_channels( void );

   osmosdr::meta_range_t get_sample_rates( void );
   double set_sample_rate( double rate );
   double get_sample_rate( void );

   osmosdr::freq_range_t get_freq_range( size_t chan = 0 );
   double set_center_freq( double freq, size_t chan = 0 );
   double get_center_freq( size_t chan = 0 );
   double set_freq_corr( double ppm, size_t chan = 0 );
   double get_freq_corr( size_t chan = 0 );

   std::vector<std::string> get_gain_names( size_t chan = 0 );
   osmosdr::gain_range_t get_gain_range( size_t chan = 0 );
   osmosdr::gain_range_t get_gain_range( const std::string & name, size_t chan = 0 );
   bool set_gain_mode( bool automatic, size_t chan = 0 );
   bool get_gain_mode( size_t chan = 0 );
   double set_gain( double gain, size_t chan = 0 );
   double set_gain( double gain, const std::string & name, size_t chan = 0 );
   double get_gain( size_t chan = 0 );
   double get_gain( const std::string & name, size_t chan = 0 );

   std::vector< std::string > get_antennas( size_t chan = 0 );
   std::string set_antenna( const std::string & antenna, size_t chan = 0 );
   std::string get_antenna( size_t chan = 0 );

   void set_dc_offset_mode( int mode, size_t chan = 0 );
   void set_dc_offset( const std::complex<double> &offset, size_t chan = 0 );

   double set_bandwidth( double bandwidth, size_t chan = 0 );
   double get_bandwidth( size_t chan = 0 );
   osmosdr::freq_range_t get_bandwidth_range( size_t chan = 0 );

private:
  void startStreaming(void);
  void stopStreaming(void);
  void reallocateBuffers(int size, int num);
  void reinitDevice(int reason);
  int checkLNA(int lna);
  void updateGains(void);
  void streamCallback(short *xi, short *xq, unsigned int firstSampleNum,
                      int grChanged, int rfChanged, int fsChanged,
                      unsigned int numSamples, unsigned int reset,
                      void *cbContext);
  void gainChangeCallback(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext);

  static void streamCallbackWrap(short *xi, short *xq, unsigned int firstSampleNum,
                                 int grChanged, int rfChanged, int fsChanged,
                                 unsigned int numSamples, unsigned int reset,
                                 void *cbContext);
  static void gainChangeCallbackWrap(unsigned int gRdB, unsigned int lnaGRdB, void *cbContext);

  bool _auto_gain;

   int _gain;
   int _gRdB;
   int _lna;
   int _bcastNotch;
   int _dabNotch;
   int _band;
   double _fsHz;
   int _decim;
   double _rfHz;
   mir_sdr_Bw_MHzT _bwType;
   mir_sdr_If_kHzT _ifType;
   int _samplesPerPacket;
   int _dcMode;
   unsigned char _hwVer;
   int _devIndex;
   std::string _antenna;
   int _biasT;

   gr_complex *_buffer;
   int _bufferOffset;
   int _bufferSpaceRemaining;
   boost::mutex _bufferMutex;
   boost::condition_variable _bufferReady;  // buffer is ready to move to other thread

   bool _streaming;
   bool _flowgraphRunning;
   bool _reinit;  // signal streamer to return after a reinit
};

#endif /* INCLUDED_SDRPLAY_SOURCE_C_H */
