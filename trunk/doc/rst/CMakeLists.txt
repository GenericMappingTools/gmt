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

# path for the cached environment and doctree files
set (SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

if (SPHINX_FOUND)
	# Create configuration file
	string (REPLACE "_" " "
		GMT_PACKAGE_VERSION_WITH_SVN_REVISION_BLANK
		${GMT_PACKAGE_VERSION_WITH_SVN_REVISION})
	configure_file (conf.py.in conf.py @ONLY)

	# Copy reST sources to build tree
	include (CopyDirIfDifferent)
	copy_dir_if_different (_docs_copy_rst_tree ${CMAKE_CURRENT_SOURCE_DIR}/source/
		${CMAKE_CURRENT_BINARY_DIR}/_source)
	add_depend_to_target (docs_depends _docs_copy_rst_tree)

	# HTML build target
	add_custom_target (docs_html
		COMMAND ${SPHINX_EXECUTABLE} -q -b html
		-c "${CMAKE_CURRENT_BINARY_DIR}"
		-d "${SPHINX_CACHE_DIR}"
		-w "${CMAKE_CURRENT_BINARY_DIR}/html.log"
		${CMAKE_CURRENT_BINARY_DIR}/_source
		${CMAKE_CURRENT_BINARY_DIR}/html
		COMMENT "Building HTML documentation...")
	add_dependencies (docs_html docs_depends)

	# PDF build target
	find_program(MAKE NAMES make gmake gnumake)
	if (MAKE)
		add_custom_target (docs_latex
			COMMAND ${SPHINX_EXECUTABLE} -q -b latex
			-c "${CMAKE_CURRENT_BINARY_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			-w "${CMAKE_CURRENT_BINARY_DIR}/latex.log"
			${CMAKE_CURRENT_BINARY_DIR}/_source
			${CMAKE_CURRENT_BINARY_DIR}/latex
			COMMAND ${MAKE} -C ${CMAKE_CURRENT_BINARY_DIR}/latex all-pdf
			COMMENT "Building CookBook PDF...")
	add_dependencies (docs_latex docs_depends)
	endif (MAKE)

	# Build and compress manpages
	if (GZIP)
		set (_gz ".gz")
		add_custom_target (docs_man
			COMMAND ${SPHINX_EXECUTABLE} -q -b man
			-c "${CMAKE_CURRENT_BINARY_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			-w "${CMAKE_CURRENT_BINARY_DIR}/man.log"
			${CMAKE_CURRENT_BINARY_DIR}/_source
			${CMAKE_CURRENT_BINARY_DIR}/man
			COMMAND ${GZIP} -9 -f ${CMAKE_CURRENT_BINARY_DIR}/man/*.?
			COMMENT "Building manpages...")
	else (GZIP)
		set (_gz)
		add_custom_target (docs_man
			COMMAND ${SPHINX_EXECUTABLE} -q -b man
			-c "${CMAKE_CURRENT_BINARY_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			${CMAKE_CURRENT_BINARY_DIR}/_source
			${CMAKE_CURRENT_BINARY_DIR}/man
			COMMENT "Building manpages...")
	endif (GZIP)
	add_dependencies (docs_man docs_depends)

	# Install targets for realease documentation
	if (SVN AND HAVE_SVN_VERSION)
		# HTML
		add_custom_target (_html_release
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			${CMAKE_CURRENT_BINARY_DIR}/html/
			${GMT_RELEASE_PREFIX}/doc_release/html
			DEPENDS docs_html svn_export_release)
		add_depend_to_target (gmt_release _html_release)
		# PDF
		add_custom_target (_pdf_release
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_CURRENT_BINARY_DIR}/latex/GMT_API.pdf
			${GMT_RELEASE_PREFIX}/doc_release/pdf/GMT_API.pdf
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_CURRENT_BINARY_DIR}/latex/GMT_Docs.pdf
			${GMT_RELEASE_PREFIX}/doc_release/pdf/GMT_Docs.pdf
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_CURRENT_BINARY_DIR}/latex/GMT_Manpages.pdf
			${GMT_RELEASE_PREFIX}/doc_release/pdf/GMT_Manpages.pdf
			DEPENDS docs_latex svn_export_release)
		add_depend_to_target (gmt_release _pdf_release)
		# Manpages
		add_custom_target (_man_release
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			${CMAKE_CURRENT_BINARY_DIR}/man/
			${GMT_RELEASE_PREFIX}/man_release
			DEPENDS docs_man svn_export_release)
		add_depend_to_target (gmt_release _man_release)
	endif (SVN AND HAVE_SVN_VERSION)
endif (SPHINX_FOUND)

# Install targets
if (NOT EXISTS ${GMT_INSTALL_EXTERNAL_DOC})
	# Install html (if available)
	install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
		DESTINATION ${GMT_DOC_PATH}/html
		COMPONENT Documentation
		OPTIONAL)

	# Install pdf (if available)
	install (FILES
		${CMAKE_CURRENT_BINARY_DIR}/latex/GMT_API.pdf
		${CMAKE_CURRENT_BINARY_DIR}/latex/GMT_Docs.pdf
		${CMAKE_CURRENT_BINARY_DIR}/latex/GMT_Manpages.pdf
		DESTINATION ${GMT_DOC_PATH}/pdf
		COMPONENT Documentation
		OPTIONAL)
endif (NOT EXISTS ${GMT_INSTALL_EXTERNAL_DOC})

if (EXISTS ${GMT_INSTALL_EXTERNAL_MAN})
	# Install manpages from external location
	set (_man_source ${GMT_INSTALL_EXTERNAL_MAN})
elseif (EXISTS ${GMT_INSTALL_EXTERNAL_MAN})
	# Install manpages from build tree
	set (_man_source ${CMAKE_CURRENT_BINARY_DIR}/man)
endif (EXISTS ${GMT_INSTALL_EXTERNAL_MAN})

# Install man (if available)
install (DIRECTORY ${_man_source}/
	DESTINATION ${GMT_MAN_PATH}/man1
	COMPONENT Runtime
	FILES_MATCHING PATTERN "*.1${_gz}")
install (DIRECTORY ${_man_source}/
	DESTINATION ${GMT_MAN_PATH}/man3
	COMPONENT Runtime
	FILES_MATCHING PATTERN "*.3${_gz}")
install (DIRECTORY ${_man_source}/
	DESTINATION ${GMT_MAN_PATH}/man5
	COMPONENT Runtime
	FILES_MATCHING PATTERN "*.5${_gz}")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
