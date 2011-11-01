#
#	$Id$
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
#	See LICENSE.TXT file for copying and redistribution conditions.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; version 2 or any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
#
# To build out-of-source do (example):
#
#	mkdir build
#	cd build
#	cmake -DCMAKE_INSTALL_PREFIX=gmt5 -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
#
# CMAKE_BUILD_TYPE can be: empty, Debug, Release, RelWithDebInfo or MinSizeRel
#
# cmake creates a new file cmake/ConfigUser.cmake if it does not already
# exist. You can configure additional options there.
#

# Make sure the user doesn't play dirty with symlinks
get_filename_component (srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component (bindir "${CMAKE_BINARY_DIR}" REALPATH)

# Disallow in-source builds
if (${srcdir} STREQUAL ${bindir})
	message(FATAL_ERROR "In-source builds are not allowed. "
	"Please create a directory and run cmake from there, passing the path "
	"to this source directory as the last argument. This process created "
	"the file `CMakeCache.txt' and the directory `CMakeFiles' in ${srcdir}. "
	"Please remove them.")
endif (${srcdir} STREQUAL ${bindir})

# Define minimum CMake version required
cmake_minimum_required (VERSION 2.8.5)

# Define project name
project (GMT)

# Where to find our CMake modules (this variable is visible in subdirectories).
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/"
	CACHE INTERNAL "Location of our custom CMake modules.")

# Include configuration options (default options and options overridden by user).
include (ConfigCMake)

# Find UNIX commands
include (FindUnixCommands)

# Configure a header file to pass some of the CMake settings to the source code
configure_file (
	"${PROJECT_SOURCE_DIR}/config.h.cmake"
	"${PROJECT_BINARY_DIR}/config.h")

# Global test target
add_custom_target (check
	COMMAND ${CMAKE_CTEST_COMMAND}
	--force-new-ctest-process -j${N_TEST_JOBS})

# Find test dependencies
find_program (GRAPHICSMAGICK gm)

# Add subdirectories
set(_manfiles_ps "" CACHE INTERNAL "Global list of PS manpages")
set(_manfiles_suppl_ps "" CACHE INTERNAL "Global list of supplement PS manpages")
add_subdirectory (src)
add_subdirectory (share) # share must be processed *after* src (GSHHS_PATH)
add_subdirectory (doc)   # share must be processed *after* src (PDF manpages)
add_subdirectory (test)

# Configuration done
message(
	"*  Installing GMT in    : ${CMAKE_INSTALL_PREFIX}\n"
	"*  GMT_SHARE_PATH       : ${GMT_SHARE_PATH}\n"
	"*  GMT_DOC_PATH         : ${GMT_DOC_PATH}\n"
	"*  NetCDF library       : ${NETCDF_LIBRARY}\n"
	"*  NetCDF include dir   : ${NETCDF_INCLUDE_DIR}\n"
	"*  GDAL library         : ${GDAL_LIBRARY}\n"
	"*  GDAL include dir     : ${GDAL_INCLUDE_DIR}\n"
	"*  Regex support        : ${GMT_CONFIG_REGEX_MESSAGE}\n"
	"*  Triangulation method : ${GMT_TRIANGULATE}\n"
	"*  File locking         : ${FLOCK}\n"
	"*  Build Matlab API     : ${GMT_MATLAB}")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
