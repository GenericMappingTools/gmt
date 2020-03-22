#
#
# Locate GMT
#
# This module accepts the following environment variables:
#
#    GMT_DIR or GMT_ROOT - Specify the location of GMT
#
# This module defines the following CMake variables:
#
#    GMT_FOUND - True if libgmt is found
#    GMT_LIBRARY - A variable pointing to the GMT library
#    GMT_INCLUDE_DIR - Where to find the headers
#    GMT_INCLUDE_DIRS - Where to find the headers
#    GMT_DEFINITIONS - Extra compiler flags

#=============================================================================
# Inspired by FindGDAL
#
# Distributed under the OSI-approved bsd license (the "License")
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See COPYING-CMAKE-SCRIPTS for more information.
#=============================================================================

# This makes the presumption that you include gmt.h like
#
#include "gmt.h"

if (UNIX AND NOT GMT_FOUND)
	# Use gmt-config to obtain the libraries
	find_program (GMT_CONFIG gmt-config
		HINTS
		${GMT_DIR}
		${GMT_ROOT}
		$ENV{GMT_DIR}
		$ENV{GMT_ROOT}
		PATH_SUFFIXES bin
		PATHS
		/sw # Fink
		/opt/local # DarwinPorts
		/opt/csw # Blastwave
		/opt
		/usr/local
	)

	if (GMT_CONFIG)
		execute_process (COMMAND ${GMT_CONFIG} --cflags
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
			OUTPUT_VARIABLE GMT_CONFIG_CFLAGS)
		if (GMT_CONFIG_CFLAGS)
			string (REGEX MATCHALL "(^| )-I[^ ]+" _gmt_dashI ${GMT_CONFIG_CFLAGS})
			string (REGEX REPLACE "(^| )-I" "" _gmt_includepath "${_gmt_dashI}")
			string (REGEX REPLACE "(^| )-I[^ ]+" "" _gmt_cflags_other ${GMT_CONFIG_CFLAGS})
		endif (GMT_CONFIG_CFLAGS)
		execute_process (COMMAND ${GMT_CONFIG} --libs
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
			OUTPUT_VARIABLE GMT_CONFIG_LIBS)
		if (GMT_CONFIG_LIBS)
			# Ensure -l is precedeced by whitespace to not match
			# '-l' in '-L/usr/lib/x86_64-linux-gnu/hdf5/serial'
			string (REGEX MATCHALL "(^| )-l[^ ]+" _gmt_dashl ${GMT_CONFIG_LIBS})
			string (REGEX REPLACE "(^| )-l" "" _gmt_lib "${_gmt_dashl}")
			string (REGEX MATCHALL "(^| )-L[^ ]+" _gmt_dashL ${GMT_CONFIG_LIBS})
			string (REGEX REPLACE "(^| )-L" "" _gmt_libpath "${_gmt_dashL}")
		endif (GMT_CONFIG_LIBS)
	endif (GMT_CONFIG)
	if (_gmt_lib)
		list (REMOVE_DUPLICATES _gmt_lib)
		list (REMOVE_ITEM _gmt_lib gmt)
	endif (_gmt_lib)
endif (UNIX AND NOT GMT_FOUND)

find_path (GMT_INCLUDE_DIR gmt.h
	HINTS
	${_gmt_includepath}
	${GMT_DIR}
	${GMT_ROOT}
	$ENV{GMT_DIR}
	$ENV{GMT_ROOT}
	PATH_SUFFIXES
	include/gmt
	include
	PATHS
	/sw # Fink
	/opt/local # DarwinPorts
	/opt/csw # Blastwave
	/opt
	/usr/local
)

find_library (GMT_LIBRARY
	NAMES gmt
	HINTS
	${_gmt_libpath}
	${GMT_DIR}
	${GMT_ROOT}
	$ENV{GMT_DIR}
	$ENV{GMT_ROOT}
	PATH_SUFFIXES lib
	PATHS
	/sw
	/opt/local
	/opt/csw
	/opt
	/usr/local
)

# find all libs that gmt-config reports
foreach (_extralib ${_gmt_lib})
	find_library (_found_lib_${_extralib}
		NAMES ${_extralib}
		PATHS ${_gmt_libpath})
	list (APPEND GMT_LIBRARY ${_found_lib_${_extralib}})
endforeach (_extralib)

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (GMT
	DEFAULT_MSG GMT_LIBRARY GMT_INCLUDE_DIR)

set (GMT_LIBRARIES ${GMT_LIBRARY})
set (GMT_INCLUDE_DIRS ${GMT_INCLUDE_DIR})
string (REPLACE "-DNDEBUG" "" GMT_DEFINITIONS "${_gmt_cflags_other}")
