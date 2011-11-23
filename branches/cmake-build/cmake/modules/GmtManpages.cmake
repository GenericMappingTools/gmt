#
# $Id$
#
# - Generates manpages from txt-files and creates installation targets
# GMT_CREATE_MANPAGES ("${MAN_FILES}" [DEPENDS [DEPENDS2] ...]])
#
#  MAN_FILES - list of manpages, e.g., user.1 system.2 special.4 formats.5
#  DEPENDS   - list of dependencies
#
# Typical use:
#  set (MAN_FILES user.1 system.2 special.4 formats.5)
#  GMT_CREATE_MANPAGES ("${MAN_FILES}")
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

if(NOT DEFINED _GMT_MANPAGES_CMAKE_)
	set(_GMT_MANPAGES_CMAKE_ "DEFINED")

	# usefull macros
	include (GmtHelperMacros)

	# Find groff front-end
	find_program (GROFF groff)

	# Find ps2pdf
	#find_package (LATEX)

	macro (GMT_CREATE_MANPAGES MAN_FILES)
		if (HAVE_TRADITIONAL_CPP)
			# parse arguments
			set (_arg_is_man_file TRUE)
			set (_man_files ${MAN_FILES})
			set (_depends)
			foreach (_arg ${ARGN})
				if (_arg_is_man_file AND _arg STREQUAL "DEPENDS")
					set (_arg_is_man_file FALSE)
				endif (_arg_is_man_file AND _arg STREQUAL "DEPENDS")
				if (_arg_is_man_file)
					list (APPEND _man_files ${_arg})
				else (_arg_is_man_file)
					list (APPEND _depends ${_arg})
				endif (_arg_is_man_file)
			endforeach (_arg ${ARGN})

			# create tag from current dirname
			tag_from_current_source_dir (_tag "_")

			# clear lists
			set (_install_sections)
			set (_target_depends)
			set (_manfiles_html)

			# set preprocessor flags
			if (MSVC)
				set (_cpp_flags /EP /nologo)
			else (MSVC)
				# GNU C
				set (_cpp_flags -E -w -P -nostdinc -traditional-cpp - <)
			endif (MSVC)

			foreach (_manfile ${_man_files})
				# strip section number
				string (REGEX MATCH "[1-8]$" _man_section ${_manfile})
				if (NOT _man_section)
					# only manfile.[1-8] supported
					message (FATAL_ERROR "cannot generate ${_man_unsupported}")
				endif (NOT _man_section)
				list (APPEND _install_sections ${_man_section})

				# generator
				string (REGEX REPLACE "\\.[1-8]$" ".txt" _man_src ${_manfile})

				# uncompressed manpage
				add_custom_command (
					OUTPUT ${_manfile}
					COMMAND ${CMAKE_C_COMPILER}
					-I${GMT_SOURCE_DIR}/src
					-I${GMT_BINARY_DIR}/src
					-I${CMAKE_CURRENT_SOURCE_DIR}
					${_cpp_flags}
					${CMAKE_CURRENT_SOURCE_DIR}/${_man_src}
					> ${_manfile}
					DEPENDS ${_man_src} ${_depends}
					VERBATIM)

				# compress manpage
				if(GZIP)
					add_custom_command (
						OUTPUT ${_manfile}.gz
						COMMAND ${GZIP} -9 -c ${_manfile} > ${_manfile}.gz
						DEPENDS ${_manfile}
						VERBATIM)
					set (_gz ".gz")
				endif(GZIP)

				# append full path + make list for each man section
				list (APPEND _manfilepaths_${_man_section}
					"${CMAKE_CURRENT_BINARY_DIR}/${_manfile}${_gz}")
				list (APPEND _target_depends "${_manfile}${_gz}")

				if (GROFF)
					# ps manpages
					add_custom_command (
						OUTPUT ${_manfile}.ps
						COMMAND ${GROFF} -mandoc -T ps
						${_manfile} > ${_manfile}.ps
						DEPENDS ${_manfile}
						VERBATIM)

					# append to list of ps manfiles
					if (_tag)
						# supplement manpage
						list (APPEND _manfiles_suppl_ps
							"${CMAKE_CURRENT_BINARY_DIR}/${_manfile}.ps")
					else (_tag)
						# gmt manpage
						list (APPEND _manfiles_ps
							"${CMAKE_CURRENT_BINARY_DIR}/${_manfile}.ps")
					endif (_tag)
					list (APPEND _target_depends ${_manfile}.ps)
				endif (GROFF)

				# html manpages
				add_custom_command (
					OUTPUT ${_manfile}.html
					COMMAND ${GMT_BINARY_DIR}/src/rman
					-S -f HTML
					< ${_manfile} > ${_manfile}.html
					DEPENDS ${_manfile} rman
					VERBATIM)

				# append to list of html manfiles
				list (APPEND _manfiles_html
					"${CMAKE_CURRENT_BINARY_DIR}/${_manfile}.html")
				list (APPEND _target_depends ${_manfile}.html)
			endforeach (_manfile)

			# manpage target
			add_custom_target (manpages${_tag} DEPENDS ${_target_depends})
			add_depend_to_target (manpages_all manpages${_tag})

			# install manpages
			list (SORT _install_sections)
			list (REMOVE_DUPLICATES _install_sections)
			foreach (_man_section ${_install_sections})
				install (FILES ${_manfilepaths_${_man_section}}
					DESTINATION ${GMT_SHARE_PATH}/man/man${_man_section}
					COMPONENT Runtime
					OPTIONAL)
			endforeach (_man_section ${_install_sections})

			# install html manpages
			install (FILES ${_manfiles_html}
				DESTINATION ${GMT_DOC_PATH}/html
				COMPONENT Documentation
				OPTIONAL)

			# remember _manfiles_ps
			if (_tag)
				set(_manfiles_suppl_ps "${_manfiles_suppl_ps}" CACHE INTERNAL
					"Global list of supplement PS manpages")
			else (_tag)
				set(_manfiles_ps "${_manfiles_ps}" CACHE INTERNAL
					"Global list of PS manpages")
			endif (_tag)

		endif (HAVE_TRADITIONAL_CPP)
	endmacro (GMT_CREATE_MANPAGES MAN_FILES)

endif(NOT DEFINED _GMT_MANPAGES_CMAKE_)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
