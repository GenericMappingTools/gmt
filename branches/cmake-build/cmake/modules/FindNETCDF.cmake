#
# $Id$
#
# Locate netcdf
#
# This module accepts the following environment variables:
#
#    NETCDF_DIR or NETCDF_ROOT - Specify the location of NetCDF
#
# This module defines the following CMake variables:
#
#    NETCDF_FOUND - True if libnetcdf is found
#    NETCDF_LIBRARY - A variable pointing to the NetCDF library
#    NETCDF_INCLUDE_DIR - Where to find the headers
#    NETCDF_INCLUDE_DIRS - Where to find the headers
#    NETCDF_DEFINITIONS - Extra compiler flags

#=============================================================================
# Inspired by FindGDAL
#
# Distributed under the OSI-approved bsd license (the "License")
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

# This makes the presumption that you are include netcdf.h like
#
#include "netcdf.h"

if(UNIX)
  # Use nc-config to obtain the libraries
  find_program(NETCDF_CONFIG nc-config
    HINTS
    ${NETCDF_DIR}
    ${NETCDF_ROOT}
    $ENV{NETCDF_DIR}
    $ENV{NETCDF_ROOT}
    PATH_SUFFIXES bin
    PATHS
    /sw # Fink
    /opt/local # DarwinPorts
    /opt/csw # Blastwave
    /opt
    )

  if(NETCDF_CONFIG)
    exec_program(${NETCDF_CONFIG} ARGS --cflags OUTPUT_VARIABLE NETCDF_CONFIG_CFLAGS)
    if(NETCDF_CONFIG_CFLAGS)
      string(REGEX MATCHALL "-I[^ ]+" _netcdf_dashI ${NETCDF_CONFIG_CFLAGS})
      string(REGEX REPLACE "-I" "" _netcdf_includepath "${_netcdf_dashI}")
      string(REGEX REPLACE "-I[^ ]+" "" _netcdf_cflags_other ${NETCDF_CONFIG_CFLAGS})
    endif(NETCDF_CONFIG_CFLAGS)
    exec_program(${NETCDF_CONFIG} ARGS --libs OUTPUT_VARIABLE NETCDF_CONFIG_LIBS)
    if(NETCDF_CONFIG_LIBS)
      string(REGEX MATCHALL "-l[^ ]+" _netcdf_dashl ${NETCDF_CONFIG_LIBS})
      string(REGEX REPLACE "-l" "" _netcdf_lib "${_netcdf_dashl}")
      string(REGEX MATCHALL "-L[^ ]+" _netcdf_dashL ${NETCDF_CONFIG_LIBS})
      string(REGEX REPLACE "-L" "" _netcdf_libpath "${_netcdf_dashL}")
    endif(NETCDF_CONFIG_LIBS)
  endif(NETCDF_CONFIG)
  if(_netcdf_lib)
    list(REMOVE_DUPLICATES _netcdf_lib)
    list(REMOVE_ITEM _netcdf_lib netcdf)
  endif(_netcdf_lib)
endif(UNIX)

find_path(NETCDF_INCLUDE_DIR netcdf.h
  HINTS
  ${_netcdf_includepath}
  ${NETCDF_DIR}
  ${NETCDF_ROOT}
  $ENV{NETCDF_DIR}
  $ENV{NETCDF_ROOT}
  PATH_SUFFIXES
  include/netcdf
  include
  PATHS
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  )

find_library(NETCDF_LIBRARY 
  NAMES netcdf
  HINTS
  ${_netcdf_libpath}
  ${NETCDF_DIR}
  ${NETCDF_ROOT}
  $ENV{NETCDF_DIR}
  $ENV{NETCDF_ROOT}
  PATH_SUFFIXES lib64 lib
  PATHS
  /sw
  /opt/local
  /opt/csw
  /opt
  /usr/freeware
  )

# find all libs that nc-config reports
foreach(_extralib ${_netcdf_lib})
  find_library(_found_lib_${_extralib}
    NAMES ${_extralib}
    HINTS ${_netcdf_libpath}
    PATHS ${_netcdf_libpath})
  list(APPEND NETCDF_LIBRARY ${_found_lib_${_extralib}})
endforeach(_extralib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NETCDF DEFAULT_MSG NETCDF_LIBRARY NETCDF_INCLUDE_DIR)

set(NETCDF_LIBRARIES ${NETCDF_LIBRARY})
set(NETCDF_INCLUDE_DIRS ${NETCDF_INCLUDE_DIR})
set(NETCDF_DEFINITIONS ${_netcdf_cflags_other})
