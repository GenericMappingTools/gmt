# This is part of the libcf library.

# This automake file handles the building of cfcheck.

# Ed Hartnett 11/23/06

# $Id$

AM_CPPFLAGS =
AM_LDFLAGS = 
LDADD =

AM_CPPFLAGS += -I${top_srcdir}/src

if USE_NETCDF_DIR
AM_CPPFLAGS += -I@NETCDFDIR@/include
AM_LDFLAGS += -L@NETCDFDIR@/lib
else #!USE_NETCDF_DIR
AM_CPPFLAGS += -I${top_srcdir}/.. -I${top_srcdir}/../include -I${top_srcdir}/../liblib
AM_LDFLAGS += ${top_builddir}/../liblib/libnetcdf.la
endif #!USE_NETCDF_DIR

AM_LDFLAGS += @EXTERN_LDFLAGS@ -lm

EXTRA_DIST = tst_cfcheck.sh

bin_PROGRAMS = cfcheck
cfcheck_SOURCES = cfcheck.c

TESTS = tst_cfcheck.sh

CLEANFILES = useage.txt
