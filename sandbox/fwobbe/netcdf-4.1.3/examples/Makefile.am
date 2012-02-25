## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This is the main automake file for netCDF. It builds the different
# netcdf directories. Not all directories are built, depending on the
# options selected during configure.

# $Id$

# If the user is building the C API, build the C examples.
if BUILD_C
C_DIR = C
endif

# If the user is building the F77 API, build the F77 examples.
if BUILD_F77
F77_DIR = F77
endif

# If the user is building the F90 API, build the F90 examples.
if BUILD_F90
F90_DIR = F90
endif

# If the user is building the C++ API, build the CXX examples.
if BUILD_CXX
CXX_DIR = CXX
endif

# Build the new netCDF-4 C++ API.
if BUILD_CXX4
CXX4_DIR = CXX4
endif

# Build ncgen/ncdump.
if BUILD_UTILITIES
CDL_DIR = CDL
endif

# These are the subdirectories that will be built. 
SUBDIRS = $(C_DIR) $(CDL_DIR) $(F77_DIR) $(F90_DIR) $(CXX_DIR)	\
$(CXX4_DIR)
