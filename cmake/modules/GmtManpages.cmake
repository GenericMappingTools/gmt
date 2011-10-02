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

# usefull macros
include (GmtHelperMacros)

macro (GMT_CREATE_MANPAGES _MAN_FILES)
	if (CMAKE_COMPILER_IS_GNUCC)
    # create tag from current dirname
		tag_from_current_source_dir (_tag "_")

		if(GZIP)
			set (_gz ".gz")
		endif(GZIP)

		# clear lists
		set (_install_sections)
		set (_target_depends)

		foreach (_manfile ${_MAN_FILES})
			# strip section number
			string (REGEX MATCH "[1-8]$" _man_section ${_manfile})
			if (NOT _man_section)
				# only manfile.[1-8] supported
				message (FATAL_ERROR "cannot generate ${_man_unsupported}")
			endif (NOT _man_section)
			list (APPEND _install_sections ${_man_section})

      # generator
			string (REGEX REPLACE "\\.[1-8]$" ".txt" _man_src ${_manfile})
			if(GZIP)
				add_custom_command (
					OUTPUT "${_manfile}${_gz}"
					COMMAND ${CMAKE_C_COMPILER} - -E -w -P -nostdinc -traditional-cpp
					-I${GMT_SOURCE_DIR}/src
					-I${GMT_BINARY_DIR}/src
					-I${CMAKE_CURRENT_SOURCE_DIR}
					< ${CMAKE_CURRENT_SOURCE_DIR}/${_man_src}
					> ${_manfile}
					COMMAND ${GZIP} -9 -f ${_manfile}
					DEPENDS ${_man_src} ${ARGN} # ARGN: list of arguments past the last expected argument
					COMMENT "Generate ${_manfile}"
					VERBATIM
					)
			else(GZIP)
				add_custom_command (
					OUTPUT ${_manfile}
					COMMAND ${CMAKE_C_COMPILER} - -E -w -P -nostdinc -traditional-cpp
					-I${GMT_SOURCE_DIR}/src
					-I${GMT_BINARY_DIR}/src
					-I${CMAKE_CURRENT_SOURCE_DIR}
					< ${CMAKE_CURRENT_SOURCE_DIR}/${_man_src}
					> ${_manfile}
					DEPENDS ${_man_src} ${ARGN} # ARGN: list of arguments past the last expected argument
					COMMENT "Generate ${_manfile}"
					VERBATIM
					)
			endif(GZIP)

			# append full path
			list (APPEND _manfilepaths_${_man_section}
				"${CMAKE_CURRENT_BINARY_DIR}/${_manfile}${_gz}")
			list (APPEND _target_depends "${_manfile}${_gz}")
		endforeach (_manfile)

		# manpage target
		add_custom_target (manpages${_tag} ALL DEPENDS ${_target_depends})
		if(TARGET manpages)
			add_dependencies(manpages manpages${_tag})
		endif(TARGET manpages)

		# install manpages
		list (SORT _install_sections)
		list (REMOVE_DUPLICATES _install_sections)
		foreach (_man_section ${_install_sections})
			install (FILES ${_manfilepaths_${_man_section}}
				DESTINATION ${GMT_SHARE_PATH}/man/man${_man_section})
		endforeach (_man_section ${_install_sections})
	else (CMAKE_COMPILER_IS_GNUCC)
		message(WARNING
			"Not creating manpages in ${CMAKE_CURRENT_SOURCE_DIR}")
	endif (CMAKE_COMPILER_IS_GNUCC)
endmacro (GMT_CREATE_MANPAGES _MAN_FILES)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
