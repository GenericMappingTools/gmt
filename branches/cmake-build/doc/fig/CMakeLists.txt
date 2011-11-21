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

# Figure files
set (_copy_only_figures GMT4_mode.png GMT5_Summit_2011.png GMT5_mode.png
	gimp-panel.png gimp-sliders.png hsv-cone.png)

set (_doc_figures GMT_-B_custom.ps GMT_-B_geo_1.ps GMT_-B_geo_2.ps
	GMT_-B_linear.ps GMT_-B_log.ps GMT_-B_pow.ps GMT_-B_time1.ps GMT_-B_time2.ps
	GMT_-B_time3.ps GMT_-B_time4.ps GMT_-B_time5.ps GMT_-B_time6.ps
	GMT_-B_time7.ps GMT_-J.ps GMT_-OK.ps GMT_-P.ps GMT_-R.ps GMT_-U.ps
	GMT_-XY.ps GMT_App_E.ps GMT_App_F_dingbats.ps GMT_App_F_iso+.ps
	GMT_App_F_stand+.ps GMT_App_F_symbol.ps GMT_App_G.ps GMT_App_J_1.ps
	GMT_App_J_2.ps GMT_App_J_3.ps GMT_App_K_1.ps GMT_App_K_2.ps GMT_App_K_3.ps
	GMT_App_K_4.ps GMT_App_K_5.ps GMT_App_M_1.ps GMT_App_M_2.ps GMT_App_N_1.ps
	GMT_App_O_1.ps GMT_App_O_2.ps GMT_App_O_3.ps GMT_App_O_4.ps GMT_App_O_5.ps
	GMT_App_O_6.ps GMT_App_O_7.ps GMT_App_O_8.ps GMT_App_O_9.ps GMT_App_P_1.ps
	GMT_App_P_2.ps GMT_Defaults_1a.ps GMT_Defaults_1b.ps GMT_Defaults_1c.ps
	GMT_RGBchart_a4.ps GMT_RGBchart_letter.ps GMT_RGBchart_tabloid.ps GMT_TM.ps
	GMT_albers.ps GMT_atan.ps GMT_az_equidistant.ps GMT_cassini.ps
	GMT_color_interpolate.ps GMT_coverlogo.ps GMT_covertext.ps GMT_eckert4.ps
	GMT_eckert6.ps GMT_equi_cyl.ps GMT_equidistant_conic.ps GMT_gall_stereo.ps
	GMT_general_cyl.ps GMT_gnomonic.ps GMT_grinten.ps GMT_hammer.ps
	GMT_lambert_az_hemi.ps GMT_lambert_az_rect.ps GMT_lambert_conic.ps
	GMT_linear.ps GMT_linear_cal.ps GMT_linear_d.ps GMT_linecap.ps GMT_log.ps
	GMT_mercator.ps GMT_miller.ps GMT_mollweide.ps GMT_nearneighbor.ps
	GMT_obl_merc.ps GMT_orthographic.ps GMT_perspective.ps GMT_polar.ps
	GMT_polyconic.ps GMT_pow.ps GMT_pstext_clearance.ps GMT_pstext_justify.ps
	GMT_registration.ps GMT_robinson.ps GMT_sinus_int.ps GMT_sinusoidal.ps
	GMT_stereographic_general.ps GMT_stereographic_polar.ps
	GMT_stereographic_rect.ps GMT_stereonets.ps GMT_transverse_merc.ps
	GMT_utm_zones.ps GMT_volcano.ps GMT_winkel.ps GMT_Environment.eps)

set (_example_figures anim_01.ps anim_02.ps
	anim_03.ps anim_04.ps example_01.ps example_02.ps example_03.ps
	example_04.ps example_04c.ps example_05.ps example_06.ps example_07.ps
	example_08.ps example_09.ps example_10.ps example_11.ps example_12.ps
	example_13.ps example_14.ps example_15.ps example_16.ps example_17.ps
	example_18.ps example_19.ps example_20.ps example_21.ps example_22.ps
	example_23.ps example_24.ps example_25.ps example_26.ps example_27.ps
	example_28.ps example_29.ps example_30.ps example_31.ps example_32.ps)

set (_pdf_fig_install GMT_Manpages.pdf GMT_RGBchart_a4.pdf
	GMT_RGBchart_tabloid.pdf GMT_Manpages_suppl.pdf GMT_RGBchart_letter.pdfs)

# Copy figures without converting
foreach (_fig ${_copy_only_figures})
	add_custom_command (OUTPUT ${_fig}
		COMMAND ${CMAKE_COMMAND}
		-E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/${_fig}
		${_fig}
		DEPENDS ${_fig})
endforeach (_fig ${_copy_only_figures})
add_custom_target (copy_other_figures DEPENDS ${_copy_only_figures})

# Convert figures to PDF
macro (_ps2raster _filename)
	set(_ADJUST_BB -A)
	foreach(_arg ${ARGN})
		if(${_arg} STREQUAL "STRIP_TIMESTAMP")
			set(_ADJUST_BB -Au)
		endif(${_arg} STREQUAL "STRIP_TIMESTAMP")
	endforeach(_arg ${ARGN})

	string (REGEX REPLACE "[.][e]?ps$" ".pdf" _pdf_fig ${_fig})
  list (APPEND _pdf_figures ${_pdf_fig})
	add_custom_command (OUTPUT ${_pdf_fig}
		COMMAND
		GMT_USERDIR=${GMT_BINARY_DIR}/share
		GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
		${GMT_BINARY_DIR}/src/ps2raster ${_ADJUST_BB} -P -Tf
		-C-sFONTPATH=${GMT_SOURCE_DIR}/doc/examples/ex31/fonts
		-D${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/${_fig}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS ps2raster ${_fig})
	#COMMENT "Generating ${_pdf_fig}")
endmacro (_ps2raster _filename)

set (_pdf_figures)

foreach (_fig ${_doc_figures})
	_ps2raster (${_fig})
endforeach (_fig ${_doc_figures})

foreach (_fig ${_example_figures})
	_ps2raster (${_fig} STRIP_TIMESTAMP)
endforeach (_fig ${_example_figures})

# Add build target
add_custom_target (pdf_figures DEPENDS ${_pdf_figures})

# Install pdf figures (if found)
foreach (_fig ${_pdf_fig_install})
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${_fig}
		DESTINATION ${GMT_DOC_PATH}/pdf
		COMPONENT Documentation
		OPTIONAL)
endforeach (_fig ${_pdf_fig_install})

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
