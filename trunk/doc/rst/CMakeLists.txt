#
# $Id$
#
# Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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

# reST documentation
find_package (Sphinx)

# path where configuration file (conf.py) is located
set (SPHINX_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")

# path for the cached environment and doctree files
set (SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

if (SPHINX_FOUND)
	# Create configuration file
	configure_file (conf.py.in "${SPHINX_CONFIG_DIR}/conf.py" @ONLY)

	# Copy reST sources to build tree
	include (CopyDirIfDifferent)
	copy_dir_if_different (_docs_copy_rst_tree ${CMAKE_CURRENT_SOURCE_DIR}/source/
		${CMAKE_CURRENT_BINARY_DIR}/_source)
	add_depend_to_target (docs_depends _docs_copy_rst_tree)

	# Add build targets
	if (WIN32)
		# this is broken until includes genreated by do_fig_includes_in_build.sh
		# are inserted verbatim into rst sources.
		# But we need to have the targets deffined otherwise the whole build fails
		add_custom_target (docs_html
			# todo: remove do_fig_includes_in_build.sh
			COMMAND ${BASH}
			${CMAKE_CURRENT_SOURCE_DIR}/do_fig_includes_in_build.sh
			png ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${SPHINX_EXECUTABLE} -q -b html
			-c "${SPHINX_CONFIG_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			${CMAKE_CURRENT_BINARY_DIR}/_source
			${CMAKE_CURRENT_BINARY_DIR}/html
			COMMENT "Building HTML documentation... it takes more than 2-3 minutes")

		add_custom_target (docs_latex
			# todo: remove do_fig_includes_in_build.sh
			COMMAND ${BASH} ${CMAKE_CURRENT_SOURCE_DIR}/do_fig_includes_in_build.sh
			pdf ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${SPHINX_EXECUTABLE} -q -b latex
			-c "${SPHINX_CONFIG_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			${CMAKE_CURRENT_BINARY_DIR}/_source
			${CMAKE_CURRENT_BINARY_DIR}/latex
			COMMAND make -C ${CMAKE_CURRENT_BINARY_DIR}/latex all-pdf
			COMMENT "Building CookBook PDF... it takes more than 2-3 minutes")
	else (WIN32)
		add_custom_target (docs_html
			# todo: remove do_fig_includes_in_build.sh
			COMMAND ${BASH}
			${CMAKE_CURRENT_SOURCE_DIR}/do_fig_includes_in_build.sh
			png ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${SPHINX_EXECUTABLE} -q -b html
			-c "${SPHINX_CONFIG_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			${CMAKE_CURRENT_BINARY_DIR}/_source
			${CMAKE_CURRENT_BINARY_DIR}/html
			COMMENT "Building HTML documentation... it takes more than 2-3 minutes")

		add_custom_target (docs_latex
			# todo: remove do_fig_includes_in_build.sh
			COMMAND ${BASH} ${CMAKE_CURRENT_SOURCE_DIR}/do_fig_includes_in_build.sh
			pdf ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
			COMMAND ${SPHINX_EXECUTABLE} -q -b latex
			-c "${SPHINX_CONFIG_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			${CMAKE_CURRENT_BINARY_DIR}/_source
			${CMAKE_CURRENT_BINARY_DIR}/latex
			COMMAND make -C ${CMAKE_CURRENT_BINARY_DIR}/latex all-pdf
			COMMENT "Building CookBook PDF... it takes more than 2-3 minutes")
	endif (WIN32)

	add_custom_target (docs_man
		${SPHINX_EXECUTABLE} -q -b man
		-c "${SPHINX_CONFIG_DIR}"
		-d "${SPHINX_CACHE_DIR}"
		${CMAKE_CURRENT_BINARY_DIR}/_source
		${CMAKE_CURRENT_BINARY_DIR}/man
		COMMENT "Building manpages...")

	add_dependencies (docs_man docs_depends)
	add_dependencies (docs_html docs_depends)
	add_dependencies (docs_latex docs_depends)
endif (SPHINX_FOUND)

# Install man (if available)
# todo: gzip manpages and sort into share/man/man1, man3, and man5
install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/man/
	DESTINATION ${GMT_SHARE_PATH}/man/man1
	COMPONENT Runtime
	OPTIONAL)

# Install html (if available)
install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
	DESTINATION ${GMT_DOC_PATH}/html
	COMPONENT Documentation
	OPTIONAL)

# Install pdf (if available)
install (FILES
	${CMAKE_CURRENT_BINARY_DIR}/latex/API.pdf
	${CMAKE_CURRENT_BINARY_DIR}/latex/GMT5.pdf
	${CMAKE_CURRENT_BINARY_DIR}/latex/man.pdf
	DESTINATION ${GMT_DOC_PATH}/pdf
	COMPONENT Documentation
	OPTIONAL)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
