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
# src/gshhs/CMakeLists.txt
#

# source files
set (GMT_GSHHS_PROGS_SRCS gshhs.c)
set (GMT_GSHHS_PROGS2_SRCS gshhs_dp.c gshhstograss.c)

# generate header file gmt_gshhs.h
gmt_set_api_header(GMT_GSHHS_API_H "${GMT_GSHHS_PROGS_SRCS}")

# libgshhs
add_library (gmt_gshhs ${GMT_GSHHS_PROGS_SRCS} ${GMT_GSHHS_API_H})
target_link_libraries (gmt_gshhs gmtlib)

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (gmt_gshhs
	PROPERTIES
	VERSION "${GMT_PACKAGE_VERSION}"
	SOVERSION ${GMT_PACKAGE_VERSION_MAJOR}
	INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
)

# create build targets for gshhs progs
string (REPLACE ".c" "" GMT_GSHHS_PROGS "${GMT_GSHHS_PROGS_SRCS}")
foreach (prog ${GMT_GSHHS_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
		PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMT;FUNC=GMT_${prog}")
	target_link_libraries (${prog} gmt_gshhs)
endforeach (prog)

string (REPLACE ".c" "" GMT_GSHHS_PROGS2 "${GMT_GSHHS_PROGS2_SRCS}")
foreach (_prog ${GMT_GSHHS_PROGS2})
	add_executable (${_prog} ${_prog}.c)
	target_link_libraries (${_prog} gmtlib)
endforeach (_prog)

# generate Mac debugging symbols
create_debug_sym (gmt_gshhs ${GMT_GSHHS_PROGS} ${GMT_GSHHS_PROGS2})

# add the install targets
install (TARGETS gmt_gshhs ${GMT_GSHHS_PROGS} ${GMT_GSHHS_PROGS2}
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)

# Generate manpages
string (REPLACE ".c" ".1" GMT_GSHHS_MAN_1
	"${GMT_GSHHS_PROGS_SRCS};${GMT_GSHHS_PROGS2_SRCS}")
gmt_create_manpages ("${GMT_GSHHS_MAN_1}" headers)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
