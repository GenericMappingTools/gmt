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

# run tests
if (DO_TESTS)
	# list of test dirs
	set (GMT_TEST_DIRS api blockmean filter1d fitcircle genper gmtconvert gmtdp
		gmtmath gmtselect gmtspatial gmtstitch gmtvector grd2rgb grd2xyz grdblend
		grdclip grdcontour grdcut grdedit grdfft grdfilter grdhisteq grdimage
		grdlandmask grdmask grdpaste grdproject grdreformat grdsample grdtrend
		grdview grdvolume greenspline kml mapproject meca mgd77 ogr potential
		project psbasemap pscoast pscontour psimage pslegend pslib psrose psscale
		pstext psxy psxyz sph splitxyz spotter time trend2d triangulate x2sys)

	# rules for copying test files to build dir
	file (GLOB_RECURSE _files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)
	list_regex_get ("[.]svn/|[.]cmake|CMakeLists.txt" _files ${_files} INVERT)
	set (_test_files)
	foreach (_file ${_files})
		add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_file}
			COMMAND ${CMAKE_COMMAND}
			-E copy_if_different
			${CMAKE_CURRENT_SOURCE_DIR}/${_file}
			${CMAKE_CURRENT_BINARY_DIR}/${_file})
		list (APPEND _test_files ${CMAKE_CURRENT_BINARY_DIR}/${_file})
	endforeach (_file ${_files})

	# add file dependencies to check target
	add_custom_target (copy_test_files
		DEPENDS ${_test_files})
	add_dependencies (check copy_test_files)

	# export HAVE_GMT_DEBUG_SYMBOLS
	get_directory_property(_dir_defs COMPILE_DEFINITIONS)
	list(FIND _dir_defs DEBUG HAVE_GMT_DEBUG_SYMBOLS)
	if (HAVE_GMT_DEBUG_SYMBOLS EQUAL -1)
		set (HAVE_GMT_DEBUG_SYMBOLS)
	else (HAVE_GMT_DEBUG_SYMBOLS EQUAL -1)
		set (HAVE_GMT_DEBUG_SYMBOLS TRUE)
	endif (HAVE_GMT_DEBUG_SYMBOLS EQUAL -1)

	configure_file (functions.sh.cmake functions.sh @ONLY)

	# Workaround cmake bug 3957: CRLF line ending
	find_package (UnixCommands)
	if (CYGWIN_INSTALL_PATH)
		find_program (D2U d2u
			${CYGWIN_INSTALL_PATH}/bin)
		execute_process (COMMAND ${D2U}
			${CMAKE_CURRENT_BINARY_DIR}/functions.sh)
	endif (CYGWIN_INSTALL_PATH)

	# add tests
	foreach (_test_dir ${GMT_TEST_DIRS})
		file(GLOB _test_scripts RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
			"${CMAKE_CURRENT_SOURCE_DIR}/${_test_dir}/*.sh")
		foreach (_test ${_test_scripts})
			add_test(NAME ${_test}
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_test_dir}
				COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/${_test})
		endforeach (_test ${_test_scripts})
	endforeach (_test_dir ${GMT_TEST_DIRS})
endif (DO_TESTS)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
