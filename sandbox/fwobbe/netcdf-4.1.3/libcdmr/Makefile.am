## This is a automake file, part of Unidata's netCDF package.
# Copyright 2010, see the COPYRIGHT file for more information.

# This builds the netcdf-4 version of the opendap client.

# $Id$

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

DISTCLEANFILES =
CLEANFILES =
EXTRA_DIST =
LDADD=

# CDMREMORE Sources
SRC=nccr.c nccrdispatch.c nccrmeta.c nccrproto.c nccrgetvara.c \
crdebug.c crutil.c \
nccrdump.c \
ncStreamx.c curlwrap.c ncaux.c \
ast_byteio.c ast_curl.c ast_internal.c \
ast_runtime.c ast_util.c ast_debug.c \
crce.c \
cceparse.c cceconstraints.c

EXTRA_DIST += ccetab.c ccelex.c

HDRS=nccrdispatch.h nccr.h nccrmeta.h nccrproto.h \
crdebug.h crutil.h \
nccrdump.h \
ncStreamx.h curlwrap.h ncaux.h \
ast.h ast_byteio.h ast_curl.h ast_internal.h \
ast_runtime.h ast_util.h ast_debug.h \
crce.h nccrnode.h \
ccetab.h cceconstraints.h cceparselex.h

if BUILD_CDMREMOTE

noinst_LTLIBRARIES = libnccr.la

# Build convenience library
libnccr_la_SOURCES = $(SRC) $(HDRS)

AM_CPPFLAGS += -I$(top_srcdir)/libsrc4 \
	      -I$(top_srcdir)/libsrc \
	      -I$(top_srcdir)/libdispatch

AM_CPPFLAGS += -I${top_srcdir}/libdispatch @EXTERN_CPPFLAGS@

# Define the load libraries for stub3
LDADD += ${top_builddir}/libcdmr/libnccr.la \
	 ${top_builddir}/libsrc4/libnetcdf4.la \
         ${top_builddir}/libsrc/libnetcdf3.la \
         ${top_builddir}/libdispatch/libdispatch.la

if BUILD_V2
LDADD += ${top_builddir}/libdispatch/libnetcdf2.la 
endif # BUILD_V2

# Add a trivial test case to check for undefined references
check_PROGRAMS = t_cdmr
TESTS = t_cdmr
TESTS_ENVIRONMENT=TOPSRCDIR=${abs_top_srcdir}
t_cdmr_SOURCES = t_cdmr.c nccrstub.c
CLEANFILES += t_cdmr

##################################################
# Generate the augmented ncstream structures
EXTRA_DIST+=ncStream.proto ncStream.h ncStreamc

ASTDIR=./ast/src/compiler

ASTJAR=${ASTDIR}/ast.jar

ncStreamx.c ncStreamx.h: ncStreamx.proto ${ASTJAR}
	java -jar ${ASTJAR} -I. ncStreamx.proto

ncStreamx.proto: ncStream.proto extends.sed
	sed -f extends.sed <ncStream.proto |tr -d '\r' >ncStreamx.proto

.PHONEY: jar bison

${ASTJAR}: 
	cd ${ASTDIR}; make -f Make0 jar

# These rule are used if someone wants to rebuild the grammar files.
# Otherwise never invoked, but records how to do it.
# BTW: note that renaming is essential because otherwise
# autoconf will forcibly delete files of the name *.tab.*

EXTRA_DIST += cce.y

bison ccetab.c ccetab.h: cce.y
	bison -v -d -t -p cce cce.y
	rm -f ccetab.c ccetab.h
	mv cce.tab.c ccetab.c
	mv cce.tab.h ccetab.h


endif # BUILD_CDMREMOTE
