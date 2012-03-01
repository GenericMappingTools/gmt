## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to libdap2. Whatever that
# is!

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# We may have to add to these later.
DISTCLEANFILES =
MAINTAINERCLEANFILES =
CLEANFILES =
EXTRA_DIST =

LDADD=

SRC= constraints3.c common34.c dapcvt.c dapalign.c dapodom.c daputil.c	\
ncdaperr.c cdf3.c cache.c dapdump.c dapdebug.c dapattr3.c		\
ncd3dispatch.c ncdap3.c ncdap3a.c getvara3.c dceconstraints.c dcetab.c	\
dceparse.c dcelex.c

HDRS= nccommon.h constraints3.h ncd3dispatch.h ncdap3.h dapalign.h	\
dapodom.h getvara.h dapnc.h daputil.h dapdebug.h dapdump.h		\
dceconstraints.h dcetab.h dceparselex.h

if USE_NETCDF4
SRC += cdf4.c constraints4.c ncd4dispatch.c ncdap4.c getvara4.c
HDRS +=constraints4.h ncd4dispatch.h ncdap4.h
endif

if BUILD_DAP

noinst_LTLIBRARIES = libdap2.la

# Build convenience library
libdap2_la_SOURCES = $(SRC) $(HDRS)
libdap2_la_CPPFLAGS = $(AM_CPPFLAGS)
libdap2_la_LIBADD =

AM_CPPFLAGS += -I${top_srcdir}/libdispatch

if USE_NETCDF4
AM_CPPFLAGS += -I$(top_srcdir)/libsrc4
endif

AM_CPPFLAGS += -I$(top_srcdir)/libsrc
AM_CPPFLAGS += -I${top_srcdir}/oc

LDADD += libdap2.la

if USE_NETCDF4
LDADD += ${top_builddir}/libsrc4/libnetcdf4.la
endif

LDADD += ${top_builddir}/libsrc/libnetcdf3.la		\
	 ${top_builddir}/oc/liboc.la			\
	 ${top_builddir}/libdispatch/libdispatch.la

if BUILD_V2
LDADD += ${top_builddir}/libdispatch/libnetcdf2.la 
endif # BUILD_V2

endif # BUILD_DAP

# These rule are used if someone wants to rebuild the grammar files.
# Otherwise never invoked, but records how to do it.
# BTW: note that renaming is essential because otherwise
# autoconf will forcibly delete files of the name *.tab.*

EXTRA_DIST += dce.y

makece::
	bison -v -d -t -p dce dce.y
	rm -f dcetab.c dcetab.h
	mv dce.tab.c dcetab.c
	mv dce.tab.h dcetab.h
