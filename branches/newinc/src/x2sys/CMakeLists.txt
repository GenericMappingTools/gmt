#
# $Id$
#
# Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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
# To modify the cmake process: Edit your cmake/ConfigUser.cmake file
#

# source files
set (SUPPL_LIB_SRCS ../mgd77/mgd77.h x2sys.h x2sys.c)
set (SUPPL_PROGS_SRCS x2sys_init.c x2sys_put.c x2sys_get.c x2sys_binlist.c
	x2sys_datalist.c x2sys_cross.c x2sys_list.c x2sys_report.c x2sys_solve.c
	x2sys_merge.c)

# example files
set (SUPPL_EXAMPLE_FILES README.x2sys)
set (SUPPL_EXAMPLE_PROGS test_x2sys.sh)

# lib sources
set (LIB_SRCS ${SUPPL_LIB_SRCS} ${SUPPL_PROGS_SRCS})

# create build targets
string (REPLACE ".c" "" _gmt_modules "${SUPPL_PROGS_SRCS}")
install_module_symlink (${_gmt_modules})

## Generate manpages
#string (REPLACE ".c" ".1" SUPPL_MAN_1 "${SUPPL_PROGS_SRCS}")
#gmt_create_manpages ("${SUPPL_MAN_1}")

# install example data
if (UNIX)
	install (FILES ${SUPPL_EXAMPLE_FILES}
		DESTINATION ${GMT_DOCDIR}/supplements/x2sys
		COMPONENT Documentation)
	install (PROGRAMS ${SUPPL_EXAMPLE_PROGS}
		DESTINATION ${GMT_DOCDIR}/supplements/x2sys
		COMPONENT Documentation)
endif (UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
