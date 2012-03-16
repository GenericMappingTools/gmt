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
	GMT_Environment.pdf gimp-panel.png gimp-sliders.png hsv-cone.png
	formatpicture.png formatting.png highres.png
	noantialias.png rendering.png withantialias.png)

# Copy figures without converting
foreach (_fig ${_fig_fig})
	add_custom_command (OUTPUT ${_fig}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/${_fig} ${_fig}
		DEPENDS ${_fig})
endforeach (_fig ${_fig_fig})

# Add build target
add_custom_command (OUTPUT done COMMAND touch done DEPENDS ${_fig_fig})
add_custom_target (fig_done DEPENDS done)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
