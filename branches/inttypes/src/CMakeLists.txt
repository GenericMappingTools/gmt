#
# $Id$
#
# Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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
# src/CMakeLists.txt
#

include (ManageString)
include (GmtHelperMacros)

# Include GMT_CREATE_MANPAGES macro
include (GmtManpages)

# Include CREATE_DEBUG_SYM macro
include (CreateDebugSym)

# Find libraries
message (STATUS "Searching dependent libraries. This may take a few minutes...")
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
	list (APPEND GMT_OPTIONAL_LIBRARIES ${MATLAB_MEX_LIBRARY})
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

include_directories (${GMT_SOURCE_DIR}/src)
include_directories (${CMAKE_CURRENT_BINARY_DIR})
#include_directories (${PROJECT_BINARY_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

# Examples and tests depend on Shorelines
find_package (GSHHG)
if (DO_EXAMPLES OR DO_TESTS AND NOT GSHHG_FOUND)
	message (FATAL_ERROR "Cannot proceed without GSHHG Shorelines. "
		"Need to either set GSHHG_ROOT or disable tests.")
endif (DO_EXAMPLES OR DO_TESTS AND NOT GSHHG_FOUND)

# These three lines are temporarily here for the beta release
set (GMT_PROGS2_SRCS gmtaverage.c)
set (GMT_PROGSPS2_SRCS testapi.c)
set (GMT_DEMOS2_SRCS gmtmercmap.c testgmt5.c testgmtio.c testio.c)

# source files
set (GMT_DEMOS_SRCS gmt.c ${GMT_DEMOS2_SRCS})

if (NOT LICENSE_RESTRICTED) # off
	# enable Shewchuk's triangle routine
	set (GMT_TRIANGULATE "Shewchuk" PARENT_SCOPE)
	set (GMT_TRIANGLE_SRCS triangle.c triangle.h)
	list (APPEND GMT_EXTRA_LICENSE_FILES ${CMAKE_CURRENT_DIR}/README.TRIANGLE)
else (NOT LICENSE_RESTRICTED) # on
	# disable Shewchuk's triangle routine
	set (GMT_TRIANGULATE "Watson" PARENT_SCOPE)
endif (NOT LICENSE_RESTRICTED)

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
	triangulate.c xyz2grd.c)
list (APPEND GMT_PROGS_SRCS ${GMT_PROGS2_SRCS})

set (GMT_PROGSPS_SRCS gmtdefaults.c gmtget.c gmtset.c
	grdcontour.c grdimage.c grdvector.c grdview.c
	psbasemap.c psclip.c pscoast.c pscontour.c
	pshistogram.c psimage.c psmask.c psrose.c
	psscale.c pstext.c pswiggle.c psxy.c psxyz.c
	pslegend.c)
list (APPEND GMT_PROGSPS_SRCS ${GMT_PROGSPS2_SRCS})

# libpslib
set (GMT_PSL_SRCS pslib.c common_string.c common_string.h common_runpath.c
	common_runpath.h gmt_notposix.h)

# libgmt
set (GMT_LIB_SRCS gmt_bcr.c gmt_calclock.c gmt_cdf.c gmt_customio.c
	gmt_fft.c gmt_grdio.c gmt_io.c  gmt_nc.c gmt_notposix.c gmt_proj.c
	gmt_shore.c gmt_stat.c gmt_support.c gmt_regexp.c gmt_vector.c
	gmtapi_parse.c gshhg_version.c gshhg_version.h
	common_math.c common_string.c common_runpath.c
	${GMT_PROGS_SRCS} ${GMT_TRIANGLE_SRCS})

set (GMT_LIB_PS_SRCS gmt_init.c gmt_map.c gmt_plot.c gmtapi_util.c
	${GMT_PROGSPS_SRCS})

# dependencies on generated files
set (GMT_GEN_HDRS gmt_color_rgb.h gmt_colornames.h gmt_datums.h
	gmt_ellipsoids.h gmt_grdkeys.h gmt_keycases.h
	gmt_keywords.h gmt_progcases.h gmt_prognames.h gmtapi_errno.h
	gmtapi_errstr.h gmt_progpurpose.h gmt_config.h
	gmtmath.h gmtmath_op.h gmtmath_explain.h
	grdmath.h grdmath_op.h grdmath_explain.h
	gmt_dimensions.h
	CACHE INTERNAL "Autogenerated GMT headers")

# header target
#add_custom_target (gmt_headers DEPENDS ${GMT_LIB_HDRS})

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
	explain_precision.txt explain_symbols.txt refs.i)

# dependencies on generated files
set (GMT_GEN_MAN_HDRS Colors.i Ellipsoids.i Fonts.i grdreformat_man.i
	gmtmath_man.i grdmath_man.i
	CACHE INTERNAL "Autogenerated GMT manpage headers")

