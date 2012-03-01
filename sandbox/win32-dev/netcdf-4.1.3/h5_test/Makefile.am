# This is part of the netCDF package.
# Copyright 2011 University Corporation for Atmospheric Research/Unidata
# See COPYRIGHT file for conditions of use.
# 
# This entire directory will be skipped, unless the configure script
# is run with --enable-netcdf-4. This directory contains tests that
# only use HDF5; these tests don't use netCDF at all.
#
# If one of these tests fails, then netCDF-4 will not work correctly.

# Set AM_CPPFLAGS and AM_LDFLAGS based on user choices.
include $(top_srcdir)/lib_flags.am

# These programs test HDF5 to make sure it will do (some of) the stuff
# that netCDF-4 needs.
H5TESTS = tst_h_files tst_h_files2 tst_h_files4 tst_h_atts		\
tst_h_atts3 tst_h_atts4 tst_h_vars tst_h_vars2 tst_h_vars3 tst_h_grps	\
tst_h_compounds tst_h_compounds2 tst_h_wrt_cmp tst_h_rd_cmp tst_h_vl	\
tst_h_opaques tst_h_strings tst_h_strings1 tst_h_strings2 tst_h_ints	\
tst_h_dimscales tst_h_dimscales1 tst_h_dimscales2 tst_h_dimscales3	\
tst_h_enums tst_h_dimscales4 #tst_h_filters

# If benchmarks were turned on, build and run a bunch more tests.
if BUILD_BENCHMARKS
H5TESTS += tst_h_mem
endif # BUILD_BENCHMARKS

# List of tests to compile.
check_PROGRAMS = $(H5TESTS)

# List of tests to run.
TESTS = $(H5TESTS)

# The parallel program is run from a script.
if TEST_PARALLEL
check_PROGRAMS += tst_h_par
TESTS += run_par_tests.sh
endif

# This will run a bunch of the test programs with valgrind, the memory
# checking tool. (Valgrind must be present for this to work.)
if USE_VALGRIND_TESTS
TESTS += run_valgrind_tests.sh 
endif # USE_VALGRIND_TESTS

# We must include these files in the distribution.
EXTRA_DIST = run_par_tests.sh run_valgrind_tests.sh		\
ref_tst_h_compounds.h5 ref_tst_h_compounds2.h5 run_par_tests.sh	\
run_valgrind_tests.sh ref_tst_compounds.nc

# Clean up test results.
CLEANFILES = tst_h_*.h5





