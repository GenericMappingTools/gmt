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

include (ManageString)

# Convert scripts to verbatim text for inclusion in TeX documentation
macro (SCRIPT_TO_VERBATIM)
		grep ("[$]Id:|functions[.]sh" _file_content ${script} INVERT)
		string (REPLACE ";" "\n" _file_content "${_file_content}")
		string_unescape (_verbatim_txt "${_file_content}" NOESCAPE_SEMICOLON)
		get_filename_component (_verbatim_file ${script} NAME_WE)
		if (_verbatim_file MATCHES anim)
			string (REGEX REPLACE ".+_([0-9][0-9][a-z]?)$" "anim_\\1.txt"
				_verbatim_file ${_verbatim_file})
		else (_verbatim_file MATCHES anim)
			string (REGEX REPLACE "job([0-9][0-9][a-z]?)$" "example_\\1.txt"
				_verbatim_file ${_verbatim_file})
		endif (_verbatim_file MATCHES anim)
		file (WRITE ${_verbatim_file} "${_verbatim_txt}")
endmacro (SCRIPT_TO_VERBATIM)

macro (SETUP)
	# List of output verbatime files for inclusion in LaTeX doc
	set (_all_verbatim)

	foreach (_script ${_animation_jobs} ${_example_jobs})
		get_filename_component (_verbatim_file ${_script} NAME_WE)
		if (_verbatim_file MATCHES anim)
			string (REGEX REPLACE ".+_([0-9][0-9][a-z]?)$" "anim_\\1.txt"
				_verbatim_file ${_verbatim_file})
		else (_verbatim_file MATCHES anim)
			string (REGEX REPLACE "job([0-9][0-9][a-z]?)$" "example_\\1.txt"
				_verbatim_file ${_verbatim_file})
		endif (_verbatim_file MATCHES anim)
		list (APPEND _all_verbatim ${GMT_BINARY_DIR}/doc/scripts/${_verbatim_file})

	# Add command that triggers generator macro when output does not exist
		add_custom_command (OUTPUT ${GMT_BINARY_DIR}/doc/scripts/${_verbatim_file}
			COMMAND ${CMAKE_COMMAND}
			-D EXAMPLES_COMMAND=script_to_verbatim
			-D script=${CMAKE_CURRENT_SOURCE_DIR}/${_script}
			-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
			-P ${CMAKE_CURRENT_LIST_FILE}
			WORKING_DIRECTORY ${GMT_BINARY_DIR}/doc/scripts
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_script})
	endforeach (_script ${_animation_jobs} ${_example_jobs})

	add_custom_target (examples_txt DEPENDS ${_all_verbatim})

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
			add_test(NAME ${_job}
				WORKING_DIRECTORY ${_job_path}
				COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/${_job})
		endforeach (_job)
	elseif (DO_EXAMPLES_WIN_BATCH_DISABLED) # (DO_EXAMPLES AND WIN32)
		# only supporting examples with installed GMT in %path%
		string (REPLACE ".sh" ".bat" _example_jobs_win "${_example_jobs}")
		foreach (_job ${_example_jobs_win})
			get_filename_component (_job_path
				${CMAKE_CURRENT_BINARY_DIR}/${_job} PATH)
			add_test(NAME ${_job}
				WORKING_DIRECTORY ${_job_path}
				COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${_job})
		endforeach (_job)
	endif(DO_EXAMPLES) # AND UNIX)
endmacro (SETUP)

# Get something done
if (EXAMPLES_COMMAND)
	set (_command_handled)
	if (${EXAMPLES_COMMAND} STREQUAL script_to_verbatim)
		script_to_verbatim ()
		set (_command_handled TRUE)
	endif (${EXAMPLES_COMMAND} STREQUAL script_to_verbatim)

	if (NOT _command_handled)
		message (SEND_ERROR "Unknown command: ${EXAMPLES_COMMAND}")
	endif (NOT _command_handled)
else (EXAMPLES_COMMAND)
	# Must be part of the actual configure (included from CMakeLists.txt).
	SETUP()
endif (EXAMPLES_COMMAND)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
