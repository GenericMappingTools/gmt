## This is a automake file, part of Unidata's netCDF package.
# Copyright 2006, see the COPYRIGHT file for more information.  

# This file builds F77 examples.

AM_LDFLAGS = 
AM_FFLAGS = 

AM_LDFLAGS += ${top_builddir}/fortran/libnetcdff.la

AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la 

AM_FFLAGS += -I$(top_srcdir)/fortran -I$(top_builddir)/fortran

# These are the example programs.
TESTPROGRAMS = simple_xy_wr simple_xy_rd sfc_pres_temp_wr	\
sfc_pres_temp_rd pres_temp_4D_wr pres_temp_4D_rd

if USE_NETCDF4
TESTPROGRAMS += simple_xy_nc4_wr simple_xy_nc4_rd
simple_xy_nc4_wr_SOURCES = simple_xy_nc4_wr.f
simple_xy_nc4_rd_SOURCES = simple_xy_nc4_rd.f
endif

check_PROGRAMS = $(TESTPROGRAMS)

EXTRA_DIST = do_comps.sh

# Here's the source code for the programs.
simple_xy_wr_SOURCES = simple_xy_wr.f
simple_xy_rd_SOURCES = simple_xy_rd.f
sfc_pres_temp_wr_SOURCES = sfc_pres_temp_wr.f
sfc_pres_temp_rd_SOURCES = sfc_pres_temp_rd.f
pres_temp_4D_wr_SOURCES = pres_temp_4D_wr.f
pres_temp_4D_rd_SOURCES = pres_temp_4D_rd.f

# Run all test programs, plus the do_comps.sh script, which checks
# that all the output files are the same as the C example output
# files.
TESTS = $(TESTPROGRAMS) do_comps.sh

# Cleaning up files created during the process.
CLEANFILES = simple_xy.nc sfc_pres_temp.nc pres_temp_4D.nc	\
simple_xy_nc4.nc
