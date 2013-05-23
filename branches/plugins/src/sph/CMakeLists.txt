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

# source files
set (SUPPL_LIB_SRCS sph.h sph.c)
set (SUPPL_PROGS_SRCS sphtriangulate.c sphdistance.c sphinterpolate.c)

# example files
set (SUPPL_EXAMPLE_FILES README.sph
	gshhs_c.txt hotspots.d lun2.txt mars370.txt)
set (SUPPL_EXAMPLE_PROGS
	sph_ex_1.sh sph_ex_2.sh sph_ex_3.sh sph_ex_4.sh)

# lib sources
set (LIB_SRCS ${SUPPL_LIB_SRCS} ${SUPPL_PROGS_SRCS})

# create build targets
string (REPLACE ".c" "" _gmt_modules "${SUPPL_PROGS_SRCS}")
install_module_symlink (${_gmt_modules})

# Generate manpages
string (REPLACE ".c" ".1" SUPPL_MAN_1 "${SUPPL_PROGS_SRCS}")
gmt_create_manpages ("${SUPPL_MAN_1}")

# install example data
if (UNIX)
	install (FILES ${SUPPL_EXAMPLE_FILES}
		DESTINATION ${GMT_DOC_PATH}/supplements/sph
		COMPONENT Documentation)
	install (PROGRAMS ${SUPPL_EXAMPLE_PROGS}
		DESTINATION ${GMT_DOC_PATH}/supplements/sph
		COMPONENT Documentation)
endif (UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
