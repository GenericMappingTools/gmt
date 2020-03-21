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

# reST documentation
find_package (Sphinx)

if (SPHINX_FOUND)
	# Create configuration file
	configure_file (conf.py.in conf.py @ONLY)

	# path for the cached environment and doctree files
	set (SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")

	# Copy reST sources to build tree
	include (CopyDirIfDifferent)
	foreach (subdir source themes _static _templates _extensions)
		copy_dir_if_different (_docs_copy_rst_tree_${subdir}
			${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/
			${CMAKE_CURRENT_BINARY_DIR}/${subdir})
		add_depend_to_target (docs_depends _docs_copy_rst_tree_${subdir})
	endforeach ()
	add_custom_target (_docs_rst_mkdir_verbatim
		COMMAND ${CMAKE_COMMAND} -E make_directory
		${CMAKE_CURRENT_BINARY_DIR}/source/_verbatim
		DEPENDS _docs_copy_rst_tree_source)

	add_custom_target (_docs_rst_mkdir_images
		COMMAND ${CMAKE_COMMAND} -E make_directory
		${CMAKE_CURRENT_BINARY_DIR}/source/_images
		DEPENDS _docs_copy_rst_tree_source)

	# clean target
	add_custom_target (_rst_clean
		# cmake<3.15 can't remove multiple directories using cmake -E remove_directory
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/_doctrees
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/_extensions
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/_static
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/_templates
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/source
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/themes
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/html
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/man
		COMMENT "Removing rst tree")
	# register with spotless target
	add_depend_to_target (spotless _rst_clean)

	# HTML build target
	add_custom_target (docs_html
		COMMAND ${SPHINX_EXECUTABLE} -q -b html
		-c "${CMAKE_CURRENT_BINARY_DIR}"
		-d "${SPHINX_CACHE_DIR}"
		-w "${CMAKE_CURRENT_BINARY_DIR}/html.log"
		${CMAKE_CURRENT_BINARY_DIR}/source
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
			${CMAKE_CURRENT_BINARY_DIR}/source
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

# Install HTML documentation
if (EXISTS ${GMT_INSTALL_EXTERNAL_DOC})
	# Install documentation from external location
	set (_doc_source ${GMT_INSTALL_EXTERNAL_DOC})
	set (_doc_dest ${GMT_DOCDIR})
else (EXISTS ${GMT_INSTALL_EXTERNAL_DOC})
	# Install html from build tree
	set (_doc_source ${CMAKE_CURRENT_BINARY_DIR}/html)
	set (_doc_dest ${GMT_DOCDIR}/html)
endif (EXISTS ${GMT_INSTALL_EXTERNAL_DOC})

install (DIRECTORY ${_doc_source}/
	DESTINATION ${_doc_dest}
	COMPONENT Documentation
	USE_SOURCE_PERMISSIONS
	OPTIONAL)

# Install man (if available)
if (EXISTS ${GMT_INSTALL_EXTERNAL_MAN})
	# Install manpages from external location
	set (_man_source ${GMT_INSTALL_EXTERNAL_MAN})
else (EXISTS ${GMT_INSTALL_EXTERNAL_MAN})
	# Install manpages from build tree
	set (_man_source ${CMAKE_CURRENT_BINARY_DIR}/man)
endif (EXISTS ${GMT_INSTALL_EXTERNAL_MAN})

install (DIRECTORY ${_man_source}/
	DESTINATION ${GMT_MANDIR}/man1
	COMPONENT Runtime
	FILES_MATCHING PATTERN "*.1.gz")
