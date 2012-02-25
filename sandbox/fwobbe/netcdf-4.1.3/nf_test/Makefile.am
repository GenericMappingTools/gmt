# This is a automake file, part of Unidata's netCDF package.
# Copyright 2005-2011, see the COPYRIGHT file for more information.

# This file builds and runs the f77 and f90 tests.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

AM_FFLAGS = -I$(top_srcdir)/fortran ${AM_CPPFLAGS}

if BUILD_F90
# Some fortran compilers think your files should not have an .f90
# extension! The value of FCFLAGS_f90 is set in the configure script,
# based on the fortran compiler.
AM_FCFLAGS = $(FCFLAGS_f90) -I$(top_srcdir)/fortran -I$(top_builddir)/f90

# This is the netCDF-3 F90 test.
AM_FFLAGS += @MOD_FLAG@../f90
AM_FCFLAGS += @MOD_FLAG@../f90

AM_FCFLAGS +=  ${AM_CPPFLAGS}
endif #BUILD_F90

# All tests need to link to fortran and C libraries.
LDADD = ${top_builddir}/fortran/libnetcdff.la 

# nf_test is the main test program. 
check_PROGRAMS = nf_test
nf_test_SOURCES = test_get.m4 test_put.m4 nf_error.F nf_test.F	\
test_read.F test_write.F util.F fortlib.c tests.inc
TESTS = nf_test

if BUILD_UTILITIES
# The create_fills shell creates the file fills.nc, needed by later
# tests.
TESTS += create_fills.sh
endif # BUILD_UTILITIES

# Did the user build the V2 F77 API? If so, run this test.
if BUILD_V2
check_PROGRAMS += tst_f77_v2
tst_f77_v2_SOURCES = tst_f77_v2.F
TESTS += tst_f77_v2
endif # BUILD_V2

# Is the user building netCDF-4?
if USE_NETCDF4

# Add these netCDF-4 f77 test programs.
check_PROGRAMS += ftst_vars ftst_vars2 ftst_vars3 ftst_vars4 ftst_vars5	\
ftst_vars6 ftst_types ftst_types2 ftst_types3 ftst_groups
ftst_types_SOURCES = ftst_types.F fortlib.c handle_err.F
ftst_types2_SOURCES = ftst_types2.F fortlib.c handle_err.F
ftst_types3_SOURCES = ftst_types3.F fortlib.c handle_err.F
ftst_vars_SOURCES = ftst_vars.F fortlib.c handle_err.F
ftst_vars2_SOURCES = ftst_vars2.F fortlib.c handle_err.F
ftst_vars3_SOURCES = ftst_vars3.F fortlib.c handle_err.F
ftst_vars4_SOURCES = ftst_vars4.F fortlib.c handle_err.F
ftst_vars5_SOURCES = ftst_vars5.F fortlib.c handle_err.F
ftst_vars6_SOURCES = ftst_vars6.F fortlib.c handle_err.F
ftst_groups_SOURCES = ftst_groups.F fortlib.c handle_err.F
TESTS += ftst_vars ftst_vars2 ftst_vars3 ftst_vars4 ftst_vars5	\
ftst_vars6 ftst_types ftst_types2 ftst_types3 ftst_groups

# Add these netCDF-4 f90 test programs.
if BUILD_F90
check_PROGRAMS += f90tst_vars tst_types tst_types2 f90tst_vars_vlen	\
tst_f90_nc4 f90tst_grps f90tst_fill f90tst_fill2 f90tst_vars3		\
f90tst_vars4
tst_f90_nc4_SOURCES = tst_f90_nc4.f90
f90tst_vars_SOURCES = f90tst_vars.f90
tst_types_SOURCES = tst_types.f90
tst_types2_SOURCES = tst_types2.f90
f90tst_vars_vlen_SOURCES = f90tst_vars_vlen.f90
f90tst_grps_SOURCES = f90tst_grps.f90
f90tst_fill_SOURCES = f90tst_fill.f90
f90tst_fill2_SOURCES = f90tst_fill2.f90
f90tst_vars3_SOURCES = f90tst_vars3.f90
f90tst_vars4_SOURCES = f90tst_vars4.f90
TESTS += f90tst_vars tst_types tst_types2 f90tst_vars_vlen tst_f90_nc4	\
f90tst_grps f90tst_fill f90tst_fill2 f90tst_vars3 f90tst_vars4

if USE_SZIP
check_PROGRAMS += f90tst_vars2 
TESTS += f90tst_vars2
f90tst_vars2_SOURCES = f90tst_vars2.f90
endif

# This is a netCDF-4 V2 test program.
if BUILD_V2
check_PROGRAMS += ftst_v2
ftst_v2_SOURCES = ftst_v2.F 
TESTS += ftst_v2
endif # BUILD_V2

# This is the netCDF-4 F90 large file test.
if LARGE_FILE_TESTS
check_PROGRAMS += tst_flarge
tst_flarge_SOURCES = tst_flarge.f90
TESTS += tst_flarge
endif # LARGE_FILE_TESTS

# This is an f90 benchmark.
if BUILD_BENCHMARKS
check_PROGRAMS += tst_io
tst_io_SOURCES = tst_io.f90
TESTS += tst_io
endif #BUILD_BENCHMARKS

# Test parallel I/O.
if TEST_PARALLEL
check_PROGRAMS += f90tst_parallel f90tst_parallel2 f90tst_parallel3	\
f90tst_nc4_par
#f90tst_parallel_fill
f90tst_parallel_SOURCES = f90tst_parallel.f90
f90tst_parallel2_SOURCES = f90tst_parallel2.f90
f90tst_parallel3_SOURCES = f90tst_parallel3.f90
f90tst_nc4_par_SOURCES = f90tst_nc4_par.f90
#f90tst_parallel_fill_SOURCES = f90tst_parallel_fill.f90
TESTS += run_f90_par_test.sh
endif # TEST_PARALLEL

endif #BUILD_F90

# Test parallel I/O for F77.
if TEST_F77
if TEST_PARALLEL
check_PROGRAMS += ftst_parallel ftst_parallel_nasa
ftst_parallel_SOURCES = ftst_parallel.F
ftst_parallel_nasa_SOURCES = ftst_parallel_nasa.F
TESTS += run_f77_par_test.sh
endif # TEST_PARALLEL
endif # TEST_F77

endif #USE_NETCDF4

if BUILD_C
# This is the fortran v2 test. It depends on the utilities being built
# to generate it's input file.
if BUILD_V2
if BUILD_UTILITIES
check_PROGRAMS += ftest
ftest_SOURCES = ftest.F fortlib.c 
TESTS += ftest
endif # BUILD_UTILITIES
endif # BUILD_V2
endif #BUILD_C

if BUILD_F90
check_PROGRAMS += tst_f90
tst_f90_SOURCES = tst_f90.f90
TESTS += tst_f90
endif

# Tell make how to turn .m4 files into .F files.
.m4.F:
	m4 $(M4FLAGS) $< >$@

# test_get.F and test_put.f need to be distributed, so that the user
# need not have m4. fills.nc is used by test program ftest.
EXTRA_DIST = test_get.F test_put.F fills.cdl create_fills.sh	\
run_f90_par_test.sh run_f77_par_test.sh

# Cleaning up files created during the process.
CLEANFILES = scratch.nc test.nc copy.nc fills.nc tst_*.nc ftst_*.nc	\
f90tst_*.nc
MAINTAINERCLEANFILES = test_get.F test_put.F
