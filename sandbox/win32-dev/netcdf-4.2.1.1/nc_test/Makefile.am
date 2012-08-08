## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This file builds and runs the nc_test program, which tests the
# netCDF-3 API for all formats.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# These files are created by the tests.
CLEANFILES = nc_test_classic.nc nc_test_64bit.nc nc_test_netcdf4.nc	\
tst_*.nc t_nc.nc large_files.nc quick_large_files.nc \
tst_diskless.nc tst_diskless2.nc \
tst_diskless3.nc tst_diskless3_file.cdl tst_diskless3_memory.cdl \
tst_diskless4.cdl tst_diskless4.nc

# These are the tests which are always run.
TESTPROGRAMS = t_nc tst_small nc_test tst_misc tst_norm tst_names	\
tst_nofill tst_nofill2 tst_nofill3 tst_atts3

if USE_NETCDF4
TESTPROGRAMS += tst_atts
endif

# if EXTRA_TESTS
# TESTPROGRAMS += 
# endif # EXTRA_TESTS

# These are the source files for the main workhorse test program,
# nc_test. If you pass nc_test, you are doing well.
nc_test_SOURCES = nc_test.c error.c test_get.c test_put.c \
test_read.c test_write.c util.c error.h tests.h 

LDADD = ${top_builddir}/liblib/libnetcdf.la 
AM_CPPFLAGS += -I$(top_builddir)/liblib

# If the user asked for large file tests, then add them.
if LARGE_FILE_TESTS
TESTPROGRAMS += quick_large_files tst_big_var6 tst_big_var2	\
tst_big_rvar tst_big_var tst_large large_files
endif # LARGE_FILE_TESTS

if BUILD_BENCHMARKS
TESTPROGRAMS += testnc3perf
testnc3perf_SOURCES = testnc3perf.c
CLEANFILES += benchmark.nc
endif


# Set up the tests.
check_PROGRAMS = $(TESTPROGRAMS)

# Build Diskless test helpers
if BUILD_DISKLESS
check_PROGRAMS += tst_diskless tst_diskless3 tst_diskless4
if USE_NETCDF4
check_PROGRAMS += tst_diskless2
endif
endif

TESTS = $(TESTPROGRAMS)

if BUILD_DISKLESS
TESTS += run_diskless.sh
if BUILD_MMAP
TESTS += run_mmap.sh
endif
if LARGE_FILE_TESTS
TESTS += run_diskless2.sh
endif
endif

# This will run a bunch of the test programs with valgrind, the memory
# checking tool. (Valgrind must be present for this to work.)
if USE_VALGRIND_TESTS
if USE_NETCDF4
TESTS_ENVIRONMENT=USE_NETCDF4=1
else
TESTS_ENVIRONMENT=USE_NETCDF4=0
endif
TESTS += run_valgrind_tests.sh
endif # USE_VALGRIND_TESTS

# Distribute the .c files so that m4 isn't required on the users
# machine.
EXTRA_DIST = test_get.m4 test_put.m4 run_valgrind_tests.sh \
run_diskless.sh run_diskless2.sh run_mmap.sh

# ref_tst_diskless2.cdl is for diff comparison and to produce tst_diskless2.c
EXTRA_DIST += ref_tst_diskless2.cdl

# Only clean these on mainatiner-clean, because they require m4 to
# regenerate.
MAINTAINERCLEANFILES = test_get.c test_put.c

# This rule tells make how to turn our .m4 files into .c files.
.m4.c:
	m4 $(AM_M4FLAGS) $(M4FLAGS) $< >$@

