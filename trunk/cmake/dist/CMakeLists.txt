#
# $Id$
#
# Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
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

# Common CPack settings:
set (CPACK_PACKAGE_NAME "GMT")
set (CPACK_PACKAGE_VENDOR "The GMT Team")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Generic Mapping Tools data processing and display software package")
set (CPACK_PACKAGE_VERSION_MAJOR ${GMT_PACKAGE_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${GMT_PACKAGE_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${GMT_PACKAGE_VERSION_PATCH})
set (CPACK_PACKAGE_VERSION ${GMT_PACKAGE_VERSION_WITH_SVN_REVISION})
if (NOT DEFINED CPACK_PACKAGE_INSTALL_DIRECTORY)
	if (WIN32)
		set (CPACK_PACKAGE_INSTALL_DIRECTORY gmt${GMT_PACKAGE_VERSION_MAJOR})
	else (WIN32)
		set (CPACK_PACKAGE_INSTALL_DIRECTORY GMT-${GMT_PACKAGE_VERSION_WITH_SVN_REVISION})
	endif (WIN32)
endif (NOT DEFINED CPACK_PACKAGE_INSTALL_DIRECTORY)
set (CPACK_PACKAGE_DESCRIPTION_FILE
	"${CMAKE_CURRENT_SOURCE_DIR}/cpack_package_description.txt")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/COPYING.COMBINED")


# Read GPL and LGPL license files
file(READ ${COPYING_GPL} _license_gpl)
file(READ ${COPYING_LGPL} _license_lgpl)

set (_license_extra)
if (NOT LICENSE_RESTRICTED)
	# Read extra license files
	foreach (_license_file ${GMT_EXTRA_LICENSE_FILES})
		file(READ ${_license_file} _license_string)
		set (_license_extra
			"${_license_extra}\n---------------------------------------------------------------------------\n\n${_license_string}")
	endforeach (_license_file ${GMT_EXTRA_LICENSE_FILES})

	# Copy combined license template
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_SOURCE_DIR}/COPYING.RESTRICTED.template
		${CMAKE_CURRENT_BINARY_DIR}/COPYING.COMBINED)
else (NOT LICENSE_RESTRICTED)
	# Clear contents of license file if it exists
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove -f
		${CMAKE_CURRENT_BINARY_DIR}/COPYING.COMBINED)
endif (NOT LICENSE_RESTRICTED)

# Append GPL, LGPL, and licenses from GMT_EXTRA_LICENSE_FILES
set (_license_combined "${_license_gpl}")
if (NOT LICENSE_RESTRICTED STREQUAL GPL)
	set (_license_combined
		"${_license_combined}\n---------------------------------------------------------------------------\n\n${_license_lgpl}")
endif (NOT LICENSE_RESTRICTED STREQUAL GPL)
set (_license_combined "${_license_combined}\n${_license_extra}")

# Append to COPYING.COMBINED or create new combined license file
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/COPYING.COMBINED "${_license_combined}")


