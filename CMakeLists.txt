#
#	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
#	Contact info: www.generic-mapping-tools.org
#-------------------------------------------------------------------------------
#
# To configure the cmake process:
#
# 1. copy the configuration template 'cmake/ConfigUserTemplate.cmake'
#    to 'cmake/ConfigUser.cmake', then edit 'cmake/ConfigUser.cmake' to
#    override basic default settings on a per-user basis;
# 2. copy the configuration template 'cmake/ConfigUserAdvancedTemplate.cmake'
#    to 'cmake/ConfigUserAdvanced.cmake', then edit 'cmake/ConfigUserAdvanced.cmake' to
#    override advanced default settings on a per-user basis.
#
# To build out-of-source do (example):
#
#	mkdir build
#	cd build
#	cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
#
# CMAKE_BUILD_TYPE can be: empty, Debug, Release, RelWithDebInfo or MinSizeRel
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
cmake_minimum_required (VERSION 2.8.7)

# Use NEW behavior with newer CMake releases
foreach (p
		CMP0025 # CMake 3.0: Compiler id for Apple Clang is now AppleClang
		CMP0026 # CMake 3.0: Disallow use of the LOCATION target property
		CMP0058 # CMake 3.3: Ninja requires custom command byproducts to be explicit
		CMP0074	# CMake 3.12: find_package uses PackageName_ROOT variables
		)
	if (POLICY ${p})
		cmake_policy (SET ${p} NEW)
	endif()
endforeach()

# Define project name and language
project (GMT C)

# Where to find our CMake modules (this variable is visible in subdirectories).
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/"
	CACHE INTERNAL "Location of our custom CMake modules." FORCE)

# Find UNIX commands
include (FindUnixCommands)
find_package (Git)

# Include configuration options (default options and options overridden by user).
include (ConfigCMake)

# Global test target
add_custom_target (check
	COMMAND ${CMAKE_CTEST_COMMAND}
	--output-on-failure --force-new-ctest-process -j${N_TEST_JOBS})

# Find test dependencies
find_program (GRAPHICSMAGICK gm)
if (DO_EXAMPLES OR DO_TESTS AND NOT GRAPHICSMAGICK)
	message (FATAL_ERROR "Cannot proceed without GraphicsMagick. "
		"Need to either install GraphicsMagick or disable tests.")
endif (DO_EXAMPLES OR DO_TESTS AND NOT GRAPHICSMAGICK)

# Add subdirectories
add_subdirectory (src)
add_subdirectory (share)      # share must be processed *after* src (GSHHG_PATH)
add_subdirectory (doc)        # doc must be processed *after* src
if (EXISTS ${GMT_SOURCE_DIR}/test/)
	add_subdirectory (test)
endif (EXISTS ${GMT_SOURCE_DIR}/test/)
add_subdirectory (cmake/dist) # make distribution bundles (always last)

# Source release target
if (GIT_FOUND AND HAVE_GIT_VERSION)
	# Export git working tree, and remove some directories and files
	# so that they are not included in the final release tarball
	add_custom_target (git_export_release
		COMMAND ${GIT_EXECUTABLE} -C ${GMT_SOURCE_DIR} checkout-index -a -f --prefix=${GMT_RELEASE_PREFIX}/
		# cmake<3.15 can't remove multiple directories using cmake -E remove_directory
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GMT_RELEASE_PREFIX}/.git
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GMT_RELEASE_PREFIX}/.github
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GMT_RELEASE_PREFIX}/admin
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GMT_RELEASE_PREFIX}/ci
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GMT_RELEASE_PREFIX}/test
		COMMAND ${CMAKE_COMMAND} -E remove ${GMT_RELEASE_PREFIX}/.azure-pipelines.yml
		COMMAND ${CMAKE_COMMAND} -E remove ${GMT_RELEASE_PREFIX}/.gitignore
	)
	add_depend_to_target (gmt_release git_export_release)
	find_program (GNUTAR NAMES gnutar gtar tar)
	find_program (XZ NAMES xz)
	if (GNUTAR AND GZIP AND XZ)
		# Targets for creating tarballs
		string (REGEX REPLACE ".*/" "" _release_dirname "${GMT_RELEASE_PREFIX}")
		add_custom_command (OUTPUT ${_release_dirname}-src.tar
			COMMAND ${GNUTAR} -c --owner 0 --group 0 --mode a=rX,u=rwX --force-local
			-f ${GMT_BINARY_DIR}/${_release_dirname}-src.tar ${_release_dirname}
			DEPENDS ${GMT_RELEASE_PREFIX}
			WORKING_DIRECTORY ${GMT_RELEASE_PREFIX}/..
			VERBATIM)
		add_custom_command (OUTPUT ${_release_dirname}-src.tar.gz
			COMMAND ${GZIP} -9 --keep --force ${GMT_BINARY_DIR}/${_release_dirname}-src.tar
			DEPENDS ${GMT_RELEASE_PREFIX} ${_release_dirname}-src.tar
			WORKING_DIRECTORY ${GMT_RELEASE_PREFIX}/..
			VERBATIM)
		add_custom_command (OUTPUT ${_release_dirname}-src.tar.xz
			COMMAND ${XZ} -9 -T 0 --keep --force ${GMT_BINARY_DIR}/${_release_dirname}-src.tar
			DEPENDS ${GMT_RELEASE_PREFIX} ${_release_dirname}-src.tar
			WORKING_DIRECTORY ${GMT_RELEASE_PREFIX}/..
			VERBATIM)
		add_custom_target (gmt_release_tar
			DEPENDS ${GMT_RELEASE_PREFIX}
			${_release_dirname}-src.tar.gz ${_release_dirname}-src.tar.xz)
	endif (GNUTAR AND GZIP AND XZ)
