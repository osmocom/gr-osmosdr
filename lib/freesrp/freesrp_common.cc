#include "freesrp_common.h"

#include <cstdlib>

#include <boost/assign.hpp>

#include <arg_helpers.h>

using namespace boost::assign;

std::shared_ptr<::FreeSRP::FreeSRP> freesrp_common::_srp;

freesrp_common::freesrp_common(const std::string &args)
{
  dict_t dict = params_to_dict(args);

  if(!_srp)
  {
    try
    {
      std::string serial = "";

      if(dict.count("freesrp"))
      {
        serial = dict["freesrp"];
      }

      if(dict.count("fx3"))
      {
        if(FreeSRP::Util::find_fx3())
        {
          // Upload firmware to FX3
          std::string firmware_path = std::string(getenv("HOME")) + "/.freesrp/fx3.img";
          if(dict["fx3"].length() > 0)
          {
            firmware_path = dict["fx3"];
          }
          FreeSRP::Util::find_fx3(true, firmware_path);
          std::cout << "FX3 programmed with '" << firmware_path << "'" << std::endl;
          // Give FX3 time to re-enumerate
          std::this_thread::sleep_for(std::chrono::milliseconds(600));
        }
        else
        {
          std::cout << "No FX3 in bootloader mode found" << std::endl;
        }
      }

      _srp.reset(new ::FreeSRP::FreeSRP(serial));

      if(dict.count("fpga") || !_srp->fpga_loaded())
      {
        std::string bitstream_path = std::string(getenv("HOME")) + "/.freesrp/fpga.bin";
        if(dict["fpga"].length() > 0)
        {
          bitstream_path = dict["fpga"];
        }
        FreeSRP::fpga_status stat = _srp->load_fpga(bitstream_path);
        switch(stat)
        {
        case FreeSRP::FPGA_CONFIG_ERROR:
          throw std::runtime_error("Could not load FPGA configuration!");
        case FreeSRP::FPGA_CONFIG_SKIPPED:
          std::cout << "FPGA already configured. Restart the FreeSRP to load a new bitstream." << std::endl;
          break;
        case FreeSRP::FPGA_CONFIG_DONE:
          std::cout << "FPGA configured with '" << bitstream_path << "'" << std::endl;
          break;
        }
      }

      std::cout << "Connected to FreeSRP" << std::endl;

      if(dict.count("loopback"))
      {
        FreeSRP::response res = _srp->send_cmd({FreeSRP::SET_LOOPBACK_EN, 1});
        if(res.error == FreeSRP::CMD_OK)
        {
          std::cout << "AD9364 in loopback mode" << std::endl;
        }
        else
        {
          throw std::runtime_error("Could not put AD9364 into loopback mode!");
        }
      }
      else
      {
        FreeSRP::response res = _srp->send_cmd({FreeSRP::SET_LOOPBACK_EN, 0});
        if(res.error != FreeSRP::CMD_OK)
        {
          throw std::runtime_error("Error disabling AD9364 loopback mode!");
        }
      }

      if(dict.count("ignore_overflow"))
      {
        _ignore_overflow = true;
      }
      else
      {
        _ignore_overflow = false;
      }
    }
    catch(const std::runtime_error& e)
    {
      std::cerr << "FreeSRP Error: " << e.what() << std::endl;
      throw std::runtime_error(e.what());
    }
  }
}

std::vector<std::string> freesrp_common::get_devices()
{
  std::vector<std::string> devices;

  std::vector<std::string> serial_numbers = ::FreeSRP::FreeSRP::list_connected();

  int index = 0;

  for(std::string &serial : serial_numbers)
  {
    index++;

    std::string str;
    str = "freesrp=" + serial + ",label='FreeSRP " + std::to_string(index) + "'";

    devices.push_back(str);
  }

  return devices;
}

size_t freesrp_common::get_num_channels( void )
{
  return 1;
}

osmosdr::meta_range_t freesrp_common::get_sample_rates( void )
{
  osmosdr::meta_range_t range;

  // Any sample rate between 1e6 and 61.44e6 can be requested.
  // This list of some integer values is used instead of
  //       range += osmosdr::range_t(1e6, 61.44e6);
  // because SoapyOsmo seems to handle the range object differently.
  range += osmosdr::range_t(1e6);
  range += osmosdr::range_t(8e6);
  range += osmosdr::range_t(16e6);
  range += osmosdr::range_t(20e6);
  range += osmosdr::range_t(40e6);
  range += osmosdr::range_t(50e6);
  range += osmosdr::range_t(61.44e6);

  return range;
}

osmosdr::freq_range_t freesrp_common::get_freq_range(size_t chan)
{
  osmosdr::meta_range_t freq_ranges;

  freq_ranges.push_back(osmosdr::range_t(7e7, 6e9, 2.4));

  return freq_ranges;
}


osmosdr::freq_range_t freesrp_common::get_bandwidth_range(size_t chan)
{
  osmosdr::meta_range_t range;

  //range += osmosdr::range_t(2e5, 56e6);

  range += osmosdr::range_t(2e5);
  range += osmosdr::range_t(1e6);
  range += osmosdr::range_t(8e6);
  range += osmosdr::range_t(16e6);
  range += osmosdr::range_t(20e6);
  range += osmosdr::range_t(40e6);
  range += osmosdr::range_t(50e6);
  range += osmosdr::range_t(56e6);

  return range;
}


double freesrp_common::set_freq_corr( double ppm, size_t chan )
{
  // TODO: Set DCXO tuning
  return 0;
}

double freesrp_common::get_freq_corr( size_t chan )
{
  // TODO: Get DCXO tuning
  return 0;
}
