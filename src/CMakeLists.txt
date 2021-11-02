#
#
# Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

##
##	Basic settings
##
include (GmtHelperMacros)
# Include CREATE_DEBUG_SYM macro and set DEBUG_BUILD
include (CreateDebugSym)

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

# Automatically add the current source- and build directories to the include path.
set (CMAKE_INCLUDE_CURRENT_DIR TRUE)

if (MSVC)
	# Disable automatic manifest generation
	string (REGEX REPLACE "/MANIFEST[^ ]*( |$)" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")

	# To expand the command line arguments in Windows, see:
	# http://msdn.microsoft.com/en-us/library/8bch7bkk.aspx
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} setargv.obj")
endif (MSVC)

##
##	Find executables needed by GMT
##
# Find executable needed by gmt
find_program (OGR2OGR ogr2ogr)
if (OGR2OGR)
	execute_process(COMMAND ogr2ogr --version OUTPUT_VARIABLE answer OUTPUT_STRIP_TRAILING_WHITESPACE)
	string (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" answer "${answer}")
	set (GMT_CONFIG_OGR2OGR_MESSAGE "yes (${answer})" CACHE INTERNAL "OGR2OGR config message")
else (OGR2OGR)
	message (WARNING "Cannot find program ogr2ogr - gmt will be unable to convert Shapefiles to GMT/OGR format")
	set (GMT_CONFIG_OGR2OGR_MESSAGE "no" CACHE INTERNAL "OGR2OGR config message")
endif (OGR2OGR)

# Find executable needed by psconvert
find_program (GDAL_TRANSLATE gdal_translate)
if (GDAL_TRANSLATE)
	execute_process(COMMAND gdal_translate --version OUTPUT_VARIABLE answer OUTPUT_STRIP_TRAILING_WHITESPACE)
	string (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" answer "${answer}")
	set (GMT_CONFIG_GDAL_TRANSLATE_MESSAGE "yes (${answer})" CACHE INTERNAL "GDAL_TRANSLATE config message")
else (GDAL_TRANSLATE)
	message (WARNING "Cannot find program gdal_translate - psconvert will be unable to convert TIFF to GeoTIFF")
	set (GMT_CONFIG_GDAL_TRANSLATE_MESSAGE "no" CACHE INTERNAL "GDAL_TRANSLATE config message")
endif (GDAL_TRANSLATE)

# Find executables needed by movie
find_program (GM gm)
if (GM)
	execute_process(COMMAND gm help OUTPUT_VARIABLE answer OUTPUT_STRIP_TRAILING_WHITESPACE)
	string (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" answer "${answer}")
	# GMT_CONFIG_GM_VERSION is used when building macOS bundle
	set (GMT_CONFIG_GM_VERSION "${answer}" CACHE INTERNAL "GM version message")
	set (GMT_CONFIG_GM_MESSAGE "yes (${answer})" CACHE INTERNAL "GM config message")
else (GM)
	message (WARNING "Cannot find GraphicsMagick program gm - gmt movie will be unable to make animated GIFs")
	set (GMT_CONFIG_GM_MESSAGE "no" CACHE INTERNAL "GM config message")
endif (GM)

find_program (FFMPEG ffmpeg)
if (FFMPEG)
	execute_process(COMMAND ffmpeg -version OUTPUT_VARIABLE answer OUTPUT_STRIP_TRAILING_WHITESPACE)
	string (REGEX MATCH "[0-9]+(.[0-9]+)?(.[0-9]+)?" answer "${answer}")
	set (GMT_CONFIG_FFMPEG_MESSAGE "yes (${answer})" CACHE INTERNAL "FFMPEG config message")
else (FFMPEG)
	message (WARNING "Cannot find ffmpeg - gmt movie will be unable to make movies from frames")
	set (GMT_CONFIG_FFMPEG_MESSAGE "no" CACHE INTERNAL "FFMPEG config message")
endif (FFMPEG)

# Find executable needed by docs
if (LINUX)
	find_program (XDGOPEN xdg-open)
	if (XDGOPEN)
		set (GMT_CONFIG_OPEN_MESSAGE "yes" CACHE INTERNAL "OPEN config message")
	else (XDGOPEN)
		message (WARNING "Cannot find program xdg-open - gmt docs will be unable to open the documentation")
		set (GMT_CONFIG_OPEN_MESSAGE "no" CACHE INTERNAL "OPEN config message")
	endif (XDGOPEN)
else (LINUX)
	set (GMT_CONFIG_OPEN_MESSAGE "yes" CACHE INTERNAL "OPEN config message")
endif (LINUX)

# Find executable needed by psconvert
if (WIN32)
	set (GMT_CONFIG_GS_MESSAGE "yes" CACHE INTERNAL "GS config message")
else (WIN32)
	find_program (GS gs)
	if (GS)
		execute_process(COMMAND gs --version OUTPUT_VARIABLE answer OUTPUT_STRIP_TRAILING_WHITESPACE)
		set (GMT_CONFIG_GS_MESSAGE "yes (${answer})" CACHE INTERNAL "GS config message")
	else (GS)
		message (WARNING "Cannot find ghostscript program gs - gmt psconvert will be unable to convert PostScript to other formats")
		set (GMT_CONFIG_GS_MESSAGE "no" CACHE INTERNAL "GS config message")
	endif (GS)
endif (WIN32)


##
##	Find libraries needed by GMT
##
message (STATUS "Searching dependent libraries. This may take a few minutes...")

# NetCDF is required
find_package (NETCDF REQUIRED)
include_directories (${NETCDF_INCLUDE_DIR})

# libcurl is required since 5.4
find_package (CURL REQUIRED)
include_directories (${CURL_INCLUDE_DIRS})
list (APPEND GMT_OPTIONAL_LIBRARIES ${CURL_LIBRARIES})
set (CURL_LIBRARY ${CURL_LIBRARIES} CACHE INTERNAL "")

if (NOT DEFINED GMT_EXCLUDE_GDAL)
	find_package (GDAL)
endif (NOT DEFINED GMT_EXCLUDE_GDAL)
if (GDAL_FOUND)
	set (HAVE_GDAL TRUE CACHE INTERNAL "System has GDAL")
	include_directories (${GDAL_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${GDAL_LIBRARIES})
endif (GDAL_FOUND)

if (NOT DEFINED GMT_EXCLUDE_GEOS)
	find_package (GEOS)
endif (NOT DEFINED GMT_EXCLUDE_GEOS)
if (GEOS_FOUND OR GEOS_LIBRARY)			# Because it fails to find GEOS on Win so I rely on ConfigUser.cmake settings
	set (HAVE_GEOS TRUE CACHE INTERNAL "System has GEOS")
	include_directories (${GEOS_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${GEOS_LIBRARY})
endif (GEOS_FOUND OR GEOS_LIBRARY)

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
	set (ZLIB_LIBRARY ${ZLIB_LIBRARY} CACHE INTERNAL "")
endif (ZLIB_FOUND)

if (APPLE)
	# Accelerate framework
	find_library (ACCELERATE_FRAMEWORK Accelerate)
	mark_as_advanced (ACCELERATE_FRAMEWORK)
	list (APPEND GMT_OPTIONAL_LIBRARIES ${ACCELERATE_FRAMEWORK})
endif (APPLE)

find_package (OpenMP)
if (GMT_OPENMP)
	set (GMT_ENABLE_OPENMP TRUE)
	message (WARNING "CMake variable GMT_OPENMP is deprecated and will be removed in the future releases. Use GMT_ENABLE_OPENMP instead.")
endif (GMT_OPENMP)
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
	set (GMT_CONFIG_GTHREAD_MESSAGE "enabled (${GLIB_VERSION})"
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

# FreeBSD uses a separate library for backtracking
if (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
	find_library (EXECINFO_LIBRARY NAMES execinfo)
	list (APPEND GMT_OPTIONAL_LIBRARIES ${EXECINFO_LIBRARY})
endif (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")

##
##	Find pre-downloaded/pre-installed GSHHG and DCW data
##  Will be automatically downloaded to user directory if not found during installing
find_package (GSHHG)
find_package (DCW)


##
##	Check for math and POSIX functions
##
include(ConfigureChecks)


##
##	License restrictions
##
if (NOT LICENSE_RESTRICTED) # off
	# enable Shewchuk's triangle routine
	set (GMT_TRIANGULATE "Shewchuk" PARENT_SCOPE)
	set (GMT_TRIANGLE_SRCS triangle.c triangle.h)
	list (APPEND GMT_EXTRA_LICENSE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/README.TRIANGLE)
	set (GMT_EXTRA_LICENSE_FILES ${GMT_EXTRA_LICENSE_FILES} PARENT_SCOPE)

	# extra ugly definitions for triangle
	set_source_files_properties (triangle.c
		PROPERTIES
		COMPILE_DEFINITIONS "NO_TIMER;TRILIBRARY;REDUCED;CDT_ONLY;ANSI_DECLARATORS")
else (NOT LICENSE_RESTRICTED) # on
	# disable Shewchuk's triangle routine
	set (GMT_TRIANGULATE "Watson" PARENT_SCOPE)
endif (NOT LICENSE_RESTRICTED)


##
##	Set list of source codes
##
# Two variables for generating gmt_${SHARED_LIB_NAME}_glue.c and gmt_${SHARED_LIB_NAME}_moduleinfo.h
set (SHARED_LIB_NAME core)
set (SHARED_LIB_PURPOSE "GMT ${SHARED_LIB_NAME}: The main modules of the Generic Mapping Tools")

# main gmt program for installing compatibility links via install_module_symlink [Windows only]
set (GMT_PROGRAM ${GMT_SOURCE_DIR}/src/gmtprogram.c)

# Legacy modules for which to install compatibility links via install_module_symlink
set (GMT_COMPAT_MODULES minmax gmtdp gmtstitch grdreformat ps2raster originator)
set (GMT_COMPAT_MODULES ${GMT_COMPAT_MODULES} PARENT_SCOPE) # needed by share/tools/gmt5syntax.in

# gmt core modules
set (GMT_PROGS_SRCS batch.c blockmean.c blockmedian.c blockmode.c dimfilter.c docs.c filter1d.c
	fitcircle.c gmt2kml.c  gmtbinstats.c gmtconnect.c gmtconvert.c gmtdefaults.c gmtget.c gmtinfo.c gmtlogo.c
	gmtmath.c gmtselect.c gmtsimplify.c gmtspatial.c gmtregress.c gmtset.c gmtvector.c gmtwhich.c
	grd2cpt.c grd2kml.c grd2xyz.c grdblend.c grdclip.c grdcontour.c grdconvert.c grdcut.c grdedit.c
	grdfft.c grdfill.c grdfilter.c grdgdal.c grdgradient.c grdhisteq.c grdimage.c grdinfo.c
	grdinterpolate.c grdlandmask.c grdmask.c grdmath.c grdmix.c grdpaste.c grdproject.c grdsample.c
	grdtrack.c grdtrend.c grdvector.c grdview.c grdvolume.c greenspline.c kml2gmt.c makecpt.c
	mapproject.c movie.c nearneighbor.c project.c psbasemap.c psclip.c pscoast.c pscontour.c
	psconvert.c psevents.c pshistogram.c psimage.c pslegend.c psmask.c psrose.c psscale.c
	pssolar.c psternary.c pstext.c pswiggle.c psxy.c psxyz.c sample1d.c spectrum1d.c sph2grd.c
	sphdistance.c sphinterpolate.c sphtriangulate.c gmtsplit.c surface.c trend1d.c trend2d.c
	triangulate.c xyz2grd.c)

# libpslib
set (GMT_PSL_SRCS postscriptlight.c declspec.h psl_config.h PSL_Standard+.h
	PSL_Standard.h PSL_ISOLatin1+.h PSL_ISOLatin1.h PSL_ISO-8859-1.h
	PSL_ISO-8859-2.h PSL_ISO-8859-3.h PSL_ISO-8859-4.h PSL_ISO-8859-5.h
	PSL_ISO-8859-6.h PSL_ISO-8859-7.h PSL_ISO-8859-8.h PSL_ISO-8859-9.h
	PSL_ISO-8859-10.h PSL_ISO-8859-11.h PSL_ISO-8859-13.h PSL_ISO-8859-14.h
	PSL_ISO-8859-15.h PSL_ISO-8859-16.h
	PSL_strings.h PSL_patterns.h)

set (GMT_MBSYSTEM_SRCS gmt_mbsystem_glue.c)

# libgmt
set (GMT_LIB_SRCS block_subs.h gmt_common_byteswap.h gmt_common_math.h
	gmt_common_runpath.h gmt_common_sighandler.h gmt_common_string.h clear.c begin.c
	gmt_color_rgb.h gmt_colornames.h gmt_media_size.h gmt_mgg_header2.h
	gmt_modern.c end.c declspec.h gmt_hidden.h gmt_remote.h gmt_remote.c
	figure.c gmt_dev.h gmt_common.h gmt_common_longoptions.h gmt_constants.h compat/qsort.h
	gmt_contour.h gmt_customio.h gmt_datums.h gmt_dcw.h gmt_decorate.h gmt_defaults.h
	gmt_ellipsoids.h gmt_error.h gmt_gdalread.h gmt_glib.h gmt_grd.h gmt_grdio.h gmt_hash.h
	gmt_internals.h gmt_io.h gmt_macros.h gmt_media_name.h gmt_modern.h
	gmt_nan.h gmt_notposix.h gmt_pennames.h gmt_enum_dict.h gmt_memory.h gmt_mb.h
	gmt_plot.h gmt_project.h gmt_prototypes.h gmt_psl.h gmt_shore.h gmt_gsformats.h
	gmt_symbol.h gmt_synopsis.h gmt_texture.h gmt_time.h gmt_cpt_masters.h
	gmt_types.h gmt_unique.h gmt_version.h gshhg_version.h postscriptlight.h
	s_rint.h triangle.h gmt.h gmt_resources.h gmt_private.h subplot.c
	gmt_error_codes.h gmt_api.c gmt_bcr.c gmt_calclock.c gmt_cdf.c
	gmt_customio.c gmt_dcw.c gmt_error.c gmt_fft.c gmt_grdio.c gmt_io.c
	gmt_memory.c gmt_nc.c gmt_notposix.c gmt_shore.c gmt_sph.c
	gmt_stat.c gmt_support.c gmt_regexp.c gmt_vector.c gmt_parse.c inset.c
	gmt_error_codes.c gshhg_version.c gmtread.c gmtwrite.c gmt_common_math.c
	gmt_common_runpath.c gmt_common_sighandler.c gmt_common_string.c compat/qsort.c
	gmt_sharedlibs.c  gmt_init.c gmt_map.c gmt_plot.c gmt_gdalcall.c
	${CMAKE_CURRENT_BINARY_DIR}/gmt_${SHARED_LIB_NAME}_moduleinfo.h
	${CMAKE_CURRENT_BINARY_DIR}/gmt_sharedlibs.h
	${CMAKE_CURRENT_BINARY_DIR}/gmt_${SHARED_LIB_NAME}_glue.c
	kiss_fft/_kiss_fft_guts.h kiss_fft/kiss_fft.c kiss_fft/kiss_fft.h
	kiss_fft/kiss_fftnd.c kiss_fft/kiss_fftnd.h
	${GMT_TRIANGLE_SRCS} ${GMT_MBSYSTEM_SRCS} ${EXTRA_SOURCES})

# source codes for testing API
if (DO_API_TESTS)
	# Include all test*.c files
	file (GLOB GMT_DEMOS_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/test*.c")
endif (DO_API_TESTS)


##
##	Generate header files
##
# dependencies on generated files
set (GMT_GEN_HDRS gmt_keycases.h gmt_keywords.h gmt_config.h gmt_dimensions.h gmt_${SHARED_LIB_NAME}_moduleinfo.h
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
	# depends on cached line numbers from these header files
	DEPENDS gmt_media_name.h gmt_pennames.h gmt_unique.h gmt_ellipsoids.h
	gmt_datums.h gmt_colornames.h gmt_cpt_masters.h
	gmt_keycases.h)

# generate gmt_core_moduleinfo.h
set (_core_modules ${GMT_PROGS_SRCS} begin.c clear.c end.c figure.c inset.c subplot.c gmtread.c gmtwrite.c)
add_custom_command (OUTPUT gmt_${SHARED_LIB_NAME}_moduleinfo.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_moduleinfo_h
	-D OUTPUT_HEADER_FILE="gmt_${SHARED_LIB_NAME}_moduleinfo.h"
	-D PROGS_SRCS="${_core_modules}"
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS ${_core_modules})

# autogenerated files target
add_custom_target (gen_gmt_headers DEPENDS ${GMT_GEN_HDRS})

# Configure header files to pass some of the CMake settings to the source code
configure_file (gmt_config.h.in gmt_config.h)
configure_file (psl_config.h.in psl_config.h)
configure_file (gmt_version.h.in gmt_version.h)
configure_file (isogmt.in isogmt${GMT_INSTALL_NAME_SUFFIX} @ONLY)
configure_file (gmt-config.in gmt-config${GMT_INSTALL_NAME_SUFFIX} @ONLY)
configure_file (gmt_sharedlibs.h.in gmt_sharedlibs.h)
configure_file (gmt_glue.c.in gmt_${SHARED_LIB_NAME}_glue.c)

##
## 	Settings for pslib
##
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

if (HAVE_DLADDR AND HAVE_LIBDL)
	# link the dynamic linking loader library
	target_link_libraries (pslib dl)
endif (HAVE_DLADDR AND HAVE_LIBDL)

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (pslib
	PROPERTIES
	OUTPUT_NAME postscriptlight
	RUNTIME_OUTPUT_NAME postscriptlight
	PREFIX "${LIB_PREFIX}"
	IMPORT_PREFIX "${LIB_PREFIX}"
	VERSION ${GMT_LIB_VERSION}
	SOVERSION ${GMT_LIB_SOVERSION}
	DEFINE_SYMBOL "LIBRARY_EXPORTS")

# If a renaming of the psl dll has been set in ConfigUserAdvanced.cmake
if (WIN32 AND PSL_DLL_RENAME)
	set_target_properties (pslib PROPERTIES RUNTIME_OUTPUT_NAME ${PSL_DLL_RENAME})
endif (WIN32 AND PSL_DLL_RENAME)


##
##	Settings for GMT core library
##
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

if (HAVE_DLADDR AND HAVE_LIBDL)
	# link the dynamic linking loader library
	target_link_libraries (gmtlib dl)
endif (HAVE_DLADDR AND HAVE_LIBDL)

if (MSVC AND FFTW3_FOUND)
	target_link_libraries (gmtlib ws2_32)
endif (MSVC AND FFTW3_FOUND)

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (gmtlib
	PROPERTIES
	OUTPUT_NAME gmt
	RUNTIME_OUTPUT_NAME gmt
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


##
## Add executables
##
# build targets for standalone gmt target and demos
string (REPLACE ".c" "" _gmt_progs "gmt.c;${GMT_DEMOS_SRCS}")
foreach (_gmt_prog ${_gmt_progs})
	add_executable (${_gmt_prog} ${_gmt_prog}.c)
	target_link_libraries (${_gmt_prog} gmtlib)
endforeach (_gmt_prog)

# Rename gmt target to prevent version clash
set_target_properties (gmt PROPERTIES OUTPUT_NAME gmt${GMT_INSTALL_NAME_SUFFIX})

# psldemo
add_executable (psldemo psldemo.h psldemo.c)
target_link_libraries (psldemo pslib)

# gshhg_version
add_executable (gshhg_version gshhg_version.h gshhg_version.c)
set_target_properties (gshhg_version
	PROPERTIES COMPILE_DEFINITIONS "STANDALONE")
target_link_libraries (gshhg_version ${NETCDF_LIBRARIES})

# script2verbatim for removing comments, replacing -ps from example scripts
add_executable (script2verbatim script2verbatim.c)


##
##	Dependencies for testing
##
# Tests depend on GMT main program and API testing programs
add_dependencies (check ${_gmt_progs} psldemo)


##
## Debugging symbols
##
# generate and install macOS/Windows debugging symbols
create_debug_sym (${GMT_BINDIR} gmt)
create_debug_sym (${GMT_BINDIR} gmtlib pslib)


##
##	Install binaries and libraries
##
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

# symlink to gmt-wrapper in bindir and libdir:
if (UNIX AND GMT_INSTALL_NAME_SUFFIX)
	get_target_property(_gmt_wrapper_name gmt OUTPUT_NAME)
	install (CODE "
	execute_process (
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		\"${_gmt_wrapper_name}\" \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${GMT_BINDIR}/gmt\")
	")
endif (UNIX AND GMT_INSTALL_NAME_SUFFIX)


##
## Install extra scripts
##
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

if (WIN32)
	install (PROGRAMS gmtswitch.bat
		DESTINATION ${GMT_BINDIR}
		COMPONENT Runtime)
endif (WIN32)


##
##	Install headers
##
# Install public headers
install (FILES gmt.h gmt_resources.h declspec.h
	DESTINATION ${GMT_INCLUDEDIR}
	COMPONENT Runtime)

# Install developer headers
if (BUILD_DEVELOPER)
	# Install Developer headers [These are clean and have no #define HAVE_* configuration]
	install (FILES postscriptlight.h gmt_common_math.h gmt_common_string.h gmt_common.h gmt_constants.h gmt_contour.h
		gmt_dcw.h gmt_decorate.h gmt_defaults.h gmt_error.h gmt_error_codes.h gmt_fft.h gmt_gdalread.h gmt_grd.h
		gmt_grdio.h gmt_hash.h gmt_io.h gmt_macros.h gmt_memory.h gmt_modern.h gmt_nan.h gmt_notposix.h gmt_plot.h
		gmt_private.h gmt_project.h gmt_prototypes.h gmt_psl.h gmt_shore.h gmt_symbol.h gmt_synopsis.h
		gmt_texture.h gmt_time.h gmt_types.h gmt_dev.h gmt_customio.h gmt_hidden.h gmt_mb.h gmt_remote.h
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


##
##	Settings for GMT and non-GMT supplemental libraries
##
if (BUILD_SUPPLEMENTS)
	if (UNIX)
		set (CMAKE_SHARED_MODULE_SUFFIX .so)
	endif (UNIX)

	# supplement directories to be built as supplement libraries
	# SUPPL_EXTRA_DIRS are directories for custom supplements.
	# See cmake/ConfigUserAdvancedTemplate.cmake for details.
	set (GMT_SUPPL_DIRS geodesy gshhg img mgd77 potential segy seis spotter x2sys ${SUPPL_EXTRA_DIRS})

	# process supplement directories and collect information for each supplement libraries
	set (GMT_SUPPL_LIBRARIES) # empty the library list
	foreach (_dir ${GMT_SUPPL_DIRS})
		# include CMake settings in supplement directories
		add_subdirectory (${_dir})

		# include supplement directories
		include_directories (${CMAKE_CURRENT_BINARY_DIR}/${_dir})

		# supplement extra (non-GMT) include files, if any
		get_subdir_var (_suppl_extra_includes SUPPL_EXTRA_INCLUDES ${_dir})
		include_directories (${_suppl_extra_includes})

		# get supplement library name ["${GMT_SUPPL_LIB_NAME}" if undefined]
		get_subdir_var (_suppl_lib_name SUPPL_LIB_NAME ${_dir})
		if (NOT _suppl_lib_name)
			set (_suppl_lib_name "${GMT_SUPPL_LIB_NAME}")
		endif (NOT _suppl_lib_name)
		list (APPEND GMT_SUPPL_LIBRARIES ${_suppl_lib_name})

		# supplement (both GMT and non-GMT) source files of modules
		get_subdir_var_files (_suppl_progs_srcs SUPPL_PROGS_SRCS ${_dir})
		list (APPEND SUPPL_${_suppl_lib_name}_PROGS_SRCS ${_suppl_progs_srcs})

		# supplement (both GMT and non-GMT) source files for each supplement library
		get_subdir_var_files (_suppl_lib_srcs SUPPL_LIB_SRCS ${_dir})
		list (APPEND SUPPL_${_suppl_lib_name}_LIB_SRCS ${_suppl_lib_srcs})

		# supplement extra libraries for each supplement library
		get_subdir_var (_suppl_extra_libraries SUPPL_EXTRA_LIBS ${_dir})
		list (APPEND SUPPL_${_suppl_lib_name}_EXTRA_LIBRARIES ${_suppl_extra_libraries})

		# library purpose of non-official supplement packages
		get_subdir_var (SUPPL_${_suppl_lib_name}_LIB_PURPOSE SUPPL_LIB_PURPOSE ${_dir})

		# rename the supplement dll [Windows only]
		get_subdir_var (SUPPL_${_suppl_lib_name}_DLL_RENAME SUPPL_DLL_RENAME ${_dir})
	endforeach (_dir)
	# remove duplicate supplement library names, so multiple supplement packages can be built into one single library
	list (REMOVE_DUPLICATES GMT_SUPPL_LIBRARIES)

	# loop over all supplement libraries
	foreach (_suppl_lib_name ${GMT_SUPPL_LIBRARIES})
		# Two variables for gmt_${SHARED_LIB_NAME}_glue.c and gmt_${SHARED_LIB_NAME}_modueinfo.h
		set (SHARED_LIB_NAME ${_suppl_lib_name})
		if (${_suppl_lib_name} STREQUAL ${GMT_SUPPL_LIB_NAME})
			set (SHARED_LIB_PURPOSE "GMT ${GMT_SUPPL_LIB_NAME}: The official supplements to the Generic Mapping Tools")
		else (${_suppl_lib_name} STREQUAL ${GMT_SUPPL_LIB_NAME})
			if (SUPPL_${_suppl_lib_name}_LIB_PURPOSE)
				set (SHARED_LIB_PURPOSE "${SUPPL_${_suppl_lib_name}_LIB_PURPOSE}")
			else (SUPPL_${_suppl_lib_name}_LIB_PURPOSE)
				set (SHARED_LIB_PURPOSE	"GMT ${_suppl_lib_name}: The non-official supplements to the Generic Mapping Tools")
			endif (SUPPL_${_suppl_lib_name}_LIB_PURPOSE)
		endif (${_suppl_lib_name} STREQUAL ${GMT_SUPPL_LIB_NAME})

		# generate gmt_${SHARED_LIB_NAME}_glue.c
		configure_file (gmt_glue.c.in gmt_${SHARED_LIB_NAME}_glue.c)
		# Needed in supplement libraries
		set (GMT_SUPPL_SRCS ${CMAKE_CURRENT_BINARY_DIR}/gmt_${SHARED_LIB_NAME}_glue.c)

		# generate gmt_${SHARED_LIB_NAME}_moduleinfo.h
		add_custom_command (OUTPUT gmt_${SHARED_LIB_NAME}_moduleinfo.h
			COMMAND ${CMAKE_COMMAND}
			-D GENERATE_COMMAND=gen_gmt_moduleinfo_h
			-D OUTPUT_HEADER_FILE="gmt_${SHARED_LIB_NAME}_moduleinfo.h"
			-D PROGS_SRCS="${SUPPL_${_suppl_lib_name}_PROGS_SRCS}"
			-D GMT_SRC=${GMT_SOURCE_DIR}
			-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
			-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
			DEPENDS ${SUPPL_${_suppl_lib_name}_PROGS_SRCS})
		add_custom_target (gen_gmt_suppl_${_suppl_lib_name}_headers DEPENDS gmt_${SHARED_LIB_NAME}_moduleinfo.h)

		if (WIN32)
			add_library (${_suppl_lib_name}
				${GMT_GEN_HDRS}
				${GMT_SUPPL_SRCS}
				${SUPPL_${_suppl_lib_name}_LIB_SRCS})
		else (WIN32)
			add_library (${_suppl_lib_name} MODULE
				${GMT_GEN_HDRS}
				${GMT_SUPPL_SRCS}
				${SUPPL_${_suppl_lib_name}_LIB_SRCS})
		endif (WIN32)

		add_dependencies (${_suppl_lib_name} gen_gmt_headers) # make supplib after gen_gmt_headers
		add_dependencies (${_suppl_lib_name} gen_gmt_suppl_${_suppl_lib_name}_headers) # make supplib after gen_gmt_suppl_headers
		add_dependencies (${_suppl_lib_name} pslib) # make supplib after pslib
		add_dependencies (${_suppl_lib_name} gmtlib) # make supplib after pslib

		# No SOVERSION & VERSION for a MODULE, only for SHARED libs
		target_link_libraries (${_suppl_lib_name}
			gmtlib
			pslib
			${SUPPL_${_suppl_lib_name}_EXTRA_LIBRARIES})

		if (HAVE_DLADDR AND HAVE_LIBDL)
			# link the dynamic linking loader library
			target_link_libraries (${_suppl_lib_name} dl)
		endif (HAVE_DLADDR AND HAVE_LIBDL)

		if (HAVE_M_LIBRARY)
			# link the math library
			target_link_libraries (${_suppl_lib_name} m)
		endif (HAVE_M_LIBRARY)

		set_target_properties (${_suppl_lib_name}
			PROPERTIES
			OUTPUT_NAME ${_suppl_lib_name}
			RUNTIME_OUTPUT_NAME ${_suppl_lib_name}
			LIBRARY_OUTPUT_DIRECTORY plugins
			RUNTIME_OUTPUT_DIRECTORY plugins
			PREFIX ""
			LINK_FLAGS "${USER_GMTLIB_LINK_FLAGS}"
			DEFINE_SYMBOL "LIBRARY_EXPORTS")

		# Rename ${GMT_SUPPL_LIB_NAME}.dll to SUPP_DLL_RENAME
		if (WIN32 AND ${_suppl_lib_name} STREQUAL "${GMT_SUPPL_LIB_NAME}" AND SUPP_DLL_RENAME)
			set_target_properties (${_suppl_lib_name} PROPERTIES RUNTIME_OUTPUT_NAME ${SUPP_DLL_RENAME})
		endif (WIN32 AND ${_suppl_lib_name} STREQUAL "${GMT_SUPPL_LIB_NAME}" AND SUPP_DLL_RENAME)

		# Rename non-GMT supplement DLLs
		if (WIN32 AND SUPPL_${_suppl_lib_name}_DLL_RENAME)
			set_target_properties (${_suppl_lib_name} PROPERTIES RUNTIME_OUTPUT_NAME ${SUPPL_${_suppl_lib_name}_DLL_RENAME})
		endif (WIN32 AND SUPPL_${_suppl_lib_name}_DLL_RENAME)

		# Testing
		add_dependencies (check ${_suppl_lib_name})

		# Debugging symbols
		if (WIN32)
			create_debug_sym (${GMT_BINDIR}/gmt_plugins ${_suppl_lib_name})
		else (WIN32)
			create_debug_sym (${GMT_LIBDIR}/gmt${GMT_INSTALL_NAME_SUFFIX}/plugins ${_suppl_lib_name})
		endif (WIN32)

		# install target
		install (TARGETS ${_suppl_lib_name}
			LIBRARY DESTINATION ${GMT_LIBDIR}/gmt${GMT_INSTALL_NAME_SUFFIX}/plugins # UNIX
			COMPONENT Runtime
			RUNTIME DESTINATION ${GMT_BINDIR}/gmt_plugins # Windows
			COMPONENT Runtime)
	endforeach (_suppl_lib_name ${GMT_SUPPL_LIBRARIES})

	# install more
	foreach (_dir ${GMT_SUPPL_DIRS})
		# get variables from subdirectory
		get_subdir_var (_suppl_name SUPPL_NAME ${_dir})
		get_subdir_var (_suppl_progs_srcs SUPPL_PROGS_SRCS ${_dir})
		get_subdir_var_files (_suppl_headers SUPPL_HEADERS ${_dir})
		get_subdir_var_files (_suppl_example_files SUPPL_EXAMPLE_FILES ${_dir})
		get_subdir_var_files (_suppl_example_progs SUPPL_EXAMPLE_PROGS ${_dir})

		# create build targets
		string (REPLACE ".c" "" _gmt_modules "${_suppl_progs_srcs}")
		install_module_symlink (${_gmt_modules})

		# install example files
		install (FILES ${_suppl_example_files}
			DESTINATION ${GMT_DOCDIR}/${GMT_SUPPL_LIB_NAME}/${_suppl_name}
			COMPONENT Documentation)

		# install example scriptss
		install (PROGRAMS ${_suppl_example_progs}
			DESTINATION ${GMT_DOCDIR}/${GMT_SUPPL_LIB_NAME}/${_suppl_name}
			COMPONENT Documentation)

		# install developer headers
		if (BUILD_DEVELOPER)
			install (FILES ${_suppl_headers}
				DESTINATION ${GMT_INCLUDEDIR}/${_suppl_name}
				COMPONENT Runtime)
		endif (BUILD_DEVELOPER)
	endforeach (_dir)
endif (BUILD_SUPPLEMENTS)


##
## Print out CFLAGS
##
# this prints out the final version of the compiler/linker flags
string (TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
string(REGEX REPLACE "[ ]+" " " _using_cflags ${CMAKE_C_FLAGS} " "
	${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}})
message (STATUS "Using CFLAGS = '${_using_cflags}'")
