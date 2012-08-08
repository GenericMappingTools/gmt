# $Id$
#
# Copyright (c) 2012, Florian Wobbe
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

# Define minimum CMake version required
cmake_minimum_required (VERSION 2.8.5)

# Warning
if (NOT WIN32)
	message (WARNING "netCDF will most probably not build with CMake on this"
		" system. You have been warned. Consider requesting general CMake supprort at"
		" netcdf-porting@unidata.ucar.edu and netcdfgroup@unidata.ucar.edu")
endif()

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

# Define project name
project (NETCDF)

# Include defaults
include (${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake)

# Where to find our CMake modules (this variable is visible in subdirectories).
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/"
  CACHE INTERNAL "Location of our custom CMake modules.")

# get_subdir_var_files (VARIABLE VAR_NAME DIR [ DIR ... ])
# example: get_subdir_var_files (SUB_LIB_SRCS LIB_SRCS ${SUB_DIRS})
macro (GET_SUBDIR_VAR_FILES VARIABLE VAR_NAME DIR_NAME)
	set (${VARIABLE}) # clear VARIABLE
	foreach (_dir ${DIR_NAME} ${ARGN})
		# get value of variable ${VAR_NAME} in dir ${_dir}:
		get_directory_property (_files DIRECTORY ${_dir} DEFINITION ${VAR_NAME})
		foreach (_file ${_files})
			# prepend dirname:
			list (APPEND ${VARIABLE} "${_dir}/${_file}")
		endforeach (_file)
	endforeach(_dir)
endmacro (GET_SUBDIR_VAR_FILES VARIABLE VAR_NAME DIR_NAME)

# Add subdirectories
foreach (_dir include fortran libdap2 libdispatch liblib libsrc libsrc4 ncdump
		ncgen nf_test oc)
	add_subdirectory (${_dir})
endforeach(_dir)

# Get list of sources for libnetcdf
get_subdir_var_files (SRCS_NETCDF SRCS_NETCDF include fortran libdap2 libdispatch liblib
	libsrc libsrc4 nf_test oc)

# Find libraries
find_package (CURL REQUIRED)
find_package (HDF4 REQUIRED)
find_package (HDF5 REQUIRED)
#find_package (SZIP REQUIRED) # provided by HDF5-config
find_package (ZLIB REQUIRED)
find_package (XDR REQUIRED)

# Extra include directories
set (EXTRA_INCLUDE_DIRECTORIES ${CURL_INCLUDE_DIRS} ${HDF4_INCLUDE_DIRS}
	${HDF5_INCLUDE_DIRS} ${SZIP_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS}
	${XDR_INCLUDE_DIRS})
list (REMOVE_DUPLICATES EXTRA_INCLUDE_DIRECTORIES)
include_directories (include fortran libdispatch oc
	libsrc libsrc4 ${EXTRA_INCLUDE_DIRECTORIES} ${NETCDF_BINARY_DIR})

# Required libraries
list (REMOVE_ITEM HDF4_LIBRARIES xdr) # we use our own xdr
set (LINK_LIBRARIES ${CURL_LIBRARIES} ${HDF4_LIBRARIES} ${HDF5_LIBRARIES}
	${SZIP_LIBRARIES} ${ZLIB_LIBRARIES} ${XDR_LIBRARIES})

if (WIN32)
  include_directories (win32)
	add_definitions (-DDLL_NETCDF -DNC_DLL_EXPORT)
	# TODO: definitions
	# WIN32;NDEBUG;_WINDOWS;CURL_STATICLIB;NETCDF_EXPORTS;DLL_NETCDF;DLL_EXPORT;pgiFortran;NC_DLL_EXPORT;USE_DISPATCH;H5_HAVE_WINDOWS;USE_NETCDF4
	add_definitions (-DHAVE_CONFIG_H -DpgiFortran -DUSE_DISPATCH
		-DH5_HAVE_WINDOWS -DUSE_NETCDF4)
endif (WIN32)

# Get version string from configure, e.g.: PACKAGE_VERSION='4.1.3'
if( EXISTS "${NETCDF_SOURCE_DIR}/configure" )
	file( STRINGS "${NETCDF_SOURCE_DIR}/configure"
		PACKAGE_VERSION
		REGEX "PACKAGE_VERSION=" )
	string (REGEX REPLACE "[^0-9]*([0-9.]+).*" "\\1" PACKAGE_VERSION ${PACKAGE_VERSION})
	string (REGEX REPLACE "([0-9]+)[.].*" "\\1" PACKAGE_VERSION_MAJOR ${PACKAGE_VERSION})
endif()
if (NOT PACKAGE_VERSION)
	message (FATAL_ERROR "Could not determine netCDF version")
endif()

configure_file (config.h.cmake config.h @ONLY)

# prefer shared libs over static
set (BUILD_SHARED_LIBS true)

# libnetcdf
add_library (netcdf ${SRCS_NETCDF})

#set (HAVE_M_LIBRARY ON) # TODO
if (HAVE_M_LIBRARY)
	# link the math library
	target_link_libraries (netcdf m)
endif (HAVE_M_LIBRARY)

# Link libs: hdf5_hldll.lib hdf5dll.lib zlib.lib szip.lib libcurl_imp.lib xdr.lib
target_link_libraries (netcdf ${LINK_LIBRARIES})

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (netcdf
	PROPERTIES
	OUTPUT_NAME netcdf
	#PREFIX lib
	#IMPORT_PREFIX lib
	VERSION ${NETCDF_PACKAGE_VERSION}
	SOVERSION ${NETCDF_PACKAGE_VERSION_MAJOR}
	INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
	DEFINE_SYMBOL "DLL_EXPORT")

# Install include file
install (FILES include/netcdf.h DESTINATION include)

if (NOT HAVE_GETOPT)
	set (SRCS_GETOPT win32/getopt.c)
endif()

# Get list of sources for ncdump
get_subdir_var_files (SRCS_NCDUMP SRCS_NCDUMP ncdump)

# build target for ncdump
add_executable (ncdump ${SRCS_NCDUMP}
	${SRCS_GETOPT}
	${NETCDF_SOURCE_DIR}/config.h)
target_link_libraries (ncdump netcdf)

# nccopy
get_subdir_var_files (SRCS_NCCOPY SRCS_NCCOPY ncdump)
add_executable (nccopy ${SRCS_NCCOPY}
	${SRCS_GETOPT}
	${NETCDF_SOURCE_DIR}/config.h)
target_link_libraries (nccopy netcdf)

# ncgen
get_subdir_var_files (SRCS_NCGEN SRCS_NCGEN ncgen)
add_executable (ncgen ${SRCS_NCGEN}
	${SRCS_GETOPT}
	${NETCDF_SOURCE_DIR}/config.h)
target_link_libraries (ncgen netcdf)

# Add install targets
install (TARGETS netcdf ncdump nccopy ncgen
	LIBRARY DESTINATION lib
	COMPONENT Runtime
	RUNTIME DESTINATION bin
	COMPONENT Runtime
	ARCHIVE DESTINATION lib)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
