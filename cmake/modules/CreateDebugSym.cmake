#
#
# - Generates Mac .dSYM bundle
# CREATE_DEBUG_SYM ( DESTINATION TARGETS )
#
#  DESTINATION - destination directory for installed targets
#  TARGETS     - list of targets
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

string(TOLOWER ${CMAKE_BUILD_TYPE} _build_type)
if (_build_type MATCHES "debug|relwithdebinfo")
	set (DEBUG_BUILD TRUE)
endif ()
set (_build_type)

if (APPLE AND DEBUG_BUILD)

	# useful macros
	include (GmtHelperMacros)

	# Check for dsymutil only on Mac
	find_program(DSYMUTIL dsymutil)

	# Macro for generating Mac debugging symbols
	macro (CREATE_DEBUG_SYM DESTINATION)
		if (DSYMUTIL AND "${CMAKE_GENERATOR}" MATCHES "Make")
			# create tag from current dirname
			tag_from_current_source_dir (_tag "_")

			# generator
			foreach (target ${ARGN}) # get all args past the last expected
				add_custom_command (TARGET ${target}
					POST_BUILD
					COMMAND ${DSYMUTIL} $<TARGET_FILE:${target}>
					COMMENT "Generating .dSYM bundle for ${target}"
					VERBATIM)

				# clean target
				get_target_property (_location ${target} LOCATION)
				get_target_property (_type ${target} TYPE)
				get_target_property (_version ${target} VERSION)
				get_filename_component (_path ${_location} PATH)
				get_filename_component (_name ${_location} NAME)
				if (_type STREQUAL "SHARED_LIBRARY")
					string (REPLACE ".dylib" ".${_version}.dylib" _name "${_name}")
				endif (_type STREQUAL "SHARED_LIBRARY")
				set (_dsym_bundle "${_path}/${_name}.dSYM")
				add_custom_target (_dsym_clean_${target}
					COMMAND ${RM} -rf ${_dsym_bundle}
					COMMENT "Removing .dSYM bundle")
				add_depend_to_target (dsym_clean${_tag} _dsym_clean_${target})

				# install target
				install (DIRECTORY ${_dsym_bundle}
					DESTINATION ${DESTINATION}
					COMPONENT Debug)
			endforeach (target)

			# register with spotless target
			add_depend_to_target (spotless dsym_clean${_tag})

		endif (DSYMUTIL AND "${CMAKE_GENERATOR}" MATCHES "Make")
	endmacro (CREATE_DEBUG_SYM _TARGETS)

elseif (MSVC AND DEBUG_BUILD)
	# Macro for installing MSVC debugging symbol files
	macro (CREATE_DEBUG_SYM DESTINATION)
		# create tag from current dirname
		tag_from_current_source_dir (_tag "_")

		foreach (target ${ARGN}) # get all args past the last expected
			# clean target
			get_target_property (_location ${target} LOCATION)
			get_filename_component (_path ${_location} PATH)
			get_filename_component (_name ${_location} NAME_WE)
			set (_pdb_file "${_path}/${_name}.pdb")
			add_custom_target (_pdb_clean_${target}
				COMMAND ${CMAKE_COMMAND} remove -f ${_pdb_file}
				COMMENT "Removing .pdb file")
			add_depend_to_target (pdb_clean${_tag} _pdb_clean_${target})

			# install target
			install (FILES ${_pdb_file}
				DESTINATION ${DESTINATION}
				COMPONENT Debug)
		endforeach (target)

		# register with spotless target
		add_depend_to_target (spotless pdb_clean${_tag})
	endmacro (CREATE_DEBUG_SYM _TARGETS)

else (APPLE AND DEBUG_BUILD)
	macro (CREATE_DEBUG_SYM _TARGETS)
		# do nothing
	endmacro (CREATE_DEBUG_SYM _TARGETS)
endif (APPLE AND DEBUG_BUILD)
