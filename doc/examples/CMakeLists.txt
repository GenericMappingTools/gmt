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

# List all PS files and convert them to PDF

file (GLOB _examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*/*.ps")

set (_examples_pdf)
set (_examples_png)

# Convert figures to PDF and PNG
foreach (_ps ${_examples})
	get_filename_component (_fig ${_ps} NAME)
	string (REPLACE ".ps" ".pdf" _pdf_fig ${_fig})
	string (REPLACE ".ps" ".png" _png_fig ${_fig})
	list (APPEND _examples_pdf ${RST_BINARY_DIR}/_images/${_pdf_fig})
	list (APPEND _examples_png ${RST_BINARY_DIR}/_images/${_png_fig})

	if (WIN32)
		add_custom_command (OUTPUT ${RST_BINARY_DIR}/_images/${_pdf_fig}
			COMMAND
			set GMT_USERDIR=${GMT_BINARY_DIR}/share
			COMMAND
			set GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
			COMMAND
			${GMT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/src/gmt psconvert -A+S0.6 -P -Tf
			-C-sFONTPATH=${GMT_SOURCE_DIR}/doc/examples/ex31/fonts
			-D${RST_BINARY_DIR}/_images
			${CMAKE_CURRENT_SOURCE_DIR}/${_ps}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS gmt_for_img_convert ${CMAKE_CURRENT_SOURCE_DIR}/${_ps})
		add_custom_command (OUTPUT ${RST_BINARY_DIR}/_images/${_png_fig}
			COMMAND
			set GMT_USERDIR=${GMT_BINARY_DIR}/share
			COMMAND
			set GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
			COMMAND
			${GMT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/src/gmt psconvert
			-A -P -E150 -TG -Qg4 -Qt4 -C-dDOINTERPOLATE
			-C-sFONTPATH=${GMT_SOURCE_DIR}/doc/examples/ex31/fonts
			-D${RST_BINARY_DIR}/_images
			${CMAKE_CURRENT_SOURCE_DIR}/${_ps}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS gmt_for_img_convert ${CMAKE_CURRENT_SOURCE_DIR}/${_ps})
	else (WIN32)
		add_custom_command (OUTPUT ${RST_BINARY_DIR}/_images/${_pdf_fig}
			COMMAND
			GMT_USERDIR=${GMT_BINARY_DIR}/share
			GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
			${GMT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/src/gmt psconvert -A+S0.6 -P -Tf
			-C-sFONTPATH=${GMT_SOURCE_DIR}/doc/examples/ex31/fonts
			-D${RST_BINARY_DIR}/_images
			${CMAKE_CURRENT_SOURCE_DIR}/${_ps}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS gmt_for_img_convert ${CMAKE_CURRENT_SOURCE_DIR}/${_ps})
		add_custom_command (OUTPUT ${RST_BINARY_DIR}/_images/${_png_fig}
			COMMAND
			GMT_USERDIR=${GMT_BINARY_DIR}/share
			GMT_SHAREDIR=${GMT_SOURCE_DIR}/share
			${GMT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/src/gmt psconvert
			-A -P -E150 -TG -Qg4 -Qt4 -C-dDOINTERPOLATE
			-C-sFONTPATH=${GMT_SOURCE_DIR}/doc/examples/ex31/fonts
			-D${RST_BINARY_DIR}/_images
			${CMAKE_CURRENT_SOURCE_DIR}/${_ps}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS gmt_for_img_convert ${CMAKE_CURRENT_SOURCE_DIR}/${_ps})
	endif (WIN32)
endforeach (_ps ${_examples})

# List of job scripts and convert to verbatim

file (GLOB _examples RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*/*.sh")

set (_examples_txt)

foreach (_script ${_examples})
	get_filename_component (_txt ${_script} NAME)
	string (REPLACE ".sh" ".txt" _txt ${_txt})
	list (APPEND _examples_txt ${RST_BINARY_DIR}/_verbatim/${_txt})

	add_custom_command (OUTPUT ${RST_BINARY_DIR}/_verbatim/${_txt}
		COMMAND ${CMAKE_COMMAND}
		-D script=${CMAKE_CURRENT_SOURCE_DIR}/${_script}
		-D txt=${RST_BINARY_DIR}/_verbatim/${_txt}
		-D no_strip=TRUE
		-D CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
		-P ${CMAKE_MODULE_PATH}/ScriptToVerbatim.cmake
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_script})
endforeach (_script ${_examples})

# Add build target
add_custom_target (_docs_examples_fig DEPENDS ${_examples_pdf} ${_examples_png})
add_custom_target (_docs_examples_verbatim DEPENDS ${_examples_txt})
add_depend_to_target (docs_depends _docs_examples_fig _docs_examples_verbatim)

# Animation
if (UNIX)
	foreach (_num 01 02 03 04)
		add_custom_command (
			OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/anim${_num}/anim_${_num}.gif
			COMMAND ${BASH} gmtest anim${_num}/anim_${_num}.sh 1
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			DEPENDS gmt_for_img_convert ${CMAKE_CURRENT_SOURCE_DIR}/anim${_num}/anim_${_num}.sh)
		add_custom_command (
			OUTPUT ${RST_BINARY_DIR}/_images/anim_${_num}.gif
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_CURRENT_BINARY_DIR}/anim${_num}/anim_${_num}.gif
			${RST_BINARY_DIR}/_images/anim_${_num}.gif
			DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/anim${_num}/anim_${_num}.gif)
		list (APPEND _animations ${RST_BINARY_DIR}/_images/anim_${_num}.gif)
	endforeach ()
	add_custom_target (animation DEPENDS ${_animations})
endif (UNIX)

# run examples (test)
if (DO_EXAMPLES) # AND UNIX)
	# this file takes care of setting up the test environment
	configure_file (gmtest.in gmtest @ONLY)

	# Workaround cmake bug 3957: CRLF line ending
	find_package (UnixCommands)
	if (CYGWIN_INSTALL_PATH)
		find_program (D2U d2u
			${CYGWIN_INSTALL_PATH}/bin)
		execute_process (COMMAND ${D2U}
			${CMAKE_CURRENT_BINARY_DIR}/gmtest)
	endif (CYGWIN_INSTALL_PATH)

	foreach (_job ${_examples})
		string (REPLACE "//" "/" _job ${_job})	# GLOB creates double slashes we do not want
		add_test (NAME ${_job}
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${BASH} gmtest ${_job})
	endforeach (_job ${_examples})
elseif (DO_EXAMPLES_WIN_BATCH_DISABLED) # (DO_EXAMPLES AND WIN32)
	# only supporting examples with installed GMT in %path%
	string (REPLACE ".sh" ".bat" _examples_win "${_examples}")
	foreach (_job ${_examples_win})
		get_filename_component (_job_path
			${CMAKE_CURRENT_BINARY_DIR}/${_job} PATH)
		add_test (NAME ${_job}
			WORKING_DIRECTORY ${_job_path}
			COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${_job})
	endforeach (_job ${_examples_win})
endif (DO_EXAMPLES) # AND UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
