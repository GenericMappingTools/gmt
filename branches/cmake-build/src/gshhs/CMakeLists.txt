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
set (GMT_GSHHS_PROGS_SRCS gshhs.c)
set (GMT_GSHHS_PROGS2_SRCS gshhs_dp.c gshhstograss.c)

# generate header file gmt_gshhs.h
gmt_set_api_header(GMT_GSHHS_API_H ${GMT_GSHHS_PROGS_SRCS})

# lib sources
set (LIB_SRCS ${GMT_GSHHS_PROGS_SRCS} ${GMT_GSHHS_API_H})

# create build targets
string (REPLACE ".c" "" GMT_GSHHS_PROGS "${GMT_GSHHS_PROGS_SRCS}")
foreach (prog ${GMT_GSHHS_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
		PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMT;FUNC=GMT_${prog}")
	target_link_libraries (${prog} gmtlib)
endforeach (prog)

string (REPLACE ".c" "" GMT_GSHHS_PROGS2 "${GMT_GSHHS_PROGS2_SRCS}")
foreach (_prog ${GMT_GSHHS_PROGS2})
	add_executable (${_prog} ${_prog}.c)
	target_link_libraries (${_prog} gmtlib)
endforeach (_prog)

# generate Mac debugging symbols
create_debug_sym (${GMT_GSHHS_PROGS} ${GMT_GSHHS_PROGS2})

# add the install targets
install (TARGETS ${GMT_GSHHS_PROGS} ${GMT_GSHHS_PROGS2}
	RUNTIME DESTINATION bin)

# add to gmt_suppl target
add_depend_to_target (gmt_suppl
	${GMT_GSHHS_PROGS}
	${GMT_GSHHS_PROGS2})

# Generate manpages
string (REPLACE ".c" ".1" GMT_GSHHS_MAN_1
	"${GMT_GSHHS_PROGS_SRCS};${GMT_GSHHS_PROGS2_SRCS}")
gmt_create_manpages ("${GMT_GSHHS_MAN_1}")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
