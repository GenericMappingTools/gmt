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
set (GMT_POTENTIAL_PROGS_SRCS redpol.c xyzokb.c)
set (GMT_POTENTIAL_PROGS2_SRCS gravfft.c)

# generate header file gmt_potential.h
gmt_set_api_header(GMT_POTENTIAL_API_H "${GMT_POTENTIAL_PROGS_SRCS}")

# lib sources
set (LIB_SRCS ${GMT_POTENTIAL_PROGS_SRCS} ${GMT_POTENTIAL_API_H})

# create build targets
string (REPLACE ".c" "" GMT_POTENTIAL_PROGS "${GMT_POTENTIAL_PROGS_SRCS}")
string (REPLACE ".c" "" GMT_POTENTIAL_PROGS2 "${GMT_POTENTIAL_PROGS2_SRCS}")
foreach (prog ${GMT_POTENTIAL_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
		PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMT;FUNC=GMT_${prog}")
	target_link_libraries (${prog} gmtlib)
endforeach (prog)

# generate Mac debugging symbols
create_debug_sym (${GMT_POTENTIAL_PROGS})

# add the install targets
install (TARGETS ${GMT_POTENTIAL_PROGS}
	RUNTIME DESTINATION bin)

# add to gmt_suppl target
add_depend_to_target (gmt_suppl ${GMT_POTENTIAL_PROGS})

# Generate manpages
string (REPLACE ".c" ".1" GMT_POTENTIAL_MAN_1 "${GMT_POTENTIAL_PROGS_SRCS}")
gmt_create_manpages ("${GMT_POTENTIAL_MAN_1}")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
