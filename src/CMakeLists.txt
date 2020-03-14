#
#
# Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
# Contact info: www.generic-mapping-tools.org
#-------------------------------------------------------------------------------
#
# To modify the cmake process, edit your cmake/ConfigUser.cmake and cmake/ConfigUserAdvanced.cmake files
#
# src/CMakeLists.txt
#

include (ManageString)
include (GmtHelperMacros)

if (UNIX AND NOT APPLE)
	set (LINUX TRUE)
else (UNIX AND NOT APPLE)
	set (LINUX FALSE)
endif (UNIX AND NOT APPLE)

# On Windows do not prefix the DLLs with the 'lib' prefix
if (WIN32)
	set(LIB_PREFIX "")
else (WIN32)
	set(LIB_PREFIX "lib")
endif (WIN32)

# Include CREATE_DEBUG_SYM macro and set DEBUG_BUILD
include (CreateDebugSym)

# Find executable needed by gmt
find_program (OGR2OGR ogr2ogr)
if (NOT OGR2OGR)
	message (WARNING "Cannot find program ogr2ogr - gmt will be unable to convert Shapefiles to GMT/OGR format")
	set (GMT_CONFIG_OGR2OGR_MESSAGE "no" CACHE INTERNAL "OGR2OGR config message")
else (NOT OGR2OGR)
	execute_process(COMMAND ogr2ogr --version OUTPUT_VARIABLE answer OUTPUT_STRIP_TRAILING_WHITESPACE)
	string (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" answer "${answer}")
	set (GMT_CONFIG_OGR2OGR_MESSAGE "yes (${answer})" CACHE INTERNAL "OGR2OGR config message")
endif (NOT OGR2OGR)

# Find executable needed by psconvert
find_program (GDAL_TRANSLATE gdal_translate)
if (NOT GDAL_TRANSLATE)
	message (WARNING "Cannot find program gdal_translate - psconvert will be unable to convert TIFF to GeoTIFF")
	set (GMT_CONFIG_GDAL_TRANSLATE_MESSAGE "no" CACHE INTERNAL "GDAL_TRANSLATE config message")
else (NOT GDAL_TRANSLATE)
	execute_process(COMMAND gdal_translate --version OUTPUT_VARIABLE answer OUTPUT_STRIP_TRAILING_WHITESPACE)
	string (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" answer "${answer}")
	set (GMT_CONFIG_GDAL_TRANSLATE_MESSAGE "yes (${answer})" CACHE INTERNAL "GDAL_TRANSLATE config message")
endif (NOT GDAL_TRANSLATE)

# Find executables needed by movie
find_program (GM gm)
if (NOT GM)
	message (WARNING "Cannot find GraphicsMagick program gm - gmt movie will be unable to make animated GIFs")
	set (GMT_CONFIG_GM_MESSAGE "no" CACHE INTERNAL "GM config message")
else (NOT GM)
	execute_process(COMMAND gm help OUTPUT_VARIABLE answer OUTPUT_STRIP_TRAILING_WHITESPACE)
	string (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" answer "${answer}")
	# GMT_CONFIG_GM_VERSION is used when building macOS bundle
	set (GMT_CONFIG_GM_VERSION "${answer}" CACHE INTERNAL "GM version message")
	set (GMT_CONFIG_GM_MESSAGE "yes (${answer})" CACHE INTERNAL "GM config message")
endif (NOT GM)

find_program (FFMPEG ffmpeg)
if (NOT FFMPEG)
	message (WARNING "Cannot find ffmpeg - gmt movie will be unable to make movies from frames")
	set (GMT_CONFIG_FFMPEG_MESSAGE "no" CACHE INTERNAL "FFMPEG config message")
else (NOT FFMPEG)
	execute_process(COMMAND ffmpeg -version OUTPUT_VARIABLE answer OUTPUT_STRIP_TRAILING_WHITESPACE)
	string (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" answer "${answer}")
	set (GMT_CONFIG_FFMPEG_MESSAGE "yes (${answer})" CACHE INTERNAL "FFMPEG config message")
endif (NOT FFMPEG)

# Find executable needed by docs
if (LINUX)
	find_program (XDGOPEN xdg-open)
	if (NOT XDGOPEN)
		message (WARNING "Cannot find program xdg-open - gmt docs will be unable to open the documentation")
		set (GMT_CONFIG_OPEN_MESSAGE "no" CACHE INTERNAL "OPEN config message")
	else (NOT XDGOPEN)
		set (GMT_CONFIG_OPEN_MESSAGE "yes" CACHE INTERNAL "OPEN config message")
	endif (NOT XDGOPEN)
else (LINUX)
	set (GMT_CONFIG_OPEN_MESSAGE "yes" CACHE INTERNAL "OPEN config message")
endif (LINUX)

# Find executable needed by psconvert
if (WIN32)
	set (GMT_CONFIG_GS_MESSAGE "yes" CACHE INTERNAL "GS config message")
else (WIN32)
	find_program (GS gs)
	if (NOT GS)
		message (WARNING "Cannot find ghostscript program gs - gmt psconvert will be unable to convert PostScript to other formats")
		set (GMT_CONFIG_GS_MESSAGE "no" CACHE INTERNAL "GS config message")
	else (NOT GS)
		execute_process(COMMAND gs --version OUTPUT_VARIABLE answer OUTPUT_STRIP_TRAILING_WHITESPACE)
		set (GMT_CONFIG_GS_MESSAGE "yes (${answer})" CACHE INTERNAL "GS config message")
	endif (NOT GS)
endif (WIN32)

# Find libraries
message (STATUS "Searching dependent libraries. This may take a few minutes...")
# NetCDF is required
find_package (NETCDF REQUIRED)
include_directories (${NETCDF_INCLUDE_DIR})

# libcurl is required since 5.4
find_package (CURL REQUIRED)
include_directories (${CURL_INCLUDE_DIRS})
list (APPEND GMT_OPTIONAL_LIBRARIES ${CURL_LIBRARIES})

if (NOT DEFINED GMT_EXCLUDE_GDAL)
	find_package (GDAL)
endif (NOT DEFINED GMT_EXCLUDE_GDAL)
if (GDAL_FOUND)
	set (HAVE_GDAL TRUE CACHE INTERNAL "System has GDAL")
	include_directories (${GDAL_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${GDAL_LIBRARIES})
endif (GDAL_FOUND)

if (NOT DEFINED GMT_EXCLUDE_PCRE)
	find_package (PCRE)
endif (NOT DEFINED GMT_EXCLUDE_PCRE)
if (NOT PCRE_FOUND AND NOT DEFINED GMT_EXCLUDE_PCRE2)
	find_package (PCRE2)
endif (NOT PCRE_FOUND AND NOT DEFINED GMT_EXCLUDE_PCRE2)
if (PCRE_FOUND)
	set (HAVE_PCRE TRUE CACHE INTERNAL "System has PCRE")
	set (GMT_CONFIG_REGEX_MESSAGE "PCRE (${PCRE_LIBRARIES})"
		CACHE INTERNAL "Regex config message")
	include_directories (${PCRE_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${PCRE_LIBRARIES})
elseif (PCRE2_FOUND)
	set (HAVE_PCRE2 TRUE CACHE INTERNAL "System has PCRE2")
	set (GMT_CONFIG_REGEX_MESSAGE "PCRE2 (${PCRE2_LIBRARIES})"
		CACHE INTERNAL "Regex config message")
	include_directories (${PCRE2_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${PCRE2_LIBRARIES})
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

if (NOT DEFINED GMT_EXCLUDE_FFTW3)
	find_package (FFTW3)
endif (NOT DEFINED GMT_EXCLUDE_FFTW3)
if (FFTW3_FOUND)
	set (HAVE_FFTW3F TRUE CACHE INTERNAL "System has single precision FFTW")
	if (FFTW3F_THREADS_LIBRARY)
		set (HAVE_FFTW3F_THREADS TRUE CACHE INTERNAL "System has single precision threaded FFTW")
	endif (FFTW3F_THREADS_LIBRARY)
	include_directories (${FFTW3_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${FFTW3F_LIBRARIES})
endif (FFTW3_FOUND)

if (NOT DEFINED GMT_EXCLUDE_LAPACK)
	find_package (LAPACK)
endif (NOT DEFINED GMT_EXCLUDE_LAPACK)
if (LAPACK_FOUND)
	set (HAVE_LAPACK TRUE CACHE INTERNAL "System has LAPACK")
	set (GMT_CONFIG_LAPACK_MESSAGE "yes" CACHE INTERNAL "LAPACK config message")
	include_directories (${LAPACK_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${LAPACK_LIBRARIES})
else (LAPACK_FOUND)
	if (LAPACK_LIBRARY)
		# Set in ConfigUserAdvanced.cmake
		set (HAVE_LAPACK TRUE CACHE INTERNAL "System has LAPACK")
		set (GMT_CONFIG_LAPACK_MESSAGE "yes" CACHE INTERNAL "LAPACK config message")
		list (APPEND GMT_OPTIONAL_LIBRARIES ${LAPACK_LIBRARY})
	else (LAPACK_LIBRARY)
		set (GMT_CONFIG_LAPACK_MESSAGE "no" CACHE INTERNAL "LAPACK config message")
	endif (LAPACK_LIBRARY)
endif (LAPACK_FOUND)

if (NOT DEFINED GMT_EXCLUDE_BLAS)
	find_package (BLAS)
endif (NOT DEFINED GMT_EXCLUDE_BLAS)
if (BLAS_FOUND)
	set (HAVE_BLAS TRUE CACHE INTERNAL "System has BLAS")
	set (GMT_CONFIG_BLAS_MESSAGE "yes" CACHE INTERNAL "BLAS config message")
	include_directories (${BLAS_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${BLAS_LIBRARIES})
else (BLAS_FOUND)
	if (BLAS_LIBRARY)
		# Set in ConfigUserAdvanced.cmake
		set (HAVE_BLAS TRUE CACHE INTERNAL "System has BLAS")
		set (GMT_CONFIG_BLAS_MESSAGE "yes" CACHE INTERNAL "BLAS config message")
		list (APPEND GMT_OPTIONAL_LIBRARIES ${BLAS_LIBRARY})
	else (BLAS_LIBRARY)
		set (GMT_CONFIG_BLAS_MESSAGE "no" CACHE INTERNAL "BLAS config message")
	endif (BLAS_LIBRARY)
endif (BLAS_FOUND)

if (NOT DEFINED GMT_EXCLUDE_ZLIB)
	find_package (ZLIB)
endif (NOT DEFINED GMT_EXCLUDE_ZLIB)
if (ZLIB_FOUND)
	set (HAVE_ZLIB TRUE CACHE INTERNAL "System has ZLIB")
	include_directories (${ZLIB_INCLUDE_DIRS})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${ZLIB_LIBRARY})
endif (ZLIB_FOUND)


if (APPLE)
	# Accelerate framework
	find_library (ACCELERATE_FRAMEWORK Accelerate)
	mark_as_advanced (ACCELERATE_FRAMEWORK)
	list (APPEND GMT_OPTIONAL_LIBRARIES ${ACCELERATE_FRAMEWORK})
endif (APPLE)


find_package (OpenMP)
if (OPENMP_FOUND AND GMT_ENABLE_OPENMP)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
	set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_SHARED_LINKER_FLAGS}")
	set (HAVE_OPENMP TRUE CACHE INTERNAL "OpenMP enabled." FORCE)
	set (GMT_CONFIG_OPENMP_MESSAGE "enabled" CACHE INTERNAL "OPENMP config message")
elseif (OPENMP_FOUND AND NOT GMT_ENABLE_OPENMP)
	message (STATUS "User variable GMT_ENABLE_OPENMP not set: disabling OpenMP.")
	set (GMT_CONFIG_OPENMP_MESSAGE "disabled (GMT_ENABLE_OPENMP not set)" CACHE INTERNAL "OPENMP config message")
else ()
	set (HAVE_OPENMP FALSE CACHE INTERNAL "OpenMP disabled." FORCE)
	set (GMT_CONFIG_OPENMP_MESSAGE "disabled" CACHE INTERNAL "OPENMP config message")
endif ()

find_package(GLIB COMPONENTS gthread)
if (GLIB_FOUND AND GMT_USE_THREADS)
	set (HAVE_GLIB_GTHREAD TRUE CACHE INTERNAL "System has GLIB component gthread")
	include_directories (${GLIB_INCLUDE_DIRS})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${GLIB_LIBRARIES})
	set (GMT_CONFIG_GTHREAD_MESSAGE "enabled"
		CACHE INTERNAL "GTHREAD config message")
endif (GLIB_FOUND AND GMT_USE_THREADS)
if (GLIB_FOUND AND NOT GMT_USE_THREADS)
	set (HAVE_GLIB_GTHREAD FALSE CACHE INTERNAL "User disabled threads")
	message (STATUS "User variable GMT_USE_THREADS not set: disabling GThread.")
	set (GMT_CONFIG_GTHREAD_MESSAGE "disabled"
		CACHE INTERNAL "GTHREAD config message")
endif (GLIB_FOUND AND NOT GMT_USE_THREADS)
if (GMT_USE_THREADS AND NOT GLIB_FOUND)
	message (FATAL_ERROR "User variable GMT_USE_THREADS set but GLIB component gthread not found.")
	set (GMT_CONFIG_GTHREAD_MESSAGE "unavailable"
		CACHE INTERNAL "GTHREAD config message")
endif (GMT_USE_THREADS AND NOT GLIB_FOUND)

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
		"Obtain dcw-gmt-<version>.tar.gz from ftp://ftp.soest.hawaii.edu/gmt and make DCW_ROOT variable point to the directory where you unarchived the files")
endif (DO_EXAMPLES OR DO_TESTS AND NOT DCW_FOUND)

if (DO_API_TESTS)
	# These lines are temporarily here for the beta release - comment to avoid in the build
	set (GMT_DEMOS_SRCS testpsl.c testgmtshell.c testgmtio.c testgrdio.c
		testapiconv.c example1.c testapi_matrix.c testapi_matrix_plot.c testapi_vector.c test_JL.c testapi_mixmatrix.c
		test_walter.c testapi_usergrid.c testapi_userdataset.c testapi_uservectors.c)
endif (DO_API_TESTS)

if (NOT LICENSE_RESTRICTED) # off
	# enable Shewchuk's triangle routine
	set (GMT_TRIANGULATE "Shewchuk" PARENT_SCOPE)
	set (GMT_TRIANGLE_SRCS triangle.c triangle.h)
	list (APPEND GMT_EXTRA_LICENSE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.TRIANGLE)
	set (GMT_EXTRA_LICENSE_FILES ${GMT_EXTRA_LICENSE_FILES} PARENT_SCOPE)
else (NOT LICENSE_RESTRICTED) # on
	# disable Shewchuk's triangle routine
	set (GMT_TRIANGULATE "Watson" PARENT_SCOPE)
endif (NOT LICENSE_RESTRICTED)

# Note: Developers can set EXTRA_MODULES in ConfigUserAdvanced.cmake to test new modules
set (GMT_PROGS_SRCS blockmean.c blockmedian.c blockmode.c docs.c dimfilter.c filter1d.c grd2kml.c
	fitcircle.c gmt2kml.c gmtconvert.c gmtlogo.c gmtmath.c gmtselect.c gmtsimplify.c
	gmtspatial.c gmtconnect.c grdgdal.c gmtregress.c gmtvector.c gmtwhich.c grd2cpt.c
	grd2xyz.c grdblend.c grdclip.c grdcut.c grdedit.c grdfft.c grdfill.c grdfilter.c
	grdgradient.c grdhisteq.c grdinfo.c grdinterpolate.c grdlandmask.c grdmask.c grdmath.c
	grdpaste.c grdproject.c grdsample.c grdconvert.c grdtrack.c movie.c
	grdtrend.c grdvolume.c greenspline.c kml2gmt.c makecpt.c mapproject.c
	gmtinfo.c nearneighbor.c project.c psconvert.c sample1d.c spectrum1d.c
	sph2grd.c sphdistance.c sphinterpolate.c sphtriangulate.c splitxyz.c
	surface.c trend1d.c trend2d.c triangulate.c xyz2grd.c gmtdefaults.c gmtget.c
	gmtset.c grdcontour.c grdimage.c grdvector.c grdview.c psbasemap.c psclip.c
	pscoast.c pscontour.c psevents.c pshistogram.c psimage.c psmask.c psrose.c psscale.c
	pssolar.c psternary.c pstext.c pswiggle.c psxy.c psxyz.c pslegend.c ${EXTRA_MODULES})

# Legacy modules for which to install compatibility links via
# install_module_symlink
set (GMT_COMPAT_MODULES minmax gmtstitch gmtdp grdreformat ps2raster originator)

# libpslib
set (GMT_PSL_SRCS postscriptlight.c declspec.h psl_config.h PSL_Standard+.h
	PSL_Standard.h PSL_ISOLatin1+.h PSL_ISOLatin1.h PSL_ISO-8859-1.h
	PSL_ISO-8859-2.h PSL_ISO-8859-3.h PSL_ISO-8859-4.h PSL_ISO-8859-5.h
	PSL_ISO-8859-6.h PSL_ISO-8859-7.h PSL_ISO-8859-8.h PSL_ISO-8859-9.h
	PSL_ISO-8859-10.h PSL_ISO-8859-13.h PSL_ISO-8859-14.h PSL_ISO-8859-15.h
	PSL_strings.h PSL_patterns.h)

set (GMT_MBSYSTEM_SRCS gmt_mbsystem_module.c gmt_mbsystem_module.h)

# libgmt
set (GMT_LIB_SRCS block_subs.h common_byteswap.h common_math.h
	common_runpath.h common_sighandler.c common_string.h clear.c begin.c
	gmt_color_rgb.h gmt_colornames.h
	gmt_modern.c end.c declspec.h gmt_hidden.h gmt_remote.h gmt_remote.c
	figure.c gmt_dev.h gmt_common.h gmt_constants.h compat/qsort.h
	gmt_contour.h gmt_customio.h gmt_datums.h gmt_dcw.h gmt_decorate.h gmt_defaults.h
	gmt_ellipsoids.h gmt_error.h gmt_gdalread.h gmt_glib.h gmt_grd.h gmt_grdio.h gmt_hash.h
	gmt_internals.h gmt_io.h gmt_macros.h gmt_media_name.h gmt_modern.h
	gmt_media_size.h gmt_mgg_header2.h gmt_sharedlibs.h gmt_memory.h gmt_mb.h
	gmt_core_module.h gmt_nan.h gmt_notposix.h gmt_pennames.h gmt_enum_dict.h
	gmt_plot.h gmt_project.h gmt_prototypes.h gmt_psl.h gmt_shore.h gmt_gsformats.h
	gmt_symbol.h gmt_synopsis.h gmt_texture.h gmt_time.h gmt_cpt_masters.h
	gmt_types.h gmt_unique.h gmt_version.h gshhg_version.h postscriptlight.h
	s_rint.h triangle.h gmt.h gmt_resources.h gmt_private.h subplot.c
	gmt_error_codes.h gmt_api.c gmt_bcr.c gmt_calclock.c gmt_cdf.c gmt_compat.c
	gmt_customio.c gmt_dcw.c gmt_error.c gmt_fft.c gmt_grdio.c gmt_io.c
	gmt_memory.c gmt_nc.c gmt_notposix.c gmt_proj.c gmt_shore.c gmt_sph.c
	gmt_stat.c gmt_support.c gmt_regexp.c gmt_vector.c gmt_parse.c inset.c
	gmt_error_codes.c gshhg_version.c gmtread.c gmtwrite.c common_math.c
	common_runpath.c common_sighandler.c common_string.c compat/qsort.c
	gmt_sharedlibs.c gmt_core_module.c gmt_init.c gmt_map.c gmt_plot.c
	gmt_gdal_librarified.c
	kiss_fft/_kiss_fft_guts.h kiss_fft/kiss_fft.c kiss_fft/kiss_fft.h
	kiss_fft/kiss_fftnd.c kiss_fft/kiss_fftnd.h
	${GMT_TRIANGLE_SRCS} ${GMT_MBSYSTEM_SRCS})

if (BUILD_SUPPLEMENTS)
	# Needed in supplib, + supplements themselves
	set (GMT_SUPPL_SRCS gmt_modern.c gmt_supplements_module.h gmt_supplements_module.c)
endif (BUILD_SUPPLEMENTS)

# dependencies on generated files
set (GMT_GEN_HDRS gmt_keycases.h
	gmt_keywords.h gmt_config.h
	gmt_dimensions.h
	CACHE INTERNAL "Autogenerated GMT headers")

# Add commands to create header files

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
	DEPENDS gmt_media_name.h gmt_pennames.h gmt_unique.h gmt_ellipsoids.h
	gmt_datums.h gmt_colornames.h gmt_cpt_masters.h gmt_datasets.h
	gmt_keycases.h)
	# depends on cached line numbers from these generated files:

# autogenerated files target
add_custom_target (gen_gmt_headers DEPENDS ${GMT_GEN_HDRS})

# Configure header files to pass some of the CMake settings to the source code
configure_file (gmt_config.h.in gmt_config.h)
configure_file (psl_config.h.in psl_config.h)
configure_file (gmt_version.h.in gmt_version.h)
configure_file (isogmt.in isogmt${GMT_INSTALL_NAME_SUFFIX} @ONLY)
configure_file (gmt-config.in gmt-config${GMT_INSTALL_NAME_SUFFIX} @ONLY)

# gmtprogram
set (GMT_PROGRAM ${GMT_SOURCE_DIR}/src/gmtprogram.c)

# extra ugly definitions for triangle
set_source_files_properties (triangle.c
	PROPERTIES
	COMPILE_DEFINITIONS "NO_TIMER;TRILIBRARY;REDUCED;CDT_ONLY;ANSI_DECLARATORS")

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
	OUTPUT_NAME postscriptlight
	PREFIX "${LIB_PREFIX}"
	IMPORT_PREFIX "${LIB_PREFIX}"
	VERSION ${GMT_LIB_VERSION}
	SOVERSION ${GMT_LIB_SOVERSION}
	DEFINE_SYMBOL "LIBRARY_EXPORTS")

# If a renaming of the psl dll has been set in ConfigUserAdvanced.cmake
if (WIN32 AND PSL_DLL_RENAME)
	set_target_properties (pslib PROPERTIES RUNTIME_OUTPUT_NAME ${PSL_DLL_RENAME})
endif (WIN32 AND PSL_DLL_RENAME)

if (BUILD_SUPPLEMENTS)
	if (UNIX)
		set (CMAKE_SHARED_MODULE_SUFFIX .so)
	endif (UNIX)
	# supplement directories (only those, which are to be included in gmtlib)
	# EXTRA_BUILD_DIRS are for testing new supplements that are not yet in git.
	# See cmake/ConfigUserAdvancedTemplate.cmake for setting this parameter.
	set (GMT_SUPPL_DIRS geodesy gshhg img mgd77 potential segy seis spotter x2sys ${EXTRA_BUILD_DIRS})
	# process supplement directories
	foreach (_dir ${GMT_SUPPL_DIRS})
		add_subdirectory (${_dir})
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

	# Include any extra files that are listed in EXTRA_MODULES_SUPPL defined in ConfigUserAdvanced.cmake
	# This include(s) will add new modules to the official GMT supplements
	if (EXTRA_MODULES_SUPPL)
		foreach (_f ${EXTRA_MODULES_SUPPL})
			include (${_f})
		endforeach(_f)
	endif (EXTRA_MODULES_SUPPL)

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
	endif (WIN32 AND SUPP_DLL_RENAME)

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

# Include any extra files that are listed in EXTRA_INCLUDE_NEWSUPPL defined in ConfigUserAdvanced.cmake
# This include(s) will create new pluggins
if (EXTRA_INCLUDE_NEWSUPPL)
	foreach (_f ${EXTRA_INCLUDE_NEWSUPPL})
		include (${_f})
	endforeach(_f)
endif (EXTRA_INCLUDE_NEWSUPPL)

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
	LINK_FLAGS "${USER_GMTLIB_LINK_FLAGS}"
	DEFINE_SYMBOL "LIBRARY_EXPORTS")

# If a renaming of the gmt dll has been set in ConfigUserAdvanced.cmake
if (WIN32 AND GMT_DLL_RENAME)
	set_target_properties (gmtlib PROPERTIES RUNTIME_OUTPUT_NAME ${GMT_DLL_RENAME})
endif (WIN32 AND GMT_DLL_RENAME)

if (HAVE_DLADDR AND HAVE_LIBDL)
	# link the dynamic linking loader library
	target_link_libraries (pslib dl)
	target_link_libraries (gmtlib dl)
endif (HAVE_DLADDR AND HAVE_LIBDL)

if (MSVC AND FFTW3_FOUND)
	target_link_libraries (gmtlib ws2_32)
endif (MSVC AND FFTW3_FOUND)

# build targets for standalone gmt target and demos
string (REPLACE ".c" "" _gmt_progs "gmt.c;${GMT_DEMOS_SRCS}")
foreach (_gmt_prog ${_gmt_progs})
	add_executable (${_gmt_prog} ${_gmt_prog}.c)
	target_link_libraries (${_gmt_prog} gmtlib)
endforeach (_gmt_prog)

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

# script2verbatim for removing comments, replacing -ps from example scripts
add_executable (script2verbatim script2verbatim.c)

if (BUILD_SUPPLEMENTS)
	# Tests depend on GMT progs and supplements:
	add_dependencies (check ${_gmt_progs} psldemo supplib)
else (BUILD_SUPPLEMENTS)
	# Tests depend on GMT progs and supplements:
	add_dependencies (check ${_gmt_progs} psldemo)
endif (BUILD_SUPPLEMENTS)

if (MSVC)
	# Disable automatic manifest generation
	string(REGEX REPLACE "/MANIFEST[^ ]*( |$)" ""
		CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")

	# To expand the command line arguments in Windows, see:
	# http://msdn.microsoft.com/en-us/library/8bch7bkk.aspx
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} setargv.obj")
endif (MSVC)

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
install (FILES gmt.h gmt_resources.h declspec.h
	DESTINATION ${GMT_INCLUDEDIR}
	COMPONENT Runtime)

if (BUILD_DEVELOPER)
	# Install Developer headers [These are clean and have no #define HAVE_* configuration]
	install (FILES postscriptlight.h common_math.h common_string.h gmt_common.h gmt_constants.h gmt_contour.h
		gmt_dcw.h gmt_decorate.h gmt_defaults.h gmt_error.h gmt_error_codes.h gmt_fft.h gmt_gdalread.h gmt_grd.h
		gmt_grdio.h gmt_hash.h gmt_io.h gmt_macros.h gmt_memory.h gmt_modern.h gmt_nan.h gmt_notposix.h gmt_plot.h
		gmt_private.h gmt_project.h gmt_prototypes.h gmt_psl.h gmt_shore.h gmt_symbol.h gmt_synopsis.h
		gmt_texture.h gmt_time.h gmt_types.h gmt_dev.h gmt_customio.h gmt_hidden.h gmt_mb.h
		gmt_core_module.h gmt_supplements_module.h compat/qsort.h
		DESTINATION ${GMT_INCLUDEDIR}
		COMPONENT Runtime)
	install (FILES compat/qsort.h
		DESTINATION ${GMT_INCLUDEDIR}/compat
		COMPONENT Runtime)
	# Install Configured Developer headers [These are produced by cmake from *.h.in files]
	install (FILES ${PROJECT_BINARY_DIR}/src/config.h ${PROJECT_BINARY_DIR}/src/gmt_config.h
		${PROJECT_BINARY_DIR}/src/gmt_dimensions.h ${PROJECT_BINARY_DIR}/src/gmt_version.h
		${PROJECT_BINARY_DIR}/src/psl_config.h
		DESTINATION ${GMT_INCLUDEDIR}
		COMPONENT Runtime)
endif (BUILD_DEVELOPER)

# symlink to gmt-wrapper in bindir and libdir:
if (UNIX AND GMT_INSTALL_NAME_SUFFIX)
	get_target_property(_gmt_wrapper_name gmt OUTPUT_NAME)
	install (CODE "
	execute_process (
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		\"${_gmt_wrapper_name}\" \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${GMT_BINDIR}/gmt\")
	")
endif (UNIX AND GMT_INSTALL_NAME_SUFFIX)

# install extra files
install (PROGRAMS
	${CMAKE_CURRENT_BINARY_DIR}/isogmt${GMT_INSTALL_NAME_SUFFIX}
	${CMAKE_CURRENT_BINARY_DIR}/gmt-config${GMT_INSTALL_NAME_SUFFIX}
	DESTINATION ${GMT_BINDIR}
	COMPONENT Runtime)

install (PROGRAMS gmtswitch
	RENAME gmtswitch${GMT_INSTALL_NAME_SUFFIX}
	DESTINATION ${GMT_BINDIR}
	COMPONENT Runtime)

install (PROGRAMS gmt_shell_functions.sh
	RENAME gmt_shell_functions${GMT_INSTALL_NAME_SUFFIX}.sh
	DESTINATION ${GMT_BINDIR}
	COMPONENT Runtime)

# Install gmt_make_module_src.sh to share/tools
install (PROGRAMS gmt_make_module_src.sh
	DESTINATION ${GMT_DATADIR}/tools
	COMPONENT Runtime)

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
