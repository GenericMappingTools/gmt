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
set (GMT_MECA_LIB_SRCS utilmeca.c nrutil.c submeca.c distaz.c)
set (GMT_MECA_PROGS_SRCS psmeca.c pspolar.c psvelo.c pscoupe.c)

# example files
set (GMT_MECA_EXAMPLE_FILES README.meca)
set (GMT_MECA_EXAMPLE_PROGS meca_1.bat meca_1.csh meca_2.csh meca_3.csh meca_4.csh)

# generate header file gmt_meca.h
gmt_set_api_header(GMT_MECA_API_H "${GMT_MECA_PROGS_SRCS}")

# make lib
add_library (gmt_meca STATIC ${GMT_MECA_LIB_SRCS}
	${GMT_MECA_PROGS_SRCS} ${GMT_MECA_API_H})
add_dependencies (gmt_meca gmt_headers)

# create build targets
string (REPLACE ".c" "" GMT_MECA_PROGS "${GMT_MECA_PROGS_SRCS}")
foreach (prog ${GMT_MECA_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
	PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMTPSL;FUNC=GMT_${prog}")
	target_link_libraries (${prog} gmtlib)
endforeach (prog)

# generate Mac debugging symbols
create_debug_sym (${GMT_MECA_PROGS})

# add the install targets
install (TARGETS ${GMT_MECA_PROGS}
	RUNTIME DESTINATION bin)

# add to gmt_suppl target
add_depend_to_target (gmt_suppl ${GMT_MECA_PROGS})

# Generate manpages
string (REPLACE ".c" ".1" GMT_MECA_MAN_1 "${GMT_MECA_PROGS_SRCS}")
gmt_create_manpages ("${GMT_MECA_MAN_1}")

# install example data
install (FILES ${GMT_MECA_EXAMPLE_FILES}
	DESTINATION
	${GMT_SHARE_PATH}/doc/gmt-${GMT_PACKAGE_VERSION}/supplements/meca)
install (PROGRAMS ${GMT_MECA_EXAMPLE_PROGS}
	DESTINATION
	${GMT_SHARE_PATH}/doc/gmt-${GMT_PACKAGE_VERSION}/supplements/meca)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
