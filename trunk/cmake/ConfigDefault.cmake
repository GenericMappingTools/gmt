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
# Useful CMake variables.
#
# There are two configuration files:
#   1) "ConfigDefault.cmake" - is version controlled and used to add new default
#      variables and set defaults for everyone.
#   2) "ConfigUser.cmake" - is not version controlled (currently listed in
#      svn:ignore property) and used to override defaults on a per-user basis.
#
# NOTE: If you want to change CMake behaviour just for yourself then copy
#      "ConfigUserTemplate.cmake" to "ConfigUser.cmake" and then edit
#      "ConfigUser.cmake" (not "ConfigDefault.cmake" or "ConfigUserTemplate.cmake").
#

# The GMT package name.
set (GMT_PACKAGE_NAME "GMT")

# a short description of the gmt project (only a few words).
set (GMT_PACKAGE_DESCRIPTION_SUMMARY "The Generic Mapping Tools")

# The GMT package version.
set (GMT_PACKAGE_VERSION_MAJOR "5")
set (GMT_PACKAGE_VERSION_MINOR "0")
set (GMT_PACKAGE_VERSION_PATCH "1b")

# The subversion revision of the GMT source code.
# This is manually set when making GMT *public* releases.
# However, when making internal releases or just an ordinary developer build, leave it
# empty; if it is empty, the revision number is automatically populated for you on build.
#set (GMT_SOURCE_CODE_CONTROL_VERSION_STRING "9047")

# The GMT package version.
set (GMT_PACKAGE_VERSION "${GMT_PACKAGE_VERSION_MAJOR}.${GMT_PACKAGE_VERSION_MINOR}.${GMT_PACKAGE_VERSION_PATCH}")

# GMT_VERSION_YEAR set to current date in cmake/modules/ConfigCMake.cmake
# if not specified here:
#set (GMT_VERSION_YEAR "2012")
#set (GSHHS_VERSION "2.2.0")
#set (MANDATE "")

# The GMT wiki location
set (GMT_TRAC_WIKI "http://gmtrac.soest.hawaii.edu/projects/gmt/wiki/")

# Use SI units per default
if (NOT UNITS)
	set (UNITS "SI")
endif (NOT UNITS)

# File locking off
if (NOT DEFINED FLOCK)
	set (FLOCK off)
endif (NOT DEFINED FLOCK)

# Monolithic install on
if (NOT DEFINED GMT_INSTALL_MONOLITHIC)
	set (GMT_INSTALL_MONOLITHIC ON)
endif (NOT DEFINED GMT_INSTALL_MONOLITHIC)

# By defailt, do not enforce GPL conformity per default. This enables routines that cannot
# normally be redistributed under the terms of the GPL such as Shewchuk's triangulation.
if (NOT DEFINED LICENSE_RESTRICTED)
	set (LICENSE_RESTRICTED off)
endif (NOT DEFINED LICENSE_RESTRICTED)

# Use compatibility mode by default
if (NOT DEFINED GMT_COMPAT)
	set (GMT_COMPAT true)
endif (NOT DEFINED GMT_COMPAT)

# Default location of release documentation. If the directory exists in the
# source tree, the files will be copied to GMT_DOC_PATH instead of creating
# PDFs from source:
if (NOT DEFINED GMT_INSTALL_EXTERNAL_DOC)
	set (GMT_INSTALL_EXTERNAL_DOC ${GMT_SOURCE_DIR}/doc_release)
endif (NOT DEFINED GMT_INSTALL_EXTERNAL_DOC)

# Default location of release manpages. If the directory exists in the source
# tree, the files will be copied to GMT_MAN_PATH instead of creating manpages
# from source:
if (NOT DEFINED GMT_INSTALL_EXTERNAL_MAN)
	set (GMT_INSTALL_EXTERNAL_MAN ${GMT_SOURCE_DIR}/man_release)
endif (NOT DEFINED GMT_INSTALL_EXTERNAL_MAN)

# Directory in which to install the release sources per default
if (NOT DEFINED GMT_RELEASE_PREFIX)
	set (GMT_RELEASE_PREFIX ${GMT_BINARY_DIR}/GMT-${GMT_PACKAGE_VERSION}-src)
