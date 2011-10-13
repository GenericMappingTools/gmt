#
# $Id$
#
# - Generates Mac .dSYM bundle
# CREATE_DEBUG_SYM ( TARGETS )
#
#  TARGETS - list of targets
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

set (_apple_debug_build)
if (APPLE)
	string(TOLOWER ${CMAKE_BUILD_TYPE} _build_type)
	if (DEBUG_BUILD OR _build_type STREQUAL "relwithdebinfo")
		set (_apple_debug_build TRUE)
	endif (DEBUG_BUILD OR _build_type STREQUAL "relwithdebinfo")
endif (APPLE)

if (_apple_debug_build)

	# usefull macros
	include (GmtHelperMacros)

	# Check for dsymutil only on Mac
	find_program(DSYMUTIL dsymutil)

	# Macro for generating Mac debugging symbols
	macro (CREATE_DEBUG_SYM _TARGETS)
		if (DSYMUTIL AND "${CMAKE_GENERATOR}" MATCHES "Make")

			# generator
			foreach (target ${ARGV}) # instead of _TARGETS we use ARGV to get all args
				add_custom_command (TARGET ${target}
					POST_BUILD
					COMMAND ${DSYMUTIL} $<TARGET_FILE:${target}>
					COMMENT "Generating .dSYM bundle for ${target}"
					VERBATIM
					)
			endforeach (target)

			# create tag from current dirname
			tag_from_current_source_dir (_tag "_")

			# clean target
			add_custom_target (dsym_clean${_tag}
				COMMAND ${RM} -rf *.dSYM
				COMMENT "Removing .dSYM bundles")

			# register with spotless target
			add_depend_to_spotless (dsym_clean${_tag})

		endif (DSYMUTIL AND "${CMAKE_GENERATOR}" MATCHES "Make")
	endmacro (CREATE_DEBUG_SYM _TARGETS)

else (_apple_debug_build)
	macro (CREATE_DEBUG_SYM _TARGETS)
		# do nothing
	endmacro (CREATE_DEBUG_SYM _TARGETS)
endif (_apple_debug_build)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
