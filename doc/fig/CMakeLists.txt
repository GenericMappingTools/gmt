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
set (_fig_fig GMT4_mode.png GMT5_Summit_2011.png GMT5_mode.png
	GMT_Environment.pdf gimp-panel.png gimp-sliders.png hsv-cone.png)

set (_fig_example anim_01.ps anim_02.ps
	anim_03.ps anim_04.ps example_01.ps example_02.ps example_03.ps
	example_04.ps example_04c.ps example_05.ps example_06.ps example_07.ps
	example_08.ps example_09.ps example_10.ps example_11.ps example_12.ps
	example_13.ps example_14.ps example_15.ps example_16.ps example_17.ps
	example_18.ps example_19.ps example_20.ps example_21.ps example_22.ps
	example_23.ps example_24.ps example_25.ps example_26.ps example_27.ps
	example_28.ps example_29.ps example_30.ps example_31.ps example_32.ps)
string (REPLACE ".ps" ".pdf" _fig_pdf "${_fig_example}")

set (_pdf_fig_install GMT_Manpages.pdf GMT_Manpages_suppl.pdf)

# Copy figures without converting
foreach (_fig ${_fig_fig})
	add_custom_command (OUTPUT ${_fig}
		COMMAND ${CMAKE_COMMAND}
		-E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/${_fig}
		${_fig}
		DEPENDS ${_fig})
endforeach (_fig ${_fig_fig})
add_custom_target (fig_fig DEPENDS ${_fig_fig})

# Convert figures to PDF
foreach (_fig ${_fig_example})
	string (REPLACE ".ps" ".pdf" _pdf_fig ${_fig})
	add_custom_command (OUTPUT ${_pdf_fig}
		COMMAND
		GMT_USERDIR=${GMT_BINARY_DIR}/share
		GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
		${GMT_BINARY_DIR}/src/ps2raster -Au -P -Tf
		-C-sFONTPATH=${GMT_SOURCE_DIR}/doc/examples/ex31/fonts
		-D${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/${_fig}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS ps2raster ${_fig})
endforeach (_fig ${_fig_example})

# Add build target
add_custom_target (fig_pdf DEPENDS ${_fig_pdf})

# Install pdf figures (if found)
foreach (_fig ${_pdf_fig_install})
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${_fig}
		DESTINATION ${GMT_DOC_PATH}/pdf
		COMPONENT Documentation
		OPTIONAL)
endforeach (_fig ${_pdf_fig_install})

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
