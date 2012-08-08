## This is an automake file, part of Unidata's NetCDF CF Library
## package. Copyright 2009, see the COPYRIGHT file for more
## information.

# This automake file is in charge of building the gridspec make_hgrid
# tools.

bin_PROGRAMS = make_hgrid gs_make_hgrid
make_hgrid_SOURCES = make_hgrid.c create_conformal_cubic_grid.c		\
create_gnomonic_cubic_grid.c create_grid_from_file.c create_hgrid.h	\
create_lonlat_grid.c
gs_make_hgrid_SOURCES = gs_make_hgrid.c create_conformal_cubic_grid.c	\
create_gnomonic_cubic_grid.c create_grid_from_file.c create_hgrid.h	\
create_lonlat_grid.c

AM_CPPFLAGS = -I${top_srcdir}/shared -I${top_srcdir}/shared/mosaic	\
-I${top_srcdir}/gridspec/shared -I${top_srcdir}/gridspec/shared/mosaic	\
-I${top_srcdir}/make_hgrid -I${top_srcdir}/gridspec/tools/fregrid	\
-I${top_srcdir}/gridspec/tools/shared					\
-I${top_srcdir}/gridspec/tools/make_coupler_mosaic			\
-I${top_srcdir}/gridspec/tools/make_topog				\
-I${top_srcdir}/gridspec/tools/make_hgrid				\
-I${top_srcdir}/gridspec/tools/make_solo_mosaic				\
-I${top_srcdir}/gridspec/tools/make_vgrid				\
-I${top_srcdir}/gridspec/tools/river_regrid				\
-I${top_srcdir}/gridspec/tools/transfer_to_mosaic_grid

AM_LDFLAGS = -L${top_builddir}/gridspec/shared			\
-L${top_builddir}/gridspec/shared/mosaic -ltoolsshared -lshared

if USE_NETCDF_DIR
AM_CPPFLAGS += -I@NETCDFDIR@/include
AM_LDFLAGS += -L@NETCDFDIR@/lib
else
AM_CPPFLAGS += -I${top_srcdir}/.. -I${top_srcdir}/../liblib
AM_LDFLAGS += ${top_builddir}/../liblib/libnetcdf.la
endif

AM_LDFLAGS += @EXTERN_LDFLAGS@ -lm

TESTS = run_tests
EXTRA_DIST = run_tests.in
CLEANFILES = horizontal_grid.nc gs_horizontal_grid.nc	\
horizontal_grid.cdl gs_horizontal_grid.cdl
DISTCLEANFILES = run_tests


