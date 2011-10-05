#
# $Id$
#
# Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# Contact info: gmt.soest.hawaii.edu
#-------------------------------------------------------------------------------
#
# doc CMakeLists.txt
#

# install target for data
install (DIRECTORY examples
	DESTINATION ${GMT_SHARE_PATH}/doc/gmt-${GMT_PACKAGE_VERSION}
	USE_SOURCE_PERMISSIONS
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	PATTERN ".cmake" EXCLUDE)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
