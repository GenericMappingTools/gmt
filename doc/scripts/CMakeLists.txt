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

# Scripts which have to be converted to verbatim (with comments)
set (_scr_nostrip GMT_App_M_2.sh GMT_App_P_1.sh GMT_App_P_2.sh)

# Scripts which have to be converted to verbatim (without comments)
set (_scr_strip GMT_-B_custom.sh GMT_-B_time1.sh GMT_-B_time2.sh
	GMT_-B_time3.sh GMT_-B_time4.sh GMT_-B_time5.sh GMT_-B_time6.sh
	GMT_-B_time7.sh GMT_albers.sh GMT_App_K_1.sh GMT_App_K_2.sh GMT_App_K_3.sh
	GMT_App_K_4.sh GMT_App_K_5.sh GMT_App_O_1.sh GMT_App_O_2.sh GMT_App_O_3.sh
	GMT_App_O_4.sh GMT_App_O_5.sh GMT_App_O_6.sh GMT_App_O_7.sh GMT_App_O_8.sh
	GMT_App_O_9.sh GMT_az_equidistant.sh GMT_cassini.sh GMT_eckert4.sh
	GMT_eckert6.sh GMT_equi_cyl.sh GMT_equidistant_conic.sh GMT_gall_stereo.sh
	GMT_general_cyl.sh GMT_gnomonic.sh GMT_grinten.sh GMT_hammer.sh
	GMT_lambert_az_hemi.sh GMT_lambert_az_rect.sh GMT_lambert_conic.sh
	GMT_linear_cal.sh GMT_linear_d.sh GMT_linear.sh GMT_log.sh GMT_mercator.sh
	GMT_miller.sh GMT_mollweide.sh GMT_obl_merc.sh GMT_orthographic.sh
	GMT_perspective.sh GMT_polar.sh GMT_polyconic.sh GMT_pow.sh GMT_robinson.sh
	GMT_sinus_int.sh GMT_sinusoidal.sh GMT_stereographic_general.sh
	GMT_stereographic_polar.sh GMT_stereographic_rect.sh GMT_stereonets.sh
	GMT_TM.sh GMT_transverse_merc.sh GMT_winkel.sh)

# Other scripts that are only used in tests
set (_scr_other GMT_-B_geo_1.sh GMT_-B_geo_2.sh GMT_-B_linear.sh GMT_-B_log.sh
	GMT_-B_pow.sh GMT_-J.sh GMT_-OK.sh GMT_-P.sh GMT_-R.sh GMT_-U.sh GMT_-XY.sh
	GMT_App_E.sh GMT_App_F.sh GMT_App_G.sh GMT_App_J.sh GMT_App_M_1.sh
	GMT_App_N.sh GMT_App_O.sh GMT_arrows.sh GMT_atan.sh GMT_color_interpolate.sh
	GMT_coverlogo.sh GMT_Defaults_1a.sh GMT_Defaults_1b.sh GMT_Defaults_1c.sh
	GMT_dummydata.sh GMT_encoding.sh GMT_linecap.sh GMT_nearneighbor.sh
	GMT_pstext_clearance.sh GMT_pstext_justify.sh GMT_registration.sh
	GMT_RGBchart.sh GMT_utm_zones.sh GMT_volcano.sh)

# 

set (_all_tests ${_scr_nostrip} ${_scr_strip} ${_scr_other})
string (REGEX REPLACE "GMT_App_O_[0-9].sh" "" _all_tests "${_all_tests}")
string (REPLACE "GMT_-U.sh" "" _all_tests "${_all_tests}")

# todo: remove redundancy
set (_all_scripts ${_scr_nostrip} ${_scr_strip})
string (REPLACE ".sh" ".txt" _all_verbatim "${_all_scripts}")

include (ManageString)

macro (SCRIPT_TO_VERBATIM)
	# Convert scripts to verbatim text for inclusion in TeX documentation
	if (no_strip)
		grep ("[$]Id:|functions[.]sh" _file_content ${script} INVERT)
	else (no_strip)
		grep ("^#|functions.sh" _file_content ${script} INVERT)
	endif (no_strip)
	string (REPLACE ";" "\n" _file_content "${_file_content}")
	string_unescape (_verbatim_txt "${_file_content}" NOESCAPE_SEMICOLON)
	get_filename_component (_verbatim_file ${script} NAME_WE)
	file (WRITE ${_verbatim_file}.txt "${_verbatim_txt}")
endmacro (SCRIPT_TO_VERBATIM)

macro (SETUP)
	if (PDFLATEX_COMPILER)
		foreach (_script ${_scr_nostrip})
			string (REPLACE ".sh" ".txt" _verbatim_file ${_script})
			# Add command that triggers generator macro when output does not exist
			add_custom_command (OUTPUT ${_verbatim_file}
				COMMAND ${CMAKE_COMMAND}
				-D SCRIPTS_COMMAND=script_to_verbatim
				-D script=${CMAKE_CURRENT_SOURCE_DIR}/${_script}
				-D no_strip=TRUE
				-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
				-P ${CMAKE_CURRENT_LIST_FILE}
				DEPENDS ${_all_scripts})
		endforeach (_script)

		foreach (_script ${_scr_strip})
			string (REPLACE ".sh" ".txt" _verbatim_file ${_script})
			add_custom_command (OUTPUT ${_verbatim_file}
				COMMAND ${CMAKE_COMMAND}
				-D SCRIPTS_COMMAND=script_to_verbatim
				-D script=${CMAKE_CURRENT_SOURCE_DIR}/${_script}
				-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
				-P ${CMAKE_CURRENT_LIST_FILE}
				DEPENDS ${_all_scripts})
		endforeach (_script)

		add_custom_target (verbatim_scripts DEPENDS ${_all_verbatim})
	endif (PDFLATEX_COMPILER)

	# run tests
	if (DO_TESTS)
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
		add_custom_target (copy_script_files
			DEPENDS ${_test_files})
		add_dependencies (check copy_script_files)

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

		foreach (_job ${_all_tests})
			get_filename_component (_job_path
				${CMAKE_CURRENT_BINARY_DIR}/${_job} PATH)
			add_test(NAME ${_job}
				WORKING_DIRECTORY ${_job_path}
				COMMAND bash ${CMAKE_CURRENT_BINARY_DIR}/${_job})
		endforeach (_job)
	endif(DO_TESTS)
endmacro (SETUP)

# Get something done
if (SCRIPTS_COMMAND)
	set (_command_handled)
	if (${SCRIPTS_COMMAND} STREQUAL script_to_verbatim)
		script_to_verbatim ()
		set (_command_handled TRUE)
	endif (${SCRIPTS_COMMAND} STREQUAL script_to_verbatim)

	if (NOT _command_handled)
		message (SEND_ERROR "Unknown command: ${SCRIPTS_COMMAND}")
	endif (NOT _command_handled)
else (SCRIPTS_COMMAND)
	# Must be part of the actual configure (included from CMakeLists.txt).
	SETUP()
endif (SCRIPTS_COMMAND)


# Add build target
#add_custom_target (pdf_figures DEPENDS ${_pdf_figures})


# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
