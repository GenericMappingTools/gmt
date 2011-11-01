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
set (GMT_SPOTTER_LIB_SRCS spotter.h libspotter.c)
set (GMT_SPOTTER_PROGS_SRCS backtracker.c grdpmodeler.c grdrotater.c
	grdspotter.c hotspotter.c originator.c rotconverter.c)

# example files
set (GMT_SPOTTER_EXAMPLE_FILES README.spotter
	DC85.d WK97.d euler.txt pac_hs.d seamounts.d)
set (GMT_SPOTTER_EXAMPLE_PROGS spotter.bat spotter.sh)

# generate header file gmt_spotter.h
gmt_set_api_header(GMT_SPOTTER_API_H ${GMT_SPOTTER_PROGS_SRCS})

# lib sources
set (LIB_SRCS ${GMT_SPOTTER_LIB_SRCS}
	${GMT_SPOTTER_PROGS_SRCS} ${GMT_SPOTTER_API_H})

# create build targets
string (REPLACE ".c" "" GMT_SPOTTER_PROGS "${GMT_SPOTTER_PROGS_SRCS}")
foreach (prog ${GMT_SPOTTER_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
	PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMTPSL;FUNC=GMT_${prog}")
	target_link_libraries (${prog} gmtlib)
endforeach (prog)

# generate Mac debugging symbols
create_debug_sym (${GMT_SPOTTER_PROGS})

# add the install targets
install (TARGETS ${GMT_SPOTTER_PROGS}
	RUNTIME DESTINATION bin)

# add to gmt_suppl target
add_depend_to_target (gmt_suppl ${GMT_SPOTTER_PROGS})

# Generate manpages
string (REPLACE ".c" ".1" GMT_SPOTTER_MAN_1 "${GMT_SPOTTER_PROGS_SRCS}")
gmt_create_manpages ("${GMT_SPOTTER_MAN_1}")

# install example data
if (UNIX)
	install (FILES ${GMT_SPOTTER_EXAMPLE_FILES}
		DESTINATION
		${GMT_DOC_PATH}/supplements/spotter)
	install (PROGRAMS ${GMT_SPOTTER_EXAMPLE_PROGS}
		DESTINATION
		${GMT_DOC_PATH}/supplements/spotter)
endif (UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
