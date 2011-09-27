# 
#	$Id$
#
#	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
#	See LICENSE.TXT file for copying and redistribution conditions.
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; version 2 or any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
#
# src/misc/CMakeLists.txt
#

# source files
set (GMT_MISC_PROGS_SRCS dimfilter.c gmtdigitize.c)

string (REPLACE ".c" "" GMT_MISC_PROGS "${GMT_MISC_PROGS_SRCS}")

include_directories (${CMAKE_CURRENT_BINARY_DIR})

foreach (prog ${GMT_MISC_PROGS})
	add_executable (${prog} ${prog}.c)
	target_link_libraries (${prog} gmtlib)
endforeach (prog)

# generate Mac debugging symbols
if (DSYMUTIL AND DEBUG_BUILD)
	foreach (target ${GMT_MISC_PROGS})
		list (APPEND DSYM_FILES $<TARGET_FILE:${target}>)
		add_custom_command (TARGET ${target}
			POST_BUILD
			COMMAND ${DSYMUTIL} $<TARGET_FILE:${target}>
			VERBATIM
		)
	endforeach (target)
endif (DSYMUTIL AND DEBUG_BUILD)

# add the install targets
install (TARGETS ${GMT_MISC_PROGS}
	RUNTIME DESTINATION bin
)

if (CMAKE_COMPILER_IS_GNUCC)
	# generate manpages
	string (REPLACE ".c" ".txt" GMT_MISC_MAN_DEPS "${GMT_MISC_PROGS_SRCS}")
	string (REPLACE ".c" ".1" GMT_MISC_MAN_1 "${GMT_MISC_PROGS_SRCS}")
	foreach (_manpage ${GMT_MISC_MAN_1})
		string (REGEX REPLACE "\\.[135]" ".txt" _man_src ${_manpage})
		add_custom_command (
			OUTPUT ${_manpage}
			COMMAND ${CMAKE_C_COMPILER} - -E -w -P -nostdinc
			-I${GMT_SOURCE_DIR}/src
			-I${GMT_BINARY_DIR}/src
			-I${CMAKE_CURRENT_SOURCE_DIR}
			< ${CMAKE_CURRENT_SOURCE_DIR}/${_man_src}
			> ${_manpage}
			DEPENDS gmt_man
			COMMENT "Generate ${_manpage}"
			VERBATIM
		)
	endforeach (_manpage)

	# manpage target
	add_custom_target (gmt_misc_man ALL DEPENDS ${GMT_MISC_MAN_1})

	# install manpages
	foreach (_manfile ${GMT_MISC_MAN_1})
		list (APPEND GMT_MISC_MAN_1_FILES "${CMAKE_CURRENT_BINARY_DIR}/${_manfile}")
	endforeach (_manfile)
	install (FILES ${GMT_MISC_MAN_1_FILES} DESTINATION ${GMT_SHARE_PATH}/man/man1)
endif (CMAKE_COMPILER_IS_GNUCC)
