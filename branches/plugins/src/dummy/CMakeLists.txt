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
#set (SUPPL_LIB_SRCS plugin.c)
#set (SUPPL_PROGS_SRCS dummy1.c dummy2.c)

# lib sources
#set (LIB_SRCS ${SUPPL_LIB_SRCS} ${SUPPL_PROGS_SRCS})

# create build targets
string (REPLACE ".c" "" _gmt_modules "${SUPPL_PROGS_SRCS}")
#install_module_symlink (${_gmt_modules})

set (CMAKE_SHARED_MODULE_PREFIX)
set (CMAKE_SHARED_MODULE_SUFFIX .so)
add_library (func1 MODULE func1.c)
add_library (func2 MODULE func2.c)

add_executable (dltest dltest.c)
add_executable (dltest2 dltest2.c common_plugin.c)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
