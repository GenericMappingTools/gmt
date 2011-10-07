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

if (DO_EXAMPLES AND UNIX)
	# run examples (test)
	file(COPY ./
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
		USE_SOURCE_PERMISSIONS
		PATTERN ".svn" EXCLUDE
		PATTERN "CMakeLists.txt" EXCLUDE
		PATTERN ".cmake" EXCLUDE)

	configure_file (functions.sh.cmake functions.sh @ONLY)

	set (GMT_EXAMPLE_DIRS anim01 anim02 anim03 anim04 ex01 ex02 ex03 ex04 ex05
		ex06 ex07 ex08 ex09 ex10 ex11 ex12 ex13 ex14 ex15 ex16 ex17 ex18 ex19 ex20
		ex21 ex22 ex23 ex24 ex25 ex26 ex27 ex28 ex29 ex30 ex31 ex32)

	foreach (_test_dir ${GMT_EXAMPLE_DIRS})
		file(GLOB _test_scripts RELATIVE ${CMAKE_CURRENT_BINARY_DIR}
			"${CMAKE_CURRENT_BINARY_DIR}/${_test_dir}/*.sh")
		foreach (_test ${_test_scripts})
			add_test(NAME ${_test}
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_test_dir}
				COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${_test})
		endforeach (_test ${_test_scripts})
	endforeach (_test_dir ${GMT_EXAMPLE_DIRS})
endif(DO_EXAMPLES AND UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
