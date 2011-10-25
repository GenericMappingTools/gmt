#
# $Id$
#
# Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
#
# Use this file to override variables in 'ConfigDefault.cmake' on a per-user basis.
# First copy 'ConfigUserDefault.cmake' to 'ConfigUser.cmake', then edit 'ConfigUser.cmake'.
# 'ConfigUser.cmake' is not version controlled (currently listed in svn:ignore property)
#
# Note: CMake considers an empty string, "FALSE", "OFF", "NO", or any string ending
# in "-NOTFOUND" to be false. (This happens to be case-insensitive, so "False",
# "off", "no", and "something-NotFound" are all false.) Other values are true. Thus
# it matters not whether you use TRUE and FALSE, ON and OFF, or YES and NO for your
# booleans.
#

# Installation path [auto]:
#set (CMAKE_INSTALL_PREFIX "prefix_path")

# Set share installation path [${CMAKE_INSTALL_PREFIX}/share/gmt-<version>]:
#set (GMT_SHARE_PATH "share_path")

# Set doc installation path [${CMAKE_INSTALL_PREFIX}/share/doc/gmt-<version>]:
#set (GMT_DOC_PATH "doc_path")

# Set path to GSHHS Shoreline Database [auto]:
#set (GSHHS_ROOT "gshhs_path")

# Set build type can be: empty, Debug, Release, RelWithDebInfo or MinSizeRel [Release]:
#set (CMAKE_BUILD_TYPE Debug)

# Set location of NetCDF (can be root directory, path to header file or path to nc-config) [auto]:
#set (NETCDF_DIR "netcdf_install_prefix")

# Set location of GDAL (can be root directory, path to header file or path to gdal-config) [auto]:
#set (GDAL_DIR "gdal_install_prefix")

# Enable compatibility mode [FALSE]:
#set (GMT_COMPAT TRUE)

# Enable file locking [FALSE]:
#set (FLOCK TRUE)

# Use fast, non-GPL triangulation routine by J. Shewchuk [FALSE]:
#set (TRIANGLE_D TRUE)

# Enable Matlab API [FALSE]:
#set (GMT_MATLAB TRUE)
# If Matlab is not found, point MATLAB_ROOT to its installation path, e.g.:
#set (MATLAB_ROOT /Applications/MATLAB_R2010a.app)  # MacOSX
#set (MATLAB_ROOT /opt/matlab-7sp1)                 # Linux

# Enable running examples/tests with "make check" (out-of-source)
# Need to set either DO_EXAMPLES, DO_TESTS or both and uncomment
# the following line:
#enable_testing()
#set (DO_EXAMPLES TRUE)
#set (DO_TESTS TRUE)
#set (N_TEST_JOBS 4)

# Extra debugging for developers:
#add_definitions(-DDEBUG)
#set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement") # recommended even for release build
#set (CMAKE_C_FLAGS "-Wextra ${CMAKE_C_FLAGS}")            # extra warnings
#set (CMAKE_C_FLAGS_DEBUG -ggdb3)                          # gdb debugging symbols
#set (CMAKE_C_FLAGS_RELEASE "-ggdb3 -O2 -Wuninitialized")  # check uninitialized variables
#set (CMAKE_LINK_DEPENDS_DEBUG_MODE TRUE)                  # debug link dependencies

# This is for gcc on Solaris to avoid "relocations remain against
# allocatable but non-writable sections" problems:
#set (USER_GMTLIB_LINK_FLAGS -mimpure-text)

# If your NetCDF library is static (not recommended, applies to Windows only)
#set (NETCDF_STATIC TRUE)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
