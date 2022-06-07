#
# Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

# run tests
if (DO_TESTS)
	# Automatically detect all subdirectories for testing
	include (GmtHelperMacros)
	get_subdir_list (GMT_TEST_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
	list (REMOVE_ITEM GMT_TEST_DIRS animation)

	if (NOT DO_API_TESTS)
		list (REMOVE_ITEM GMT_TEST_DIRS api)
	endif (NOT DO_API_TESTS)

	if (NOT DO_SUPPLEMENT_TESTS)
		list (REMOVE_ITEM GMT_TEST_DIRS geodesy)
		list (REMOVE_ITEM GMT_TEST_DIRS gmtmex)
		list (REMOVE_ITEM GMT_TEST_DIRS gshhg)
		list (REMOVE_ITEM GMT_TEST_DIRS img)
		list (REMOVE_ITEM GMT_TEST_DIRS mgd77)
		list (REMOVE_ITEM GMT_TEST_DIRS potential)
		list (REMOVE_ITEM GMT_TEST_DIRS pssac)
		list (REMOVE_ITEM GMT_TEST_DIRS segy)
		list (REMOVE_ITEM GMT_TEST_DIRS seis)
		list (REMOVE_ITEM GMT_TEST_DIRS spotter)
		list (REMOVE_ITEM GMT_TEST_DIRS x2sys)
	endif (NOT DO_SUPPLEMENT_TESTS)

	# Remove latex-specific tests if latex or dvips not found
	if (NOT LATEX OR NOT DVIPS)
		list (REMOVE_ITEM GMT_TEST_DIRS latex)
	endif (NOT LATEX OR NOT DVIPS)

	# export HAVE_GMT_DEBUG_SYMBOLS
	get_directory_property (_dir_defs COMPILE_DEFINITIONS)
	list (FIND _dir_defs DEBUG HAVE_GMT_DEBUG_SYMBOLS)
	if (HAVE_GMT_DEBUG_SYMBOLS EQUAL -1)
		set (HAVE_GMT_DEBUG_SYMBOLS)
	else (HAVE_GMT_DEBUG_SYMBOLS EQUAL -1)
		set (HAVE_GMT_DEBUG_SYMBOLS TRUE)
	endif (HAVE_GMT_DEBUG_SYMBOLS EQUAL -1)

	configure_file (gmtest.in gmtest @ONLY)

	# add tests
	foreach (_test_dir ${GMT_TEST_DIRS})
		file (GLOB _test_scripts RELATIVE ${CMAKE_SOURCE_DIR}
			"${CMAKE_CURRENT_SOURCE_DIR}/${_test_dir}/*.sh")
		foreach (_test ${_test_scripts})
			add_test (NAME ${_test}
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
				COMMAND ${BASH} gmtest ${_test})
		endforeach (_test ${_test_scripts})
	endforeach (_test_dir ${GMT_TEST_DIRS})
endif (DO_TESTS)
