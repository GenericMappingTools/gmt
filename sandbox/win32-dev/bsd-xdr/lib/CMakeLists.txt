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

# Library sources
set (LIBXDR_HDRS_PRIVATE xdr_private.h)
set (LIBXDR_SRCS xdr.c xdr_array.c xdr_float.c xdr_mem.c xdr_rec.c
	xdr_reference.c xdr_sizeof.c xdr_stdio.c xdr_private.c)

# Additional link libraries
if (MSVC)
  set (XDR_LINK_LIBRARIES ws2_32.lib)
endif ()

# Build target
add_library (xdr ${LIBXDR_HDRS} ${LIBXDR_HDRS_PRIVATE} ${LIBXDR_SRCS})
target_link_libraries (xdr ${XDR_LINK_LIBRARIES})

# Set the build version (VERSION) and the API version (SOVERSION)
set_target_properties (xdr
	PROPERTIES
	OUTPUT_NAME xdr
	VERSION ${XDR_PACKAGE_VERSION}
	SOVERSION ${XDR_PACKAGE_VERSION_MAJOR}
	INSTALL_RPATH "${CMAKE_INSTALL_RPATH}"
	DEFINE_SYMBOL "LIBRARY_EXPORTS")

# Add install targets
install (TARGETS xdr
	LIBRARY DESTINATION lib
	COMPONENT Runtime
	RUNTIME DESTINATION bin
	COMPONENT Runtime
	ARCHIVE DESTINATION lib)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
