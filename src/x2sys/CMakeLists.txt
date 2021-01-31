#
#
# Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

set (SUPPL_NAME x2sys)
set (SUPPL_HEADERS x2sys.h)
set (SUPPL_PROGS_SRCS x2sys_init.c x2sys_put.c x2sys_get.c x2sys_binlist.c
	x2sys_datalist.c x2sys_cross.c x2sys_list.c x2sys_report.c x2sys_solve.c
	x2sys_merge.c)
set (SUPPL_LIB_SRCS ${SUPPL_PROGS_SRCS} x2sys.c)
set (SUPPL_EXAMPLE_FILES README.x2sys)
set (SUPPL_EXAMPLE_PROGS test_x2sys.sh)
