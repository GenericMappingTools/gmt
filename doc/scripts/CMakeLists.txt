#
# $Id$
#
# Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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
	GMT_App_F_stand+_iso+.sh GMT_App_F_symbol.sh GMT_App_F_symbol_dingbats.sh
	GMT_App_G.sh GMT_App_J_1.sh GMT_App_J_2.sh GMT_App_J_3.sh GMT_App_M_1a.sh GMT_App_M_1b.sh
	GMT_App_N_1.sh GMT_arrows.sh GMT_atan.sh GMT_color_interpolate.sh
	GMT_chunking.sh GMT_coverlogo.sh GMT_Defaults_1a.sh GMT_Defaults_1b.sh
	GMT_Defaults_1c.sh GMT_linecap.sh GMT_nearneighbor.sh
	GMT_pstext_clearance.sh GMT_pstext_justify.sh GMT_registration.sh
	GMT_utm_zones.sh GMT_volcano.sh)

# Scripts that are only used for testing, figures are not in the documentation
set (_scr_test GMT_RGBchart.sh)
# This is no test: GMT_encoding.sh

set (_scripts_tests ${_scr_nostrip} ${_scr_strip} ${_scr_other} ${_scr_test})

set (_scripts_ps ${_scr_nostrip} ${_scr_strip} ${_scr_other}
	GMT_-U.ps GMT_RGBchart_a4.ps GMT_RGBchart_letter.ps GMT_RGBchart_tabloid.ps)
string (REPLACE ".sh" ".ps" _scripts_ps "${_scripts_ps}")

set (_scripts_pdf)
set (_scripts_png)
set (_scripts_txt)
set (_install_pdf GMT_RGBchart_a4.pdf GMT_RGBchart_tabloid.pdf GMT_RGBchart_letter.pdf)

