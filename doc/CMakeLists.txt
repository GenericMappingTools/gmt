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

include (GmtHelperMacros)

# Install license files
if (NOT LICENSE_RESTRICTED STREQUAL GPL)
	install (FILES
		${COPYING_LGPL}
		DESTINATION ${GMT_DOCDIR}/copying
		COMPONENT Runtime)
endif (NOT LICENSE_RESTRICTED STREQUAL GPL)
install (FILES
	${GMT_SOURCE_DIR}/LICENSE.TXT
	${COPYING_GPL}
	${GMT_EXTRA_LICENSE_FILES}
	DESTINATION ${GMT_DOCDIR}/copying
	COMPONENT Runtime)

# reST documentation
add_subdirectory (rst)
set (RST_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/rst/_source)
add_subdirectory (fig)

# Add tests and make verbatim copies from scripts
add_subdirectory (scripts)
add_subdirectory (examples)

if (EXISTS ${GMT_INSTALL_EXTERNAL_DOC})
	# Install documentation files from external location

	install (DIRECTORY ${GMT_INSTALL_EXTERNAL_DOC}/
		DESTINATION ${GMT_DOCDIR}
		COMPONENT Documentation
		USE_SOURCE_PERMISSIONS)
endif (EXISTS ${GMT_INSTALL_EXTERNAL_DOC})

if (NOT WIN32)
	# Do not install .bat files
	set (_exclude_bat "*.bat")
endif (NOT WIN32)

# Install target for examples
install (DIRECTORY examples
	DESTINATION ${GMT_DOCDIR}
	COMPONENT Documentation
	USE_SOURCE_PERMISSIONS
	PATTERN "${_exclude_bat}" EXCLUDE
	PATTERN ".svn" EXCLUDE
	PATTERN "CMakeLists.txt" EXCLUDE
	REGEX "[.](cmake|in|ps)$" EXCLUDE)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
