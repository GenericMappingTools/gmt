#
#
# Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
# See LICENSE.TXT file for copying and redistribution conditions.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation; version 3 or any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
# for more details.
#
# Contact info: www.generic-mapping-tools.org
# ----------------------------------------------------------------------------

# INSTRUCTIONS TO USERS:
#
# 1. Copy 'ConfigUserTemplate.cmake' to 'ConfigUser.cmake' and make any edits
#    related to install directory, the whereabouts of GSHHS, DCW.
# 2. If you are an advanced user who wishes to tinker with the more advanced
#    settings, then copy 'ConfigUserAdvancedTemplate.cmake' to 'ConfigUserAdvanced.cmake',
#    explore and make changes to your ConfigUserAdvanced.cmake file
#    to override variables in 'ConfigDefault.cmake' on a per-user basis.
# 3. Follow the rest of the installation instructions in BUILDING.md.
#
# 'ConfigUser.cmake' and 'ConfigUserAdvanced.cmake' are not version controlled
# (currently listed in .gitignore).
#
# Note: CMake considers an empty string, "FALSE", "OFF", "NO", or any string
# ending in "-NOTFOUND" to be false (this happens to be case-insensitive, so
# "False", "off", "no", and "something-NotFound" are all false).  Other values
# are true.  Thus it does not matter whether you use TRUE and FALSE, ON and
# OFF, or YES and NO for your booleans.

##
## Section 1: Installation paths
##
# Set install name suffix used for directories and gmt executables [undefined]:
#set (GMT_INSTALL_NAME_SUFFIX "suffix")

# Install into traditional directory structure. Disable to install a
# distribution type directory structure (doc and share separated) [on]:
#set (GMT_INSTALL_TRADITIONAL_FOLDERNAMES OFF)

# Make executables relocatable on supported platforms (relative RPATH) [FALSE]:
#set (GMT_INSTALL_RELOCATABLE TRUE)

# Exclude optional GDAL, PCRE, PCRE2, FFTW3, LAPACK, BLAS, ZLIB dependencies even if you have them installed [FALSE]
#set (GMT_EXCLUDE_GDAL TRUE)
#set (GMT_EXCLUDE_PCRE TRUE)
#set (GMT_EXCLUDE_PCRE2 TRUE)
#set (GMT_EXCLUDE_FFTW3 TRUE)
#set (GMT_EXCLUDE_LAPACK TRUE)
#set (GMT_EXCLUDE_BLAS TRUE)
#set (GMT_EXCLUDE_ZLIB TRUE)

# ============================================================================
# Advanced configuration begins here.  Usually it is not necessary to edit any
# settings below.  You should know what you are doing if you do though.  Note:
# installation paths are relative to ${CMAKE_INSTALL_PREFIX} unless absolute
# path is given.
# ============================================================================

# Set binary installation path [bin]:
#set (GMT_BINDIR "bin")

# Set library installation path [lib or lib64]:
#set (GMT_LIBDIR "lib")

# Set include installation path [include/gmt${GMT_INSTALL_NAME_SUFFIX}]:
#set (GMT_INCLUDEDIR "include/gmt")

# Set share installation path [share or share/gmt${GMT_INSTALL_NAME_SUFFIX}]:
#set (GMT_DATADIR "share/gmt")

# Set doc installation path [share/doc or share/doc/gmt${GMT_INSTALL_NAME_SUFFIX}]:
#set (GMT_DOCDIR "share/doc/gmt")

# Set manpage installation path [share/man or share/doc/gmt${GMT_INSTALL_NAME_SUFFIX}/man]:
#set (GMT_MANDIR "share/doc/gmt/man")

# Install documentation files from this external location instead of creating
# new HTML documents from scratch [${GMT_SOURCE_DIR}/doc_release]:
#set (GMT_INSTALL_EXTERNAL_DOC OFF)

# Install manual pages from this external location instead of creating the
# manpages from scratch [${GMT_SOURCE_DIR}/man_release]:
#set (GMT_INSTALL_EXTERNAL_MAN OFF)

##
## Section 2: Build dependencies (should only be needed if CMake cannot
## automatically detect the rights version or path.)
##

