#
#
# Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
# Contact info: www.generic-mapping-tools.org
#-------------------------------------------------------------------------------
#
# To modify the cmake process: Edit your cmake/ConfigUser.cmake file
#

set (SUPPL_NAME mgd77)

# source files
set (SUPPL_HEADERS cm4_functions.h mgd77.h mgd77_IGF_coeffs.h mgd77_codes.h
	mgd77_e77.h mgd77_functions.h mgd77_init.h mgd77_recalc.h mgd77_rls_coeffs.h
	mgd77defaults.h mgd77magref.h mgd77sniffer.h mgd77snifferdefaults.h)
set (SUPPL_LIB_SRCS mgd77.c mgd77_functions.c cm4_functions.c)
set (SUPPL_PROGS_SRCS mgd77convert.c mgd77header.c mgd77info.c mgd77list.c
	mgd77magref.c mgd77manage.c mgd77path.c mgd77sniffer.c mgd77track.c)

# example files
set (SUPPL_EXAMPLE_FILES README.mgd77)

# lib sources
set (LIB_SRCS ${SUPPL_LIB_SRCS} ${SUPPL_PROGS_SRCS})

# create build targets
string (REPLACE ".c" "" _gmt_modules "${SUPPL_PROGS_SRCS}")
install_module_symlink (${_gmt_modules})

# install example data
install (FILES ${SUPPL_EXAMPLE_FILES}
	DESTINATION ${GMT_DOCDIR}/supplements/${SUPPL_NAME}
	COMPONENT Documentation)

# install headers
if (BUILD_DEVELOPER)
	install (FILES ${SUPPL_HEADERS}
		DESTINATION ${GMT_INCLUDEDIR}/${SUPPL_NAME}
		COMPONENT Runtime)
endif (BUILD_DEVELOPER)
