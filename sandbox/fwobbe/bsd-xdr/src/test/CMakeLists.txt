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

# Define source files
set (TEST_HDRS test_common.h test_data.h test_xdrs.h)

set (TEST_XDRMEM_SRCS ${TEST_HDRS} ${LIBXDR_HDRS}
	xdrmem_test.c test_common.c test_data.c test_xdrs.c
	${GETOPT_SRCS} ${MKDTEMP_SRCS})

set (TEST_XDRSTDIO_SRCS ${TEST_HDRS} ${LIBXDR_HDRS}
	xdrstdio_test.c test_common.c test_data.c test_xdrs.c
	${GETOPT_SRCS} ${MKDTEMP_SRCS})

set (TEST_XDRSIZEOF_SRCS ${TEST_HDRS} ${LIBXDR_HDRS}
	xdrsizeof_test.c test_common.c test_data.c test_xdrs.c
	${GETOPT_SRCS})

# Build targets
add_executable (xdrmem_test ${TEST_XDRMEM_SRCS})
target_link_libraries (xdrmem_test xdr)
add_executable (xdrstdio_test ${TEST_XDRSTDIO_SRCS})
target_link_libraries (xdrstdio_test xdr)
add_executable (xdrsizeof_test ${TEST_XDRSIZEOF_SRCS})
target_link_libraries (xdrsizeof_test xdr)

# Add install targets
install (TARGETS xdrmem_test xdrstdio_test xdrsizeof_test
	LIBRARY DESTINATION lib
	COMPONENT Runtime
	RUNTIME DESTINATION bin
	COMPONENT Runtime
	ARCHIVE DESTINATION lib)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
