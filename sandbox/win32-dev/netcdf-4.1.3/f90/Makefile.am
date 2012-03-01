## This is an automake file, part of Unidata's netCDF package.
# Copyright 2005-2006, see the COPYRIGHT file for more information.

# This file controls the building of the fortran 90 API. All testing
# of the F90 API is done in the nf_test directory.

AM_LDFLAGS =

# This is the F90 API man page.
man_MANS = netcdf_f90.3
EXTRA_DIST = $(man_MANS)

# This, plus either netcdf4.f90 or netcdf.f90, is the main source.
libnetcdff90_la_SOURCES = typeSizes.f90 

# The file netcdf.f90 includes all of these.
libnetcdff90_la_DEPENDENCIES = netcdf_attributes.f90			\
netcdf_constants.f90 netcdf_dims.f90 netcdf_expanded.f90		\
netcdf_externals.f90 netcdf_file.f90 netcdf_overloads.f90		\
netcdf_text_variables.f90 netcdf_variables.f90 netcdf_visibility.f90	\
netcdf_eightbyte.f90

# Is the user building netCDF-4?
if USE_NETCDF4

# Use netcdf4.f90 to build the library.
libnetcdff90_la_SOURCES += netcdf4.f90 

# These are the extra netCDF-4 F90 source files.
libnetcdff90_la_DEPENDENCIES += netcdf4_func.f90 netcdf4_externals.f90	\
netcdf4_visibility.f90 netcdf4_constants.f90 netcdf4.f90		\
netcdf4_eightbyte.f90 netcdf4_variables.f90 netcdf4_file.f90

NETCDF_O = netcdf4.o

else # not USE_NETCDF4

# Use netcdf.f90 to get the library.
libnetcdff90_la_SOURCES += netcdf.f90 
libnetcdff90_la_DEPENDENCIES += netcdf.f90 netcdf3_file.f90

NETCDF_O = netcdf.o

endif # USE_NETCDF4

# Some fortran compilers change the file names of .mod files to all
# uppercase. Just to be special.
if UPPER_CASE_MOD
nodist_include_HEADERS = TYPESIZES.mod NETCDF.mod
TYPESIZES.mod: typeSizes.o
NETCDF.mod: $(NETCDF_O)
BUILT_SOURCES = TYPESIZES.mod
else
nodist_include_HEADERS = typesizes.mod netcdf.mod 
typesizes.mod: typeSizes.o
netcdf.mod: $(NETCDF_O)
BUILT_SOURCES = typesizes.mod
endif

# This is what we are building: a convenience library of F90 functions.
noinst_LTLIBRARIES = libnetcdff90.la

# Since these are included from either netcdf.f90 or netcdf4.f90, we
# have to explicitly make sure they make it into the distribution.
EXTRA_DIST += netcdf_attributes.f90 netcdf_constants.f90		\
netcdf_dims.f90 netcdf_expanded.f90 netcdf4_eightbyte.f90		\
netcdf_eightbyte.f90 netcdf_externals.f90 netcdf_file.f90		\
netcdf3_file.f90 netcdf4_file.f90 netcdf_overloads.f90			\
netcdf_text_variables.f90 netcdf_variables.f90 netcdf4_variables.f90	\
netcdf_visibility.f90 netcdf4_func.f90 netcdf4_externals.f90		\
netcdf4_visibility.f90 netcdf4_constants.f90

CLEANFILES = typesizes.mod netcdf.mod 

# Turn of parallel builds in this directory.
.NOTPARALLEL:

