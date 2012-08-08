## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to build netCDF-4. The
# generated makefile is not run unless the user selected to build
# netCDF-4.

# $Id$

# Cause C preprocessor to search current and parent directory.
AM_CPPFLAGS =  -I$(top_srcdir) -I$(top_srcdir)/include

# OC Sources; include the daptab.[ch] to avoid the need for bison by user
SRC=oc.c \
daplex.c dapparse.c daptab.c \
occlientparams.c occompile.c occurlfunctions.c \
ocdata.c ocdebug.c ocdump.c  \
ocinternal.c ocnode.c \
ochttp.c \
ocrc.c ocread.c ocutil.c \
ocbytes.c oclist.c ocuri.c oclog.c \
xxdr.c

HDRS=oc.h ocx.h \
dapparselex.h daptab.h \
occlientparams.h occompile.h occonstraints.h occurlfunctions.h \
ocdata.h ocdatatypes.h ocdebug.h ocdump.h \
ocinternal.h ocnode.h \
ochttp.h ocrc.h ocread.h ocutil.h \
ocbytes.h oclist.h ocuri.h oclog.h \
xxdr.h

if BUILD_DAP
noinst_LTLIBRARIES = liboc.la
liboc_la_SOURCES = $(SRC) $(HDRS) 
liboc_la_CPPFLAGS = $(AM_CPPFLAGS)
endif

# These rule are used if someone wants to rebuild the grammar files.
# Otherwise never invoked, but records how to do it.
# BTW: note that renaming is essential because otherwise
# autoconf will forcibly delete files of the name *.tab.*

.PHONEY: bison

bison:: dap.y
	rm -f dap.tab.c dap.tab.h
	bison --debug -d -p dap dap.y
	mv dap.tab.c daptab.c; mv dap.tab.h daptab.h


