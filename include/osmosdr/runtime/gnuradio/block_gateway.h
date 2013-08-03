/* -*- c++ -*- */
/*
 * Copyright 2011-2013 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
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

#ifndef INCLUDED_RUNTIME_BLOCK_GATEWAY_H
#define INCLUDED_RUNTIME_BLOCK_GATEWAY_H

#include <gnuradio/api.h>
#include <gnuradio/block.h>
#include <gnuradio/feval.h>

namespace gr {
  
  /*!
   * The work type enum tells the gateway what kind of block to
   * implement.  The choices are familiar gnuradio block overloads
   * (sync, decim, interp).
   */
  enum block_gw_work_type {
    GR_BLOCK_GW_WORK_GENERAL,
    GR_BLOCK_GW_WORK_SYNC,
    GR_BLOCK_GW_WORK_DECIM,
    GR_BLOCK_GW_WORK_INTERP,
  };

  /*!
   * Shared message structure between python and gateway.
   * Each action type represents a scheduler-called function.
   */
  struct block_gw_message_type {
    enum action_type {
      ACTION_GENERAL_WORK, //dispatch work
      ACTION_WORK, //dispatch work
      ACTION_FORECAST, //dispatch forecast
      ACTION_START, //dispatch start
      ACTION_STOP, //dispatch stop
    };

    action_type action;

    int general_work_args_noutput_items;
    std::vector<int> general_work_args_ninput_items;
    std::vector<void *> general_work_args_input_items; //TODO this should be const void*, but swig cant int cast it right
    std::vector<void *> general_work_args_output_items;
    int general_work_args_return_value;

    int work_args_ninput_items;
    int work_args_noutput_items;
    std::vector<void *> work_args_input_items; //TODO this should be const void*, but swig cant int cast it right
    std::vector<void *> work_args_output_items;
    int work_args_return_value;

    int forecast_args_noutput_items;
    std::vector<int> forecast_args_ninput_items_required;

    bool start_args_return_value;

    bool stop_args_return_value;
  };

  /*!
   * The gateway block which performs all the magic.
   *
   * The gateway provides access to all the gr::block routines.
   * The methods prefixed with gr::block__ are renamed
   * to class methods without the prefix in python.
   */
  class GR_RUNTIME_API block_gateway : virtual public gr::block
  {
  public:
    // gr::block_gateway::sptr
    typedef boost::shared_ptr<block_gateway> sptr;
    
    /*!
     * Make a new gateway block.
     * \param handler the swig director object with callback
     * \param name the name of the block (Ex: "Shirley")
     * \param in_sig the input signature for this block
     * \param out_sig the output signature for this block
     * \param work_type the type of block overload to implement
     * \param factor the decimation or interpolation factor
     * \return a new gateway block
     */
    static sptr make(gr::feval_ll *handler,
                     const std::string &name,
                     gr::io_signature::sptr in_sig,
                     gr::io_signature::sptr out_sig,
                     const block_gw_work_type work_type,
                     const unsigned factor);

    //! Provide access to the shared message object
    virtual block_gw_message_type &block_message(void) = 0;

    long block__unique_id(void) const {
      return gr::block::unique_id();
    }

    std::string block__name(void) const {
      return gr::block::name();
    }

    unsigned block__history(void) const {
      return gr::block::history();
    }

    void block__set_history(unsigned history) {
      return gr::block::set_history(history);
    }

    void block__set_fixed_rate(bool fixed_rate) {
      return gr::block::set_fixed_rate(fixed_rate);
    }

    bool block__fixed_rate(void) const {
      return gr::block::fixed_rate();
    }

    void block__set_output_multiple(int multiple) {
      return gr::block::set_output_multiple(multiple);
    }

    int block__output_multiple(void) const {
      return gr::block::output_multiple();
    }

    void block__consume(int which_input, int how_many_items) {
      return gr::block::consume(which_input, how_many_items);
    }

    void block__consume_each(int how_many_items) {
      return gr::block::consume_each(how_many_items);
    }

    void block__produce(int which_output, int how_many_items) {
      return gr::block::produce(which_output, how_many_items);
    }

    void block__set_relative_rate(double relative_rate) {
      return gr::block::set_relative_rate(relative_rate);
    }

    double block__relative_rate(void) const {
      return gr::block::relative_rate();
    }

    uint64_t block__nitems_read(unsigned int which_input) {
      return gr::block::nitems_read(which_input);
    }

    uint64_t block__nitems_written(unsigned int which_output) {
      return gr::block::nitems_written(which_output);
    }

  protected:

  };

} /* namespace gr */

#endif /* INCLUDED_RUNTIME_BLOCK_GATEWAY_H */
