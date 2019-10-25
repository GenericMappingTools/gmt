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

# reST documentation
find_package (Sphinx)

# path for the cached environment and doctree files
set (SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

if (SPHINX_FOUND)
	# Create configuration file
	configure_file (conf.py.in conf.py @ONLY)

	# Copy reST sources to build tree
	include (CopyDirIfDifferent)
	copy_dir_if_different (_docs_copy_rst_tree_source
		${CMAKE_CURRENT_SOURCE_DIR}/source/
		${CMAKE_CURRENT_BINARY_DIR}/_source)
	add_depend_to_target (docs_depends _docs_copy_rst_tree_source)
	copy_dir_if_different (_docs_copy_rst_tree_themes
		${CMAKE_CURRENT_SOURCE_DIR}/themes/
		${CMAKE_CURRENT_BINARY_DIR}/_themes)
	add_depend_to_target (docs_depends _docs_copy_rst_tree_themes)
	copy_dir_if_different (_docs_copy_rst_tree_static
		${CMAKE_CURRENT_SOURCE_DIR}/_static/
		${CMAKE_CURRENT_BINARY_DIR}/_static)
	add_depend_to_target (docs_depends _docs_copy_rst_tree_static)
	copy_dir_if_different (_docs_copy_rst_tree_templates
		${CMAKE_CURRENT_SOURCE_DIR}/_templates/
		${CMAKE_CURRENT_BINARY_DIR}/_templates)
	add_depend_to_target (docs_depends _docs_copy_rst_tree_templates)
	copy_dir_if_different (_docs_copy_rst_tree_extensions
		${CMAKE_CURRENT_SOURCE_DIR}/_extensions/
		${CMAKE_CURRENT_BINARY_DIR}/_extensions)
	add_depend_to_target (docs_depends _docs_copy_rst_tree_extensions)
	add_custom_target (_docs_rst_mkdir_verbatim
		COMMAND ${CMAKE_COMMAND} -E make_directory
		${CMAKE_CURRENT_BINARY_DIR}/_source/_verbatim
		DEPENDS _docs_copy_rst_tree_source)

	# clean target
	add_custom_target (_rst_clean
		COMMAND ${CMAKE_COMMAND} -E remove_directory
		${CMAKE_CURRENT_BINARY_DIR}/_doctrees
		COMMAND ${CMAKE_COMMAND} -E remove_directory
		${CMAKE_CURRENT_BINARY_DIR}/_source
		COMMAND ${CMAKE_COMMAND} -E remove_directory
		${CMAKE_CURRENT_BINARY_DIR}/_themes
		COMMAND ${CMAKE_COMMAND} -E remove_directory
		${CMAKE_CURRENT_BINARY_DIR}/html
		COMMAND ${CMAKE_COMMAND} -E remove_directory
		${CMAKE_CURRENT_BINARY_DIR}/man
		COMMENT "Removing rst tree")
	# register with spotless target
	add_depend_to_target (spotless _rst_clean)

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

	# Build and compress manpages
	if (GZIP)
		add_custom_target (docs_man
			COMMAND ${SPHINX_EXECUTABLE} -q -b man
			-c "${CMAKE_CURRENT_BINARY_DIR}"
			-d "${SPHINX_CACHE_DIR}"
			-w "${CMAKE_CURRENT_BINARY_DIR}/man.log"
			${CMAKE_CURRENT_BINARY_DIR}/_source
			${CMAKE_CURRENT_BINARY_DIR}/man
			COMMAND ${GZIP} -9 -n -f ${CMAKE_CURRENT_BINARY_DIR}/man/*.?
			COMMENT "Building manpages...")
		add_dependencies (docs_man docs_depends)
	endif (GZIP)

	# Install targets for release documentation
	if (GIT_FOUND AND HAVE_GIT_VERSION)
		# HTML
		add_custom_target (_html_release
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			${CMAKE_CURRENT_BINARY_DIR}/html/
			${GMT_RELEASE_PREFIX}/doc_release/html
			DEPENDS docs_html git_export_release)
		add_depend_to_target (gmt_release _html_release)
		# Manpages
		add_custom_target (_man_release
			COMMAND ${CMAKE_COMMAND} -E copy_directory
			${CMAKE_CURRENT_BINARY_DIR}/man/
			${GMT_RELEASE_PREFIX}/man_release
			DEPENDS docs_man git_export_release)
		add_depend_to_target (gmt_release _man_release)
	endif (GIT_FOUND AND HAVE_GIT_VERSION)
endif (SPHINX_FOUND)

# Install targets
if (NOT EXISTS ${GMT_INSTALL_EXTERNAL_DOC})
	# Install html (if available)
	install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
		DESTINATION ${GMT_DOCDIR}/html
		COMPONENT Documentation
		OPTIONAL)
endif (NOT EXISTS ${GMT_INSTALL_EXTERNAL_DOC})

if (EXISTS ${GMT_INSTALL_EXTERNAL_MAN})
	# Install manpages from external location
	set (_man_source ${GMT_INSTALL_EXTERNAL_MAN})
else (EXISTS ${GMT_INSTALL_EXTERNAL_MAN})
	# Install manpages from build tree
	set (_man_source ${CMAKE_CURRENT_BINARY_DIR}/man)
endif (EXISTS ${GMT_INSTALL_EXTERNAL_MAN})

# Install man (if available)
install (DIRECTORY ${_man_source}/
	DESTINATION ${GMT_MANDIR}/man1
	COMPONENT Runtime
	FILES_MATCHING PATTERN "*.1.gz")
