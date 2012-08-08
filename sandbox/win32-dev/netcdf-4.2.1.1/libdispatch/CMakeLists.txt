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

# libnetcdf sources in libdispatch
set (SRCS_NETCDF ${SRCS_NETCDF}
	dparallel.c dcopy.c dfile.c ddim.c datt.c dattinq.c dattput.c dattget.c
	derror.c dvar.c dvarget.c dvarput.c dvarinq.c ddispatch.c dnclog.c dstring.c
	dutf8proc.c utf8proc_data.h nc_uri.c nclist.c ncbytes.c nchashmap.c nctime.c
	dsubstrate.c
	dgroup.c dvlen.c dcompound.c dtype.c denum.c dopaque.c ncaux.c
	)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
