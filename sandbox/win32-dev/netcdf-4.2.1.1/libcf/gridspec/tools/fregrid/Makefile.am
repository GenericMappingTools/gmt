## This is an automake file, part of Unidata's NetCDF CF Library
## package. Copyright 2009, see the COPYRIGHT file for more
## information.

# This automake file is in charge of building the gridspec
# fregrid tool.

# $Id$

bin_PROGRAMS = fregrid gs_fregrid
fregrid_SOURCES = bilinear_interp.c bilinear_interp.h		\
conserve_interp.c conserve_interp.h fregrid.c fregrid_util.c	\
fregrid_util.h globals.h
gs_fregrid_SOURCES = bilinear_interp.c bilinear_interp.h	\
conserve_interp.c conserve_interp.h gs_fregrid.c fregrid_util.c	\
fregrid_util.h globals.h

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