# Convert figures to PDF and PNG
foreach (_fig ${_scripts_ps})
	string (REPLACE ".ps" ".pdf" _pdf_fig ${_fig})
	string (REPLACE ".ps" ".png" _png_fig ${_fig})
	list (APPEND _scripts_pdf ${RST_BINARY_DIR}/_images/${_pdf_fig})
	list (APPEND _scripts_png ${RST_BINARY_DIR}/_images/${_png_fig})
	FILE(MAKE_DIRECTORY ${RST_BINARY_DIR}/_images)
	if (WIN32)
		add_custom_command (OUTPUT ${RST_BINARY_DIR}/_images/${_pdf_fig}
			COMMAND
			set GMT_USERDIR=${GMT_BINARY_DIR}/share
			COMMAND
			set GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
			COMMAND
			${GMT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/src/gmt ps2raster -A -P -Tf
			-D${RST_BINARY_DIR}/_images
			${CMAKE_CURRENT_SOURCE_DIR}/${_fig}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS gmt_for_img_convert ${_fig})
		add_custom_command (OUTPUT ${RST_BINARY_DIR}/_images/${_png_fig}
			COMMAND
			set GMT_USERDIR=${GMT_BINARY_DIR}/share
			COMMAND
			set GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
			COMMAND
			${GMT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/src/gmt ps2raster
			-A -P -E150 -TG -Qg4 -Qt4 -C-dDOINTERPOLATE
			-D${RST_BINARY_DIR}/_images
			${CMAKE_CURRENT_SOURCE_DIR}/${_fig}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS gmt_for_img_convert ${_fig})
	else (WIN32)
		add_custom_command (OUTPUT ${RST_BINARY_DIR}/_images/${_pdf_fig}
			COMMAND
			GMT_USERDIR=${GMT_BINARY_DIR}/share
			GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
			${GMT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/src/gmt ps2raster -A -P -Tf
			-D${RST_BINARY_DIR}/_images
			${CMAKE_CURRENT_SOURCE_DIR}/${_fig}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS gmt_for_img_convert ${_fig})
		add_custom_command (OUTPUT ${RST_BINARY_DIR}/_images/${_png_fig}
			COMMAND
			GMT_USERDIR=${GMT_BINARY_DIR}/share
			GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
			${GMT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/src/gmt ps2raster
			-A -P -E150 -TG -Qg4 -Qt4 -C-dDOINTERPOLATE
			-D${RST_BINARY_DIR}/_images
			${CMAKE_CURRENT_SOURCE_DIR}/${_fig}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS gmt_for_img_convert ${_fig})
	endif (WIN32)
endforeach (_fig ${_scripts_ps})

foreach (_script ${_scr_nostrip})
	string (REPLACE ".sh" ".txt" _txt ${_script})
	list (APPEND _scripts_txt ${RST_BINARY_DIR}/_verbatim/${_txt})
	FILE(MAKE_DIRECTORY ${RST_BINARY_DIR}/_verbatim)
	# Add command that triggers generator macro when output does not exist
	add_custom_command (OUTPUT ${RST_BINARY_DIR}/_verbatim/${_txt}
		COMMAND ${GMT_BINARY_DIR}/src/script2verbatim
		${CMAKE_CURRENT_SOURCE_DIR}/${_script}
		${RST_BINARY_DIR}/_verbatim/${_txt}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS script2verbatim _docs_rst_mkdir_verbatim ${CMAKE_CURRENT_SOURCE_DIR}/${_script})
endforeach (_script)

foreach (_script ${_scr_strip})
	string (REPLACE ".sh" ".txt" _txt ${_script})
	list (APPEND _scripts_txt ${RST_BINARY_DIR}/_verbatim/${_txt})
	FILE(MAKE_DIRECTORY ${RST_BINARY_DIR}/_verbatim)
	add_custom_command (OUTPUT ${RST_BINARY_DIR}/_verbatim/${_txt}
		COMMAND ${GMT_BINARY_DIR}/src/script2verbatim --strip-comments
		${CMAKE_CURRENT_SOURCE_DIR}/${_script}
		${RST_BINARY_DIR}/_verbatim/${_txt}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS script2verbatim _docs_rst_mkdir_verbatim ${CMAKE_CURRENT_SOURCE_DIR}/${_script})
endforeach (_script)

# Add build target
add_custom_target (_docs_scripts_fig DEPENDS ${_scripts_pdf} ${_scripts_png})
add_custom_target (_docs_scripts_verbatim DEPENDS ${_scripts_txt})
add_depend_to_target (docs_depends _docs_scripts_fig _docs_scripts_verbatim)

# run tests
if (DO_TESTS)
	# this file takes care of setting up the test environment
	configure_file (gmtest.in gmtest @ONLY)

	# Workaround cmake bug 3957: CRLF line ending
	if (CYGWIN_INSTALL_PATH)
		find_program (D2U d2u
			${CYGWIN_INSTALL_PATH}/bin)
		execute_process (COMMAND ${D2U}
			${CMAKE_CURRENT_BINARY_DIR}/gmtest)
	endif (CYGWIN_INSTALL_PATH)

	foreach (_job ${_scripts_tests})
		add_test (NAME ${_job}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${BASH} gmtest ${_job})
	endforeach (_job)
endif (DO_TESTS)

if (NOT EXISTS ${GMT_INSTALL_EXTERNAL_DOC})
	# Install pdf figures (if found) when there is no external location
	foreach (_fig ${_install_pdf})
		install (FILES ${RST_BINARY_DIR}/_images/${_fig}
			DESTINATION ${GMT_DOCDIR}/pdf
			COMPONENT Documentation
			OPTIONAL)
	endforeach (_fig ${_install_pdf})
endif (NOT EXISTS ${GMT_INSTALL_EXTERNAL_DOC})

# Install target for realease documentation
if (SVN AND HAVE_SVN_VERSION)
	foreach (_fig ${_install_pdf})
		add_custom_target (_${_fig}_release
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${RST_BINARY_DIR}/_images/${_fig}
			${GMT_RELEASE_PREFIX}/doc_release/pdf/${_fig}
			DEPENDS ${RST_BINARY_DIR}/_images/${_fig} svn_export_release)
		add_depend_to_target (gmt_release _${_fig}_release)
	endforeach (_fig ${_install_pdf})
endif (SVN AND HAVE_SVN_VERSION)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