# Add commands to create header files
add_custom_command (OUTPUT gmt_datums.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_datums_h
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS Datums.txt)

add_custom_command (OUTPUT gmt_colornames.h gmt_color_rgb.h Colors.i
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_colors_h
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS Colors.txt)

add_custom_command (OUTPUT Fonts.i
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_ps_font_info
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS ${GMT_SOURCE_DIR}/share/pslib/PS_font_info.d)

add_custom_command (OUTPUT gmt_ellipsoids.h Ellipsoids.i
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_ellipsoids
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS Ellipsoids.txt)

add_custom_command (OUTPUT gmt_grdkeys.h grdreformat_man.i
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_grdkeys_h
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS gmt_customio.c)

add_custom_command (OUTPUT gmt_keycases.h gmt_keywords.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_keywords_h
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS gmt_keywords.txt gmt_keywords.d)

add_custom_command (OUTPUT gmtapi_errno.h gmtapi_errstr.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmtapi_err_h
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS gmtapi_errors.d)

add_custom_command (OUTPUT gmt_dimensions.h
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_dimensions_h
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS gmt_media_name.h gmt_pennames.h gmt_unique.h
	gmt_datums.h gmt_prognames.h gmt_colornames.h gmt_ellipsoids.h
	gmt_grdkeys.h gmt_keycases.h)
	# depends on cached line numbers from these generated files:

add_custom_command (OUTPUT gmtmath.h gmtmath_op.h gmtmath_explain.h gmtmath_man.i
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_gmt_math_h
	-D GMT=GMT
	-D gmt=gmt
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS gmtmath.c)

add_custom_command (OUTPUT grdmath.h grdmath_op.h grdmath_explain.h grdmath_man.i
	COMMAND ${CMAKE_COMMAND}
	-D GENERATE_COMMAND=gen_grd_math_h
	-D GMT=GRD
	-D gmt=grd
	-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
	-D GMT_SRC=${GMT_SOURCE_DIR}
	-P ${CMAKE_MODULE_PATH}/GmtGenExtraHeaders.cmake
	DEPENDS grdmath.c)

configure_file (gmt_config.h.in gmt_config.h)
configure_file (gmt_version.h.in gmt_version.h)
configure_file (isogmt.in isogmt @ONLY)

# gmtprogram
set (GMT_PROGRAM ${GMT_SOURCE_DIR}/src/gmtprogram.c)

# extra ugly definitions for triangle
set_source_files_properties (triangle
	PROPERTIES
	COMPILE_DEFINITIONS "NO_TIMER;TRILIBRARY;REDUCED;CDT_ONLY;ANSI_DECLARATORS")

# libpslib
add_library (pslib ${GMT_PSL_SRCS})

if (HAVE_M_LIBRARY)
	# link the math library
	target_link_libraries (pslib m)
endif (HAVE_M_LIBRARY)

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (pslib
	PROPERTIES
	OUTPUT_NAME psl
	PREFIX lib
	IMPORT_PREFIX lib
	VERSION ${GMT_PACKAGE_VERSION}
	SOVERSION ${GMT_PACKAGE_VERSION_MAJOR}
	INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
	DEFINE_SYMBOL "LIBRARY_EXPORTS")

# If a renaming of the psl dll has been set in ConfigUser.cmake
if (WIN32 AND PSL_DLL_RENAME)
	set_target_properties (pslib PROPERTIES RUNTIME_OUTPUT_NAME ${PSL_DLL_RENAME})
endif ()

# supplement directories (only those, which are to be included in gmtlib)
set (GMT_SUPPL_DIRS dbase gshhs imgsrc meca mgd77 potential segy sph spotter
	x2sys)

# other supplement directories (not included in gmtlib)
set (GMT_OTHER_SUPPL_DIRS misc xgrid)

# path to all binary dirs (used to set PATH in test scripts)
set (GMT_BINARY_DIR_PATH ${CMAKE_CURRENT_BINARY_DIR})

# process supplement directories
foreach (_dir ${GMT_SUPPL_DIRS} ${GMT_OTHER_SUPPL_DIRS})
	add_subdirectory (${_dir})
	list (APPEND GMT_BINARY_DIR_PATH ${CMAKE_CURRENT_BINARY_DIR}/${_dir})
endforeach (_dir)

# make UNIX PATH variable
string (REPLACE ";" ":" GMT_BINARY_DIR_PATH "${GMT_BINARY_DIR_PATH}")
set (GMT_BINARY_DIR_PATH ${GMT_BINARY_DIR_PATH} CACHE INTERNAL
	"UNIX PATH to all binary dirs")

# supplement library files
get_subdir_var_files (GMT_SUPPL_LIB_SRCS LIB_SRCS ${GMT_SUPPL_DIRS})

