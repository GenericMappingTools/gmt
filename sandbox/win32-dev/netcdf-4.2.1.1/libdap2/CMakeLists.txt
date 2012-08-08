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

# libnetcdf sources in libdap2
set (SRCS_NETCDF ${SRCS_NETCDF}
	constraints3.c common34.c dapcvt.c dapalign.c dapodom.c daputil.c
	ncdaperr.c cdf3.c cache.c dapdump.c dapdebug.c dapattr3.c
	ncd3dispatch.c ncdap3.c ncdap3a.c getvara3.c dceconstraints.c dcetab.c
	dceparse.c dcelex.c
	nccommon.h constraints3.h ncd3dispatch.h ncdap3.h dapalign.h
	dapodom.h getvara.h dapnc.h daputil.h dapdebug.h dapdump.h
	dceconstraints.h dcetab.h dceparselex.h
	)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
