#
# Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

if (SPHINX_FOUND)
	# Convert figures to PNG
	file (GLOB _examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*/*.ps")
	set (_examples_png)
	foreach (_ps ${_examples})
		get_filename_component (_fig ${_ps} NAME)
		string (REPLACE ".ps" ".png" _png_fig ${_fig})
		list (APPEND _examples_png ${RST_BINARY_DIR}/_images/${_png_fig})
		add_custom_command (OUTPUT ${RST_BINARY_DIR}/_images/${_png_fig}
			COMMAND ${CMAKE_COMMAND} -E env
			GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
			${GMT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/src/gmt psconvert
			-A -P -E150 -Tg -Qg4 -Qt4
			-C-sFONTPATH="${GMT_SOURCE_DIR}/doc/examples/ex31/fonts"
			-D${RST_BINARY_DIR}/_images
			${CMAKE_CURRENT_SOURCE_DIR}/${_ps}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS gmt_for_img_convert ${CMAKE_CURRENT_SOURCE_DIR}/${_ps})
	endforeach (_ps ${_examples})

	# Convert scripts to verbatim
	file (GLOB _examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*/*.sh")
	set (_examples_txt)
	foreach (_script ${_examples})
		get_filename_component (_txt ${_script} NAME)
		string (REPLACE ".sh" ".txt" _txt ${_txt})
		list (APPEND _examples_txt ${RST_BINARY_DIR}/_verbatim/${_txt})
		add_custom_command (OUTPUT ${RST_BINARY_DIR}/_verbatim/${_txt}
			COMMAND ${GMT_BINARY_DIR}/src/script2verbatim
			${CMAKE_CURRENT_SOURCE_DIR}/${_script}
			${RST_BINARY_DIR}/_verbatim/${_txt}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS script2verbatim _docs_rst_mkdir_verbatim ${CMAKE_CURRENT_SOURCE_DIR}/${_script})
	endforeach (_script ${_examples})

	# Add build target
	add_custom_target (_docs_examples_verbatim DEPENDS ${_examples_txt})
	add_custom_target (_docs_html_examples_fig DEPENDS ${_examples_png})
	add_depend_to_target (docs_depends _docs_html_examples_fig _docs_examples_verbatim)

endif (SPHINX_FOUND)

# run examples (test)
file (GLOB _examples RELATIVE ${CMAKE_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/ex*/*.sh")
# Remove a few known failed examples from tests
list (REMOVE_ITEM _examples "doc/examples/ex51/ex51.sh")
list (REMOVE_ITEM _examples "doc/examples/ex49/ex49.sh")
if (DO_EXAMPLES AND BASH)
	foreach (_job ${_examples} ${_anims})
		add_test (NAME ${_job}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${BASH} ${CMAKE_BINARY_DIR}/test/gmtest ${_job})
	endforeach (_job ${_examples} ${_anims})
endif (DO_EXAMPLES AND BASH)