# Set URL to GMT Data server [auto]:
#set (GMT_DATA_SERVER "data_server_url")

# Set location of NetCDF (can be root directory, path to header file or path
# to nc-config) [auto]:
#set (NETCDF_ROOT "netcdf_install_prefix")

# Set location of GDAL (can be root directory, path to header file or path to
# gdal-config) [auto]:
#set (GDAL_ROOT "gdal_install_prefix")

# Set location of PCRE (can be root directory, path to header file or path to
# pcre-config) [auto]:
#set (PCRE_ROOT "pcre_install_prefix")
# Alternatively, set location of PCRE2 (can be root directory, path to header file or path to
# pcre2-config) [auto]:
#set (PCRE2_ROOT "pcre2_install_prefix")

# Set location of single precision FFTW (can be root directory or path to
# header file) [auto]:
#set (FFTW3_ROOT "fftw_install_prefix")

# Set location of ZLIB (can be root directory or path to header file) [auto]:
#set (ZLIB_ROOT "zlib_install_prefix")

# Set location of CURL (can be root directory or path to header file) [auto]:
#set (CURL_ROOT "curl_install_prefix")

# Set location of GLIB component gthread [auto].  This is an optional (and
# experimental) option which you need to enable:
#set (GMT_USE_THREADS TRUE)
# If pkg-config is not installed (e.g. on Windows) you need to specify these:
#set (GLIB_INCLUDE_DIR c:/path/to/glib-dev/include/glib-2.0)
#set (GLIB_LIBRARIES c:/path/to/glib-dev/lib/glib-2.0.lib)

# Set LAPACK location. Use this when want to link with LAPACK and it's not found automatically
#set (LAPACK_LIBRARY "V:/lapack-3.5.0/build/lib/liblapack.lib")
#set (BLAS_LIBRARY "V:/lapack-3.5.0/build/lib/libblas.lib")

##
## Section 3: GMT features
##

# Enforce GPL or LGPL conformity. Use this to disable routines that cannot be
# redistributed under the terms of the GPL or LGPL such as Shewchuk's
# triangulation (valid values are GPL, LGPL and off) [off]:
#set (LICENSE_RESTRICTED GPL)

# Allow building of OpenMP if compiler supports it
#set (GMT_ENABLE_OPENMP TRUE)

# Configure default units (possible values are SI and US) [SI]:
#set (UNITS "US")

# Enable building of shared libraries [TRUE] (disable to use static libraries;
# not recommended; on non-x86 architectures uncomment the next option as well):
#set (BUILD_SHARED_LIBS FALSE)

# Create position independent code on all targets [auto] (needed for static
# build on non-x86):
#set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# Build GMT shared lib with supplemental modules [TRUE]:
#set (BUILD_SUPPLEMENTS FALSE)

# Build/Install GMT Developer include files [TRUE]:
# This installs the extra include files and configured files needed by 3rd-party
# developers.  Until we build a separate gmt-devel we include them in the main
# Distribution.
#set (BUILD_DEVELOPER FALSE)

##
## Section 4: Advanced tweaking
##

#
# Testing and development
#

# Enable running examples/tests with "ctest" or "make check" (out-of-source).
# Need to set either DO_EXAMPLES, DO_TESTS or both and uncomment the following
# line.
#enable_testing()
#set (DO_EXAMPLES TRUE)
#set (DO_TESTS TRUE)
#set (DO_ANIMATIONS TRUE)
# Number of parallel test jobs with "make check":
#set (N_TEST_JOBS 4)

# Enable this option to run GMT programs from within ${GMT_BINARY_DIR} without
# installing or setting GMT_SHAREDIR and GMT_USERDIR first. This is required
# for testing [OFF]:
#set (SUPPORT_EXEC_IN_BINARY_DIR ON)

# Uncomment the following line to enable running low-level C tests of the API
#set (DO_API_TESTS ON)

# List extra sub-dirs of 'src' with a CMakeList.txt to build non-module codes
# that link against the full gmt libs (not just the API; for building codes
# that only need the GMT API, see the gmt-custom project).
#set (EXTRA_BUILD_DIRS apidemo)

