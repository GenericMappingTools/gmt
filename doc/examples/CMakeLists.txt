#
#
# Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

# List all PS files and convert them to PNG

file (GLOB _examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*/*.ps")

set (_examples_png)

# Convert figures to PNG
foreach (_ps ${_examples})
	get_filename_component (_fig ${_ps} NAME)
	string (REPLACE ".ps" ".png" _png_fig ${_fig})
	list (APPEND _examples_png ${RST_BINARY_DIR}/_images/${_png_fig})

	add_custom_command (OUTPUT ${RST_BINARY_DIR}/_images/${_png_fig}
		COMMAND ${CMAKE_COMMAND} -E env
		GMT_USERDIR=${GMT_BINARY_DIR}/share
		GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
		${GMT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/src/gmt psconvert
		-A -P -E150 -Tg -Qg4 -Qt4
		-C-sFONTPATH="${GMT_SOURCE_DIR}/doc/examples/ex31/fonts"
		-D${RST_BINARY_DIR}/_images
		${CMAKE_CURRENT_SOURCE_DIR}/${_ps}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS gmt_for_img_convert ${CMAKE_CURRENT_SOURCE_DIR}/${_ps})
endforeach (_ps ${_examples})

# List of job scripts and convert to verbatim
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

# Animations
if (UNIX AND DO_ANIMATIONS)
	configure_file (animate.in animate @ONLY)

	foreach (_num 01 02 03 04 05 06 07 08)
		add_custom_command (
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/anim${_num}/anim${_num}.gif
			COMMAND ${BASH} animate anim${_num}/anim${_num}.sh
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS gmt_for_img_convert ${CMAKE_CURRENT_SOURCE_DIR}/anim${_num}/anim${_num}.sh)
		add_custom_command (
			OUTPUT ${RST_BINARY_DIR}/_images/anim${_num}.gif
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_CURRENT_BINARY_DIR}/anim${_num}/anim${_num}.gif
			${RST_BINARY_DIR}/_images/anim${_num}.gif
			DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/anim${_num}/anim${_num}.gif)
		list (APPEND _animations ${RST_BINARY_DIR}/_images/anim${_num}.gif)
	endforeach ()

	# copy video files from anim 04, 06, 07, 08
	foreach (_num 04 06 07 08)
		add_custom_command (
			OUTPUT ${RST_STATIC_DIR}/anim${_num}.mp4
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_CURRENT_BINARY_DIR}/anim${_num}/anim${_num}.mp4
			${RST_STATIC_DIR}/anim${_num}.mp4
			DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/anim${_num}/anim${_num}.gif)
		list (APPEND _animations ${RST_STATIC_DIR}/anim${_num}.mp4)
	endforeach ()
	add_custom_target (animation DEPENDS ${_animations})

	# clean target
	foreach (_num 01 02 03 04 05 06 07 08)
		add_custom_target (_anim_clean${_num}
			COMMAND ${CMAKE_COMMAND} -E remove_directory
			${CMAKE_CURRENT_BINARY_DIR}/anim${_num}
			COMMENT "Removing animation ${_num}")
		add_depend_to_target (spotless _anim_clean${_num})
	endforeach ()
endif (UNIX AND DO_ANIMATIONS)

# run examples (test)
if (DO_EXAMPLES AND BASH)
	# this file takes care of setting up the test environment
	configure_file (gmtest.in gmtest @ONLY)
	foreach (_job ${_examples})
		add_test (NAME ${_job}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${BASH} gmtest ${_job})
	endforeach (_job ${_examples})
endif (DO_EXAMPLES AND BASH)
