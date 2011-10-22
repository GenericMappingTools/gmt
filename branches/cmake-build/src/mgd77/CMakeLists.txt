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
set (GMT_MGD77_LIB_SRCS mgd77.h mgd77_IGF_coeffs.h mgd77_init.h mgd77_recalc.h
	mgd77magref.h mgd77defaults.h mgd77_functions.h cm4_functions.h
	mgd77.c cm4_functions.c)
set (GMT_MGD77_PROGS_SRCS mgd77convert.o mgd77info.o mgd77list.o
	mgd77path.o mgd77manage.o mgd77magref.o mgd77sniffer.o)
set (GMT_MGD77_PROGSPS_SRCS mgd77track.c)

# generate header file gmt_mgd77.h
gmt_set_api_header(GMT_MGD77_API_H "${GMT_MGD77_PROGS_SRCS}")

# lib sources
set (LIB_SRCS
	${GMT_MGD77_LIB_SRCS}
	${GMT_MGD77_PROGS_SRCS}
	${GMT_MGD77_PROGSPS_SRCS}
	${GMT_MGD77_API_H})

# create build targets
string (REPLACE ".c" "" GMT_MGD77_PROGS "${GMT_MGD77_PROGS_SRCS}")
string (REPLACE ".c" "" GMT_MGD77_PROGSPS "${GMT_MGD77_PROGSPS_SRCS}")
foreach (prog ${GMT_MGD77_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
	PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMT;FUNC=GMT_${prog}")
	target_link_libraries (${prog} gmtlib)
endforeach (prog)
foreach (prog ${GMT_MGD77_PROGSPS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
	PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMTPSL;FUNC=GMT_${prog}")
	target_link_libraries (${prog} gmtlib)
endforeach (prog)

# generate Mac debugging symbols
create_debug_sym (${GMT_MGD77_PROGS})

# add the install targets
install (TARGETS ${GMT_MGD77_PROGS}
	RUNTIME DESTINATION bin)

# add to gmt_suppl target
add_depend_to_target (gmt_suppl ${GMT_MGD77_PROGS})

# Generate manpages
string (REPLACE ".c" ".1" GMT_MGD77_MAN_1 "${GMT_MGD77_PROGS_SRCS}")
gmt_create_manpages ("${GMT_MGD77_MAN_1}")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
