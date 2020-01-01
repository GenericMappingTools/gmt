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
set (RST_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/rst/source)
add_subdirectory (fig)

# Add tests and make verbatim copies from scripts
add_subdirectory (scripts)
add_subdirectory (examples)

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
	PATTERN "CMakeLists.txt" EXCLUDE
	PATTERN "gmt.history" EXCLUDE
	REGEX "[.](cmake|in|ps)$" EXCLUDE)
