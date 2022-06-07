#
#	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
cmake_minimum_required (VERSION 2.8.12)
message ("CMake version: ${CMAKE_VERSION}")

# Use NEW behavior with newer CMake releases
foreach (p
		CMP0025 # CMake 3.0: Compiler id for Apple Clang is now AppleClang
		CMP0026 # CMake 3.0: Disallow use of the LOCATION target property
		CMP0058 # CMake 3.3: Ninja requires custom command byproducts to be explicit
		CMP0074	# CMake 3.12: find_package uses PackageName_ROOT variables
		CMP0115 # CMake 3.20: Source file extensions must be explicit.
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

# Find latex and dvips for LaTeX integration
find_program (LATEX latex)
find_program (DVIPS dvips)

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
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GMT_RELEASE_PREFIX}/.dvc
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GMT_RELEASE_PREFIX}/.git
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GMT_RELEASE_PREFIX}/.github
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GMT_RELEASE_PREFIX}/admin
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GMT_RELEASE_PREFIX}/ci
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${GMT_RELEASE_PREFIX}/test
		COMMAND ${CMAKE_COMMAND} -E remove ${GMT_RELEASE_PREFIX}/.gitignore
		COMMAND ${CMAKE_COMMAND} -E remove ${GMT_RELEASE_PREFIX}/doc/examples/.gitignore
		COMMAND ${CMAKE_COMMAND} -E remove ${GMT_RELEASE_PREFIX}/doc/examples/images.dvc
		COMMAND ${CMAKE_COMMAND} -E remove ${GMT_RELEASE_PREFIX}/doc/scripts/.gitignore
		COMMAND ${CMAKE_COMMAND} -E remove ${GMT_RELEASE_PREFIX}/doc/scripts/images.dvc
	)
	add_depend_to_target (gmt_release git_export_release)
	find_program (GNUTAR NAMES gnutar gtar tar)
	find_program (XZ NAMES xz)
	if (GNUTAR AND GZIP AND XZ)
		# Targets for creating tarballs
		get_filename_component (_release_dirname "${GMT_RELEASE_PREFIX}" NAME)
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

# Get suffix for libraries
set (_lib_suffix ${CMAKE_SHARED_LIBRARY_SUFFIX})

# Get name of GMT core library
get_target_property (_name gmtlib RUNTIME_OUTPUT_NAME)
get_target_property (_prefix gmtlib PREFIX)
set (GMT_CORE_LIB_NAME ${_prefix}${_name}${_lib_suffix})

# Get name of PSL library
get_target_property (_name pslib RUNTIME_OUTPUT_NAME)
get_target_property (_prefix pslib PREFIX)
set (PSL_LIB_NAME ${_prefix}${_name}${_lib_suffix})

# Get name of shared supplemental library
if (BUILD_SUPPLEMENTS)
	get_target_property (_name ${GMT_SUPPL_LIB_NAME} RUNTIME_OUTPUT_NAME)
	get_target_property (_prefix ${GMT_SUPPL_LIB_NAME} PREFIX)
	set (GMT_SUPPL_LIBRARY ${_prefix}${_name}${CMAKE_SHARED_MODULE_SUFFIX})
	set (SUPPL "yes [${GMT_SUPPL_LIBRARY}]")
else (BUILD_SUPPLEMENTS)
	set (SUPPL "no")
endif (BUILD_SUPPLEMENTS)

if (BUILD_DEVELOPER)
	set (DEVEL "yes")
else (BUILD_DEVELOPER)
	set (DEVEL "no")
endif (BUILD_DEVELOPER)

if (SUPPL_EXTRA_DIRS)
	set (PROTO ${SUPPL_EXTRA_DIRS})
else (SUPPL_EXTRA_DIRS)
	set (PROTO "none")
endif (SUPPL_EXTRA_DIRS)

if (GMT_INSTALL_MODULE_LINKS)
	set (LINKS "yes")
else (GMT_INSTALL_MODULE_LINKS)
	set (LINKS "no")
endif (GMT_INSTALL_MODULE_LINKS)

# Configure header file to pass some of the CMake settings to the source code
configure_file (src/config.h.in src/config.h)

if (DCW_PATH)
	set (GMT_CONFIG_DCW_MESSAGE "${DCW_PATH} (${DCW_VERSION})")
else (DCW_PATH)
	set (GMT_CONFIG_DCW_MESSAGE "${DCW_PATH} (GMT will download on demand)")
endif (DCW_PATH)
if (GSHHG_PATH)
	set (GMT_CONFIG_GSHHG_MESSAGE "${GSHHG_PATH} (${GSHHG_VERSION})")
else (GSHHG_PATH)
	set (GMT_CONFIG_GSHHG_MESSAGE "${GSHHG_PATH} (GMT will download on demand)")
endif (GSHHG_PATH)

# Configuration done
message(
	"*\n"
	"*  GMT Version:               : ${GMT_PACKAGE_VERSION_WITH_GIT_REVISION}\n"
	"*\n"
	"*  Options:\n"
	"*  Found GSHHG database       : ${GMT_CONFIG_GSHHG_MESSAGE}\n"
	"*  Found DCW-GMT database     : ${GMT_CONFIG_DCW_MESSAGE}\n"
	"*  Found GMT data server      : ${GMT_DATA_SERVER}\n"
	"*  NetCDF library             : ${NETCDF_LIBRARY}\n"
	"*  NetCDF include dir         : ${NETCDF_INCLUDE_DIR}\n"
	"*  Curl library               : ${CURL_LIBRARY}\n"
	"*  Curl include dir           : ${CURL_INCLUDE_DIR}\n"
	"*  GDAL library               : ${GDAL_LIBRARY}\n"
	"*  GDAL include dir           : ${GDAL_INCLUDE_DIR}\n"
	"*  GEOS library               : ${GEOS_LIBRARY}\n"
	"*  GEOS include dir           : ${GEOS_INCLUDE_DIR}\n"
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
