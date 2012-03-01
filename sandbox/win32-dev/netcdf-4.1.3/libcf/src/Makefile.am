## This is an automake file, part of Unidata's NetCDF CF Library package.
# Copyright 2006, see the COPYRIGHT file for more information.

# THis automake file is in charge of building the src directory.

# $Id$

# Initalize these.
AM_CPPFLAGS = -I$(top_srcdir)/.. -I$(top_srcdir)/../include  -I${top_srcdir}/../liblib
AM_LDFLAGS = ${lib_LTLIBRARIES} ${top_builddir}/../ncdump/nctime.o	\
${top_builddir}/../liblib/libnetcdf.la @EXTERN_LDFLAGS@ -lm

# Tell automake that libcf.h is a header file we want to install.
include_HEADERS = libcf.h

# This is our output, the NetCDF CF Library.
lib_LTLIBRARIES = libcf.la

# These linker flags specify libtool version info.
libcf_la_LDFLAGS = -version-info 0:0:0

# This is the coordinate systems stuff.
libcf_la_SOURCES = nccoord.c libcf.h libcf_int.h cffiles.c cfvars.c	\
cfcvars.c cfsubsets.c cferror.c cferror.h cdTimeConv.c cdmsint.h	\
cdms.h cddrs.h drscdf.h cdunif.h

# These files are created by the tests.
CLEANFILES = libcf.mod tst_coords_classic.nc tst_coords_64bit.nc	\
tst_coords_netcdf4_classic.nc tst_coords_netcdf4.nc tst_vars.nc		\
tst_files.nc tst_cvars.nc tst_vert_sigma.nc tst_vert_hybrid_sigma.nc	\
tst_subsets_simple_sigma.nc tst_subsets_nw.nc tst_subsets_sw.nc		\
tst_subsets_se.nc tst_subsets_ne.nc tst_f03_file.nc

tst_coords_SOURCES = tst_coords.c nc_tests.h
tst_files_SOURCES = tst_files.c nc_tests.h
tst_vars_SOURCES = tst_vars.c nc_tests.h
tst_cvars_SOURCES = tst_cvars.c nc_tests.h
tst_vert_SOURCES = tst_vert.c nc_tests.h
tst_subsets_SOURCES = tst_subsets.c nc_tests.h

check_PROGRAMS = tst_coords tst_files tst_vars tst_cvars tst_vert	\
tst_subsets

if EXTRA_DATA_TESTS
check_PROGRAMS += tst_example_data
tst_example_data_SOURCES = tst_example_data.c nc_tests.h
endif # EXTRA_DATA_TESTS

# Rule to build Fortran 2003 code.
#cfcvars.o:cfcvsars.f90
#	$(FC) -c $(AM_FCFLAGS) $(FCFLAGS) -c $? -o $@

# tst_f03_file.o:tst_f03_file.f90
# 	$(FC) -c $(AM_FCFLAGS) $(FCFLAGS) -c $? -o $@

TESTS = $(check_PROGRAMS)



