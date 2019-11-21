#
#
# Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
# Contact info: www.generic-mapping-tools.org
#-------------------------------------------------------------------------------

# Include all PNG and JPG files
file (GLOB _fig_fig RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	"${CMAKE_CURRENT_SOURCE_DIR}/*.png"
	"${CMAKE_CURRENT_SOURCE_DIR}/*.jpg")
# Remove some unused figures
list (REMOVE_ITEM _fig_fig
	formatting.png highres.png noantialias.png withantialias.png)

# Copy figures without converting
FILE(MAKE_DIRECTORY ${RST_BINARY_DIR}/_images)
set (_fig_fig_out)
foreach (_fig ${_fig_fig})
	add_custom_command (OUTPUT ${RST_BINARY_DIR}/_images/${_fig}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_SOURCE_DIR}/${_fig} ${RST_BINARY_DIR}/_images/${_fig}
		DEPENDS ${_fig})
	list (APPEND _fig_fig_out ${RST_BINARY_DIR}/_images/${_fig})
endforeach (_fig ${_fig_fig})

# Add build target
add_custom_target (_docs_copy_fig DEPENDS ${_fig_fig_out})
add_depend_to_target (docs_depends _docs_copy_fig)
