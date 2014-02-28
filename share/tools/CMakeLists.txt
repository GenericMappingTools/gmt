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
# share/tools/CMakeLists.txt
#

configure_file (gmt5syntax.in gmt5syntax @ONLY)

install (PROGRAMS
	gmt_functions.sh
	gmt_aliases.csh
	ncdeflate
	${CMAKE_CURRENT_BINARY_DIR}/gmt5syntax
	DESTINATION ${GMT_DATADIR}/tools
	COMPONENT Runtime)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
