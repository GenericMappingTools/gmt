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

if (APPLE)
	# fix bundle
	install (CODE "
	set (BU_CHMOD_BUNDLE_ITEMS TRUE CACHE INTERNAL
		\"make bundle items writable before invoking install_name_tool\")
	if (CMAKE_INSTALL_PREFIX MATCHES \"_CPack_Packages.+[.]app/Contents/Resources\")
		include(BundleUtilities)
		fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/bin/gmt${CMAKE_EXECUTABLE_SUFFIX}\" \"\" \"\")
	endif()
	" COMPONENT Runtime)

	SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "My funky project")
	SET(CPACK_PACKAGE_VENDOR "Me, myself, and I")
	SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
	SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
	SET(CPACK_PACKAGE_VERSION_MAJOR "5")
	SET(CPACK_PACKAGE_VERSION_MINOR "0")
	SET(CPACK_PACKAGE_VERSION_PATCH "0")
	SET(CPACK_PACKAGE_INSTALL_DIRECTORY "GMT5.0.0")

	set (CPACK_BUNDLE_NAME "GMT5")
	set (CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")
	set (CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/GMT.icns")
	set (CPACK_BUNDLE_STARTUP_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/startup_macosx.sh")

	#set (CPACK_STRIP_FILES FALSE)
	#set (CPACK_SOURCE_STRIP_FILES FALSE)

	set (CPACK_GENERATOR Bundle TGZ) # TGZ Bundle TBZ2)
	#set (CPACK_GENERATOR "DEB")
	#set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Dr. No")

	#set (CPACK_MONOLITHIC_INSTALL TRUE)
	#set (CPACK_MONOLITHIC_INSTALL FALSE)
	#set (CPACK_RPM_COMPONENT_INSTALL ON)
	set (CPACK_ArchiveGenerator_COMPONENT_INSTALL ON)
	set (CPACK_TGZ_COMPONENT_INSTALL ON)
	#set (CPACK_Bundle_COMPONENT_INSTALL ON)

	# One package per COMPONENT:
	#set(CPACK_COMPONENTS_GROUPING "IGNORE")

	set (CPACK_COMPONENTS_ALL Runtime) # Documentation)
	set(CPACK_COMPONENT_RUNTIME_GROUP "Runtime")
	set(CPACK_COMPONENT_LEFTOVER_GROUP "Documentation")

	#set(CPACK_COMPONENTS_IGNORE_GROUPS 1)

	set (CPACK_COMPONENT_DOCUMENTATION_DISABLED ON)
	set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "GMT Runtime")
	set(CPACK_COMPONENT_RUNTIME_DESCRIPTION
		"GMT programs, libraries and shared files.")
	set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION
		"Documentation and Example files.")

	SET(CPACK_COMPONENT_UNSPECIFIED_HIDDEN FALSE)
	SET(CPACK_COMPONENT_UNSPECIFIED_REQUIRED FALSE)

	include (CPack) # This must always be last

endif(APPLE)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
