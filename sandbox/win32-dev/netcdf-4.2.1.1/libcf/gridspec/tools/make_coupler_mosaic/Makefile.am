## This is an automake file, part of Unidata's NetCDF CF Library
## package. Copyright 2009, see the COPYRIGHT file for more
## information.

# This automake file is in charge of building the gridspec
# make_coupler_mosaic tool.

# $Id$

bin_PROGRAMS = make_coupler_mosaic gs_coupler
make_coupler_mosaic_SOURCES = make_coupler_mosaic.c
gs_coupler_SOURCES = gs_coupler.c

# Initalize these.
AM_CPPFLAGS = -I../shared -I../../shared/mosaic
AM_LDFLAGS = -L../shared -L../../shared/mosaic
LDADD = -ltoolsshared -lshared -lnetcdf -lm

if USE_NETCDF_DIR
AM_CPPFLAGS += -I@NETCDFDIR@/include
AM_LDFLAGS += -L@NETCDFDIR@/lib
LDADD += -lnetcdf
else

AM_LDFLAGS += ${top_srcdir}/../liblib/libnetcdf.la @EXTERN_LDFLAGS@
AM_CPPFLAGS += -I$(top_srcdir)/../liblib @EXTERN_CFLAGS@

LDADD +=  ${top_srcdir}/../liblib/libnetcdf.la @EXTERN_LDFLAGS@

endif

