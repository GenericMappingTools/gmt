## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to build netCDF-4. The
# generated makefile is not run unless the user selected to build
# netCDF-4.

# $Id$

# Cause C preprocessor to search current and parent directory.
AM_CPPFLAGS =  -I$(top_srcdir) -I$(top_srcdir)/libsrc

# OC Sources
SRC= curlfunctions.c dapparse.c daplex.c ocuri.c http.c ocbytes.c	\
oc.c occompile.c occontent.c ocdata.c ocdebug.c ocdrno.c ocdump.c	\
ocinternal.c oclist.c oclog.c ocnode.c ocutil.c occlientparams.c	\
ocxdr_stdio.c rc.c read.c

HDRS= curlfunctions.h constraints.h dapparselex.h ocuri.h http.h ocbytes.h \
occontent.h ocdata.h ocdatatypes.h ocdebug.h ocdrno.h ocdump.h oc.h	\
ocinternal.h oclist.h oclog.h ocnode.h occlientparams.h ocutil.h rc.h	\
read.h

GRAMFILES=dap.y
# autoconf persists in removing the bison output unless it is renamed
GRAMC=daptab.c
GRAMH=daptab.h

if BUILD_DAP
noinst_LTLIBRARIES = liboc.la
# Watch out; the gram files must be first
liboc_la_SOURCES = ${GRAMC} ${GRAMH} $(SRC) $(HDRS) 
liboc_la_CPPFLAGS = $(AM_CPPFLAGS)
endif

EXTRA_DIST = ${GRAMFILES}

# These rule are used if someone wants to rebuild the grammar files.
# Otherwise never invoked, but records how to do it.
# BTW: note that renaming is essential because otherwise
# autoconf will forcibly delete files of the name *.tab.*

.PHONEY: bison

bison daptab.c daptab.h: dap.y
	rm -f dap.tab.c dap.tab.h
	bison --debug -d -p dap dap.y
	mv dap.tab.c daptab.c; mv dap.tab.h daptab.h
