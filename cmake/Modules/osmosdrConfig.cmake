INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_OSMOSDR osmosdr)

FIND_PATH(
    OSMOSDR_INCLUDE_DIRS
    NAMES osmosdr/api.h
    HINTS $ENV{OSMOSDR_DIR}/include
        ${PC_OSMOSDR_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    OSMOSDR_LIBRARIES
    NAMES gnuradio-osmosdr
    HINTS $ENV{OSMOSDR_DIR}/lib
        ${PC_OSMOSDR_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/osmosdrTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OSMOSDR DEFAULT_MSG OSMOSDR_LIBRARIES OSMOSDR_INCLUDE_DIRS)
MARK_AS_ADVANCED(OSMOSDR_LIBRARIES OSMOSDR_INCLUDE_DIRS)
