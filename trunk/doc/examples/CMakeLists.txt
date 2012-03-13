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

# List of job scrips
set (_animation_jobs anim01/anim_01.sh anim02/anim_02.sh anim03/anim_03.sh
	  anim04/anim_04.sh)

set (_example_jobs ex01/job01.sh ex02/job02.sh ex03/job03.sh ex04/job04.sh
	ex04/job04c.sh ex05/job05.sh ex06/job06.sh ex07/job07.sh ex08/job08.sh
	ex09/job09.sh ex10/job10.sh ex11/job11.sh ex12/job12.sh ex13/job13.sh
	ex14/job14.sh ex15/job15.sh ex16/job16.sh ex17/job17.sh ex18/job18.sh
	ex19/job19.sh ex20/job20.sh ex21/job21.sh ex22/job22.sh ex23/job23.sh
	ex24/job24.sh ex25/job25.sh ex26/job26.sh ex27/job27.sh ex28/job28.sh
	ex29/job29.sh ex30/job30.sh ex31/job31.sh ex32/job32.sh)

set (_all_jobs ${_animation_jobs} ${_example_jobs})

# List of output verbatime files for inclusion in LaTeX doc
set (_examples_txt)

foreach (_script ${_all_jobs})
	get_filename_component (_txt ${_script} NAME_WE)
	if (_txt MATCHES anim)
		string (REGEX REPLACE ".+_([0-9][0-9][a-z]?)$" "anim_\\1.txt"
			_txt ${_txt})
	else (_txt MATCHES anim)
		string (REGEX REPLACE "job([0-9][0-9][a-z]?)$" "example_\\1.txt"
			_txt ${_txt})
	endif (_txt MATCHES anim)
	list (APPEND _examples_txt ${CMAKE_CURRENT_BINARY_DIR}/${_txt})

# Add command that triggers generator macro when output does not exist
	add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_txt}
		COMMAND ${CMAKE_COMMAND}
		-D script=${CMAKE_CURRENT_SOURCE_DIR}/${_script}
		-D txt=${CMAKE_CURRENT_BINARY_DIR}/${_txt}
		-D nostrip=TRUE
		-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
		-P ${CMAKE_MODULE_PATH}/ScriptToVerbatim.cmake
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_script} ${CMAKE_MODULE_PATH}/ScriptToVerbatim.cmake)
endforeach (_script ${_animation_jobs} ${_example_jobs})

add_custom_target (examples_txt DEPENDS ${_examples_txt})

# run examples (test)
if (DO_EXAMPLES) # AND UNIX)
	# this file takes care of setting up the test environment
	configure_file (functions.sh.cmake functions.sh @ONLY)

	# Workaround cmake bug 3957: CRLF line ending
	find_package (UnixCommands)
	if (CYGWIN_INSTALL_PATH)
		find_program (D2U d2u
			${CYGWIN_INSTALL_PATH}/bin)
		execute_process (COMMAND ${D2U}
			${CMAKE_CURRENT_BINARY_DIR}/functions.sh)
	endif (CYGWIN_INSTALL_PATH)

	foreach (_job ${_all_jobs})
		get_filename_component (_job_path
			${CMAKE_CURRENT_BINARY_DIR}/${_job} PATH)
		add_test (NAME ${_job}
			WORKING_DIRECTORY ${_job_path}
			COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/${_job})
	endforeach (_job)
elseif (DO_EXAMPLES_WIN_BATCH_DISABLED) # (DO_EXAMPLES AND WIN32)
	# only supporting examples with installed GMT in %path%
	string (REPLACE ".sh" ".bat" _example_jobs_win "${_example_jobs}")
	foreach (_job ${_example_jobs_win})
		get_filename_component (_job_path
			${CMAKE_CURRENT_BINARY_DIR}/${_job} PATH)
		add_test (NAME ${_job}
			WORKING_DIRECTORY ${_job_path}
			COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${_job})
	endforeach (_job)
endif (DO_EXAMPLES) # AND UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