# include supplement directories
foreach (_dir ${GMT_SUPPL_DIRS})
	include_directories (${CMAKE_CURRENT_BINARY_DIR}/${_dir})
endforeach (_dir)

# libgmt
add_library (gmtlib
	${GMT_GEN_HDRS}
	${GMT_LIB_SRCS}
	${GMT_LIB_PS_SRCS}
	${GMT_SUPPL_LIB_SRCS})

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
	PREFIX lib
	IMPORT_PREFIX lib
	VERSION ${GMT_PACKAGE_VERSION}
	SOVERSION ${GMT_PACKAGE_VERSION_MAJOR}
	INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
	LINK_FLAGS "${USER_GMTLIB_LINK_FLAGS}"
	DEFINE_SYMBOL "LIBRARY_EXPORTS")

if (HAVE_DLADDR AND HAVE_LIBDL)
	# link the dynamic linking loader library
	target_link_libraries (pslib dl)
	target_link_libraries (gmtlib dl)
endif (HAVE_DLADDR AND HAVE_LIBDL)

# If a renaming of the gmt dll has been set in ConfigUser.cmake
if (WIN32 AND GMT_DLL_RENAME)
	set_target_properties (gmtlib PROPERTIES RUNTIME_OUTPUT_NAME ${GMT_DLL_RENAME})
endif ()

# create build targets for gmt progs
string (REPLACE ".c" "" GMT_PROGS "${GMT_PROGS_SRCS}")
foreach (_gmtprog ${GMT_PROGS})
	add_executable (${_gmtprog} ${GMT_PROGRAM})
	set_target_properties (${_gmtprog} PROPERTIES
		COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMT;FUNC=GMT_${_gmtprog}")
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

# make list of main gmt and supplement targets
get_subdir_var (_suppl_progs SUPPL_PROGS ${GMT_SUPPL_DIRS})
get_subdir_var (_suppl_progsps SUPPL_PROGSPS ${GMT_SUPPL_DIRS})
set (_all_progs ${GMT_PROGS} ${_suppl_progs})
set (_all_progsps ${GMT_PROGSPS} ${_suppl_progsps})
string (REPLACE ";" ":GMTAPI_GMT;" _all_progs "${_all_progs}:GMTAPI_GMT")
string (REPLACE ";" ":GMTAPI_GMTPSL;" _all_progsps "${_all_progsps}:GMTAPI_GMTPSL")
list (APPEND _all_progs ${_all_progsps})
list (SORT _all_progs)

# generate gmt_prognames.h and gmt_progcases.h
gen_gmt_prog_names_cases_h (${_all_progs})

# generate gmt_suppl_modules.h
# TODO: this is ugly. should all go into gmt_modules.h
get_subdir_var (_suppl_api_hdrs SUPPL_API_H ${GMT_SUPPL_DIRS})
set (_suppl_api_includes)
foreach (_hdrfile ${_suppl_api_hdrs})
	list (APPEND _suppl_api_includes "#include \"${_hdrfile}\"")
endforeach (_hdrfile ${_suppl_api_hdrs})
string (REPLACE ";" "\n" _suppl_api_includes "${_suppl_api_includes}")
file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/gmt_suppl_modules.h
	"${_suppl_api_includes}\n")

# generate gmt_progpurpose.h
get_subdir_var_files (_suppl_progs_srcs SUPPL_PROGS_SRCS ${GMT_SUPPL_DIRS})
get_subdir_var_files (_suppl_progsps_srcs SUPPL_PROGSPS_SRCS ${GMT_SUPPL_DIRS})
gen_gmt_progpurpose_h (GMT_PROG_PURPOSE
	${GMT_PROGS_SRCS} ${GMT_PROGSPS_SRCS}
	${_suppl_progs_srcs} ${_suppl_progsps_srcs})

# build targets for demos
string (REPLACE ".c" "" GMT_DEMOS "${GMT_DEMOS_SRCS}")
foreach (_gmtdemo ${GMT_DEMOS})
	add_executable (${_gmtdemo} ${_gmtdemo}.c)
	target_link_libraries (${_gmtdemo} gmtlib)
endforeach (_gmtdemo)

configure_file (psldemo.h.in psldemo.h @ONLY)
add_executable (psldemo psldemo.h psldemo.c)
target_link_libraries (psldemo pslib)

# rman for converting html manpages
if (HAVE_GETOPT)
	add_executable (rman EXCLUDE_FROM_ALL rman.c)
else (HAVE_GETOPT)
	add_executable (rman EXCLUDE_FROM_ALL rman.c getopt.c getopt.h)
endif (HAVE_GETOPT)

