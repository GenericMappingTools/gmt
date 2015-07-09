#
#	$Id$
#
#	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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
# To modify the cmake process: Edit your cmake/ConfigUser.cmake file
#
# To build out-of-source do (example):
#
#	mkdir build
#	cd build
#	cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
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

# Use NEW behavior with newer CMake releases
foreach(p
		CMP0025 # CMake 3.0: Compiler id for Apple Clang is now AppleClang
		)
	if(POLICY ${p})
		cmake_policy(SET ${p} NEW)
	endif()
endforeach()

# Use OLD behavior with newer CMake releases
foreach(p
		CMP0026 # CMake 3.0: Disallow use of the LOCATION target property
		)
	if(POLICY ${p})
		cmake_policy(SET ${p} OLD)
	endif()
endforeach()

# Define project name and language
project (GMT C)

# Where to find our CMake modules (this variable is visible in subdirectories).
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/"
	CACHE INTERNAL "Location of our custom CMake modules." FORCE)

# Include configuration options (default options and options overridden by user).
include (ConfigCMake)

# Find UNIX commands
include (FindUnixCommands)
find_program (SVN svn)

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
#set(_manfiles_ps "" CACHE INTERNAL "Global list of PS manpages")
add_subdirectory (src)
add_subdirectory (share)      # share must be processed *after* src (GSHHG_PATH)
add_subdirectory (doc)        # share must be processed *after* src (PDF manpages)
add_subdirectory (test)
add_subdirectory (cmake/dist) # make distribution bundles (always last)

# Source release target
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
		add_custom_command (OUTPUT ${_release_dirname}-src.tar.gz
			COMMAND ${GNUTAR} -cz --owner 0 --group 0 --mode a=rX,u=rwX
			-f ${GMT_BINARY_DIR}/${_release_dirname}-src.tar.gz ${_release_dirname}
			DEPENDS ${GMT_RELEASE_PREFIX}
			WORKING_DIRECTORY ${GMT_RELEASE_PREFIX}/..
			VERBATIM)
		add_custom_command (OUTPUT ${_release_dirname}-src.tar.bz2
			COMMAND ${GNUTAR} -cj --owner 0 --group 0 --mode a=rX,u=rwX
			-f ${GMT_BINARY_DIR}/${_release_dirname}-src.tar.bz2 ${_release_dirname}
			DEPENDS ${GMT_RELEASE_PREFIX}
			WORKING_DIRECTORY ${GMT_RELEASE_PREFIX}/..
			VERBATIM)
		add_custom_target (gmt_release_tar
			DEPENDS ${GMT_RELEASE_PREFIX}
			${_release_dirname}-src.tar.gz ${_release_dirname}-src.tar.bz2)
	endif (GNUTAR)
endif (SVN AND HAVE_SVN_VERSION)

get_target_property (_location gmtlib LOCATION)
get_filename_component (GMT_CORE_LIB_NAME ${_location} NAME)

get_target_property (_location pslib LOCATION)
get_filename_component (PSL_LIB_NAME ${_location} NAME)

if (BUILD_SUPPLEMENTS)
	# Get name of shared supplemental library
	get_target_property (_location supplib LOCATION)
	get_filename_component (GMT_SUPPL_LIB_NAME ${_location} NAME)
	set (SUPPL "yes [${GMT_SUPPL_LIB_NAME}]")
else (BUILD_SUPPLEMENTS)
	set (SUPPL "no")
endif (BUILD_SUPPLEMENTS)

if (BUILD_DEVELOPER)
	set (DEVEL "yes")
else (BUILD_DEVELOPER)
	set (DEVEL "no")
endif (BUILD_DEVELOPER)

if (BUILD_SHARED_LIBS)
	set (BUILD_MODE "shared")
else (BUILD_SHARED_LIBS)
	set (BUILD_MODE "static")
endif (BUILD_SHARED_LIBS)

if (EXTRA_BUILD_DIRS)
	set (PROTO ${EXTRA_BUILD_DIRS})
else (EXTRA_BUILD_DIRS)
	set (PROTO "none")
endif (EXTRA_BUILD_DIRS)

# Configure header file to pass some of the CMake settings to the source code
configure_file (src/config.h.in src/config.h)

# Configuration done
message(
	"*  Options:\n"
	"*  Found GSHHG database       : ${GSHHG_PATH} (${GSHHG_VERSION})\n"
	"*  Found DCW-GMT database     : ${DCW_PATH}\n"
	"*  NetCDF library             : ${NETCDF_LIBRARY}\n"
	"*  NetCDF include dir         : ${NETCDF_INCLUDE_DIR}\n"
	"*  GDAL library               : ${GDAL_LIBRARY}\n"
	"*  GDAL include dir           : ${GDAL_INCLUDE_DIR}\n"
	"*  FFTW library               : ${FFTW3F_LIBRARY}\n"
	"*  FFTW include dir           : ${FFTW3_INCLUDE_DIR}\n"
	"*  Accelerate Framework       : ${ACCELERATE_FRAMEWORK}\n"
	"*  Regex support              : ${GMT_CONFIG_REGEX_MESSAGE}\n"
	"*  ZLIB library               : ${ZLIB_LIBRARY}\n"
	"*  ZLIB include dir           : ${ZLIB_INCLUDE_DIR}\n"
	"*  LAPACK library             : ${GMT_CONFIG_LAPACK_MESSAGE}\n"
	"*  License restriction        : ${LICENSE_RESTRICTED}\n"
	"*  Triangulation method       : ${GMT_TRIANGULATE}\n"
	"*  OpenMP support             : ${GMT_CONFIG_OPENMP_MESSAGE}\n"
	"*  GLIB GTHREAD support       : ${GMT_CONFIG_GTHREAD_MESSAGE}\n"
	"*  PTHREAD support            : ${GMT_CONFIG_PTHREAD_MESSAGE}\n"
	"*  Build mode                 : ${BUILD_MODE}\n"
	"*  Build GMT core             : always [${GMT_CORE_LIB_NAME}]\n"
	"*  Build PSL library          : always [${PSL_LIB_NAME}]\n"
	"*  Build GMT supplements      : ${SUPPL}\n"
	"*  Build GMT Developer        : ${DEVEL}\n"
	"*  Build proto supplements    : ${PROTO}\n"
	"*\n"
	"*  Locations:\n"
	"*  Installing GMT in          : ${CMAKE_INSTALL_PREFIX}\n"
	"*  GMT_DATADIR                : ${CMAKE_INSTALL_PREFIX}/${GMT_DATADIR}\n"
	"*  GMT_DOCDIR                 : ${CMAKE_INSTALL_PREFIX}/${GMT_DOCDIR}\n"
	"*  GMT_MANDIR                 : ${CMAKE_INSTALL_PREFIX}/${GMT_MANDIR}")

# For debugging: print all set variables
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#	message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
