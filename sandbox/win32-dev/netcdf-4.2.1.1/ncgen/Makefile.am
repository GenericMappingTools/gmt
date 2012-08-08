## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.
# This file builds and runs the ncgen program.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am
LDADD = ${top_builddir}/liblib/libnetcdf.la

# Build ncgen from the listed sources.
bin_PROGRAMS = ncgen

ncgen_SOURCES=main.c cdata.c bindata.c genchar.c cvt.c data.c debug.c	\
escapes.c genc.c genbin.c generr.c genlib.c getfill.c odom.c offsets.c	\
semantics.c ncgentab.c dump.c util.c bytebuffer.c list.c data.h		\
debug.h generr.h genlib.h includes.h ncgen.h odom.h offsets.h dump.h	\
util.h bytebuffer.h list.h genf77.c f77data.c genj.c jdata.c nciter.h	\
nciter.c ConvertUTF.c ConvertUTF.h

# This is the man page.
man_MANS = ncgen.1

# These files all need to be distributed.
EXTRA_DIST = ncgen.y ncgenyy.c ncgen.l $(man_MANS) internals.html	\
run_tests.sh run_nc4_tests.sh c0.cdl ref_camrun.cdl

# This shell script causes ncgen to build a classic and a 64-bit
# offset file from a cdl file shipped with the distribution.
if !BUILD_DLL
TESTS = run_tests.sh 
if USE_NETCDF4
TESTS += run_nc4_tests.sh
endif # USE_NETCDF4
endif # !BUILD_DLL

CLEANFILES = c0.nc c0_64.nc c0_4.nc c0_4c.nc ref_camrun.c

# These rule are used if someone wants to rebuild ncgenyy.c or ncgentab.c
# Otherwise never invoked, but records how to do it.
# BTW: note that renaming is essential because otherwise
# autoconf will forcibly delete files of the name *.tab.*

makeparser::
	flex -Pncg -8 ncgen.l
	rm -f ncgenyy.c
	mv lex.ncg.c ncgenyy.c
	bison -pncg -t ncgen.y
	rm -f ncgentab.c
	mv ncgen.tab.c ncgentab.c
