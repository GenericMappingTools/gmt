# This is part of Unidata's netCDF package.  Copyright 2005-2011, see
# the COPYRIGHT file for more information.

# This automake file generates the Makefile to build netCDF-4.

include $(top_srcdir)/lib_flags.am

libnetcdf4_la_CPPFLAGS = ${AM_CPPFLAGS}

# This turns on declspec magic in netcdf.h for windows DLLs.
if BUILD_DLL
libnetcdf4_la_CPPFLAGS += -DDLL_EXPORT
endif

# This is our output. The netCDF-4 convenience library.
noinst_LTLIBRARIES = libnetcdf4.la
libnetcdf4_la_SOURCES = nc4internal.h nc4dispatch.c nc4dispatch.h	\
nc4attr.c nc4dim.c nc4file.c nc4grp.c nc4type.c nc4var.c ncfunc.c	\
nc4internal.c nc4hdf.c error4.c







