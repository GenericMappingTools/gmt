#
# $Id$
#
# Copyright (c) 1991-2018 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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
	gmt_links.sh
	gmt_prepmex.sh
	gmt_make_custom_code.sh
	gmt_uninstall.sh
	ncdeflate
	${CMAKE_CURRENT_BINARY_DIR}/gmt5syntax
	DESTINATION ${GMT_DATADIR}/tools
	COMPONENT Runtime)

if (NOT DEFINED BASH_COMPLETION_DIR)
	set (BASH_COMPLETION_DIR etc/bash_completion.d)
endif (NOT DEFINED BASH_COMPLETION_DIR)

if (GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
	install (PROGRAMS
		gmt_completion.bash
		DESTINATION ${GMT_DATADIR}/tools
		COMPONENT Runtime)
else (GMT_INSTALL_TRADITIONAL_FOLDERNAMES)
	install (FILES
		gmt_completion.bash
		DESTINATION ${BASH_COMPLETION_DIR}
		COMPONENT Runtime
		RENAME gmt)
endif (GMT_INSTALL_TRADITIONAL_FOLDERNAMES)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
