#
# $Id$
#
# Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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

# source files
set (SUPPL_LIB_SRCS mgd77.h mgd77_IGF_coeffs.h mgd77_codes.h mgd77_init.h
	mgd77_recalc.h mgd77magref.h mgd77defaults.h mgd77_functions.h
	cm4_functions.h mgd77.c mgd77_functions.c cm4_functions.c)
set (SUPPL_PROGS_SRCS mgd77convert.c mgd77info.c mgd77list.c
	mgd77magref.c mgd77manage.c mgd77path.c mgd77sniffer.c mgd77track.c)

# lib sources
set (LIB_SRCS ${SUPPL_LIB_SRCS} ${SUPPL_PROGS_SRCS})

# create build targets
string (REPLACE ".c" "" SUPPL_PROGS "${SUPPL_PROGS_SRCS}")
foreach (prog ${SUPPL_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
		PROPERTIES COMPILE_DEFINITIONS "MODULE_ID=k_mod_${prog}")
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
string (REPLACE ".c" ".1" SUPPL_MAN_1 "${SUPPL_PROGS_SRCS}")
gmt_create_manpages ("${SUPPL_MAN_1}")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
