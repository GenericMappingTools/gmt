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

ASTDIR=./ast/src/compiler
ASTJAR=${ASTDIR}/ast.jar

# CDMREMORE Sources
SRC=nccrcvt.c nccr.c nccrdispatch.c crmeta.c nccrproto.c nccrgetvarx.c \
crdebug.c crutil.c \
nccrdump.c \
ncStreamx.c curlwrap.c \
ast_byteio.c ast_curl.c ast_internal.c \
ast_runtime.c ast_util.c ast_debug.c \
cceparse.c cceconstraints.c

EXTRA_DIST += ccetab.c ccelex.c

HDRS=includes.h \
nccrdispatch.h nccr.h crmeta.h nccrproto.h nccrgetvarx.h \
crpath.h crdebug.h crutil.h \
nccrdump.h \
ncStreamx.h curlwrap.h \
ast.h ast_byteio.h ast_curl.h ast_internal.h \
ast_runtime.h ast_util.h ast_debug.h \
nccrnode.h \
nccrcvt.h \
ccetab.h cceconstraints.h cceparselex.h

if BUILD_CDMREMOTE

noinst_LTLIBRARIES = libnccr.la

# Build convenience library
libnccr_la_SOURCES = $(SRC) $(HDRS)
libnccr_la_CPPFLAGS = $(AM_CPPFLAGS)
libnccr_la_LIBADD = 

#if USE_NETCDF4
#LDADD += ${top_builddir}/libsrc4/libnetcdf4.la
#endif
#
#LDADD += ${top_builddir}/libsrc/libnetcdf3.la		\
#	 ${top_builddir}/oc/liboc.la			\
#	 ${top_builddir}/libdispatch/libdispatch.la
#
#
## Add a trivial test case to check for undefined references
#check_PROGRAMS = t_cdmr
#TESTS = t_cdmr
#TESTS_ENVIRONMENT=TOPSRCDIR=${abs_top_srcdir}
#t_cdmr_SOURCES = t_cdmr.c nccrstub.c
#CLEANFILES += t_cdmr

##################################################
# These rule are used if someone wants to rebuild the grammar files.
# Otherwise never invoked, but records how to do it.
# BTW: note that renaming is essential because otherwise
# autoconf will forcibly delete files of the name *.tab.*

EXTRA_DIST += cce.y

parser ccetab.c ccetab.h: cce.y
	bison -v -d -t -p cce cce.y
	rm -f ccetab.c ccetab.h
	mv cce.tab.c ccetab.c
	mv cce.tab.h ccetab.h

##################################################
# Generate the augmented ncstream structures

EXTRA_DIST += ast.jar ncStream.proto ncStream.h ncStream.c extends.sed

ncStreamx.c ncStreamx.h: ncStreamx.proto ast.jar
	java -jar ast.jar -WdT -I. ncStreamx.proto

ncStreamx.proto: ncStream.proto extends.sed
	sed -f extends.sed <ncStream.proto |tr -d '\r' >ncStreamx.proto

##################################################

jar::
	cd ${ASTDIR}; make -f Make0 jar
	cp ${ASTJAR} .

endif # BUILD_CDMREMOTE

