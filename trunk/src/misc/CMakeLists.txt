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
#

# misc is GPL only
if (LICENSE_RESTRICTED AND NOT LICENSE_RESTRICTED STREQUAL GPL)
	return()
endif (LICENSE_RESTRICTED AND NOT LICENSE_RESTRICTED STREQUAL GPL)

# source files
set (SUPPL_PROGS_SRCS dimfilter.c)

# lib sources
set (LIB_SRCS ${SUPPL_PROGS_SRCS})

# example files
set (SUPPL_EXAMPLE_FILES README.misc)
set (SUPPL_EXAMPLE_PROGS dim.template.sh)

# create build targets
string (REPLACE ".c" "" SUPPL_PROGS "${SUPPL_PROGS_SRCS}")
foreach (prog ${SUPPL_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	string (TOUPPER "${prog}" UCASEname)
	set_target_properties (${prog}
		PROPERTIES COMPILE_DEFINITIONS "MODULE_ID=GMT_ID_${UCASEname}")
	if (WIN32)
		set_target_properties(${prog} PROPERTIES LINK_FLAGS " /MANIFEST:NO")
	endif ()
	target_link_libraries (${prog} gmtlib)
endforeach (prog)

# generate Mac debugging symbols
create_debug_sym (${SUPPL_PROGS})

# add the install targets
install (TARGETS ${SUPPL_PROGS}
	RUNTIME DESTINATION ${GMT_BINDIR}
	COMPONENT Runtime)

# add to gmt_suppl target
add_depend_to_target (gmt_suppl ${SUPPL_PROGS})

# Generate manpages
string (REPLACE ".c" ".1" SUPPL_MAN_1
	"${SUPPL_PROGS_SRCS}")
gmt_create_manpages ("${SUPPL_MAN_1}")

# install example data
if (UNIX)
	install (FILES ${SUPPL_EXAMPLE_FILES}
		DESTINATION ${GMT_DOC_PATH}/supplements/misc
		COMPONENT Documentation)
	install (PROGRAMS ${SUPPL_EXAMPLE_PROGS}
		DESTINATION ${GMT_DOC_PATH}/supplements/misc
		COMPONENT Documentation)
endif (UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
