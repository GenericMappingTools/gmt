# This is a automake file, part of Unidata's netCDF package.
# Copyright 2006, see the COPYRIGHT file for more information.

# This file builds the C examples.

# $Id$

LDADD = -lm
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS =

# These are the netCDF-3 examples. 
TESTPROGRAMS = simple_xy_wr simple_xy_rd sfc_pres_temp_wr	\
sfc_pres_temp_rd pres_temp_4D_wr pres_temp_4D_rd

# To build netcdf-4, or not to build netcdf-4, that is the question...
if USE_NETCDF4
# These are the extra netCDF-4 examples. 
TESTPROGRAMS += simple_nc4_wr simple_nc4_rd simple_xy_nc4_wr	\
simple_xy_nc4_rd
endif #USE_NETCDF4

AM_CPPFLAGS += -I$(top_builddir)/liblib
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la

check_PROGRAMS = $(TESTPROGRAMS)
TESTS = $(TESTPROGRAMS)

# This will run a bunch of the test programs with valgrind, the memory
# checking tool. (Valgrind must be present for this to work.)
if USE_VALGRIND_TESTS
TESTS += run_valgrind_tests.sh
endif # USE_VALGRIND_TESTS
if USE_NETCDF4
if USE_VALGRIND_TESTS
TESTS += run_nc4_valgrind_tests.sh
endif # USE_VALGRIND_TESTS
endif # USE_NETCDF4

# These files are created by the tests.
CLEANFILES = sfc_pres_temp.nc simple_xy.nc pres_temp_4D.nc	\
simple_nc4.nc simple_xy_nc4.nc

EXTRA_DIST = run_valgrind_tests.sh run_nc4_valgrind_tests.sh
