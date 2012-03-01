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

# Check if we have a proper getopt_long() implementation
include(CheckFunctionExists)
set(CMAKE_REQUIRED_INCLUDES "getopt.h")
check_function_exists("getopt_long" HAVE_GETOPT_LONG)
set(CMAKE_REQUIRED_INCLUDES)

if (NOT HAVE_GETOPT_LONG)
	set (GETOPT_SRCS
		${CMAKE_CURRENT_SOURCE_DIR}/getopt.h
		${CMAKE_CURRENT_SOURCE_DIR}/getopt_long.c)
	include_directories (${CMAKE_CURRENT_SOURCE_DIR})
endif ()

# Check for mkdtemp
check_function_exists("mkdtemp" HAVE_MKDTEMP)
if (NOT HAVE_MKDTEMP)
	set (MKDTEMP_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/mkdtemp.c)
endif ()

# Include dir of inttypes.h for MSVC
if (MSVC)
	include_directories (${CMAKE_CURRENT_SOURCE_DIR}/msvc)
endif ()

# Include test dir
add_subdirectory (test)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
