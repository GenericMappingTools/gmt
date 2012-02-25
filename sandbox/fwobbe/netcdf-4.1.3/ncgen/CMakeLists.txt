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

# sources of ncgen
set (SRCS_NCGEN main.c cdata.c bindata.c genchar.c cvt.c data.c debug.c
escapes.c genc.c genbin.c generr.c genlib.c getfill.c odom.c offsets.c
semantics.c ncgentab.c dump.c util.c bytebuffer.c list.c data.h
debug.h generr.h genlib.h includes.h ncgen.h odom.h offsets.h dump.h
util.h bytebuffer.h list.h genf77.c f77data.c genj.c jdata.c nciter.h
nciter.c ConvertUTF.c ConvertUTF.h)

# vim: textwidth=78 noexpandtab tabstop=2 softtabstop=2 shiftwidth=2
