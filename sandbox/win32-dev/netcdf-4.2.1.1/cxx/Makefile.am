## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.
# This file builds the C++ interface.

include $(top_srcdir)/lib_flags.am

# nctst creates a simple data file in each format, then uses ncdump to
# create CDL files, which are compared with some reference files
# shipped with the code. tst_failure is a program that is intended to
# fail.
TESTFILES = nctst$(EXEEXT) tst_failure$(EXEEXT) 
tst_many_writes_SOURCES = tst_many_writes.cpp
nctst_SOURCES = nctst.cpp
tst_failure_SOURCES = tst_failure.cpp
XFAIL_TESTS = tst_failure$(EXEEXT)

if USE_NETCDF4
if BUILD_BENCHMARKS	
# This is for valgrind script.
TESTS_ENVIRONMENT=USE_NETCDF4=1
# This is a netCDF-4 test.
TESTFILES += tst_many_writes$(EXEEXT)
#tst_many_writes_SOURCES = tst_many_writes.cpp
endif # BUILD_BENCHMARKS
endif # USE_NETCDF4

# Build and run these tests.
check_PROGRAMS = $(TESTFILES)
TESTS = $(TESTFILES)

# This will run a bunch of the test programs with valgrind, the memory
# checking tool. (Valgrind must be present for this to work.)
if USE_VALGRIND_TESTS
TESTS += run_valgrind_tests.sh
endif # USE_VALGRIND_TESTS

# If the utilities have been built, run these extra tests.
if !BUILD_DLL
if BUILD_UTILITIES
TESTS += run_nc_tests.sh
endif
endif

AM_CPPFLAGS += -I${top_srcdir}/liblib
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la
LDADD = $(top_builddir)/cxx/libnetcdf_c++.la  ${AM_LDFLAGS}

# These headers will be installed in the users header directory.
include_HEADERS = netcdfcpp.h ncvalues.h netcdf.hh

# This is our output library. 
lib_LTLIBRARIES = libnetcdf_c++.la
libnetcdf_c___la_SOURCES = netcdf.cpp ncvalues.cpp
libnetcdf_c___la_LDFLAGS = -version-number 4:1:0
libnetcdf_c___la_CPPFLAGS = ${AM_CPPFLAGS}
libnetcdf_c___la_LIBADD = $(top_builddir)/liblib/libnetcdf.la

# If we are building the dll (under MinGW) then set the correct flags.
if BUILD_DLL
libnetcdf_c___la_CPPFLAGS += -DDLL_EXPORT
libnetcdf_c___la_LDFLAGS += -no-undefined	\
-Wl,--output-def,netcdf_c++dll.def
endif

CLEANFILES = nctst_classic.nc nctst_64bit_offset.nc nctst_netcdf4.nc	\
nctst_netcdf4_classic.nc nctst_classic.cdl nctst_64bit_offset.cdl	\
tst_*.nc 

EXTRA_DIST = ref_nctst.cdl ref_nctst_64bit_offset.cdl			\
ref_nctst_netcdf4.cdl ref_nctst_netcdf4_classic.cdl run_nc_tests.sh	\
run_nc4_tests.sh run_valgrind_tests.sh