endif (GIT_FOUND AND HAVE_GIT_VERSION)

get_target_property (_name gmtlib OUTPUT_NAME)
get_target_property (_prefix gmtlib PREFIX)
if (BUILD_SHARED_LIBS)
    set (GMT_CORE_LIB_NAME ${_prefix}${_name}${CMAKE_SHARED_LIBRARY_SUFFIX})
else (BUILD_SHARED_LIBS)
    set (GMT_CORE_LIB_NAME ${_prefix}${_name}${CMAKE_STATIC_LIBRARY_SUFFIX})
endif (BUILD_SHARED_LIBS)

get_target_property (_name pslib OUTPUT_NAME)
get_target_property (_prefix pslib PREFIX)
if (BUILD_SHARED_LIBS)
    set (PSL_LIB_NAME ${_prefix}${_name}${CMAKE_SHARED_LIBRARY_SUFFIX})
else (BUILD_SHARED_LIBS)
    set (PSL_LIB_NAME ${_prefix}${_name}${CMAKE_STATIC_LIBRARY_SUFFIX})
endif (BUILD_SHARED_LIBS)

if (BUILD_SUPPLEMENTS)
	# Get name of shared supplemental library
	get_target_property (_name supplib OUTPUT_NAME)
	get_target_property (_prefix supplib PREFIX)
	set (GMT_SUPPL_LIB_NAME ${_prefix}${_name}${CMAKE_SHARED_MODULE_SUFFIX})
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

if (GMT_INSTALL_MODULE_LINKS)
	set (LINKS "yes")
else (GMT_INSTALL_MODULE_LINKS)
	set (LINKS "no")
endif (GMT_INSTALL_MODULE_LINKS)

# Configure header file to pass some of the CMake settings to the source code
configure_file (src/config.h.in src/config.h)

# Configuration done
message(
	"*\n"
	"*  GMT Version:               : ${GMT_PACKAGE_VERSION_WITH_GIT_REVISION}\n"
	"*\n"
	"*  Options:\n"
	"*  Found GSHHG database       : ${GSHHG_PATH} (${GSHHG_VERSION})\n"
	"*  Found DCW-GMT database     : ${DCW_PATH} (${DCW_VERSION})\n"
	"*  Found GMT data server      : ${GMT_DATA_SERVER}\n"
	"*  NetCDF library             : ${NETCDF_LIBRARY}\n"
	"*  NetCDF include dir         : ${NETCDF_INCLUDE_DIR}\n"
	"*  Curl library               : ${CURL_LIBRARY}\n"
	"*  Curl include dir           : ${CURL_INCLUDE_DIR}\n"
	"*  GDAL library               : ${GDAL_LIBRARY}\n"
	"*  GDAL include dir           : ${GDAL_INCLUDE_DIR}\n"
	"*  FFTW library               : ${FFTW3F_LIBRARY}\n"
	"*  FFTW threads library       : ${FFTW3F_THREADS_LIBRARY}\n"
	"*  FFTW include dir           : ${FFTW3_INCLUDE_DIR}\n"
	"*  Accelerate Framework       : ${ACCELERATE_FRAMEWORK}\n"
	"*  Regex support              : ${GMT_CONFIG_REGEX_MESSAGE}\n"
	"*  ZLIB library               : ${ZLIB_LIBRARY}\n"
	"*  ZLIB include dir           : ${ZLIB_INCLUDE_DIR}\n"
	"*  LAPACK library             : ${GMT_CONFIG_LAPACK_MESSAGE}\n"
	"*  BLAS library               : ${GMT_CONFIG_BLAS_MESSAGE}\n"
	"*  License restriction        : ${LICENSE_RESTRICTED}\n"
	"*  Triangulation method       : ${GMT_TRIANGULATE}\n"
	"*  OpenMP support             : ${GMT_CONFIG_OPENMP_MESSAGE}\n"
	"*  GLIB GTHREAD support       : ${GMT_CONFIG_GTHREAD_MESSAGE}\n"
	"*  Build generator            : ${CMAKE_GENERATOR}\n"
	"*  Build mode                 : ${BUILD_MODE}\n"
	"*  Build GMT core             : always [${GMT_CORE_LIB_NAME}]\n"
	"*  Build PSL library          : always [${PSL_LIB_NAME}]\n"
	"*  Build GMT supplements      : ${SUPPL}\n"
	"*  Build GMT for developers   : ${DEVEL}\n"
	"*  Build proto supplements    : ${PROTO}\n"
	"*  Build module links         : ${LINKS}\n"
	"*  Found Ghostscript (gs)     : ${GMT_CONFIG_GS_MESSAGE}\n"
	"*  Found GraphicsMagick (gm)  : ${GMT_CONFIG_GM_MESSAGE}\n"
	"*  Found ffmpeg               : ${GMT_CONFIG_FFMPEG_MESSAGE}\n"
	"*  Found open                 : ${GMT_CONFIG_OPEN_MESSAGE}\n"
	"*  Found ogr2ogr              : ${GMT_CONFIG_OGR2OGR_MESSAGE}\n"
	"*  Found gdal_translate       : ${GMT_CONFIG_GDAL_TRANSLATE_MESSAGE}\n"
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
