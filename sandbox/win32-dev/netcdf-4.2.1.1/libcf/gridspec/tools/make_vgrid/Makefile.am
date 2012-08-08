## This is an automake file, part of Unidata's NetCDF CF Library
## package. Copyright 2009, see the COPYRIGHT file for more
## information.

# This automake file is in charge of building the gridspec
# make_vgrid tool.

# $Id$

bin_PROGRAMS = make_vgrid gs_make_vgrid
make_vgrid_SOURCES = create_vgrid.c create_vgrid.h make_vgrid.c
gs_make_vgrid_SOURCES = create_vgrid.c create_vgrid.h gs_make_vgrid.c

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

