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

# usefull macros
include (GMTmacros)

# Check for dsymutil only on Mac
if (APPLE)
	find_program(DSYMUTIL dsymutil)
endif (APPLE)

# Macro for generating Mac debugging symbols
macro (CREATE_DEBUG_SYM _TARGETS)
	if (DSYMUTIL AND DEBUG_BUILD AND "${CMAKE_GENERATOR}" MATCHES "Make")

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

	endif (DSYMUTIL AND DEBUG_BUILD AND "${CMAKE_GENERATOR}" MATCHES "Make")
endmacro (CREATE_DEBUG_SYM _TARGETS)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
