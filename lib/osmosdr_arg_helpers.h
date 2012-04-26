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

#ifndef OSMOSDR_ARG_HELPERS_H
#define OSMOSDR_ARG_HELPERS_H

#include <iostream>
#include <vector>
#include <map>

#include <gr_io_signature.h>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>

inline std::vector< std::string > args_to_vector( const std::string &args )
{
  std::vector< std::string > result;

  boost::escaped_list_separator<char> separator("\\", " ", "'");
  typedef boost::tokenizer< boost::escaped_list_separator<char> > tokenizer_t;
  tokenizer_t tokens(args, separator);

  BOOST_FOREACH(std::string token, tokens)
    result.push_back(token);

  return result;
}

inline std::vector< std::string > params_to_vector( const std::string &params )
{
  std::vector< std::string > result;

  boost::escaped_list_separator<char> separator("\\", ",", "'");
  typedef boost::tokenizer< boost::escaped_list_separator<char> > tokenizer_t;
  tokenizer_t tokens(params, separator);

  BOOST_FOREACH(std::string token, tokens)
    result.push_back(token);

  return result;
}

typedef std::map< std::string, std::string > dict_t;

inline dict_t params_to_dict( const std::string &params )
{
  dict_t result;

  std::vector< std::string > param_list = params_to_vector( params );
  BOOST_FOREACH(std::string param, param_list) {
    std::cout << "D'" << param << "'" << std::endl;
  }

  return result;
}

inline gr_io_signature_sptr args_to_io_signature( const std::string &args )
{
  std::vector< std::string > arg_list = args_to_vector( args );

  const size_t nchan = std::max<size_t>(arg_list.size(), 1);
  return gr_make_io_signature(nchan, nchan, sizeof(gr_complex));
}

#endif // OSMOSDR_ARG_HELPERS_H
