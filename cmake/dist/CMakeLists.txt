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

#	set (CMAKE_MODULE_PATH \"${CMAKE_SOURCE_DIR}/cmake/modules/\"
#	CACHE INTERNAL \"Location of our custom CMake modules.\")

#include (InstallRequiredSystemLibraries)

# install dependent libraries
#set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
# ${NETCDF_LIBRARIES}
# ${GMT_OPTIONAL_LIBRARIES}
# CACHE INTERNAL "Dependent libraries")

# Common CPack settings:
set (CPACK_PACKAGE_NAME "GMT")
set (CPACK_PACKAGE_VENDOR "The GMT Team")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "The Generic Mapping Tools data processing and display software package")
set (CPACK_PACKAGE_VERSION_MAJOR ${GMT_PACKAGE_VERSION_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${GMT_PACKAGE_VERSION_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${GMT_PACKAGE_VERSION_PATCH})
set (CPACK_PACKAGE_VERSION ${GMT_PACKAGE_VERSION})
set (CPACK_PACKAGE_INSTALL_DIRECTORY GMT-${GMT_PACKAGE_VERSION_WITH_SVN_REVISION})
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

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

#set (CPACK_STRIP_FILES FALSE)
#set (CPACK_SOURCE_STRIP_FILES FALSE)

#set (CPACK_GENERATOR "DEB")
#set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Dr. No")

#set (CPACK_MONOLITHIC_INSTALL TRUE)
#set (CPACK_RPM_COMPONENT_INSTALL ON)

#set(CPACK_COMPONENT_RUNTIME_GROUP "Runtime")
#set(CPACK_COMPONENT_LEFTOVER_GROUP "Documentation")

#set(CPACK_COMPONENTS_IGNORE_GROUPS 1)

set (CPACK_COMPONENT_UNSPECIFIED_HIDDEN FALSE)
set (CPACK_COMPONENT_UNSPECIFIED_REQUIRED FALSE)

if (WIN32 AND NOT CYGWIN)
	# By default, do not warn when built on machines using only VS Express:
	if (NOT DEFINED CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS)
		set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)
	endif ()

	# Install MSVC runtime DLLs:
	include (InstallRequiredSystemLibraries)

	set (CPACK_GENERATOR Bundle ZIP NSIS)
	set (CPACK_NSIS_DISPLAY_NAME ${CPACK_PACKAGE_NAME})
	set (CPACK_NSIS_MODIFY_PATH ON)

	# StartMenu items:
	set (CPACK_NSIS_MENU_LINKS "bin/startup_win.bat" "GMT Command Prompt")

	# Install startup batch file:
	install (PROGRAMS startup_win.bat
		DESTINATION bin
		COMPONENT Runtime)

	# Fix bundle:
	install (CODE "
	if (CMAKE_INSTALL_PREFIX MATCHES \"_CPack_Packages\")
		include(BundleUtilities)
		fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/bin/gmt${CMAKE_EXECUTABLE_SUFFIX}\" \"\" \"\")
	endif()
	" COMPONENT Runtime)
endif(WIN32 AND NOT CYGWIN)

if (APPLE)
	set (CPACK_GENERATOR Bundle TGZ TBZ2)

	set (CPACK_BUNDLE_NAME ${CPACK_PACKAGE_NAME})
	set (CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")
	set (CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/GMT.icns")
	set (CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/startup_macosx.sh")

	# Fix bundle:
	install (CODE "
	set (BU_CHMOD_BUNDLE_ITEMS TRUE CACHE INTERNAL
		\"make bundle items writable before invoking install_name_tool\")
	if (CMAKE_INSTALL_PREFIX MATCHES \"_CPack_Packages.+[.]app/Contents/Resources\")
		include(BundleUtilities)
		fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/bin/gmt${CMAKE_EXECUTABLE_SUFFIX}\" \"\" \"\")
	endif()
	" COMPONENT Runtime)
endif(APPLE)

# This must always be last:
include (CPack)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
