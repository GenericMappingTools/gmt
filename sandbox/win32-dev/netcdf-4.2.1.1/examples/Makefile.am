## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This is the main automake file for netCDF. It builds the different
# netcdf directories. Not all directories are built, depending on the
# options selected during configure.

# Build ncgen/ncdump.
if BUILD_UTILITIES
CDL_DIR = CDL
endif

# These are the subdirectories that will be built. 
SUBDIRS = C $(CDL_DIR)
