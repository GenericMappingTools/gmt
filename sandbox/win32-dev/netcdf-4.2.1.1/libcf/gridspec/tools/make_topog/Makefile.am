## This is an automake file, part of Unidata's NetCDF CF Library
## package. Copyright 2009, see the COPYRIGHT file for more
## information.

# This automake file is in charge of building the gridspec
# make_topog tool.

# $Id$

bin_PROGRAMS = make_topog gs_make_topog
make_topog_SOURCES = make_topog.c  topog.c  topog.h
gs_make_topog_SOURCES = make_topog.c  topog.c  topog.h

# Initalize these.
AM_CPPFLAGS =
AM_LDFLAGS =
LDADD =

AM_CPPFLAGS += -I${top_srcdir}/shared -I${top_srcdir}/shared/mosaic
AM_LDFLAGS += -L${top_builddir}/shared -L${top_builddir}/shared/mosaic -ltoolsshared -lshared

if USE_NETCDF_DIR
AM_CPPFLAGS += -I@NETCDFDIR@/include
AM_LDFLAGS += -L@NETCDFDIR@/lib
else
AM_CPPFLAGS += -I${top_srcdir}/../liblib
AM_LDFLAGS += ${top_builddir}/../liblib/libnetcdf.la
endif

AM_LDFLAGS += @EXTERN_LDFLAGS@ -lm


TESTS = run_tests
EXTRA_DIST = run_tests.in
CLEANFILES = topog.nc gs_topog.nc topog.cdl gs_topog.cdl
DISTCLEANFILES = run_tests
