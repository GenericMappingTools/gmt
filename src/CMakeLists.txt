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
# src/CMakeLists.txt
#

include (GmtHelperMacros)

# Include GMT_CREATE_MANPAGES macro
include (GmtManpages)

# Include CREATE_DEBUG_SYM macro
include (CreateDebugSym)

find_package (NETCDF REQUIRED)
include_directories (${NETCDF_INCLUDE_DIR})
add_definitions (${NETCDF_DEFINITIONS})

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

find_package (Matlab)
if (GMT_MATLAB AND MATLAB_FOUND)
	# We want the Matlab API
	set (GMT_MATLAB yes PARENT_SCOPE)
	include_directories (${MATLAB_INCLUDE_DIR})
	list (APPEND GMT_OPTIONAL_LIBRARIES ${MATLAB_LIBRARIES})
else (GMT_MATLAB AND MATLAB_FOUND)
	# Don't build Matlab API
	set (GMT_MATLAB no PARENT_SCOPE)
endif (GMT_MATLAB AND MATLAB_FOUND)

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

# find GSHHS shorelines
find_package (GSHHS)

include_directories (${GMT_SOURCE_DIR}/src)
include_directories (${CMAKE_CURRENT_BINARY_DIR})
#include_directories (${PROJECT_BINARY_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)


# These three lines are temporarily here for the beta release
set (GMT_PROGS2_SRCS gmtaverage.c)
set (GMT_PROGSPS2_SRCS testapi.c)
set (GMT_DEMOS2_SRCS testgmt5.c testgmtio.c testio.c)

# source files
set (GMT_DEMOS_SRCS gmt.c ${GMT_DEMOS2_SRCS})

if (TRIANGLE_D)
	set (GMT_TRIANGLE_SRCS triangle.c triangle.h)
endif (TRIANGLE_D)

set (GMT_PROGS_SRCS blockmean.c blockmedian.c blockmode.c
	colmath.c filter1d.c fitcircle.c gmt2kml.c
	gmtconvert.c gmtdp.c gmtmath.c gmtselect.c
	gmtspatial.c gmtstitch.c gmtvector.c gmtwhich.c
	grd2cpt.c grd2rgb.c grd2xyz.c grdblend.c
	grdclip.c grdcut.c grdedit.c grdfft.c
	grdfilter.c grdgradient.c grdhisteq.c grdinfo.c
	grdlandmask.c grdmask.c grdmath.c grdpaste.c
	grdproject.c grdsample.c grdreformat.c grdtrack.c
	grdtrend.c grdvolume.c greenspline.c kml2gmt.c
	makecpt.c mapproject.c minmax.c nearneighbor.c
	project.c ps2raster.c sample1d.c spectrum1d.c
	splitxyz.c surface.c trend1d.c trend2d.c
	triangulate.c xyz2grd.c
)
list (APPEND GMT_PROGS_SRCS ${GMT_PROGS2_SRCS})

set (GMT_PROGSPS_SRCS gmtdefaults.c gmtget.c gmtset.c
	grdcontour.c grdimage.c grdvector.c grdview.c
	psbasemap.c psclip.c pscoast.c pscontour.c
	pshistogram.c psimage.c psmask.c psrose.c
	psscale.c pstext.c pswiggle.c psxy.c psxyz.c
	pslegend.c
)
list (APPEND GMT_PROGSPS_SRCS ${GMT_PROGSPS2_SRCS})

# libpslib
set (GMT_PSL_SRCS pslib.c gmt_notposix.h)

# libgmt
set (GMT_LIB_SRCS gmt_bcr.c gmt_calclock.c gmt_cdf.c gmt_customio.c
	gmt_fft.c gmt_grdio.c gmt_io.c  gmt_nc.c gmt_notposix.c gmt_proj.c
	gmt_shore.c gmt_stat.c gmt_support.c gmt_regexp.c gmt_vector.c
	gmt_version.c gmtapi_parse.c ${GMT_PROGS_SRCS} ${GMT_TRIANGLE_SRCS}
)

set (GMT_LIB_PS_SRCS gmt_init.c gmt_map.c gmt_plot.c gmtapi_util.c ${GMT_PROGSPS_SRCS})

