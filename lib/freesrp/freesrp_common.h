#ifndef INCLUDED_FREESRP_COMMON_H
#define INCLUDED_FREESRP_COMMON_H

#include <memory>
#include <vector>
#include <string>

#include "osmosdr/ranges.h"

#include <freesrp.hpp>

class freesrp_common
{
protected:
    freesrp_common(const std::string &args);
public:
    static std::vector<std::string> get_devices();

    size_t get_num_channels( void );
    osmosdr::meta_range_t get_sample_rates( void );
    osmosdr::freq_range_t get_freq_range( size_t chan = 0 );
    osmosdr::freq_range_t get_bandwidth_range( size_t chan = 0 );
    double set_freq_corr( double ppm, size_t chan = 0 );
    double get_freq_corr( size_t chan = 0 );
protected:
    static std::shared_ptr<::FreeSRP::FreeSRP> _srp;
    bool _ignore_overflow = false;
};

#endif
