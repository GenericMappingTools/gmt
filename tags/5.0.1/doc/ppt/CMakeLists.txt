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
set (_copy_only_figures formatpicture.png formatting.png highres.png
	noantialias.png rendering.png withantialias.png)

# Copy figures without converting
foreach (_fig ${_copy_only_figures})
	add_custom_command (OUTPUT ${_fig}
		COMMAND ${CMAKE_COMMAND}
		-E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/${_fig}
		${_fig}
		DEPENDS ${_fig})
endforeach (_fig ${_copy_only_figures})

# Add build target
add_custom_target (copy_ppt_figures DEPENDS ${_copy_only_figures})
add_dependencies (copy_other_figures copy_ppt_figures)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
