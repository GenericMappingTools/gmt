#
# $Id$
#
# Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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
set (SUPPL_LIB_SRCS spotter.h libspotter.c)
set (SUPPL_PROGS_SRCS backtracker.c grdpmodeler.c grdrotater.c
	grdspotter.c hotspotter.c originator.c rotconverter.c)

# example files
set (SUPPL_EXAMPLE_FILES README.spotter
	DC85.d WK97.d pac_hs.d seamounts.d)
set (SUPPL_EXAMPLE_PROGS spotter.bat spotter.sh)

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
		DESTINATION ${GMT_DOCDIR}/supplements/spotter
		COMPONENT Documentation)
	install (PROGRAMS ${SUPPL_EXAMPLE_PROGS}
		DESTINATION ${GMT_DOCDIR}/supplements/spotter
		COMPONENT Documentation)
endif (UNIX)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
