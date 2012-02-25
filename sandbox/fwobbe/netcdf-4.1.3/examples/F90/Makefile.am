## This is a automake file, part of Unidata's netCDF package.
# Copyright 2006, see the COPYRIGHT file for more information.  

# This file builds F90 examples.

# Some fortran compilers think your files should not have an .f90
# extension! The value of FCFLAGS_f90 is set in the configure script,
# based on the fortran compiler.
AM_FCFLAGS = $(FCFLAGS_f90)
AM_F90FLAGS = $(FCFLAGS_f90)
AM_FFLAGS = $(FCFLAGS_f90)
AM_LDFLAGS = 

# Tell the compiler where to find the .mod files. The .mod file is
# built, and therefore found in the build directory, not the source
# directory.
AM_FCFLAGS += -I$(top_srcdir) -I$(top_srcdir)/fortran	\
-I$(top_builddir)/f90 @MOD_FLAG@$(top_builddir)/f90
AM_FFLAGS += ${AM_FCFLAGS}

AM_LDFLAGS += ${top_builddir}/fortran/libnetcdff.la
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la

# These are the example programs.
TESTPROGRAMS = simple_xy_wr simple_xy_rd sfc_pres_temp_wr	\
sfc_pres_temp_rd pres_temp_4D_wr pres_temp_4D_rd

TESTSCRIPTS = do_comps.sh

# Here's the source code for the programs.
simple_xy_wr_SOURCES = simple_xy_wr.f90
simple_xy_rd_SOURCES = simple_xy_rd.f90
sfc_pres_temp_wr_SOURCES = sfc_pres_temp_wr.f90
sfc_pres_temp_rd_SOURCES = sfc_pres_temp_rd.f90
pres_temp_4D_wr_SOURCES = pres_temp_4D_wr.f90
pres_temp_4D_rd_SOURCES = pres_temp_4D_rd.f90

# Cleaning up files created during the process.
CLEANFILES = simple_xy.nc sfc_pres_temp.nc pres_temp_4D.nc 

if USE_NETCDF4

if EXTRA_EXAMPLE_TESTS
# This will cause netCDF-4 versions of the example programs to be
# built.
BUILT_SOURCES = nc4_simple_xy_wr.f90 nc4_sfc_pres_temp_wr.f90	\
nc4_pres_temp_4D_wr.f90

# How to build the example.
nc4_simple_xy_wr_SOURCES = nc4_simple_xy_wr.f90
nc4_sfc_pres_temp_wr_SOURCES = nc4_sfc_pres_temp_wr.f90
nc4_pres_temp_4D_wr_SOURCES = nc4_pres_temp_4D_wr.f90

# Add example to the tests run.
TESTPROGRAMS += nc4_simple_xy_wr nc4_sfc_pres_temp_wr	\
nc4_pres_temp_4D_wr

# Add this test script.
TESTSCRIPTS += run_nc4_comps.sh

# Make sure all the files created by our netcdf-4 tests get cleaned.
CLEANFILES += nc4_simple_xy_wr.f90 simple_xy.cdl			\
nc4_sfc_pres_temp_wr.f90 sfc_pres_temp.cdl nc4_pres_temp_4d_wr.f90	\
pres_temp_4d.cdl nc4_simple_xy.nc nc4_pres_temp_4d.nc			\
nc4_sfc_pres_temp.nc
endif #EXTRA_EXAMPLE_TESTS

# Optionally add parallel i/o examples.
if TEST_PARALLEL
TESTPROGRAMS += simple_xy_par_wr simple_xy_par_rd
simple_xy_par_wr_SOURCES = simple_xy_par_wr.f90
simple_xy_par_rd_SOURCES = simple_xy_par_rd.f90
TESTSCRIPTS += run_f90_par_examples.sh
CLEANFILES += simple_xy_par.nc
endif # TEST_PARALLEL

# NetCDF-4 examples.
TESTPROGRAMS += simple_xy_nc4_wr simple_xy_nc4_rd
simple_xy_nc4_wr_SOURCES = simple_xy_nc4_wr.f90
simple_xy_nc4_rd_SOURCES = simple_xy_nc4_rd.f90
CLEANFILES += simple_xy_nc4.nc
endif #USE_NETCDF4

# This target tells how to build one of the netCDF-4 examples.
# Note old version used sed -i, but -i is not universally recognized
nc4_simple_xy_wr.f90:
	sed -e 's/nf90_clobber/nf90_hdf5/' simple_xy_wr.f90 \
	| sed -e 's/simple_xy/nc4_simple_xy/' > nc4_simple_xy_wr.f90

# This target tells how to build one of the netCDF-4 examples.
nc4_sfc_pres_temp_wr.f90:
	sed -e 's/nf90_clobber/nf90_hdf5/' sfc_pres_temp_wr.f90 \
	| sed -e 's/sfc_pres_temp/nc4_sfc_pres_temp/' > nc4_sfc_pres_temp_wr.f90

# This target tells how to build one of the netCDF-4 examples.
nc4_pres_temp_4D_wr.f90:
	sed -e 's/nf90_clobber/nf90_hdf5/' pres_temp_4D_wr.f90 \
	| sed -e 's/pres_temp_4D/nc4_pres_temp_4D/' > nc4_pres_temp_4D_wr.f90

# Build these test programs.
check_PROGRAMS = $(TESTPROGRAMS)

# Make sure the script which compares file outputs is included in the
# dist.
EXTRA_DIST = do_comps.sh run_nc4_comps.sh run_f90_par_examples.sh

# Run all test programs, plus the do_comps.sh script, which checks
# that all the output files are the same as the C example output
# files.
TESTS = $(TESTPROGRAMS) $(TESTSCRIPTS)
