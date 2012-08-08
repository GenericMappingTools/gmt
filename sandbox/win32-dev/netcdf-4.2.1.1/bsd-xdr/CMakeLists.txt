# $Id$
#
# Copyright (c) 2012, Florian Wobbe
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

# Define minimum CMake version required
cmake_minimum_required (VERSION 2.8.5)

# Warning
if (NOT WIN32)
	message (WARNING "XDR will most probably not build with CMake on this"
		" system. You have been warned.")
endif()

# Make sure the user doesn't play dirty with symlinks
get_filename_component (srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component (bindir "${CMAKE_BINARY_DIR}" REALPATH)

# Disallow in-source builds
if (${srcdir} STREQUAL ${bindir})
  message(FATAL_ERROR "In-source builds are not allowed. "
    "Please create a directory and run cmake from there, passing the path "
    "to this source directory as the last argument. This process created "
    "the file `CMakeCache.txt' and the directory `CMakeFiles' in ${srcdir}. "
    "Please remove them.")
endif (${srcdir} STREQUAL ${bindir})

# Define project name
project (XDR)

# The XDR package version.
set (XDR_PACKAGE_VERSION_MAJOR "1")
set (XDR_PACKAGE_VERSION_MINOR "0")
set (XDR_PACKAGE_VERSION_PATCH "0")
set (XDR_PACKAGE_VERSION
	"${XDR_PACKAGE_VERSION_MAJOR}.${XDR_PACKAGE_VERSION_MINOR}.${XDR_PACKAGE_VERSION_PATCH}")

# Include defaults
include (${CMAKE_SOURCE_DIR}/cmake/ConfigUser.cmake)

# Extra include directories
include_directories (${XDR_SOURCE_DIR})

# Prefer shared libs over static
set (BUILD_SHARED_LIBS true)

# Add subdirectories
add_subdirectory (lib)
add_subdirectory (src)

# Public header files
set (LIBXDR_HDRS rpc/xdr.h rpc/types.h)

# Install target for public headers
install (FILES
	${LIBXDR_HDRS}
	DESTINATION include/rpc)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