# List extra new modules for testing without adding them to the module list
#set (EXTRA_MODULES newmodule1.c newmodule2.c)

# List extra new supplemental modules for testing without adding them to the module list
#set (EXTRA_MODULES_SUPPL newsuppl1.c newsuppl2.c)

# Directory in which to install the release sources per default
# [${GMT_BINARY_DIR}/gmt-${GMT_PACKAGE_VERSION}]:
#set (GMT_RELEASE_PREFIX "release-src-prefix")

# If set to false, image conversion from PS images to PNG and PDF does
# not depend on the gmt binary target. Note: "make gmt" is then required
# before docs_depends [TRUE].
#set (GMT_DOCS_DEPEND_ON_GMT FALSE)

#
# Debugging
#

# Set build type can be: empty, Debug, Release, RelWithDebInfo or MinSizeRel [Release]:
#set (CMAKE_BUILD_TYPE Debug)

# Extra debugging for developers:
#if ( CMAKE_GENERATOR STREQUAL "Xcode" )
#   #	So Xcode can find the supplemental plug-ins during debug sessions
#	add_definitions(-DXCODER)			# Handle a debug plugin directory
#	add_definitions(-DDEBUG_MODERN)			# To set PPID == 0 during Xcode test
#	message("Add Xcode definition for GMT")
#endif()
# Uncomment these two statements if you are a developer debugging GMT:
#add_definitions(-DDEBUG)
#add_definitions(-DMEMDEBUG) # Turn on memory tracking see gmt_support.c for extra info
#add_definitions(-DUSE_COMMON_LONG_OPTIONS) 	# Turn on testing of upcoming long-option syntax for common GMT options
#add_definitions(-DUSE_MODULE_LONG_OPTIONS) 	# Turn on testing of upcoming long-option syntax for module options
#set (CMAKE_C_FLAGS "-Wall -Wdeclaration-after-statement ${CMAKE_C_FLAGS}") # recommended even for release build
#set (CMAKE_C_FLAGS "-Wextra ${CMAKE_C_FLAGS}")            # extra warnings
#set (CMAKE_C_FLAGS_DEBUG -ggdb3)                          # gdb debugging symbols
#set (CMAKE_LINK_DEPENDS_DEBUG_MODE TRUE)                  # debug link dependencies
#if (HAVE_OPENMP)
#	set (CMAKE_C_FLAGS_RELEASE "-ggdb3 -O2 -Wuninitialized -flax-vector-conversions")  # check uninitialized variables
#else (HAVE_OPENMP)
#	set (CMAKE_C_FLAGS_RELEASE "-ggdb3 -O2 -Wuninitialized")  # check uninitialized variables
#endif (HAVE_OPENMP)

#
# System specific tweaks
#

# This is for GCC on Solaris to avoid "relocations remain against allocatable
# but non-writable sections" problems:
#set (USER_GMTLIB_LINK_FLAGS -mimpure-text)

# This may be needed to enable strdup and extended math functions with GCC and
# Suncc on Solaris:
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__EXTENSIONS__")

# Do not warn when building with Windows SDK or Visual Studio Express:
#set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_NO_WARNINGS ON)

# Manually select runtime library when compiling with Windows SDK or Visual
# Studio Express:
#set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS c:/Windows/System32/msvcr100.dll)

# If your NetCDF library is static (not recommended, applies to Windows only)
#set (NETCDF_STATIC TRUE)

# If want to rename the DLLs to something else than the default (e.g. to
# append the bitness - Windows only)
# WARNING: if using this option it is mandatory that the suffix starts with an underscore.
#if (WIN32)
# set (BITAGE 32)
# # Detect if we are building a 32 or 64 bits version
# if (CMAKE_SIZEOF_VOID_P EQUAL 8)
#   set (BITAGE 64)
# endif ()
# set (GMT_DLL_RENAME gmt_w${BITAGE})
# set (PSL_DLL_RENAME psl_w${BITAGE})
#endif(WIN32)

# On Windows Visual C 2012 needs _ALLOW_KEYWORD_MACROS to build
#if(MSVC11)
#  add_definitions(/D_ALLOW_KEYWORD_MACROS)
#endif(MSVC11)
