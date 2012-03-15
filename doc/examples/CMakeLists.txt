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

# List all PS files and convert them to PDF

file (GLOB _examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*/*.ps")

set (_examples_pdf)

foreach (_ps ${_examples})
	get_filename_component (_pdf ${_ps} NAME)
	string (REPLACE ".ps" ".pdf" _pdf ${_pdf})
	list (APPEND _examples_pdf ${CMAKE_CURRENT_BINARY_DIR}/${_pdf})

	add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_pdf}
		COMMAND
		GMT_USERDIR=${GMT_BINARY_DIR}/share
		GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
		${GMT_BINARY_DIR}/src/ps2raster -Au -P -Tf
		-C-sFONTPATH=${GMT_SOURCE_DIR}/doc/examples/ex31/fonts
		-F${CMAKE_CURRENT_BINARY_DIR}/${_pdf}
		${CMAKE_CURRENT_SOURCE_DIR}/${_ps}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS ps2raster ${CMAKE_CURRENT_SOURCE_DIR}/${_ps})
endforeach (_ps ${_examples})

# List of job scripts and convert to verbatim

file (GLOB _examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*/*.sh")

set (_examples_txt)

foreach (_script ${_examples})
	get_filename_component (_txt ${_script} NAME)
	string (REPLACE ".sh" ".txt" _txt ${_txt})
	list (APPEND _examples_txt ${CMAKE_CURRENT_BINARY_DIR}/${_txt})

	add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_txt}
		COMMAND ${CMAKE_COMMAND}
		-D script=${CMAKE_CURRENT_SOURCE_DIR}/${_script}
		-D txt=${CMAKE_CURRENT_BINARY_DIR}/${_txt}
		-D nostrip=TRUE
		-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
		-P ${CMAKE_MODULE_PATH}/ScriptToVerbatim.cmake
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_script} ${CMAKE_MODULE_PATH}/ScriptToVerbatim.cmake)
endforeach (_script ${_examples})

# Add build target
add_custom_command (OUTPUT .done COMMAND touch .done DEPENDS ${_examples_txt} ${_examples_pdf})
add_custom_target (examples_done DEPENDS .done)

# run examples (test)
if (DO_EXAMPLES) # AND UNIX)
	# this file takes care of setting up the test environment
	configure_file (check.in check @ONLY)

	# Workaround cmake bug 3957: CRLF line ending
	find_package (UnixCommands)
	if (CYGWIN_INSTALL_PATH)
		find_program (D2U d2u
			${CYGWIN_INSTALL_PATH}/bin)
		execute_process (COMMAND ${D2U}
			${CMAKE_CURRENT_BINARY_DIR}/check)
	endif (CYGWIN_INSTALL_PATH)

	foreach (_job ${_examples})
		string (REPLACE "//" "/" _job ${_job})	# GLOB creates double slashes we do not want
		add_test (NAME ${_job}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND check ${CMAKE_CURRENT_SOURCE_DIR}/${_job})
	endforeach (_job ${_examples})
elseif (DO_EXAMPLES_WIN_BATCH_DISABLED) # (DO_EXAMPLES AND WIN32)
	# only supporting examples with installed GMT in %path%
	string (REPLACE ".sh" ".bat" _examples_win "${_examples}")
	foreach (_job ${_examples_win})
		get_filename_component (_job_path
			${CMAKE_CURRENT_BINARY_DIR}/${_job} PATH)
		add_test (NAME ${_job}
			WORKING_DIRECTORY ${_job_path}
			COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${_job})
	endforeach (_job ${_examples_win})
endif (DO_EXAMPLES) # AND UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
