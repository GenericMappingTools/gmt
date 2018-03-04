#
# $Id$
#
# Locate WebKit
#
# This module accepts the following environment variables:
#
#    WEBKIT_DIR or WEBKIT_ROOT - Specify the location of WEBKIT
#
# This module defines the following CMake variables:
#
#    WEBKIT_FOUND - True if the webkit2 library is found
#    WEBKIT_LIBRARY - A variable pointing to the WEBKIT library
#    WEBKIT_INCLUDE_DIR - Where to find the headers

#=============================================================================
# Copyright 2007-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See COPYING-CMAKE-SCRIPTS for more information.
#=============================================================================
# Note: this file is not an exact copy of the original file from Kitware.
#       It has been modified for the needs of GMT.

#
# $WEBKIT_DIR is an environment variable that would
# correspond to the ./configure --prefix=$WEBKIT_DIR
# used in building webkit.
#
# Derived from the GDAL cmake file by Eric Wing

# This makes the presumption that you are include webkit2.h like
#
#include "webkit2.h"

SET( GTK_VERSION "3.0")
SET( WEBKIT_VERSION "4.0")

if (DEFINED WEBKIT_ROOT AND NOT WEBKIT_ROOT)
	set (WEBKIT_LIBRARY "" CACHE INTERNAL "")
	set (WEBKIT_INCLUDE_DIR "" CACHE INTERNAL "")
	return()
endif (DEFINED WEBKIT_ROOT AND NOT WEBKIT_ROOT)

if (UNIX AND NOT WEBKIT_FOUND)
	find_program (WEBKIT_CONFIG pkg-config
		HINTS
		${WEBKIT_DIR}
		${WEBKIT_ROOT}
		$ENV{WEBKIT_DIR}
		$ENV{WEBKIT_ROOT}
		PATH_SUFFIXES bin
		PATHS
		/sw # Fink
		/opt/local # DarwinPorts
		/opt/csw # Blastwave
		/opt
		/usr/local
	)

	if (WEBKIT_CONFIG)
		execute_process (COMMAND ${WEBKIT_CONFIG} --cflags gtk+-${GTK_VERSION} webkit2gtk-${WEBKIT_VERSION}
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
			OUTPUT_VARIABLE WEBKIT_CONFIG_CFLAGS)
		if (WEBKIT_CONFIG_CFLAGS)
			string (REGEX MATCHALL "-I[^ ]+" _webkit_dashI ${WEBKIT_CONFIG_CFLAGS})
			string (REGEX REPLACE "-I" "" _webkit_includepath "${_webkit_dashI}")
			string (REGEX REPLACE "-I[^ ]+" "" _webkit_cflags_other ${WEBKIT_CONFIG_CFLAGS})
		endif (WEBKIT_CONFIG_CFLAGS)
		execute_process (COMMAND ${WEBKIT_CONFIG} --libs gtk+-${GTK_VERSION} webkit2gtk-${WEBKIT_VERSION}
			ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
			OUTPUT_VARIABLE WEBKIT_CONFIG_LIBS)
		if (WEBKIT_CONFIG_LIBS)
			string (REGEX MATCHALL "-l[^ ]+" _webkit_dashl ${WEBKIT_CONFIG_LIBS})
			string (REGEX REPLACE "-l" "" _webkit_lib "${_webkit_dashl}")
			string (REGEX MATCHALL "-L[^ ]+" _webkit_dashL ${WEBKIT_CONFIG_LIBS})
			string (REGEX REPLACE "-L" "" _webkit_libpath "${_webkit_dashL}")
		endif (WEBKIT_CONFIG_LIBS)
	endif (WEBKIT_CONFIG)
endif (UNIX AND NOT WEBKIT_FOUND)

# find all include dirs that pkg-config --cflags reported
foreach (_extrainc ${_webkit_includepath})
	list (APPEND WEBKIT_INCLUDE_DIR ${_webkit_includepath})
endforeach (_extrainc)

# find all libs that pkg-config --libs reported
foreach (_extralib ${_webkit_lib})
	find_library (_found_lib_${_extralib}
		NAMES ${_extralib}
		PATHS ${_webkit_libpath})
	list (APPEND WEBKITF_LIBRARY ${_found_lib_${_extralib}})
endforeach (_extralib)

set (WEBKIT_LIBRARIES ${WEBKIT_LIBRARY})
set (WEBKIT_INCLUDE_DIRS ${WEBKIT_INCLUDE_DIR})
