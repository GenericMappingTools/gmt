#
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
# See COPYING-CMAKE-SCRIPTS for more information.
#=============================================================================

# This makes the presumption that you are include netcdf.h like
#
#include "netcdf.h"

if (UNIX AND NOT NETCDF_FOUND)
	# Use nc-config to obtain the libraries
	find_program (NETCDF_CONFIG nc-config
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
		/usr/local
	)

	if (NETCDF_CONFIG)
		execute_process (COMMAND ${NETCDF_CONFIG} --cflags
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
			OUTPUT_VARIABLE NETCDF_CONFIG_CFLAGS)
		if (NETCDF_CONFIG_CFLAGS)
			string (REGEX MATCHALL "(^| )-I[^ ]+" _netcdf_dashI ${NETCDF_CONFIG_CFLAGS})
			string (REGEX REPLACE "(^| )-I" "" _netcdf_includepath "${_netcdf_dashI}")
			string (REGEX REPLACE "(^| )-I[^ ]+" "" _netcdf_cflags_other ${NETCDF_CONFIG_CFLAGS})
		endif (NETCDF_CONFIG_CFLAGS)
		execute_process (COMMAND ${NETCDF_CONFIG} --libs
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
			OUTPUT_VARIABLE NETCDF_CONFIG_LIBS)
		if (NETCDF_CONFIG_LIBS)
			# Ensure -l is precedeced by whitespace to not match
			# '-l' in '-L/usr/lib/x86_64-linux-gnu/hdf5/serial'
			string (REGEX MATCHALL "(^| )-l[^ ]+" _netcdf_dashl ${NETCDF_CONFIG_LIBS})
			string (REGEX REPLACE "(^| )-l" "" _netcdf_lib "${_netcdf_dashl}")
			string (REGEX MATCHALL "(^| )-L[^ ]+" _netcdf_dashL ${NETCDF_CONFIG_LIBS})
			string (REGEX REPLACE "(^| )-L" "" _netcdf_libpath "${_netcdf_dashL}")
		endif (NETCDF_CONFIG_LIBS)
	endif (NETCDF_CONFIG)
	if (_netcdf_lib)
		list (REMOVE_DUPLICATES _netcdf_lib)
		list (REMOVE_ITEM _netcdf_lib netcdf)
	endif (_netcdf_lib)
endif (UNIX AND NOT NETCDF_FOUND)

find_path (NETCDF_INCLUDE_DIR netcdf.h
	HINTS
	${_netcdf_includepath}
	${NETCDF_DIR}
	${NETCDF_ROOT}
	$ENV{NETCDF_DIR}
	$ENV{NETCDF_ROOT}
	PATH_SUFFIXES
	include/netcdf
	include/netcdf-4
	include/netcdf-3
	include
	PATHS
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	/usr/local
)

find_library (NETCDF_LIBRARY
	NAMES netcdf
	HINTS
	${_netcdf_libpath}
	${NETCDF_DIR}
	${NETCDF_ROOT}
	$ENV{NETCDF_DIR}
	$ENV{NETCDF_ROOT}
	PATH_SUFFIXES lib
	PATHS
	/sw
	/opt/local
	/opt/csw
	/opt
	/usr/local
)

# find all libs that nc-config reports
foreach (_extralib ${_netcdf_lib})
	find_library (_found_lib_${_extralib}
		NAMES ${_extralib}
		PATHS ${_netcdf_libpath})
	list (APPEND NETCDF_LIBRARY ${_found_lib_${_extralib}})
endforeach (_extralib)

if (NETCDF_LIBRARY AND NETCDF_INCLUDE_DIR AND NOT HAVE_NETCDF4)
	# Ensure that NetCDF with version 4 extensions is installed
	include (CMakePushCheckState)
	include (CheckSymbolExists)
	cmake_push_check_state() # save state of CMAKE_REQUIRED_*
	set (CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${NETCDF_INCLUDE_DIR})
	set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${NETCDF_LIBRARY})
	set (HAVE_NETCDF4 HAVE_NETCDF4) # to force check_symbol_exists again
	check_symbol_exists (nc_def_var_deflate netcdf.h HAVE_NETCDF4)
	cmake_pop_check_state() # restore state of CMAKE_REQUIRED_*
	if (NOT HAVE_NETCDF4)
		message (SEND_ERROR "Library found but netCDF-4/HDF5 format unsupported. Do not configure netCDF-4 with --disable-netcdf-4.")
	endif (NOT HAVE_NETCDF4)
endif (NETCDF_LIBRARY AND NETCDF_INCLUDE_DIR AND NOT HAVE_NETCDF4)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (NETCDF
	DEFAULT_MSG NETCDF_LIBRARY NETCDF_INCLUDE_DIR HAVE_NETCDF4)

set (NETCDF_LIBRARIES ${NETCDF_LIBRARY})
set (NETCDF_INCLUDE_DIRS ${NETCDF_INCLUDE_DIR})
string (REPLACE "-DNDEBUG" "" NETCDF_DEFINITIONS "${_netcdf_cflags_other}")
