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

	# Add build targets
	if (WIN32)
		add_custom_target (docs_html
			COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/do_fig_includes.bat
			COMMAND ${SPHINX_EXECUTABLE} -q -b html
			-c "${SPHINX_CONFIG_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			${CMAKE_CURRENT_SOURCE_DIR}/source
			${CMAKE_CURRENT_BINARY_DIR}/html
			COMMENT "Building HTML documentation. Be patient, it takes more than 2 minutes")

		add_custom_target (docs_latex
			COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/do_fig_includes.bat pdf
			COMMAND ${SPHINX_EXECUTABLE} -q -b latex
			-c "${SPHINX_CONFIG_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			${CMAKE_CURRENT_SOURCE_DIR}/source
			${CMAKE_CURRENT_BINARY_DIR}/latex
			COMMENT "Building CookBook PDF. Be patient, it takes more than 2 minutes")
	else (WIN32)
		add_custom_target (docs_html
			COMMAND /bin/bash ${CMAKE_CURRENT_SOURCE_DIR}/do_fig_includes.sh
			COMMAND ${SPHINX_EXECUTABLE} -q -b html
			-c "${SPHINX_CONFIG_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			${CMAKE_CURRENT_SOURCE_DIR}/source
			${CMAKE_CURRENT_BINARY_DIR}/html
			COMMENT "Building HTML documentation. Be patient, it takes more than 2 minutes")

		add_custom_target (docs_latex
			COMMAND /bin/bash ${CMAKE_CURRENT_SOURCE_DIR}/do_fig_includes.sh pdf
			COMMAND ${SPHINX_EXECUTABLE} -q -b latex
			-c "${SPHINX_CONFIG_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			${CMAKE_CURRENT_SOURCE_DIR}/source
			${CMAKE_CURRENT_BINARY_DIR}/latex
			COMMENT "Building CookBook PDF. Be patient, it takes more than 2 minutes")
	endif (WIN32)

	add_custom_target (docs_man
		${SPHINX_EXECUTABLE} -q -b man
		-c "${SPHINX_CONFIG_DIR}"
		-d "${SPHINX_CACHE_DIR}"
		${CMAKE_CURRENT_SOURCE_DIR}/source
		${CMAKE_CURRENT_BINARY_DIR}/man
		COMMENT "Building manpages")
endif (SPHINX_FOUND)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
