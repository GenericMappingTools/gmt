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

# Scripts that are not used in the documentation (but the figures are)
# do not include GMT_-U.sh as this would always fail
set (_scr_other GMT_-B_geo_1.sh GMT_-B_geo_2.sh GMT_-B_linear.sh GMT_-B_log.sh
	GMT_-B_pow.sh GMT_-J.sh GMT_-OK.sh GMT_-P.sh GMT_-R.sh GMT_-XY.sh
	GMT_App_E.sh GMT_App_F_dingbats.sh GMT_App_F_iso+.sh GMT_App_F_stand+.sh
	GMT_App_F_symbol.sh GMT_App_G.sh GMT_App_J_1.sh GMT_App_J_2.sh GMT_App_J_3.sh
	GMT_App_M_1.sh GMT_App_N_1.sh GMT_arrows.sh GMT_atan.sh GMT_color_interpolate.sh
	GMT_coverlogo.sh GMT_Defaults_1a.sh GMT_Defaults_1b.sh GMT_Defaults_1c.sh
	GMT_linecap.sh GMT_nearneighbor.sh GMT_pstext_clearance.sh GMT_pstext_justify.sh GMT_registration.sh
	GMT_utm_zones.sh GMT_volcano.sh)

# Scripts that are only used for testing, figures are not in the documentation
set (_scr_test GMT_RGBchart.sh)

set (_scripts_tests ${_scr_nostrip} ${_scr_strip} ${_scr_other} ${_scr_test})

set (_scripts_txt ${_scr_nostrip} ${_scr_strip})
string (REPLACE ".sh" ".txt" _scripts_txt "${_scripts_txt}")

set (_scripts_ps ${_scr_nostrip} ${_scr_strip} ${_scr_other}
	GMT_-U.ps GMT_RGBchart_a4.ps GMT_RGBchart_letter.ps GMT_RGBchart_tabloid.ps)
string (REPLACE ".sh" ".ps" _scripts_ps "${_scripts_ps}")

set (_scripts_pdf)
set (_install_pdf GMT_RGBchart_a4.pdf GMT_RGBchart_tabloid.pdf GMT_RGBchart_letter.pdf)

# Convert figures to PDF
foreach (_fig ${_scripts_ps})
	string (REPLACE ".ps" ".pdf" _pdf_fig ${_fig})
	list (APPEND _scripts_pdf ${_pdf_fig})
	add_custom_command (
		OUTPUT ${_pdf_fig}
		COMMAND
		GMT_USERDIR=${GMT_BINARY_DIR}/share
		GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
		${GMT_BINARY_DIR}/src/ps2raster -A -P -Tf
		-D${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/${_fig}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS ps2raster ${_fig})
endforeach (_fig ${_scripts_ps})

if (PDFLATEX_COMPILER)
	foreach (_script ${_scr_nostrip})
		string (REPLACE ".sh" ".txt" _txt ${_script})
		# Add command that triggers generator macro when output does not exist
		add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_txt}
			COMMAND ${CMAKE_COMMAND}
			-D script=${CMAKE_CURRENT_SOURCE_DIR}/${_script}
			-D txt=${CMAKE_CURRENT_BINARY_DIR}/${_txt}
			-D no_strip=TRUE
			-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
			-P ${CMAKE_MODULE_PATH}/ScriptToVerbatim.cmake
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_script} ${CMAKE_MODULE_PATH}/ScriptToVerbatim.cmake)
	endforeach (_script)

	foreach (_script ${_scr_strip})
		string (REPLACE ".sh" ".txt" _txt ${_script})
		add_custom_command (OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_txt}
			COMMAND ${CMAKE_COMMAND}
			-D SCRIPTS_COMMAND=script_to_verbatim
			-D script=${CMAKE_CURRENT_SOURCE_DIR}/${_script}
			-D txt=${CMAKE_CURRENT_BINARY_DIR}/${_txt}
			-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
			-P ${CMAKE_MODULE_PATH}/ScriptToVerbatim.cmake
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_script} ${CMAKE_MODULE_PATH}/ScriptToVerbatim.cmake)
	endforeach (_script)

	add_custom_target (scripts_txt DEPENDS ${_scripts_txt})
endif (PDFLATEX_COMPILER)

# run tests
if (DO_TESTS)
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

	foreach (_job ${_scripts_tests})
		add_test (NAME ${_job}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/${_job})
	endforeach (_job)
endif (DO_TESTS)

# Add build target
add_custom_target (scripts_pdf DEPENDS ${_scripts_pdf})

if (NOT EXISTS ${GMT_INSTALL_EXTERNAL_DOC})
	# Install pdf figures (if found) when there is no external location
	foreach (_fig ${_install_pdf})
		install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${_fig}
			DESTINATION ${GMT_DOC_PATH}/pdf
			COMPONENT Documentation
			OPTIONAL)
	endforeach (_fig ${_install_pdf})
endif (NOT EXISTS ${GMT_INSTALL_EXTERNAL_DOC})

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
