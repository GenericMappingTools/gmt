#
# $Id$
#
# Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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
	set (GMT_TEST_DIRS api byteswap blockmean blockmedian filter1d fitcircle
		gdal genper gmt_core gmtconvert gmtmath gmtselect gmtsimplify gmtspatial
		gmtconnect gmtvector grd2rgb grd2xyz grdblend grdclip grdcontour grdcut
		grdedit grdfft grdfilter grdgradient grdhisteq grdimage grdlandmask
		grdmask grdmath grdpaste grdproject grdconvert grdsample grdtrack
		grdtrend grdvector grdview grdvolume greenspline gshhg kml mapproject ogr
		project psbasemap pscoast pscontour pshistogram psimage pslegend pslib
		psrose psscale pstext psxy psxyz sample1d spectrum1d sph sph2grd splitxyz
		surface time trend2d triangulate img meca mgd77 potential spotter x2sys)

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
