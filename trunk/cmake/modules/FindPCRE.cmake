#
# $Id$
#
# Locate pcre
#
# This module accepts the following environment variables:
#
#    PCRE_DIR or PCRE_ROOT - Specify the location of PCRE
#
# This module defines the following CMake variables:
#
#    PCRE_FOUND - True if libpcre is found
#    PCRE_LIBRARY - A variable pointing to the PCRE library
#    PCRE_INCLUDE_DIR - Where to find the headers

#=============================================================================
# Inspired by FindGDAL
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

# This makes the presumption that you are include pcre.h like
#
#include "pcre.h"

if (UNIX AND NOT PCRE_FOUND)
  # Use pcre-config to obtain the library version (this should hopefully
  # allow us to -lpcre1.x.y where x.y are correct version)
  # For some reason, libpcre development packages do not contain
  # libpcre.so...
  find_program (PCRE_CONFIG pcre-config
    HINTS
    ${PCRE_DIR}
    ${PCRE_ROOT}
    $ENV{PCRE_DIR}
    $ENV{PCRE_ROOT}
    PATH_SUFFIXES bin
    PATHS
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt)

  if (PCRE_CONFIG)
    execute_process (COMMAND ${PCRE_CONFIG} --cflags
      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
      OUTPUT_VARIABLE PCRE_CONFIG_CFLAGS)
    if (PCRE_CONFIG_CFLAGS)
      string (REGEX MATCHALL "-I[^ ]+" _pcre_dashI ${PCRE_CONFIG_CFLAGS})
      string (REGEX REPLACE "-I" "" _pcre_includepath "${_pcre_dashI}")
      string (REGEX REPLACE "-I[^ ]+" "" _pcre_cflags_other ${PCRE_CONFIG_CFLAGS})
    endif (PCRE_CONFIG_CFLAGS)
    execute_process (COMMAND ${PCRE_CONFIG} --libs
      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
      OUTPUT_VARIABLE PCRE_CONFIG_LIBS)
    if (PCRE_CONFIG_LIBS)
      string (REGEX MATCHALL "-l[^ ]+" _pcre_dashl ${PCRE_CONFIG_LIBS})
      string (REGEX REPLACE "-l" "" _pcre_lib "${_pcre_dashl}")
      string (REGEX MATCHALL "-L[^ ]+" _pcre_dashL ${PCRE_CONFIG_LIBS})
      string (REGEX REPLACE "-L" "" _pcre_libpath "${_pcre_dashL}")
    endif (PCRE_CONFIG_LIBS)
  endif (PCRE_CONFIG)
endif (UNIX AND NOT PCRE_FOUND)

find_path (PCRE_INCLUDE_DIR pcre.h
  HINTS
  ${_pcre_includepath}
  ${PCRE_DIR}
  ${PCRE_ROOT}
  $ENV{PCRE_DIR}
  $ENV{PCRE_ROOT}
  PATH_SUFFIXES
  include/pcre
  include/PCRE
  include
  PATHS
  ~/Library/Frameworks/pcre.framework/Headers
  /Library/Frameworks/pcre.framework/Headers
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt)

find_library (PCRE_LIBRARY
  NAMES ${_pcre_lib} pcre pcre.0 PCRE
  HINTS
  ${PCRE_DIR}
  ${PCRE_ROOT}
  $ENV{PCRE_DIR}
  $ENV{PCRE_ROOT}
  ${_pcre_libpath}
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks/pcre.framework
  /Library/Frameworks/pcre.framework
  /sw
  /opt/local
  /opt/csw
  /opt)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (PCRE DEFAULT_MSG PCRE_LIBRARY PCRE_INCLUDE_DIR)

set (PCRE_LIBRARIES ${PCRE_LIBRARY})
set (PCRE_INCLUDE_DIRS ${PCRE_INCLUDE_DIR})
