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
  cache.c
  cdf3.c
  cdf4.c
  common34.c
  constraints3.c
  constraints4.c
  dapalign.c
  dapattr3.c
  dapcvt.c
  dapdebug.c
  dapdump.c
  dapodom.c
  daputil.c
  dceconstraints.c
  dcelex.c
  dceparse.c
  dcetab.c
  getvara3.c
  getvara4.c
  ncd3dispatch.c
  ncd4dispatch.c
  ncdap3.c
  ncdap3a.c
  ncdap4.c
  ncdaperr.c
)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
