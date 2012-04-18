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
set (SUPPL_LIB_SRCS ../mgd77/mgd77.h x2sys.h x2sys.c)
set (SUPPL_PROGS_SRCS x2sys_init.c x2sys_put.c x2sys_get.c x2sys_binlist.c
	x2sys_datalist.c x2sys_cross.c x2sys_list.c x2sys_report.c x2sys_solve.c
	x2sys_merge.c)

# example files
set (SUPPL_EXAMPLE_FILES README.x2sys)
set (SUPPL_EXAMPLE_PROGS test_x2sys.sh)

# generate header file gmt_x2sys.h
gmt_set_api_header(SUPPL_API_H ${SUPPL_PROGS_SRCS})

# lib sources
set (LIB_SRCS ${SUPPL_LIB_SRCS}
	${SUPPL_PROGS_SRCS} ${SUPPL_API_H})

# create build targets
string (REPLACE ".c" "" SUPPL_PROGS "${SUPPL_PROGS_SRCS}")
foreach (prog ${SUPPL_PROGS})
	add_executable (${prog} ${GMT_PROGRAM})
	set_target_properties (${prog}
		PROPERTIES COMPILE_DEFINITIONS "FUNC_MODE=GMTAPI_GMT;FUNC=GMT_${prog}")
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

# install example data
if (UNIX)
	install (FILES ${SUPPL_EXAMPLE_FILES}
		DESTINATION ${GMT_DOC_PATH}/supplements/x2sys
		COMPONENT Documentation)
	install (PROGRAMS ${SUPPL_EXAMPLE_PROGS}
		DESTINATION ${GMT_DOC_PATH}/supplements/x2sys
		COMPONENT Documentation)
endif (UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