set (GMT_LIB_HDRS gmt_datums.h gmt_prognames.h gmt_progcases.h
	gmt_colornames.h gmt_color_rgb.h gmt_ellipsoids.h gmt_grdkeys.h
	gmt_keycases.h gmt_keywords.h gmtapi_errno.h gmtapi_errstr.h
	gmt_dimensions.h gmtmath.h gmtmath_op.h gmtmath_explain.h grdmath.h
	grdmath_op.h grdmath_explain.h gmt_progpurpose.h gmt_config.h)

# lib targets
set (GMT_LIBS gmtlib pslib)

# manpages
string (REPLACE ".c" ".1" GMT_MAN_1 "${GMT_PROGS_SRCS};${GMT_PROGSPS_SRCS}")
list (APPEND GMT_MAN_1 gmt.1 gmt_shell_functions.1 gmtlogo.1 isogmt.1 gmtswitch.1)
set (GMT_MAN_3 pslib.3)
set (GMT_MAN_5 gmtcolors.5 gmt.conf.5)
set (GMT_MAN_DEPS common_options.txt
	explain_-A.txt explain_-B.txt explain_-B_full.txt explain_-I.txt
	explain_-J.txt explain_-J_full.txt explain_-Jz.txt explain_-Jz_full.txt
	explain_-K.txt explain_-K_full.txt explain_-L_scale.txt explain_-O.txt
	explain_-O_full.txt explain_-P.txt explain_-P_full.txt explain_-R.txt
	explain_-R_full.txt explain_-Rgeo.txt explain_-Rz.txt explain_-Rz_full.txt
	explain_-T_rose.txt explain_-U.txt explain_-U_full.txt explain_-V.txt
	explain_-V_full.txt explain_-XY.txt explain_-XY_full.txt
	explain_-aspatial.txt explain_-aspatial_full.txt explain_-bi.txt
	explain_-bi_full.txt explain_-bo.txt explain_-bo_full.txt explain_-c.txt
	explain_-c_full.txt explain_-f.txt explain_-f_full.txt explain_-g.txt
	explain_-g_full.txt explain_-h.txt explain_-h_full.txt explain_-icols.txt
	explain_-icols_full.txt explain_-n.txt explain_-n_full.txt
	explain_-ocols.txt explain_-ocols_full.txt explain_-s.txt
	explain_-s_full.txt explain_-t.txt explain_-t_full.txt explain_colon.txt
	explain_color.txt explain_commonitems.txt explain_contdump.txt
	explain_contlabel.txt explain_distunits.txt explain_fill.txt
	explain_float.txt explain_font.txt explain_grd_coord.txt
	explain_grd_inout.txt explain_grd_input.txt explain_grd_output.txt
	explain_gshhs.txt explain_help.txt explain_intables.txt
	explain_labelinfo.txt explain_nodereg.txt explain_nodereg_full.txt
	explain_pen.txt explain_perspective.txt explain_perspective_full.txt
	explain_precision.txt explain_symbols.txt refs.i
)

# add generated depends
set (GMT_MAN_DEPS ${GMT_MAN_DEPS} Colors.i Fonts.i Ellipsoids.i
	grdreformat_man.i gmtmath_man.i grdmath_man.i)

# rules for generating missing files
include (GmtGenExtraHeaders)

# generate gmt_progpurpose.h
gmt_make_purpose_h (
	GMT_PROG_PURPOSE
	${GMT_PROGS_SRCS}
	${GMT_PROGSPS_SRCS})

configure_file (gmt_config.h.cmake gmt_config.h)
configure_file (gmt_version.h.cmake gmt_version.h)
configure_file (isogmt.cmake isogmt @ONLY)

# gmtprogram
set (GMT_PROGRAM ${GMT_SOURCE_DIR}/src/gmtprogram.c)

# extra ugly definitions for triangle
set_source_files_properties (triangle
	PROPERTIES
	COMPILE_DEFINITIONS "NO_TIMER;TRILIBRARY;REDUCED;CDT_ONLY")

# libpslib
add_library (pslib ${GMT_PSL_SRCS})

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (pslib
	PROPERTIES
	VERSION ${GMT_PACKAGE_VERSION}
	SOVERSION ${GMT_PACKAGE_VERSION_MAJOR}
	INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
	DEFINE_SYMBOL "LIBRARY_EXPORTS"
)