# generate Mac debugging symbols
create_debug_sym (${GMT_LIBS} ${GMT_PROGS} ${GMT_PROGSPS} ${GMT_DEMOS})

# Tests depend on GMT progs and supplements:
add_dependencies (check ${GMT_PROGSPS} ${GMT_PROGS} ${GMT_DEMOS} psldemo
	gmt_suppl)

# Rename gmt target on non-monolithic installs to prevent version clash
if (NOT GMT_INSTALL_MONOLITHIC)
	set_target_properties (gmt
		PROPERTIES
		OUTPUT_NAME gmt${GMT_INSTALL_NAME_SUFFIX})
endif (NOT GMT_INSTALL_MONOLITHIC)

# add the install targets
install (TARGETS ${GMT_LIBS} ${GMT_PROGS} ${GMT_PROGSPS} ${GMT_DEMOS}
	ARCHIVE DESTINATION ${GMT_LIBDIR}
	COMPONENT Runtime
	LIBRARY DESTINATION ${GMT_LIBDIR}
	COMPONENT Runtime
	RUNTIME DESTINATION ${GMT_BINDIR}
	COMPONENT Runtime)

if (WIN32 AND _debug_build)
	# install program database files
	set (_pdb_files)
	if (NOT DEFINED PSL_DLL_RENAME)
		foreach (_target libpsl libgmt ${GMT_PROGS} ${GMT_PROGSPS} ${GMT_DEMOS})
			list (APPEND _pdb_files "${CMAKE_CURRENT_BINARY_DIR}/${_target}.pdb")
		endforeach ()
	else ()
		foreach (_target lib${PSL_DLL_RENAME} lib${GMT_DLL_RENAME} ${GMT_PROGS} ${GMT_PROGSPS} ${GMT_DEMOS})
			list (APPEND _pdb_files "${CMAKE_CURRENT_BINARY_DIR}/${_target}.pdb")
		endforeach ()
	endif ()
	install (FILES ${_pdb_files}
		DESTINATION ${GMT_BINDIR}
		COMPONENT Runtime)
endif (WIN32 AND _debug_build)

# Install public headers
install (FILES declspec.h gmtapi.h
	DESTINATION ${GMT_INCDIR}
	COMPONENT Runtime)

# Non-monolithic installs:
if (UNIX AND NOT GMT_INSTALL_MONOLITHIC)
	# symlink to gmt-wrapper in bindir and libdir:
	get_target_property(_gmt_wrapper_name gmt OUTPUT_NAME)
	file (RELATIVE_PATH _rpath /bin /${GMT_BINDIR}/${_gmt_wrapper_name})
	install (CODE "
	execute_process (
		COMMAND ${CMAKE_COMMAND} -E make_directory
		\${CMAKE_INSTALL_PREFIX}/bin)
	execute_process (
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		${_rpath} \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/${_gmt_wrapper_name}
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		${_gmt_wrapper_name} \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${GMT_BINDIR}/gmt)
	")

	# create symlinks in libdir/..:
	file (RELATIVE_PATH _rpath_share /${GMT_LIBDIR} /${GMT_SHARE_PATH})
	string (REGEX REPLACE "^[.][.]/(.*)" "\\1" _rpath_share "${_rpath_share}")
	file (RELATIVE_PATH _rpath_man /${GMT_LIBDIR} /${GMT_MAN_PATH})
	string (REGEX REPLACE "^[.][.]/(.*)" "\\1" _rpath_man "${_rpath_man}")
	install (CODE "
	execute_process (
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		${_rpath_share} \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${GMT_LIBDIR}/../share
		COMMAND ${CMAKE_COMMAND} -E create_symlink
		${_rpath_man} \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${GMT_LIBDIR}/../man)
	")
endif (UNIX AND NOT GMT_INSTALL_MONOLITHIC)

# install extra files
install (PROGRAMS
	${CMAKE_CURRENT_BINARY_DIR}/isogmt
	gmtlogo
	gmtswitch
	gmt_shell_functions.sh
	ncdeflate.sh
	DESTINATION ${GMT_BINDIR}
	COMPONENT Runtime)
if (WIN32)
	install (PROGRAMS
		gmtswitch.bat
		DESTINATION ${GMT_BINDIR}
		COMPONENT Runtime)
endif (WIN32)

# Generate manpages
gmt_create_manpages (${GMT_MAN_1} ${GMT_MAN_3} ${GMT_MAN_5}
	DEPENDS ${GMT_MAN_DEPS} ${GMT_GEN_MAN_HDRS})

# this prints out the final version of the compiler/linker flags
string (TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
string(REGEX REPLACE "[ ]+" " " _using_cflags ${CMAKE_C_FLAGS} " "
	${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}} " " ${NETCDF_DEFINITIONS})
message (STATUS "Using CFLAGS = '${_using_cflags}'")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
