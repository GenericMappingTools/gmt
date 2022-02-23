#
# Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
# CMake settings for supplemental package:
#
#   SUPPL_NAME:          name of the supplemental package
#   SUPPL_HEADERS:       header files. Will be installed if BUILD_DEVELOPER is TRUE
#   SUPPL_PROGS_SRCS:    list of C source codes for supplemental modules
#   SUPPL_LIB_SRCS:      list of C source codes for supplemental library
#   SUPPL_EXAMPLE_FILES: README and other example files
#   SUPPL_EXAMPLE_PROGS: Example scripts
#

set (SUPPL_NAME mgd77)
set (SUPPL_HEADERS cm4_functions.h mgd77.h mgd77_IGF_coeffs.h mgd77_codes.h
	mgd77_e77.h mgd77_functions.h mgd77_init.h mgd77_recalc.h mgd77_rls_coeffs.h
	mgd77defaults.h mgd77magref.h mgd77sniffer.h mgd77snifferdefaults.h)
set (SUPPL_PROGS_SRCS mgd77convert.c mgd77header.c mgd77info.c mgd77list.c
	mgd77magref.c mgd77manage.c mgd77path.c mgd77sniffer.c mgd77track.c)
set (SUPPL_LIB_SRCS ${SUPPL_PROGS_SRCS} mgd77.c mgd77_functions.c cm4_functions.c)
set (SUPPL_EXAMPLE_FILES README.mgd77)
