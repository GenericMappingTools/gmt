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

# libnetcdf sources in oc
set (SRCS_NETCDF ${SRCS_NETCDF}
	oc.c daplex.c dapparse.c daptab.c occlientparams.c occompile.c
	occurlfunctions.c ocdata.c ocdebug.c ocdump.c ocinternal.c ocnode.c ochttp.c
	ocrc.c ocread.c ocutil.c ocbytes.c oclist.c ocuri.c oclog.c xxdr.c
	oc.h ocx.h dapparselex.h daptab.h occlientparams.h occompile.h occonstraints.h
	occurlfunctions.h ocdata.h ocdatatypes.h ocdebug.h ocdump.h ocinternal.h
	ocnode.h ochttp.h ocrc.h ocread.h ocutil.h ocbytes.h oclist.h ocuri.h oclog.h
	xxdr.h
)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
