## This is an automake file, part of Unidata's NetCDF CF Library
## package. Copyright 2009, see the COPYRIGHT file for more
## information.

# This automake file is in charge of building the gridspec
# tools/shared convenience library.

noinst_LTLIBRARIES = libshared.la 
libshared_la_SOURCES = constant.h create_xgrid.c create_xgrid.h	\
gradient_c2l.c gradient_c2l.h interp.c interp.h mosaic_util.c	\
mosaic_util.h read_mosaic.c read_mosaic.h

AM_CPPFLAGS =
AM_LDFLAGS = 
LDADD =

AM_CPPFLAGS += -I${top_srcdir}/src

AM_CPPFLAGS += ${top_srcdir}/../liblib
AM_LDFLAGS += ${top_builddir}/../liblib/libnetcdf.la

AM_LDFLAGS += @EXTERN_LDFLAGS@ -lm





