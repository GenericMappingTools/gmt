## This is an automake file, part of Unidata's NetCDF CF Library
## package. Copyright 2009, see the COPYRIGHT file for more
## information.

# This automake file is in charge of building the gridspec
# tools/shared convenience library.

# $Id$

noinst_LTLIBRARIES = libtoolsshared.la 
libtoolsshared_la_SOURCES = mpp.c mpp_domain.c mpp_domain.h mpp.h	\
mpp_io.c mpp_io.h tool_util.c tool_util.h create_lonlat_grid.c		\
create_gnomonic_cubic_grid.c create_grid_from_file.c			\
create_conformal_cubic_grid.c fregrid_util.c conserve_interp.c		\
bilinear_interp.c topog.c get_contact.c create_vgrid.c			\
transfer_to_mosaic.c

# Initalize these.
AM_LDFLAGS =
LDADD =

AM_CPPFLAGS = -I${top_srcdir}/gridspec/shared				\
-I${top_srcdir}/gridspec/shared/mosaic -I${top_srcdir}/make_hgrid	\
-I${top_srcdir}/gridspec/tools/fregrid					\
-I${top_srcdir}/gridspec/tools/make_coupler_mosaic			\
-I${top_srcdir}/gridspec/tools/make_topog				\
-I${top_srcdir}/gridspec/tools/make_hgrid				\
-I${top_srcdir}/gridspec/tools/make_solo_mosaic				\
-I${top_srcdir}/gridspec/tools/make_vgrid				\
-I${top_srcdir}/gridspec/tools/river_regrid				\
-I${top_srcdir}/gridspec/tools/transfer_to_mosaic_grid

if USE_NETCDF_DIR
AM_CPPFLAGS += -I@NETCDFDIR@/include
AM_LDFLAGS += -L@NETCDFDIR@/lib
else
AM_CPPFLAGS += -I${top_srcdir}/.. -I${top_srcdir}/../liblib 
AM_LDFLAGS += ${top_builddir}/../liblib/libnetcdf.la
endif

AM_LDFLAGS += @EXTERN_LDFLAGS@ -lm