# Install components:
set (CPACK_COMPONENTS_ALL Runtime Documentation)
set (CPACK_COMPONENT_DOCUMENTATION_DISABLED ON)
set (CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Runtime files")
set (CPACK_COMPONENT_RUNTIME_DESCRIPTION
	"GMT programs, libraries and shared files.")
set (CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION
	"Documentation and Example files.")

# Create two installation types with pre-selected components:
set (CPACK_ALL_INSTALL_TYPES Full Minimal)
set (CPACK_INSTALL_TYPE_FULL_DISPLAY_NAME "Everything")
set (CPACK_COMPONENT_RUNTIME_INSTALL_TYPES Minimal Full)
set (CPACK_COMPONENT_DOCUMENTATION_INSTALL_TYPES Full)

# One package per COMPONENT:
set(CPACK_COMPONENTS_GROUPING "IGNORE")

# Enable component install for archive generators:
set (CPACK_ARCHIVE_COMPONENT_INSTALL ON)

# Debian:
set (CPACK_DEBIAN_PACKAGE_MAINTAINER "GMT Team <gmt-help@lists.hawaii.edu>")
set (CPACK_DEBIAN_PACKAGE_SECTION science)
#set (CPACK_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION_SUMMARY})
#set (CPACK_PACKAGE_DESCRIPTION "GMT is an open source collection of ~65 tools
#for manipulating geographic and Cartesian data sets (including filtering,
#trend fitting, gridding, projecting, etc.) and producing Encapsulated
#PostScript File (EPS) illustrations ranging from simple x-y plots via contour
#maps to artificially illuminated surfaces and 3-D perspective views; the GMT
#supplements add another ~70 more specialized tools. GMT supports over 30 map
#projections and transformations and comes with support data such as GSHHS
#coastlines, rivers, and political boundaries.")

#set (CPACK_MONOLITHIC_INSTALL TRUE)
set (CPACK_RPM_COMPONENT_INSTALL ON)

#set(CPACK_COMPONENT_RUNTIME_GROUP "Runtime")
#set(CPACK_COMPONENT_LEFTOVER_GROUP "Documentation")

#set(CPACK_COMPONENTS_IGNORE_GROUPS 1)

set (CPACK_COMPONENT_UNSPECIFIED_HIDDEN FALSE)
set (CPACK_COMPONENT_UNSPECIFIED_REQUIRED FALSE)


# Windows:
if (WIN32 AND NOT CYGWIN)
	# By default, do not warn when built on machines using only VS Express:
	#set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)

	# Install MSVC runtime DLLs:
	include (InstallRequiredSystemLibraries)

	# InstallRequiredSystemLibraries installs the runtime libraries without
	# specifying COMPONENT Runtime. We have to do it manually here:
	set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
	install (PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
		DESTINATION ${CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION}
		COMPONENT Runtime)

	set (CPACK_GENERATOR ZIP NSIS)
	set (CPACK_NSIS_DISPLAY_NAME
		"${CPACK_PACKAGE_NAME}${GMT_PACKAGE_VERSION_MAJOR}")
	if (NOT DEFINED CPACK_NSIS_INSTALL_ROOT)
		set (CPACK_NSIS_INSTALL_ROOT "c:\\\\programs")
	endif (NOT DEFINED CPACK_NSIS_INSTALL_ROOT)
	set (CPACK_NSIS_MODIFY_PATH ON)
	string(REPLACE "/" "\\\\" _native_share_path "${GMT_SHARE_PATH}")
	set (CPACK_NSIS_ADD_ENVVAR1 "GMT5_SHAREDIR")
	set (CPACK_NSIS_ADD_ENVVAL1 "\\\$INSTDIR\\\\${_native_share_path}")

	# StartMenu items:
	set (CPACK_NSIS_MENU_LINKS
		"${GMT_BINDIR}/startup_win.bat" "GMT Command Prompt"
		"${GMT_DOC_PATH}" "Documentation")

	# Install startup batch file:
	install (PROGRAMS startup_win.bat
		DESTINATION ${GMT_BINDIR}
		COMPONENT Runtime)

	# Fix bundle:
	install (CODE "
	if (CMAKE_INSTALL_PREFIX MATCHES \"_CPack_Packages\")
		include(BundleUtilities)
		fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${GMT_BINDIR}/gmtset${CMAKE_EXECUTABLE_SUFFIX}\" \"\" \"\")
	endif ()
	" COMPONENT Runtime)
endif(WIN32 AND NOT CYGWIN)

# MacOSX:
if (APPLE)
	# Generate startup script:
	configure_file (startup_macosx.sh.in startup_macosx.sh @ONLY)
	configure_file (Info.plist.in Info.plist @ONLY)

	# Configure CPack:
	set (CPACK_GENERATOR Bundle TGZ TBZ2)
	set (CPACK_BUNDLE_NAME
		"${CPACK_PACKAGE_NAME}-${GMT_PACKAGE_VERSION_WITH_SVN_REVISION}")
	set (CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/GMT.icns")
	set (CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
	set (CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/startup_macosx.sh")

	# Fix bundle:
	install (CODE "
	set (BU_CHMOD_BUNDLE_ITEMS TRUE CACHE INTERNAL
		\"make bundle items writable before invoking install_name_tool\")
	if (CMAKE_INSTALL_PREFIX MATCHES \"_CPack_Packages.+[.]app/Contents/Resources\")
		cmake_policy (SET CMP0009 NEW)
		cmake_policy (SET CMP0011 NEW)
		include(BundleUtilities NO_POLICY_SCOPE)
		fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/${GMT_BINDIR}/gmtset${CMAKE_EXECUTABLE_SUFFIX}\" \"\" \"\")
	endif ()
	" COMPONENT Runtime)

	# Workaround: move documentation:
	install (CODE "
	if (CMAKE_INSTALL_PREFIX MATCHES \"_CPack_Packages.+[.]app/Contents/Resources\")
		execute_process (COMMAND ${CMAKE_COMMAND} -E rename
			\${CMAKE_INSTALL_PREFIX}/${GMT_DOC_PATH}
			\${CMAKE_INSTALL_PREFIX}/../../../Documentation)
	endif ()
	" COMPONENT Documentation)
endif (APPLE)

# Linux, Cygwin & Co:
if (UNIX AND NOT APPLE)
	set (CPACK_GENERATOR TGZ TBZ2)
endif (UNIX AND NOT APPLE)

# This must always be last:
include (CPack)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
