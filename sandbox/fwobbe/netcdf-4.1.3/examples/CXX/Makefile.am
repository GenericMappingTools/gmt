## This is a automake file, part of Unidata's netCDF package.
# Copyright 2006, see the COPYRIGHT file for more information.  

# This file builds CXX examples.

# $Id$

# We will need to find the C++ header files.
AM_CXXFLAGS = -I$(top_srcdir)/cxx 
AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/cxx
AM_LDFLAGS=
LDADD =

AM_CPPFLAGS += -I$(top_builddir)/liblib
AM_CXXFLAGS += -I$(top_builddir)/liblib
AM_LDFLAGS += ${top_builddir}/cxx/libnetcdf_c++.la	\
${top_builddir}/liblib/libnetcdf.la

# These are the example programs.
TESTPROGRAMS = simple_xy_wr simple_xy_rd sfc_pres_temp_wr	\
sfc_pres_temp_rd pres_temp_4D_wr pres_temp_4D_rd
check_PROGRAMS = $(TESTPROGRAMS)

# Ship also this script, which compares the output of these C++
# examples with the output of the C examples. The output data files
# are identical.
EXTRA_DIST = do_comps.sh

# Here's the source code for the programs.
simple_xy_wr_SOURCES = simple_xy_wr.cpp
simple_xy_rd_SOURCES = simple_xy_rd.cpp
sfc_pres_temp_wr_SOURCES = sfc_pres_temp_wr.cpp
sfc_pres_temp_rd_SOURCES = sfc_pres_temp_rd.cpp
pres_temp_4D_wr_SOURCES = pres_temp_4D_wr.cpp
pres_temp_4D_rd_SOURCES = pres_temp_4D_rd.cpp

# Run all example programs, plus the do_comps.sh script, which checks
# that all the output files are the same as the C example output
# files.
TESTS = $(TESTPROGRAMS) do_comps.sh

# Cleaning up files created during the process.
CLEANFILES = simple_xy.nc sfc_pres_temp.nc pres_temp_4D.nc

