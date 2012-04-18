#
#	$Id$
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
#	See LICENSE.TXT file for copying and redistribution conditions.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU Lesser General Public License as published by
#	the Free Software Foundation; version 3 or any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU Lesser General Public License for more details.
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
	CACHE INTERNAL "Location of our custom CMake modules." FORCE)

# Include configuration options (default options and options overridden by user).
include (ConfigCMake)

# Find UNIX commands
include (FindUnixCommands)

# Global test target
add_custom_target (check
	COMMAND ${CMAKE_CTEST_COMMAND}
	--force-new-ctest-process -j${N_TEST_JOBS})

# Find test dependencies
find_program (GRAPHICSMAGICK gm)
if (DO_EXAMPLES OR DO_TESTS AND NOT GRAPHICSMAGICK)
	message (FATAL_ERROR "Cannot proceed without GraphicsMagick. "
		"Need to either install GraphicsMagick or disable tests.")
endif (DO_EXAMPLES OR DO_TESTS AND NOT GRAPHICSMAGICK)

# Add subdirectories
set(_manfiles_ps "" CACHE INTERNAL "Global list of PS manpages")
set(_manfiles_suppl_ps "" CACHE INTERNAL "Global list of supplement PS manpages")
add_subdirectory (src)
add_subdirectory (share)      # share must be processed *after* src (GSHHG_PATH)
add_subdirectory (doc)        # share must be processed *after* src (PDF manpages)
add_subdirectory (test)
add_subdirectory (cmake/dist) # make distribution bundles (always last)

# Source release target
find_program (SVN svn)
if (SVN AND HAVE_SVN_VERSION)
	# Export svn working tree
	add_custom_target (svn_export_release
		COMMAND ${SVN} --force export
		${GMT_SOURCE_DIR} ${GMT_RELEASE_PREFIX})
	add_depend_to_target (gmt_release svn_export_release)
	find_program (GNUTAR NAMES gnutar gtar tar)
	if (GNUTAR)
		# Targets for creating tarballs
		string (REGEX REPLACE ".*/" "" _release_dirname "${GMT_RELEASE_PREFIX}")
		add_custom_command (OUTPUT ${_release_dirname}.tar.gz
			COMMAND ${GNUTAR} -cz --owner 0 --group 0 --mode a=rX,u=rwX
			-f ${GMT_BINARY_DIR}/${_release_dirname}.tar.gz ${_release_dirname}
			DEPENDS ${GMT_RELEASE_PREFIX}
			WORKING_DIRECTORY ${GMT_RELEASE_PREFIX}/..
			VERBATIM)
		add_custom_command (OUTPUT ${_release_dirname}.tar.bz2
			COMMAND ${GNUTAR} -cj --owner 0 --group 0 --mode a=rX,u=rwX
			-f ${GMT_BINARY_DIR}/${_release_dirname}.tar.bz2 ${_release_dirname}
			DEPENDS ${GMT_RELEASE_PREFIX}
			WORKING_DIRECTORY ${GMT_RELEASE_PREFIX}/..
			VERBATIM)
		add_custom_target (gmt_release_tar
			DEPENDS ${GMT_RELEASE_PREFIX}
			${_release_dirname}.tar.gz ${_release_dirname}.tar.bz2)
	endif (GNUTAR)
endif (SVN AND HAVE_SVN_VERSION)

# Configure a header file to pass some of the CMake settings to the source code
configure_file (
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h")

# Configuration done
message(
	"*  Options:\n"
	"*  Found GSHHG database : ${GSHHG_PATH} (${GSHHG_VERSION})\n"
	"*  NetCDF library       : ${NETCDF_LIBRARY}\n"
	"*  NetCDF include dir   : ${NETCDF_INCLUDE_DIR}\n"
	"*  GDAL library         : ${GDAL_LIBRARY}\n"
	"*  GDAL include dir     : ${GDAL_INCLUDE_DIR}\n"
	"*  Regex support        : ${GMT_CONFIG_REGEX_MESSAGE}\n"
	"*  Triangulation method : ${GMT_TRIANGULATE}\n"
	"*  File locking         : ${FLOCK}\n"
	"*  Compatibility mode   : ${GMT_COMPAT}\n"
	"*  Build Matlab API     : ${GMT_MATLAB}\n"
	"*\n"
	"*  Locations:\n"
	"*  Installing GMT in    : ${CMAKE_INSTALL_PREFIX}\n"
	"*  GMT_SHARE_PATH       : ${CMAKE_INSTALL_PREFIX}/${GMT_SHARE_PATH}\n"
	"*  GMT_DOC_PATH         : ${CMAKE_INSTALL_PREFIX}/${GMT_DOC_PATH}\n"
	"*  GMT_MAN_PATH         : ${CMAKE_INSTALL_PREFIX}/${GMT_MAN_PATH}")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
