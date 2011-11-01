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
set (GMT_X2SYS_LIB_SRCS ../mgd77/mgd77.h x2sys.h x2sys.c)
set (GMT_X2SYS_PROGS_SRCS x2sys_init.c x2sys_put.c x2sys_get.c x2sys_binlist.c
	x2sys_datalist.c x2sys_cross.c x2sys_list.c x2sys_report.c x2sys_solve.c
	x2sys_merge.c)

# example files
set (GMT_X2SYS_EXAMPLE_FILES README.x2sys)
set (GMT_X2SYS_EXAMPLE_PROGS test_x2sys.sh)

# generate header file gmt_x2sys.h
gmt_set_api_header(GMT_X2SYS_API_H ${GMT_X2SYS_PROGS_SRCS})

# lib sources
set (LIB_SRCS ${GMT_X2SYS_LIB_SRCS}
	${GMT_X2SYS_PROGS_SRCS} ${GMT_X2SYS_API_H})

# create build targets
string (REPLACE ".c" "" GMT_X2SYS_PROGS "${GMT_X2SYS_PROGS_SRCS}")
foreach (prog ${GMT_X2SYS_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
	PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMT;FUNC=GMT_${prog}")
	target_link_libraries (${prog} gmtlib)
endforeach (prog)

# generate Mac debugging symbols
create_debug_sym (${GMT_X2SYS_PROGS})

# add the install targets
install (TARGETS ${GMT_X2SYS_PROGS}
	RUNTIME DESTINATION bin)

# add to gmt_suppl target
add_depend_to_target (gmt_suppl ${GMT_X2SYS_PROGS})

# Generate manpages
string (REPLACE ".c" ".1" GMT_X2SYS_MAN_1 "${GMT_X2SYS_PROGS_SRCS}")
gmt_create_manpages ("${GMT_X2SYS_MAN_1}")

# install example data
if (UNIX)
	install (FILES ${GMT_X2SYS_EXAMPLE_FILES}
		DESTINATION
		${GMT_DOC_PATH}/supplements/x2sys)
	install (PROGRAMS ${GMT_X2SYS_EXAMPLE_PROGS}
		DESTINATION
		${GMT_DOC_PATH}/supplements/x2sys)
endif (UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
