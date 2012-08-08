# This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.
# This file builds and runs the ncgen program.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# Link to the netCDF library.
ncgen3_LDADD = ${top_builddir}/liblib/libnetcdf.la 

# Build ncgen from the listed sources.
bin_PROGRAMS = ncgen3
ncgen3_SOURCES = main.c load.c escapes.c getfill.c init.c	\
genlib.c generic.h ncgen.h genlib.h ncgentab.h ncgentab.c

man_MANS = ncgen3.1

# These files all need to be distributed.
EXTRA_DIST = ncgen.y ncgenyy.c ncgen.l c0.cdl run_tests.sh	\
run_nc4_tests.sh $(man_MANS)

# There is a netcdf classic and netcdf-4 test script, but don't run
# them for DLL builds.
if !BUILD_DLL
TESTS = run_tests.sh 
if USE_NETCDF4
TESTS += run_nc4_tests.sh
endif # USE_NETCDF4
endif # !BUILD_DLL

CLEANFILES = c0.nc c0_64.nc c0_4.nc c0_4c.nc

# This is used if someone wants to rebuild ncgenyy.c or ncgentab.c
# Otherwise never invoked, but records how to do it. Don't forget to
# manually insert #include <config.h> in ncgenyy.c!
ncgenyy.c: ncgen.l
	flex -Pncg -8 ncgen.l
	mv lex.ncg.c ncgenyy.c

ncgentab.c ncgentab.h: ncgen.y
	bison -pncg -d ncgen.y
	mv ncgen.tab.c ncgentab.c
	mv ncgen.tab.h ncgentab.h
