#
# $Id$
#
# Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; version 3 or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
#
# To modify the cmake process: Edit your cmake/ConfigUser.cmake file
#
# src/CMakeLists.txt
#

include (ManageString)
include (GmtHelperMacros)

# On Windows do not prefix the DLLs with the 'lib' prefix
if (WIN32)
	set(LIB_PREFIX "")
else (WIN32)
	set(LIB_PREFIX "lib")
endif (WIN32)

# Include CREATE_DEBUG_SYM macro and set DEBUG_BUILD
include (CreateDebugSym)

# Find libraries
message (STATUS "Searching dependent libraries. This may take a few minutes...")
find_package (NETCDF REQUIRED)
include_directories (${NETCDF_INCLUDE_DIR})

find_package (GDAL)
if (GDAL_FOUND)
	set (HAVE_GDAL TRUE CACHE INTERNAL "System has GDAL")
	include_directories (${GDAL_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${GDAL_LIBRARIES})
endif (GDAL_FOUND)

find_package (PCRE)
if (PCRE_FOUND)
	set (HAVE_PCRE TRUE CACHE INTERNAL "System has PCRE")
	set (GMT_CONFIG_REGEX_MESSAGE "PCRE (${PCRE_LIBRARIES})"
		CACHE INTERNAL "Regex config message")
	include_directories (${PCRE_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${PCRE_LIBRARIES})
else (PCRE_FOUND)
	find_package (REGEX)
	if (REGEX_FOUND)
		set (HAVE_POSIX_ERE TRUE CACHE INTERNAL "System has regex.h")
		set (GMT_CONFIG_REGEX_MESSAGE "POSIX Regex (${REGEX_INCLUDE_DIR}/regex.h)"
			CACHE INTERNAL "Regex config message")
		include_directories (${REGEX_INCLUDE_DIR})
		# I don't think its needed but just in case:
		list (APPEND GMT_OPTIONAL_LIBRARIES ${REGEX_LIBRARIES})
	else (REGEX_FOUND)
		set (GMT_CONFIG_REGEX_MESSAGE "unsupported"
			CACHE INTERNAL "Regex config message")
	endif (REGEX_FOUND)
endif (PCRE_FOUND)

find_package (FFTW3)
if (FFTW3_FOUND)
	set (HAVE_FFTW3F TRUE CACHE INTERNAL "System has single precision FFTW")
	if (FFTW3F_THREADS_LIBRARY)
		set (HAVE_FFTW3F_THREADS TRUE CACHE INTERNAL "System has single precision threadded FFTW")
	endif (FFTW3F_THREADS_LIBRARY)
	include_directories (${FFTW3_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${FFTW3F_LIBRARIES})
endif (FFTW3_FOUND)

find_package (ZLIB)
if (ZLIB_FOUND)
	set (HAVE_ZLIB TRUE CACHE INTERNAL "System has ZLIB")
	include_directories (${ZLIB_INCLUDE_DIRS})
endif (ZLIB_FOUND)

if (APPLE)
	# Accelerate framework
	find_library (ACCELERATE_FRAMEWORK Accelerate)
	mark_as_advanced (ACCELERATE_FRAMEWORK)
	list (APPEND GMT_OPTIONAL_LIBRARIES ${ACCELERATE_FRAMEWORK})
endif (APPLE)

find_package (OpenMP)
if (OPENMP_FOUND AND GMT_OPENMP)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_C_FLAGS}")
	set (HAVE_OPENMP TRUE CACHE INTERNAL "OpenMP enabled." FORCE)
else (OPENMP_FOUND AND GMT_OPENMP)
	set (HAVE_OPENMP FALSE CACHE INTERNAL "OpenMP disabled." FORCE)
endif (OPENMP_FOUND AND GMT_OPENMP)
if (OPENMP_FOUND AND NOT GMT_OPENMP)
	message (STATUS "User variable GMT_OPENMP not set: disabling OpenMP.")
endif (OPENMP_FOUND AND NOT GMT_OPENMP)

find_package (Threads)

# check for math and POSIX functions
include(ConfigureChecks)

include_directories (${GMT_SOURCE_DIR}/src)
include_directories (${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

# Examples and tests depend on Shorelines
find_package (GSHHG)
if (DO_EXAMPLES OR DO_TESTS AND NOT GSHHG_FOUND)
	message (FATAL_ERROR "Cannot proceed without GSHHG Shorelines. "
		"Need to either set GSHHG_ROOT or disable tests.")
endif (DO_EXAMPLES OR DO_TESTS AND NOT GSHHG_FOUND)

# Examples and tests depend on DCW
find_package (DCW)
if (DO_EXAMPLES OR DO_TESTS AND NOT DCW_FOUND)
	message (FATAL_ERROR "Cannot proceed without DCW polygons. "
		"Need to either set DCW_ROOT or disable tests."
		"Obtain dcw-gmt-<version>.tar.gz or dcw-gmt-<version>.zip from ftp.soest.hawaii.edu://gmt5 and make DCW_ROOT variable point to the directory where you unarchived the files")
endif (DO_EXAMPLES OR DO_TESTS AND NOT DCW_FOUND)

# These lines are temporarily here for the beta release
set (GMT_DEMOS_SRCS testapi.c testgmt5.c testgmtio.c testgrdio.c testio.c)

if (NOT LICENSE_RESTRICTED) # off
	# enable Shewchuk's triangle routine
	set (GMT_TRIANGULATE "Shewchuk" PARENT_SCOPE)
	set (GMT_TRIANGLE_SRCS triangle.c triangle.h)
	list (APPEND GMT_EXTRA_LICENSE_FILES ${CMAKE_CURRENT_DIR}/README.TRIANGLE)
else (NOT LICENSE_RESTRICTED) # on
	# disable Shewchuk's triangle routine
	set (GMT_TRIANGULATE "Watson" PARENT_SCOPE)
endif (NOT LICENSE_RESTRICTED)

set (GMT_PROGS_SRCS blockmean.c blockmedian.c blockmode.c filter1d.c
	fitcircle.c gmt2kml.c gmtconvert.c gmtmath.c gmtselect.c gmtsimplify.c
	gmtspatial.c gmtconnect.c gmtvector.c gmtwhich.c grd2cpt.c grd2rgb.c
	grd2xyz.c grdblend.c grdclip.c grdcut.c grdedit.c grdfft.c grdfilter.c
	grdgradient.c grdhisteq.c grdinfo.c grdlandmask.c grdmask.c grdmath.c
	grdpaste.c grdproject.c grdraster.c grdsample.c grdreformat.c grdtrack.c
	grdtrend.c grdvolume.c greenspline.c kml2gmt.c makecpt.c mapproject.c
	gmtinfo.c nearneighbor.c project.c ps2raster.c sample1d.c spectrum1d.c
	sph2grd.c sphdistance.c sphinterpolate.c sphtriangulate.c splitxyz.c
	surface.c trend1d.c trend2d.c triangulate.c xyz2grd.c gmtdefaults.c gmtget.c
	gmtset.c grdcontour.c grdimage.c grdvector.c grdview.c psbasemap.c psclip.c
	pscoast.c pscontour.c pshistogram.c psimage.c psmask.c psrose.c psscale.c
	pstext.c pswiggle.c psxy.c psxyz.c pslegend.c)

# Legacy modules for which to install compatibility links via
# install_module_symlink
set (GMT_COMPAT_MODULES minmax gmtstitch gmtdp)

# libpslib
set (GMT_PSL_SRCS pslib.c common_string.c common_string.h common_runpath.c
	common_runpath.h declspec.h gmt_config.h gmt_notposix.h)

# libgmt
set (GMT_LIB_SRCS block_subs.h common_byteswap.h common_math.h
	common_runpath.h common_sighandler.c common_string.h declspec.h getopt.h
	gmt_dev.h gmt_calclock.h gmt_colors.h gmt_common.h gmt_constants.h
	gmt_contour.h gmt_crossing.h gmt_customio.h gmt_dcw.h gmt_defaults.h
	gmt_error.h gmt_gdalread.h gmt_grd.h gmt_grdio.h gmt_hash.h
	gmt_internals.h gmt_io.h gmt_lib.h gmt_macros.h gmt_map.h gmt_media_name.h
	gmt_media_size.h gmt_mgg_header2.h gmt_sharedlibs.h gmt_memory.h
	gmt_core_module.h gmt_nan.h gmt_notposix.h gmt_option.h gmt_pennames.h
	gmt_plot.h gmt_project.h gmt_prototypes.h gmt_ps.h gmt_shore.h gmt_stat.h
	gmt_support.h gmt_symbol.h gmt_texture.h gmt_structs.h
	gmt_types.h gmt_unique.h gmt_version.h gshhg_version.h pslib.h
	s_rint.h triangle.h gmt.h gmt_resources.h gmt_enums.h gmt_private.h
	gmt_error_codes.h gmt_api.c gmt_bcr.c gmt_calclock.c gmt_cdf.c gmt_compat.c
	gmt_customio.c gmt_dcw.c gmt_error.c gmt_fft.c gmt_grdio.c gmt_io.c
	gmt_memory.c gmt_nc.c gmt_notposix.c gmt_proj.c gmt_shore.c gmt_sph.c
	gmt_stat.c gmt_support.c gmt_regexp.c gmt_vector.c gmt_parse.c
	gmt_error_codes.c gshhg_version.c gmtread.c gmtwrite.c common_math.c
	common_runpath.c common_sighandler.c common_string.c compat/qsort.c
	gmt_sharedlibs.c gmt_core_module.c gmt_init.c gmt_map.c gmt_plot.c
	kiss_fft/_kiss_fft_guts.h kiss_fft/kiss_fft.c kiss_fft/kiss_fft.h
	kiss_fft/kiss_fftnd.c kiss_fft/kiss_fftnd.h
	${GMT_TRIANGLE_SRCS})

if (BUILD_SUPPLEMENTS)
	# Needed in supplib, + supplements themselves
	set (GMT_SUPPL_SRCS gmt_supplements_module.h gmt_supplements_module.c)
endif (BUILD_SUPPLEMENTS)

# dependencies on generated files
set (GMT_GEN_HDRS gmt_color_rgb.h gmt_colornames.h gmt_datums.h
	gmt_ellipsoids.h gmt_keycases.h
	gmt_keywords.h gmt_config.h
	gmtmath.h gmtmath_op.h gmtmath_explain.h
	grdmath.h grdmath_op.h grdmath_explain.h
	gmt_dimensions.h
	CACHE INTERNAL "Autogenerated GMT headers")

# Add commands to create header files
add_custom_command (OUTPUT gmt_datums.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_datums_h
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS Datums.txt)

add_custom_command (OUTPUT gmt_colornames.h gmt_color_rgb.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_colors_h
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS Colors.txt)

add_custom_command (OUTPUT gmt_ellipsoids.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_ellipsoids
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS Ellipsoids.txt)

add_custom_command (OUTPUT gmt_keycases.h gmt_keywords.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_keywords_h
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS gmt_keywords.txt gmt_keywords.d)

add_custom_command (OUTPUT gmt_dimensions.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_dimensions_h
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS gmt_media_name.h gmt_pennames.h gmt_unique.h
	gmt_datums.h gmt_colornames.h gmt_ellipsoids.h
	gmt_keycases.h)
	# depends on cached line numbers from these generated files:

add_custom_command (OUTPUT gmtmath.h gmtmath_op.h gmtmath_explain.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_math_h
	-D GMT=GMT
	-D gmt=gmt
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS gmtmath.c)

add_custom_command (OUTPUT grdmath.h grdmath_op.h grdmath_explain.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_grd_math_h
	-D GMT=GRD
	-D gmt=grd
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS grdmath.c)

# autogenerated files target
add_custom_target (gen_gmt_headers DEPENDS ${GMT_GEN_HDRS})

configure_file (gmt_config.h.in gmt_config.h)
configure_file (gmt_version.h.in gmt_version.h)
configure_file (isogmt.in isogmt${GMT_INSTALL_NAME_SUFFIX} @ONLY)
configure_file (gmt-config.in gmt-config${GMT_INSTALL_NAME_SUFFIX} @ONLY)

# gmtprogram
set (GMT_PROGRAM ${GMT_SOURCE_DIR}/src/gmtprogram.c)

# extra ugly definitions for triangle
set_source_files_properties (triangle.c
	PROPERTIES
	COMPILE_DEFINITIONS "NO_TIMER;TRILIBRARY;REDUCED;CDT_ONLY;ANSI_DECLARATORS")

if (MSVC)
	# Disable automatic manifest generation
	string(REGEX REPLACE "/MANIFEST[^ ]*( |$)" ""
		CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")

	# To expand the command line arguments in Windows, see:
	# http://msdn.microsoft.com/en-us/library/8bch7bkk.aspx
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} setargv.obj")
endif(MSVC)

# libpslib
add_library (pslib
	${GMT_GEN_HDRS}
	${GMT_PSL_SRCS})

add_dependencies (pslib gen_gmt_headers) # make pslib after gen_gmt_headers

if (HAVE_M_LIBRARY)
	# link the math library
	target_link_libraries (pslib m)
endif (HAVE_M_LIBRARY)

if (HAVE_ZLIB)
	# for DEFLATE encoding
  target_link_libraries (pslib ${ZLIB_LIBRARIES})
endif (HAVE_ZLIB)

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (pslib
	PROPERTIES
	OUTPUT_NAME psl
	PREFIX "${LIB_PREFIX}"
	IMPORT_PREFIX "${LIB_PREFIX}"
	VERSION ${GMT_LIB_VERSION}
	SOVERSION ${GMT_LIB_SOVERSION}
	INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
	DEFINE_SYMBOL "LIBRARY_EXPORTS")

# If a renaming of the psl dll has been set in ConfigUser.cmake
if (WIN32 AND PSL_DLL_RENAME)
	set_target_properties (pslib PROPERTIES RUNTIME_OUTPUT_NAME ${PSL_DLL_RENAME})
endif ()

if (BUILD_SUPPLEMENTS)
	# supplement directories (only those, which are to be included in gmtlib)
	# EXTRA_BUILD_DIRS are for testing new supplements that are not yet in
	# subversion. See cmake/ConfigUserTemplate.cmake for setting this parameter.
	set (GMT_SUPPL_DIRS gshhg img meca mgd77 misc potential segy
		spotter x2sys ${EXTRA_BUILD_DIRS})
endif (BUILD_SUPPLEMENTS)

# path to all binary dirs (used to set PATH in test scripts)
set (GMT_BINARY_DIR_PATH "${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_CONFIG_TYPE:-.}")

if (BUILD_SUPPLEMENTS)
	# process supplement directories
	if (UNIX)
		set (CMAKE_SHARED_MODULE_SUFFIX .so)
	endif (UNIX)
	foreach (_dir ${GMT_SUPPL_DIRS})
		add_subdirectory (${_dir})
		list (APPEND GMT_BINARY_DIR_PATH
			"${CMAKE_CURRENT_BINARY_DIR}/${_dir}/\${CMAKE_CONFIG_TYPE:-.}")
	endforeach (_dir)
	# supplement library files
	get_subdir_var_files (GMT_SUPPL_LIB_SRCS LIB_SRCS ${GMT_SUPPL_DIRS})

	# include supplement directories
	foreach (_dir ${GMT_SUPPL_DIRS})
		include_directories (${CMAKE_CURRENT_BINARY_DIR}/${_dir})
	endforeach (_dir)
	# libgmtsuppl
	if (WIN32)
		add_library (supplib
			${GMT_GEN_HDRS}
			${GMT_SUPPL_SRCS}
			${GMT_SUPPL_LIB_SRCS})
	else (WIN32)
		add_library (supplib MODULE
			${GMT_GEN_HDRS}
			${GMT_SUPPL_SRCS}
			${GMT_SUPPL_LIB_SRCS})
	endif (WIN32)

	add_dependencies (supplib gen_gmt_headers) # make supplib after gen_gmt_headers
	add_dependencies (supplib pslib) # make supplib after pslib
	add_dependencies (supplib gmtlib) # make supplib after pslib

	# No SOVERSION & VERSION for a MODULE, only for SHARED libs
	target_link_libraries (supplib
		gmtlib
		pslib)

	set_target_properties (supplib
		PROPERTIES
		OUTPUT_NAME supplements
		RUNTIME_OUTPUT_NAME supplements
		LIBRARY_OUTPUT_DIRECTORY plugins
		RUNTIME_OUTPUT_DIRECTORY plugins
		PREFIX ""
		LINK_FLAGS "${USER_GMTLIB_LINK_FLAGS}"
		DEFINE_SYMBOL "LIBRARY_EXPORTS")

	if (WIN32 AND SUPP_DLL_RENAME)
		set_target_properties (supplib PROPERTIES RUNTIME_OUTPUT_NAME ${SUPP_DLL_RENAME})
	endif ()

	if (HAVE_DLADDR AND HAVE_LIBDL)
		# link the dynamic linking loader library
		target_link_libraries (supplib dl)
	endif (HAVE_DLADDR AND HAVE_LIBDL)

	if (HAVE_M_LIBRARY)
		# link the math library
		target_link_libraries (supplib m)
	endif (HAVE_M_LIBRARY)

	# install target
	install (TARGETS supplib
		LIBRARY DESTINATION ${GMT_LIBDIR}/gmt${GMT_INSTALL_NAME_SUFFIX}/plugins # UNIX
		COMPONENT Runtime
		RUNTIME DESTINATION ${GMT_BINDIR}/gmt_plugins # Windows
		COMPONENT Runtime)
endif (BUILD_SUPPLEMENTS)

# make UNIX PATH variable
string (REPLACE ";" ":" GMT_BINARY_DIR_PATH "${GMT_BINARY_DIR_PATH}")
set (GMT_BINARY_DIR_PATH ${GMT_BINARY_DIR_PATH} CACHE INTERNAL
	"UNIX PATH to all binary dirs")

# libgmt
add_library (gmtlib
	${GMT_GEN_HDRS}
	${GMT_LIB_SRCS}
	${GMT_PROGS_SRCS})

add_dependencies (gmtlib gen_gmt_headers) # make gmtlib after gen_gmt_headers
add_dependencies (gmtlib pslib) # make gmtlib after pslib

target_link_libraries (gmtlib
	${NETCDF_LIBRARIES}
	${GMT_OPTIONAL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	pslib)

if (HAVE_M_LIBRARY)
	# link the math library
	target_link_libraries (gmtlib m)
endif (HAVE_M_LIBRARY)

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (gmtlib
	PROPERTIES
	OUTPUT_NAME gmt
	PREFIX "${LIB_PREFIX}"
	IMPORT_PREFIX "${LIB_PREFIX}"
	VERSION ${GMT_LIB_VERSION}
	SOVERSION ${GMT_LIB_SOVERSION}
	INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
	LINK_FLAGS "${USER_GMTLIB_LINK_FLAGS}"
	DEFINE_SYMBOL "LIBRARY_EXPORTS")

# If a renaming of the gmt dll has been set in ConfigUser.cmake
if (WIN32 AND GMT_DLL_RENAME)
	set_target_properties (gmtlib PROPERTIES RUNTIME_OUTPUT_NAME ${GMT_DLL_RENAME})
endif ()

if (HAVE_DLADDR AND HAVE_LIBDL)
	# link the dynamic linking loader library
	target_link_libraries (pslib dl)
	target_link_libraries (gmtlib dl)
endif (HAVE_DLADDR AND HAVE_LIBDL)

if (MSVC AND FFTW3_FOUND)
	target_link_libraries(gmtlib ws2_32)
endif (MSVC AND FFTW3_FOUND)

# build targets for standalone gmt target and demos
string (REPLACE ".c" "" _gmt_progs "gmt.c;${GMT_DEMOS_SRCS}")
foreach (_gmt_prog ${_gmt_progs})
	add_executable (${_gmt_prog} ${_gmt_prog}.c)
	target_link_libraries (${_gmt_prog} gmtlib)
endforeach (_gmt_prog)

configure_file (psldemo.h.in psldemo.h @ONLY)
add_executable (psldemo psldemo.h psldemo.c)
target_link_libraries (psldemo pslib)

# Rename gmt target to prevent version clash
set_target_properties (gmt PROPERTIES OUTPUT_NAME gmt${GMT_INSTALL_NAME_SUFFIX})

# generate and install Mac/Windows debugging symbols
create_debug_sym (${GMT_BINDIR} gmt)
if (WIN32)
	create_debug_sym (${GMT_BINDIR} gmtlib pslib)
	create_debug_sym (${GMT_BINDIR}/gmt_plugins supplib)
else (WIN32)
	create_debug_sym (${GMT_LIBDIR} gmtlib pslib)
	create_debug_sym (${GMT_LIBDIR}/gmt${GMT_INSTALL_NAME_SUFFIX}/plugins supplib)
endif (WIN32)

# gshhg_version
add_executable (gshhg_version gshhg_version.h gshhg_version.c)
set_target_properties (gshhg_version
	PROPERTIES COMPILE_DEFINITIONS "STANDALONE")
target_link_libraries (gshhg_version ${NETCDF_LIBRARIES})

if (BUILD_SUPPLEMENTS)
	# Tests depend on GMT progs and supplements:
	add_dependencies (check ${_gmt_progs} psldemo supplib)
else (BUILD_SUPPLEMENTS)
	# Tests depend on GMT progs and supplements:
	add_dependencies (check ${_gmt_progs} psldemo)
endif (BUILD_SUPPLEMENTS)

# add the install targets
install (TARGETS gmtlib pslib gmt
	ARCHIVE DESTINATION ${GMT_LIBDIR}
	COMPONENT Runtime
	LIBRARY DESTINATION ${GMT_LIBDIR}
	COMPONENT Runtime
	RUNTIME DESTINATION ${GMT_BINDIR}
	COMPONENT Runtime)

string (REPLACE ".c" "" _gmt_modules "${GMT_PROGS_SRCS}")
install_module_symlink (${_gmt_modules} ${GMT_COMPAT_MODULES})

# Install public headers
install (FILES gmt.h gmt_option.h gmt_resources.h declspec.h
	DESTINATION ${GMT_INCLUDEDIR}
	COMPONENT Runtime)

# symlink to gmt-wrapper in bindir and libdir:
if (UNIX AND GMT_INSTALL_NAME_SUFFIX)
	get_target_property(_gmt_wrapper_name gmt OUTPUT_NAME)
	install (CODE "
	execute_process (
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		${_gmt_wrapper_name} \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${GMT_BINDIR}/gmt)
	")
endif (UNIX AND GMT_INSTALL_NAME_SUFFIX)

# install extra files
install (PROGRAMS
	${CMAKE_CURRENT_BINARY_DIR}/isogmt${GMT_INSTALL_NAME_SUFFIX}
	${CMAKE_CURRENT_BINARY_DIR}/gmt-config${GMT_INSTALL_NAME_SUFFIX}
	DESTINATION ${GMT_BINDIR}
	COMPONENT Runtime)

install (CODE "
execute_process (
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_CURRENT_SOURCE_DIR}/gmtlogo
	\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${GMT_BINDIR}/gmtlogo${GMT_INSTALL_NAME_SUFFIX})
execute_process (
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_CURRENT_SOURCE_DIR}/gmtswitch
	\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${GMT_BINDIR}/gmtswitch${GMT_INSTALL_NAME_SUFFIX})
execute_process (
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	${CMAKE_CURRENT_SOURCE_DIR}/gmt_shell_functions.sh
	\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${GMT_BINDIR}/gmt_shell_functions${GMT_INSTALL_NAME_SUFFIX}.sh)
")

if (WIN32)
	install (PROGRAMS
		gmtswitch.bat
		DESTINATION ${GMT_BINDIR}
		COMPONENT Runtime)
endif (WIN32)

# this prints out the final version of the compiler/linker flags
string (TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
string(REGEX REPLACE "[ ]+" " " _using_cflags ${CMAKE_C_FLAGS} " "
	${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}})
message (STATUS "Using CFLAGS = '${_using_cflags}'")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
