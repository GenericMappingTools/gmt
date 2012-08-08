## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This Makefile controls the building of the dispatch layer of the
# netCDF library. The dispatch layer decides whether to call the
# netcdf-classic code, netcdf-4 code, nc3 dap code, or nc4 dap
# code. It also contains code that sit above the dispatch layer, like
# the v2 API.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# This is our output, the dispatch convenience library.
noinst_LTLIBRARIES = libdispatch.la
libdispatch_la_CPPFLAGS = ${AM_CPPFLAGS}

# The source files.
libdispatch_la_SOURCES = parallel.c copy.c file.c dim.c att.c error.c	\
var.c dispatch.c nc_uri.c nclist.c ncbytes.c \
nchashmap.c \
nclog.c nclog.h

# Add functions only found in netCDF-4.
if USE_NETCDF4
libdispatch_la_SOURCES += nc4.c
endif # USE_NETCDF4

# Turn on pre-processor flag when building a DLL for windows.
if BUILD_DLL
libdispatch_la_CPPFLAGS += -DDLL_EXPORT
endif # BUILD_DLL

# Add V2 API convenience library if needed.
if BUILD_V2
noinst_LTLIBRARIES += libnetcdf2.la 
libnetcdf2_la_SOURCES = v2i.c
libnetcdf2_la_CPPFLAGS = ${AM_CPPFLAGS} -DDLL_EXPORT
endif # BUILD_V2

