#
# $Id$
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
#

# source files
if (UNIX)
	set (GMT_MISC_PROGS_SRCS dimfilter.c gmtdigitize.c)
elseif (WIN32)
	set (GMT_MISC_PROGS_SRCS dimfilter.c)
endif (UNIX)

# example files
set (GMT_MISC_EXAMPLE_FILES README.misc)
set (GMT_MISC_EXAMPLE_PROGS dim.template.sh)

# create build targets
string (REPLACE ".c" "" GMT_MISC_PROGS "${GMT_MISC_PROGS_SRCS}")
foreach (prog ${GMT_MISC_PROGS})
	add_executable (${prog} ${prog}.c)
	target_link_libraries (${prog} gmtlib)
endforeach (prog)

# generate Mac debugging symbols
create_debug_sym (${GMT_MISC_PROGS})

# add the install targets
install (TARGETS ${GMT_MISC_PROGS}
	RUNTIME DESTINATION bin
)

# add to gmt_suppl target
add_depend_to_target (gmt_suppl ${GMT_MISC_PROGS})

# Generate manpages
string (REPLACE ".c" ".1" GMT_MISC_MAN_1 "${GMT_MISC_PROGS_SRCS}")
gmt_create_manpages ("${GMT_MISC_MAN_1}")

# install example data
if (UNIX)
	install (FILES ${GMT_MISC_EXAMPLE_FILES}
		DESTINATION
		${GMT_SHARE_PATH}/doc/gmt-${GMT_PACKAGE_VERSION}/supplements/misc)
	install (PROGRAMS ${GMT_MISC_EXAMPLE_PROGS}
		DESTINATION
		${GMT_SHARE_PATH}/doc/gmt-${GMT_PACKAGE_VERSION}/supplements/misc)
endif (UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
