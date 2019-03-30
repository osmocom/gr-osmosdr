if(NOT LIBBLADERF_FOUND)
  pkg_check_modules (LIBBLADERF_PKG libbladeRF)
  if (LIBBLADERF_PKG_FOUND AND LIBBLADERF_PKG_VERSION VERSION_LESS "2")
    message( FATAL_ERROR "Install version 2 or greater of libbladeRF."
      " Current version ( ${LIBBLADERF_PKG_VERSION} ) is out of date." )
  endif()
  find_path(LIBBLADERF_INCLUDE_DIRS NAMES libbladeRF.h
    PATHS
    ${LIBBLADERF_PKG_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
  )

  find_library(LIBBLADERF_LIBRARIES NAMES bladeRF
    PATHS
    ${LIBBLADERF_PKG_LIBRARY_DIRS}
    /usr/lib
    /usr/local/lib
  )

if(LIBBLADERF_INCLUDE_DIRS AND LIBBLADERF_LIBRARIES)
  set(LIBBLADERF_FOUND TRUE CACHE INTERNAL "libbladeRF found")
  message(STATUS "Found libbladeRF: ${LIBBLADERF_INCLUDE_DIRS}, ${LIBBLADERF_LIBRARIES}")
else(LIBBLADERF_INCLUDE_DIRS AND LIBBLADERF_LIBRARIES)
  set(LIBBLADERF_FOUND FALSE CACHE INTERNAL "libbladeRF found")
  message(STATUS "libbladeRF not found.")
endif(LIBBLADERF_INCLUDE_DIRS AND LIBBLADERF_LIBRARIES)

mark_as_advanced(LIBBLADERF_LIBRARIES LIBBLADERF_INCLUDE_DIRS)

endif(NOT LIBBLADERF_FOUND)
