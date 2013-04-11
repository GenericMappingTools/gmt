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
# share CMakeLists.txt
#

set (_gmt_share_dirs conf DCW cpt custom dbase mgd77 mgg pattern
	pslib time x2sys)

# install target for data
install (DIRECTORY ${_gmt_share_dirs}
	DESTINATION ${GMT_SHARE_PATH}
	COMPONENT Runtime
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	REGEX "[.](cmake|in)$" EXCLUDE)

# install version file (used during runtime to check for correct share dir)
configure_file (VERSION.in VERSION @ONLY NEWLINE_STYLE LF)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/VERSION
	DESTINATION ${GMT_SHARE_PATH}
	COMPONENT Runtime)

# only attempt to install shorelines when requested and path is known
if (GSHHG_PATH AND COPY_GSHHG)
	install (DIRECTORY ${GSHHG_PATH}/
		DESTINATION ${GMT_SHARE_PATH}/coast
		COMPONENT GSHHG)
	# set installed location
	set (GSHHG_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${GMT_SHARE_PATH}/coast)
	# create coastline.conf which is needed for in-build-dir ctest
	configure_file (coastline.conf.in coastline.conf @ONLY)
	message ("${GSHHG_INSTALL_PATH}")
else (GSHHG_PATH AND COPY_GSHHG)
	# set installed location
	set (GSHHG_INSTALL_PATH ${GSHHG_PATH})
endif (GSHHG_PATH AND COPY_GSHHG)

# add subtrees
add_subdirectory (conf)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