# libgmt
add_library (gmtlib ${GMT_LIB_HDRS} ${GMT_LIB_SRCS} ${GMT_LIB_PS_SRCS})

# make sure headers are generated first
#add_dependencies(gmtlib headers)

target_link_libraries (gmtlib
	${NETCDF_LIBRARIES}
	${GMT_OPTIONAL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	pslib
)

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (gmtlib
	PROPERTIES
	OUTPUT_NAME gmt
	VERSION ${GMT_PACKAGE_VERSION}
	SOVERSION ${GMT_PACKAGE_VERSION_MAJOR}
	INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
	LINK_FLAGS "${USER_GMTLIB_LINK_FLAGS}"
	DEFINE_SYMBOL "LIBRARY_EXPORTS"
)

# create build targets for gmt progs
string (REPLACE ".c" "" GMT_PROGS "${GMT_PROGS_SRCS}")
foreach (_gmtprog ${GMT_PROGS})
	add_executable (${_gmtprog} ${GMT_PROGRAM})
	set_target_properties (${_gmtprog} PROPERTIES
		COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMT;FUNC=GMT_${_gmtprog}"
	)
	target_link_libraries (${_gmtprog} gmtlib)
endforeach (_gmtprog)

# create build targets for gmt progsps
string (REPLACE ".c" "" GMT_PROGSPS "${GMT_PROGSPS_SRCS}")
foreach (_gmtprogps ${GMT_PROGSPS})
	add_executable (${_gmtprogps} ${GMT_PROGRAM})
	set_target_properties (${_gmtprogps}
		PROPERTIES COMPILE_DEFINITIONS
		"FUNC_MODE=GMTAPI_GMTPSL;FUNC=GMT_${_gmtprogps}")
	target_link_libraries (${_gmtprogps} gmtlib)
endforeach (_gmtprogps)

# generate gmt_modules.h
set (GMT_SUPPL_STRING "modules")
gmt_set_api_header(GMT_MODULES_API_H ${GMT_PROGS} ${GMT_PROGSPS})

# build targets for demos
string (REPLACE ".c" "" GMT_DEMOS "${GMT_DEMOS_SRCS}")
foreach (_gmtdemo ${GMT_DEMOS})
	add_executable (${_gmtdemo} ${_gmtdemo}.c)
	target_link_libraries (${_gmtdemo} gmtlib)
endforeach (_gmtdemo)

configure_file (psldemo.c.cmake psldemo.c @ONLY)
add_executable (psldemo psldemo.c)
target_link_libraries (psldemo pslib)

# generate Mac debugging symbols
create_debug_sym (${GMT_LIBS} ${GMT_PROGS} ${GMT_PROGSPS})

# add the install targets
install (TARGETS ${GMT_LIBS} ${GMT_PROGS} ${GMT_PROGSPS} ${GMT_DEMOS} psldemo
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)

# install extra files
if (UNIX)
	install (PROGRAMS
		${CMAKE_CURRENT_BINARY_DIR}/isogmt
		gmtlogo
		gmtswitch
		gmt_shell_functions.sh
		DESTINATION bin)
elseif (WIN32)
	install (PROGRAMS
		gmtswitch.bat
		DESTINATION bin)
endif (UNIX)

# Generate manpages
gmt_create_manpages (${GMT_MAN_1} ${GMT_MAN_3} ${GMT_MAN_5}
	DEPENDS ${GMT_MAN_DEPS})

# add subtrees
add_subdirectory (dbase)
add_subdirectory (gshhs)
add_subdirectory (imgsrc)
add_subdirectory (meca)
#add_subdirectory (mgd77) # have to rewrite header generators
add_subdirectory (misc)
add_subdirectory (potential)
add_subdirectory (segy)
# add_subdirectory (sph) # have to rewrite generators
add_subdirectory (spotter)
#add_subdirectory (x2sys) # depends on mgd77
add_subdirectory (xgrid)

# this prints out the final version of the compiler/linker flags
string (TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
string(REGEX REPLACE "[ ]+" " " _using_cflags ${CMAKE_C_FLAGS} " "
	${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}} " " ${NETCDF_DEFINITIONS})
message (STATUS "Using CFLAGS = '${_using_cflags}'")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
