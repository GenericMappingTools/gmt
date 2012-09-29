#
# $Id$
#
# Locate fftw3
#
# This module accepts the following environment variables:
#
#    FFTW3_DIR or FFTW3_ROOT - Specify the location of FFTW3
#
# This module defines the following CMake variables:
#
#    FFTW3_FOUND - True if libfftw3 is found
#    FFTW3F_LIBRARY - A variable pointing to the FFTW3 single precision library
#    FFTW3F_THREADS_LIBRARY - Threaded single precision library
#    FFTW3_INCLUDE_DIR - Where to find the headers

if (DEFINED FFTW3_ROOT AND NOT FFTW3_ROOT)
  set (FFTW3F_LIBRARY "" CACHE INTERNAL "")
  set (FFTW3F_THREADS_LIBRARY "" CACHE INTERNAL "")
  set (FFTW3_INCLUDE_DIR "" CACHE INTERNAL "")
  return()
endif()

find_path (FFTW3_INCLUDE_DIR fftw3.h
  HINTS
  ${FFTW3_DIR}
  ${FFTW3_ROOT}
  $ENV{FFTW3_DIR}
  $ENV{FFTW3_ROOT}
  PATH_SUFFIXES include
  PATHS
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt)

# single precision
find_library (FFTW3F_LIBRARY
  NAMES fftw3f
  HINTS
  ${FFTW3_DIR}
  ${FFTW3_ROOT}
  $ENV{FFTW3_DIR}
  $ENV{FFTW3_ROOT}
  PATH_SUFFIXES lib64 lib
  PATHS
  /sw
  /opt/local
  /opt/csw
  /opt)

get_filename_component (_fftw3_libpath ${FFTW3F_LIBRARY} PATH)

# threaded single precision
find_library (FFTW3F_THREADS_LIBRARY
  NAMES fftw3f_threads
  HINTS ${_fftw3_libpath})

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (FFTW3 DEFAULT_MSG FFTW3F_LIBRARY FFTW3_INCLUDE_DIR)

set (FFTW3F_LIBRARIES ${FFTW3F_LIBRARY} ${FFTW3F_THREADS_LIBRARY})
set (FFTW3_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})
