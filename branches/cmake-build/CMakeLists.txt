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
get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

# Disallow in-source builds
if(${srcdir} STREQUAL ${bindir})
  message(FATAL_ERROR "In-source builds are not allowed. "
  "Please create a directory and run cmake from there, passing the path "
  "to this source directory as the last argument. This process created "
  "the file `CMakeCache.txt' and the directory `CMakeFiles' in ${srcdir}. "
  "Please remove them.")
endif(${srcdir} STREQUAL ${bindir})

# Define minimum CMake version required
cmake_minimum_required( VERSION 2.8 )

# Define project name
project( GMT )

# Where to find our CMake modules (this variable is visible in subdirectories).
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

# Include configuration options (default options and options overridden by user).
include (ConfigCMake)

# Configure a header file to pass some of the CMake settings to the source code
configure_file(
  "${PROJECT_SOURCE_DIR}/config.h.cmake"
  "${PROJECT_BINARY_DIR}/config.h")

# Add subdirectories
add_subdirectory( src )
add_subdirectory( share )

# Configuration done
message(
  "** Installing GMT in ${CMAKE_INSTALL_PREFIX}\n"
  "** GMT_SHARE_PATH: ${GMT_SHARE_PATH}\n"
  "** GDAL: ${HAVE_GDAL}\n"
  "** Triangulation method: ${TRIANGULATE}\n"
  "** File locking: ${FLOCK}"
  )
