## This is a automake file, part of Unidata's netCDF package.
# Copyright 2011, see the COPYRIGHT file for more information.

# This file builds and runs DAP tests.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

LDADD = ${top_builddir}/liblib/libnetcdf.la 
AM_CPPFLAGS += -I$(top_builddir)/liblib

# Set up the tests; do the .sh first, then .c
check_PROGRAMS = 
TESTS = 
TESTS_ENVIRONMENT=TOPSRCDIR=${abs_top_srcdir}

# Make nctestserver be the first test to be built
check_PROGRAMS += nctestserver
nctestserver_SOURCES = nctestserver.c

TESTS += tst_ncdap3.sh

# remote tests are optional
# because the server may be down or inaccessible

if ENABLE_DAP_REMOTE_TESTS

TESTS += tst_remote3.sh

if ENABLE_DAP_LONG_TESTS
 TESTS += tst_longremote3.sh
endif

test_cvt3_SOURCES = test_cvt.c
test_vara_SOURCES = test_vara.c
test_partvar_SOURCES = test_partvar.c
test_varm3_SOURCES = test_varm3.c
t_dap3a_SOURCES = t_dap3a.c

TESTS += t_dap3a test_cvt3 test_vara test_partvar
TESTS += test_varm3
TESTS += t_dap3a

check_PROGRAMS += t_dap3a test_cvt3 test_vara test_partvar
check_PROGRAMS += test_varm3
check_PROGRAMS += t_dap3a

endif #ENABLE_DAP_REMOTE_TESTS


if USE_NETCDF4

#TESTS += tst_ncdap4.sh

if ENABLE_DAP_REMOTE_TESTS

#TESTS += tst_remote4.sh

if ENABLE_DAP_LONG_TESTS
#TESTS += tst_longremote4.sh
endif

# Conversion tests

#test_cvt4_SOURCES = test_cvt.c
#t_dap4_SOURCES = t_dap4.c
#check_PROGRAMS += test_cvt4 t_dap4
#TESTS += test_cvt4 t_dap4

endif ENABLE_DAP_REMOTE_TESTS

endif #USE_NETCDF4

# Need to add subdirs
SUBDIRS = testdata3 expected3 expected4 expectremote3 expectremote4

EXTRA_DIST = tst_ncdap3.sh tst_ncdap4.sh \
             tst_remote3.sh tst_remote4.sh \
             tst_longremote3.sh tst_longremote4.sh \
             tst_ncdap.sh tst_ncdap_shared.sh tst_remote.sh \
	     t_dap.c

CLEANFILES = test_varm3 test_cvt3 results/*.dmp results/*.das results/*.dds datadds* t_dap3a

if USE_NETCDF4
CLEANFILES += test_cvt4 t_dap4
endif

# This rule are used if someone wants to rebuild t_dap3a.c
# Otherwise never invoked, but records how to do it.
t_dap3a.c: t_dap.c
	echo "#define NETCDF3ONLY" > ./t_dap3a.c
	cat t_dap.c >> t_dap3a.c

t_dap4a.c: t_dap.c
	cat t_dap.c >> ./t_dap4a.c

# One last thing
BUILT_SOURCES = .dodsrc

.dodsrc:
	echo "#DODSRC" >.dodsrc

DISTCLEANFILES = .dodsrc results/* results