endif (NOT DEFINED GMT_RELEASE_PREFIX)


# The GMT copyright - string version to be used in a source file.
set (GMT_COPYRIGHT_STRING)
set (GMT_COPYRIGHT_STRING "${GMT_COPYRIGHT_STRING}Copyright 1991-${GMT_VERSION_YEAR} Paul Wessel, Walter H. F. Smith, R. Scharroo, J. Luis\\n")
set (GMT_COPYRIGHT_STRING "${GMT_COPYRIGHT_STRING}This program comes with NO WARRANTY, to the extent permitted by law.\\n")
set (GMT_COPYRIGHT_STRING "${GMT_COPYRIGHT_STRING}You may redistribute copies of this program under the terms of the\\n")
set (GMT_COPYRIGHT_STRING "${GMT_COPYRIGHT_STRING}GNU General Public License.\\n")
set (GMT_COPYRIGHT_STRING "${GMT_COPYRIGHT_STRING}For more information about these matters, see the file named LICENSE.TXT.\\n")
set (GMT_COPYRIGHT_STRING "${GMT_COPYRIGHT_STRING}\\n")


# You can set the build configuration type as a command-line argument to 'cmake' using -DCMAKE_BUILD_TYPE:STRING=Debug for example.
# If no build configuration type was given as a command-line option to 'cmake' then a default cache entry is set here.
# A cache entry is what appears in the 'CMakeCache.txt' file that CMake generates - you can edit that file directly or use the CMake GUI to edit it.
# The user can then set this parameter via the CMake GUI before generating the native build system.
# NOTE: this is not needed for visual studio because it has multiple configurations in the ide (and CMake includes them all).
# however makefile generators can only have one build type (to have multiple build types you'll need multiple out-of-place builds - one for each build type).
#
# The following are some valid build configuration types:
# 1) Debug - no optimisation with debug info.
# 2) Release - release build optimised for speed.
# 3) RelWithDebInfo - release build optimised for speed with debug info.
# 4) MinSizeRel - release build optimised for size.

# The following is from http://mail.kde.org/pipermail/kde-buildsystem/2008-November/005112.html...
#
# "The way to identify whether a generator is multi-configuration is to
# check whether CMAKE_CONFIGURATION_TYPES is set.  The VS/XCode generators
# set it (and ignore CMAKE_BUILD_TYPE).  The Makefile generators do not
# set it (and use CMAKE_BUILD_TYPE).  If CMAKE_CONFIGURATION_TYPES is not
# already set, don't set it."
#
if (NOT DEFINED CMAKE_CONFIGURATION_TYPES)
	if (NOT DEFINED CMAKE_BUILD_TYPE)
		# Should we set build type to RelWithDebInfo for developers and
		# to release for general public (ie when GPLATES_SOURCE_RELEASE is true) ?
		# Currently it's Release for both.
		set (CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel ${extra_build_configurations}."
		FORCE)
	endif (NOT DEFINED CMAKE_BUILD_TYPE)
endif (NOT DEFINED CMAKE_CONFIGURATION_TYPES)


# Turn this on if you want to...
#  Unix: see compiler commands echoed to console and messages about make
#  entering and leaving directories.
#  VisualStudio: see compiler commands.
# Setting CMAKE_VERBOSE_MAKEFILE to 'true'...
#  Unix: puts 'VERBOSE=1' in the top Makefile.
#  VisualStudio: sets SuppressStartupBanner to FALSE.
# If CMAKE_VERBOSE_MAKEFILE is set to 'false' and you want to turn on
# verbosity temporarily you can...
#  Unix: type 'make VERBOSE=1'  on the command-line when building.
#  VisualStudio: change SuppressStartupBanner to 'no' in "project
#  settings->configuration properties->*->general".
if (NOT DEFINED CMAKE_VERBOSE_MAKEFILE)
	set (CMAKE_VERBOSE_MAKEFILE false)
endif (NOT DEFINED CMAKE_VERBOSE_MAKEFILE)

# prefer shared libs over static
set (BUILD_SHARED_LIBS true)
set (CMAKE_FIND_STATIC LAST)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
