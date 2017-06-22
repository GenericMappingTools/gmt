#
# $Id$
#
# Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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

# run tests
if (DO_TESTS)
	# list of test dirs
	if (DO_MODERN_TESTS)
		set (GMT_TEST_DIRS
		        modern_api modern_blockmean modern_blockmedian modern_blockmode
		        modern_byteswap modern_filter1d modern_fitcircle modern_gdal
		        modern_genper modern_gmtconnect modern_gmtconvert modern_gmt_core
		        modern_gmtlogo modern_gmtmath modern_gmtregress modern_gmtselect
		        modern_gmtsimplify modern_gmtspatial modern_gmtvector modern_grd2cpt
		        modern_grd2rgb modern_grd2xyz modern_grdblend modern_grdclip
		        modern_grdcontour modern_grdconvert modern_grdcut modern_grdedit
		        modern_grdfft modern_grdfilter modern_grdgradient modern_grdimage
		        modern_grdlandmask modern_grdmask modern_grdmath modern_grdpaste
		        modern_grdproject modern_grdsample modern_grdtrack modern_grdtrend
		        modern_grdvector modern_grdview modern_grdvolume modern_greenspline
		        modern_gshhg modern_img modern_kml modern_mapproject modern_meca
		        modern_mgd77 modern_nearneighbor modern_ogr modern_postscriptlight
		        modern_psclip modern_potential modern_project modern_psbasemap modern_pscoast
		        modern_pscontour modern_pshistogram modern_psimage modern_pslegend
		        modern_psmask modern_psrose modern_pssac modern_psscale modern_pssolar
		        modern_psternary modern_pstext modern_pswiggle modern_psxy modern_psxyz
		        modern_sample1d modern_segy modern_spectrum1d modern_sph modern_sph2grd
		        modern_splitxyz modern_spotter modern_surface modern_time
		        modern_trend1d modern_trend2d modern_triangulate modern_x2sys)
	else (DO_MODERN_TESTS)
		set (GMT_TEST_DIRS api byteswap blockmean blockmedian blockmode filter1d fitcircle gdal
			genper gmt_core gmtconvert gmtlogo gmtmath gmtregress gmtselect gmtsimplify gmtspatial
			gmtconnect gmtvector grd2rgb grd2xyz grdblend grdclip grdcontour grdcut
			grdedit grdfft grdfilter grdgradient grdhisteq grdimage grdinfo grdlandmask
			grdmask grdmath grdpaste grdproject grdconvert grdsample grdtrack
			grdtrend grdvector grdview grdvolume greenspline gshhg kml mapproject nearneighbor ogr
			project psbasemap pscoast pscontour pshistogram psimage pslegend psmask postscriptlight psclip
			psrose pssac psscale pssolar psternary pstext pswiggle psxy psxyz sample1d segy spectrum1d sph sph2grd
			splitxyz surface time trend1d trend2d triangulate img meca mgd77 potential spotter x2sys)
	endif (DO_MODERN_TESTS)

	# export HAVE_GMT_DEBUG_SYMBOLS
	get_directory_property (_dir_defs COMPILE_DEFINITIONS)
	list (FIND _dir_defs DEBUG HAVE_GMT_DEBUG_SYMBOLS)
	if (HAVE_GMT_DEBUG_SYMBOLS EQUAL -1)
		set (HAVE_GMT_DEBUG_SYMBOLS)
	else (HAVE_GMT_DEBUG_SYMBOLS EQUAL -1)
		set (HAVE_GMT_DEBUG_SYMBOLS TRUE)
	endif (HAVE_GMT_DEBUG_SYMBOLS EQUAL -1)

	configure_file (gmtest.in gmtest @ONLY)

	# Workaround cmake bug 3957: CRLF line ending
	find_package (UnixCommands)
	if (CYGWIN_INSTALL_PATH)
		find_program (D2U d2u
			${CYGWIN_INSTALL_PATH}/bin)
		execute_process (COMMAND ${D2U}
			${CMAKE_CURRENT_BINARY_DIR}/gmtest)
	endif (CYGWIN_INSTALL_PATH)

	# add tests
	foreach (_test_dir ${GMT_TEST_DIRS})
		# find files RELATIVE so that test NAMEs are not absolute paths
		file (GLOB _test_scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			"${CMAKE_CURRENT_SOURCE_DIR}/${_test_dir}/*.sh")
		foreach (_test ${_test_scripts})
			add_test (NAME ${_test}
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
				COMMAND ${BASH} gmtest ${_test})
		endforeach (_test ${_test_scripts})
	endforeach (_test_dir ${GMT_TEST_DIRS})
endif (DO_TESTS)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
