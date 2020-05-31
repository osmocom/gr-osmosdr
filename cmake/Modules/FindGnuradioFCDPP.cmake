if(NOT GNURADIO_FCDPP_FOUND)
  pkg_check_modules (GNURADIO_FCDPP_PKG libgnuradio-fcdproplus)
  find_path(GNURADIO_FCDPP_INCLUDE_DIRS NAMES fcdproplus/api.h
    PATHS
    ${GNURADIO_FCDPP_PKG_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
  )

  find_library(GNURADIO_FCDPP_LIBRARIES NAMES gnuradio-fcdproplus
    PATHS
    ${GNURADIO_FCDPP_PKG_LIBRARY_DIRS}
    /usr/lib
    /usr/local/lib
  )

if(GNURADIO_FCDPP_INCLUDE_DIRS AND GNURADIO_FCDPP_LIBRARIES)
  set(GNURADIO_FCDPP_FOUND TRUE CACHE INTERNAL "gnuradio-fcdproplus found")
  message(STATUS "Found gnuradio-fcdproplus: ${GNURADIO_FCDPP_INCLUDE_DIRS}, ${GNURADIO_FCDPP_LIBRARIES}")
else(GNURADIO_FCDPP_INCLUDE_DIRS AND GNURADIO_FCDPP_LIBRARIES)
  set(GNURADIO_FCDPP_FOUND FALSE CACHE INTERNAL "gnuradio-fcdproplus found")
  message(STATUS "gnuradio-fcdproplus not found.")
endif(GNURADIO_FCDPP_INCLUDE_DIRS AND GNURADIO_FCDPP_LIBRARIES)

mark_as_advanced(GNURADIO_FCDPP_LIBRARIES GNURADIO_FCDPP_INCLUDE_DIRS)

endif(NOT GNURADIO_FCDPP_FOUND)
