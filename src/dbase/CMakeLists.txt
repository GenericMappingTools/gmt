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
set (GMT_DBASE_PROGS_SRCS grdraster.c)

# generate header file gmt_dbase.h
gmt_set_api_header(GMT_DBASE_API_H "${GMT_DBASE_PROGS_SRCS}")

# make lib
add_library (gmt_dbase ${GMT_DBASE_PROGS_SRCS} ${GMT_DBASE_API_H})
target_link_libraries (gmt_dbase gmtlib)

# set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (gmt_dbase
	PROPERTIES
	VERSION "${GMT_PACKAGE_VERSION}"
	SOVERSION ${GMT_PACKAGE_VERSION_MAJOR}
	INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
	DEFINE_SYMBOL "LIBRARY_EXPORTS"
	)

# create build targets
string (REPLACE ".c" "" GMT_DBASE_PROGS "${GMT_DBASE_PROGS_SRCS}")
foreach (prog ${GMT_DBASE_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
		PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMT;FUNC=GMT_${prog}")
	target_link_libraries (${prog} gmt_dbase)
endforeach (prog)

# generate Mac debugging symbols
create_debug_sym (${GMT_DBASE_PROGS})

# add the install targets
install (TARGETS gmt_dbase ${GMT_DBASE_PROGS}
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)

# add to gmt_suppl target
add_depend_to_target (gmt_suppl gmt_dbase ${GMT_DBASE_PROGS})

# Generate manpages
string (REPLACE ".c" ".1" GMT_DBASE_MAN_1 "${GMT_DBASE_PROGS_SRCS}")
gmt_create_manpages ("${GMT_DBASE_MAN_1}")

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
