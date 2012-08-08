#                                               -*- Autoconf -*-
## Process this file with autoconf to produce a configure script.

# This is part of Unidata's netCDF package. Copyright 2005-2012, see
# the COPYRIGHT file for more information.

# Recall that ${VAR-exp} expands to $VAR if var is set (even to null),
# and to exp otherwise. 

## This puts the cvs ID tag in the output configure script.
AC_REVISION([$Id$])

# Running autoconf on this file will trigger a warning if 
# autoconf is not at least the specified version.
AC_PREREQ([2.59])

# Initialize with name, version, and support email address. 
AC_INIT([netCDF], [4.2.1.1], [support-netcdf@unidata.ucar.edu])

# Create the VERSION file, which contains the package version from
# AC_INIT.
echo -n AC_PACKAGE_VERSION>VERSION
AC_SUBST(PACKAGE_VERSION)

AC_MSG_NOTICE([netCDF AC_PACKAGE_VERSION])

# Keep libtool macros in an m4 directory.
AC_CONFIG_MACRO_DIR([m4])

# Find out about the host we're building on.
AC_CANONICAL_HOST

# Find out about the target we're building for.
AC_CANONICAL_TARGET

AC_CONFIG_HEADERS([config.h])

# This call is required by automake.
AM_INIT_AUTOMAKE([foreign dist-zip subdir-objects])

# Check for the existence of this file before proceeding.
AC_CONFIG_SRCDIR([libsrc/nc.c])

AC_MSG_NOTICE([checking user options])

# If --enable-dll is specified the DLL will be built. This only works
# on mingw.
AC_MSG_CHECKING([whether a win32 DLL is desired])
AC_ARG_ENABLE([dll],
              [AS_HELP_STRING([--enable-dll],
                              [build a win32 DLL (only works on mingw)])])
test "x$enable_dll" = xyes || enable_dll=no
AC_MSG_RESULT([$enable_dll])
if test "x$enable_dll" = xyes; then
   AC_DEFINE(DLL_NETCDF, 1, [set this only when building a DLL under MinGW])
   AC_DEFINE(DLL_EXPORT, 1, [set this only when building a DLL under MinGW])
fi
AM_CONDITIONAL(BUILD_DLL, [test x$enable_dll = xyes])

# Did the user specify a default minimum blocksize (NCIO_MINBLOCKSIZE) for posixio?
AC_MSG_CHECKING([whether a NCIO_MINBLOCKSIZE was specified])
AC_ARG_WITH([minblocksize],
              [AS_HELP_STRING([--with-minblocksize=<integer>],
                              [Specify minimum I/O blocksize for netCDF classic and 64-bit offset format files.])],
            [NCIO_MINBLOCKSIZE=$with_minblocksize], [NCIO_MINBLOCKSIZE=256])
AC_MSG_RESULT([$NCIO_MINBLOCKSIZE])
AC_DEFINE_UNQUOTED([NCIO_MINBLOCKSIZE], [$NCIO_MINBLOCKSIZE], [min blocksize for posixio.])

AC_ARG_ENABLE([doxygen],
  [AS_HELP_STRING([--enable-doxygen],
    [Enable generation of documentation.])])
test "x$enable_doxygen" = xyes || enable_doxygen=no
AM_CONDITIONAL([BUILD_DOCS], [test "x$enable_doxygen" = xyes]) 

AC_ARG_ENABLE([internal-docs],
  [AS_HELP_STRING([--enable-internal-docs],
    [Include documentation of library internals. This is of interest only to those developing the netCDF library.])])
test "x$enable_internal_docs" = xyes || enable_internal_docs=no
AC_SUBST([BUILD_INTERNAL_DOCS], [$enable_internal_docs]) 

AC_MSG_CHECKING([if fsync support is enabled])
AC_ARG_ENABLE([fsync],
              [AS_HELP_STRING([--disable-fsync],
                              [disable fsync support])])
test "x$enable_fsync" = xno || enable_fsync=yes
AC_MSG_RESULT($enable_fsync)
if test "x$enable_fsync" = xyes ; then
AC_DEFINE([USE_FSYNC], [1], [if true, include experimental fsync code])
fi

# Does the user want to run extra tests with valgrind?
AC_MSG_CHECKING([whether extra valgrind tests should be run])	
AC_ARG_ENABLE([valgrind-tests],
              [AS_HELP_STRING([--enable-valgrind-tests],
                              [build with valgrind-tests (valgrind is required, static builds only)])])
test "x$enable_valgrind_tests" = xyes || enable_valgrind_tests=no
AC_MSG_RESULT($enable_valgrind_tests)

# Does the user want to build netcdf-4?
AC_MSG_CHECKING([whether we should try to build netCDF-4])
AC_ARG_ENABLE([netcdf-4], [AS_HELP_STRING([--enable-netcdf-4],
              [build with netcdf-4 (HDF5 and zlib required)])])
test "x$enable_netcdf_4" = xno || enable_netcdf_4=yes
AC_MSG_RESULT([$enable_netcdf_4])

# Synonym
# --enable-netcdf-4 is the controlling enable switch
if test "x$enable_netcdf_4" = "x" ; then
  AC_ARG_ENABLE([netcdf4], [AS_HELP_STRING([--enable-netcdf4],
              [(just a synonym for enable-netcdf-4)])])
  enable_netcdf_4="$enable_netcdf4"
fi

# Does the user want to turn on HDF4 read ability?
AC_MSG_CHECKING([whether reading of HDF4 SD files is to be enabled])
AC_ARG_ENABLE([hdf4], [AS_HELP_STRING([--enable-hdf4],
              [build netcdf-4 with HDF4 read capability (HDF4, HDF5 and zlib required)])])
test "x$enable_hdf4" = xyes || enable_hdf4=no
AC_MSG_RESULT($enable_hdf4)

# Does the user want to turn on extra HDF4 file tests?
AC_MSG_CHECKING([whether to fetch some sample HDF4 files from Unidata ftp site to test HDF4 reading (requires wget)])
AC_ARG_ENABLE([hdf4-file-tests], [AS_HELP_STRING([--enable-hdf4-file-tests],
              [get some HDF4 files from Unidata ftp site and test that they can be read])])
test "x$enable_hdf4" = xyes -a "x$enable_hdf4_file_tests" = xyes || enable_hdf4_file_tests=no
if test "x$enable_hdf4_file_tests" = xyes; then
   AC_DEFINE([USE_HDF4_FILE_TESTS], 1, [If true, use use wget to fetch some sample HDF4 data, and then test against it.])
fi
AC_MSG_RESULT($enable_hdf4_file_tests)

# Does the user want to turn on PNETCDF read ability?
AC_MSG_CHECKING([whether parallel I/O for classic and 64-bit offset files using parallel-netcdf is to be enabled])
AC_ARG_ENABLE([pnetcdf], [AS_HELP_STRING([--enable-pnetcdf],
              [build netcdf-4 with parallel I/O for classic and 64-bit offset files using parallel-netcdf])])
test "x$enable_pnetcdf" = xyes || enable_pnetcdf=no
AC_MSG_RESULT($enable_pnetcdf)

# Does the user want to run extra example tests
AC_MSG_CHECKING([whether extra example tests should be run])
AC_ARG_ENABLE([extra-example-tests],
              [AS_HELP_STRING([--enable-extra-example-tests],
                              [Run extra example tests; requires GNU sed. Ignored if \
                               netCDF-4 is not enabled.])])
test "x$enable_extra_example_tests" = xyes || enable_extra_example_tests=no
AC_MSG_RESULT($enable_extra_example_tests)

# Does the user want to run extra parallel tests when parallel netCDF-4 is built?
AC_MSG_CHECKING([whether parallel IO tests should be run])
AC_ARG_ENABLE([parallel-tests],
              [AS_HELP_STRING([--enable-parallel-tests],
                              [Run extra parallel IO tests. Ignored if \
                               netCDF-4 is not enabled, or built on a system \
                               without parallel I/O support.])])
test "x$enable_parallel_tests" = xyes || enable_parallel_tests=no
AC_MSG_RESULT($enable_parallel_tests)

# Did the user specify a default chunk size?
AC_MSG_CHECKING([whether a default chunk size in bytes was specified])
AC_ARG_WITH([default-chunk-size],
              [AS_HELP_STRING([--with-default-chunk-size=<integer>],
                              [Specify default size of chunks in bytes.])],
            [DEFAULT_CHUNK_SIZE=$with_default_chunk_size], [DEFAULT_CHUNK_SIZE=4194304])
AC_MSG_RESULT([$DEFAULT_CHUNK_SIZE])
AC_DEFINE_UNQUOTED([DEFAULT_CHUNK_SIZE], [$DEFAULT_CHUNK_SIZE], [default chunk size in bytes])

# Did the user specify a max per-var cache size?
AC_MSG_CHECKING([whether a maximum per-variable cache size for HDF5 was specified])
AC_ARG_WITH([max-default-cache-size],
              [AS_HELP_STRING([--with-max-default-cache-size=<integer>],
                              [Specify maximum size (in bytes) for the default per-var chunk cache.])],
            [MAX_DEFAULT_CACHE_SIZE=$with_max_default_cache_size], [MAX_DEFAULT_CACHE_SIZE=67108864])
AC_MSG_RESULT([$MAX_DEFAULT_CACHE_SIZE])
AC_DEFINE_UNQUOTED([MAX_DEFAULT_CACHE_SIZE], [$MAX_DEFAULT_CACHE_SIZE], [max size of the default per-var chunk cache.])

# Did the user specify a number of chunks in default per-var cache size?
AC_MSG_CHECKING([whether a number of chunks for the default per-variable cache was specified])
AC_ARG_WITH([default-chunks-in-cache],
              [AS_HELP_STRING([--with-default-chunks-in-cache=<integer>],
                              [Specify the number of chunks to store in default per-variable cache.])],
            [DEFAULT_CHUNKS_IN_CACHE=$with_default_chunks_in_cache], [DEFAULT_CHUNKS_IN_CACHE=10])
AC_MSG_RESULT([$DEFAULT_CHUNKS_IN_CACHE])
AC_DEFINE_UNQUOTED([DEFAULT_CHUNKS_IN_CACHE], [$DEFAULT_CHUNKS_IN_CACHE], [num chunks in default per-var chunk cache.])

# Did the user specify a default cache size?
AC_MSG_CHECKING([whether a default file cache size for HDF5 was specified])
AC_ARG_WITH([chunk-cache-size],
              [AS_HELP_STRING([--with-chunk-cache-size=<integer>],
                              [Specify default file cache chunk size for HDF5 files in bytes.])],
            [CHUNK_CACHE_SIZE=$with_chunk_cache_size], [CHUNK_CACHE_SIZE=4194304])
AC_MSG_RESULT([$CHUNK_CACHE_SIZE])
AC_DEFINE_UNQUOTED([CHUNK_CACHE_SIZE], [$CHUNK_CACHE_SIZE], [default file chunk cache size in bytes.])

# Did the user specify a default cache nelems?
AC_MSG_CHECKING([whether a default file cache maximum number of elements for HDF5 was specified])
AC_ARG_WITH([chunk-cache-nelems],
              [AS_HELP_STRING([--with-chunk-cache-nelems=<integer>],
                              [Specify default maximum number of elements in the file chunk cache chunk for HDF5 files (should be prime number).])],
            [CHUNK_CACHE_NELEMS=$with_chunk_cache_nelems], [CHUNK_CACHE_NELEMS=1009])
AC_MSG_RESULT([$CHUNK_CACHE_NELEMS])
AC_DEFINE_UNQUOTED([CHUNK_CACHE_NELEMS], [$CHUNK_CACHE_NELEMS], [default file chunk cache nelems.])

# Did the user specify a default cache preemption?
AC_MSG_CHECKING([whether a default cache preemption for HDF5 was specified])
AC_ARG_WITH([chunk-cache-preemption],
              [AS_HELP_STRING([--with-chunk-cache-preemption=<float between 0 and 1 inclusive>],
                              [Specify default file chunk cache preemption policy for HDF5 files (a number between 0 and 1, inclusive).])],
            [CHUNK_CACHE_PREEMPTION=$with_chunk_cache_preemption], [CHUNK_CACHE_PREEMPTION=0.75])
AC_MSG_RESULT([$CHUNK_CACHE_PREEMPTION])
AC_DEFINE_UNQUOTED([CHUNK_CACHE_PREEMPTION], [$CHUNK_CACHE_PREEMPTION], [default file chunk cache preemption policy.])

# Does the user want to enable netcdf-4 logging?
AC_MSG_CHECKING([whether netCDF-4 logging is enabled])
AC_ARG_ENABLE([logging],
              [AS_HELP_STRING([--enable-logging],
                              [enable logging capability (only applies when netCDF-4 is built). \
			      This debugging features is only of interest to netCDF developers. \
			      Ignored if netCDF-4 is not enabled.])])   
test "x$enable_logging" = xyes || enable_logging=no
AC_MSG_RESULT([$enable_logging])

# Capture the state of the --enable-cdmremote flag
AC_MSG_CHECKING([whether cdmremote client is to be built])
AC_ARG_ENABLE([cdmremote],
                 [AS_HELP_STRING([--enable-cdmremote],
                                 [build with cdmremote client support.])])
test "x$enable_cdmremote" = xyes || enable_cdmremote=no
# CDMREMOTE requires netCDF-4
if test "x$enable_netcdf_4" = "xno" ; then enable_cdmremote=no ; fi
AC_MSG_RESULT($enable_cdmremote)

## Capture the state of the --enable-dap flag
AC_MSG_CHECKING([whether DAP client is to be built])
AC_ARG_ENABLE([dap],
                 [AS_HELP_STRING([--disable-dap],
                                 [build without DAP client support.])])
test "x$enable_dap" = xno || enable_dap=yes
AC_MSG_RESULT($enable_dap)

# OC now has its own version of rpc (really xdr), so no longer need the library
## Capture the state of the --enable-rpc flag
#AC_MSG_CHECKING([whether rpc client and server are to be built])
#AC_ARG_ENABLE([rpc],
#                 [AS_HELP_STRING([--enable-rpc],
#                                 [build with rpc client and server support.])])
#test "x$enable_rpc" = xyes || enable_rpc=no
#AC_MSG_RESULT($enable_rpc)

# Curl support is required if and only if any of these flags are set:
# 1. --enable-dap
# 2. --enable-cdmremote
# 3. --enable-rpc

if test "x$enable_dap" = "xyes" -o "x$enable_cdmremote" = "xyes" -o "x$enable_rpc" = "xyes" ; then
require_curl=yes
else
require_curl=no
fi

# See if the user provided us with a curl library
# Do an initial lib test for curl, but suppress the default action
AC_CHECK_LIB([curl],[curl_easy_setopt],[found_curl=yes],[found_curl=no])
#AC_CHECK_LIB([curl.dll],[curl_easy_setopt])
# If curl is required but there is no curl, then complain
if test $require_curl = yes ; then
  # Removed. Why assume no curl if we are building DLL?
  #if test $enable_dll = yes ; then
  #  AC_MSG_NOTICE([libcurl not found; continuing])
  #elif test $found_curl = no ; then
  if test $found_curl = no ; then
    AC_MSG_NOTICE([libcurl not found; disabling remote protocol(s) support])
    enable_dap=no
    enable_cdmremote=no
    enable_rpc=no
  elif test $found_curl = yes ; then
    # Redo the check lib to actually add -lcurl
    #AC_CHECK_LIB([curl], [curl_easy_setopt])
    AC_SEARCH_LIBS([curl_easy_setopt],[curl curl.dll], [], [])
  fi
fi

# Default is now to always do the short remote tests
AC_MSG_CHECKING([whether dap remote testing should be enabled (default on)])
AC_ARG_ENABLE([dap-remote-tests],
              [AS_HELP_STRING([--disable-dap-remote-tests],
                                 [disable dap remote tests])])
test "x$enable_dap_remote_tests" = xno || enable_dap_remote_tests=yes
if test "x$enable_dap" = "xno" ; then
  enable_dap_remote_tests=no
fi
AC_MSG_RESULT($enable_dap_remote_tests)

# Control if groups are supported in [netcdf4]dap2 code
AC_MSG_CHECKING([whether [netcdf4] group names should be enabled (default on)])
AC_ARG_ENABLE([dap-groups],
              [AS_HELP_STRING([--disable-dap-groups],
                                 [disable [netcdf4] dap group names])])
test "x$enable_groups" = xno || enable_dap_groups=yes
if test "x$enable_dap" = "xno" -o "x$enable_enable_netcdf_4" = "xno" ; then
  AC_MSG_NOTICE([dap groups disabled because dap disabled or netcdf-4 disabled])
  enable_dap_groups=no
fi
AC_MSG_RESULT($enable_dap_groups)
if test "x$enable_dap_groups" = xyes; then
AC_DEFINE([ENABLE_DAP_GROUPS], [1], [if true, enable DAP group names])
fi

# Set the config.h flags
if test "x$enable_dap" = xyes; then
   AC_DEFINE([USE_DAP], [1], [if true, build DAP Client])
   AC_DEFINE([ENABLE_DAP], [1], [if true, build DAP Client])
fi

if test "x$enable_dap_remote_tests" = xyes; then
   AC_DEFINE([ENABLE_DAP_REMOTE_TESTS], [1], [if true, do remote tests])
fi

if test "x$enable_cdmremote" = xyes; then
   AC_DEFINE([USE_CDMREMOTE], [1], [if true, build CDMREMOTE Client])
fi

if test "x$enable_rpc" = xyes; then
   AC_DEFINE([BUILD_RPC], [1], [if true, build RPC Client and Server])
fi

AC_MSG_CHECKING([whether the time-consuming dap tests should be enabled (default off)])
AC_ARG_ENABLE([dap-long-tests],
              [AS_HELP_STRING([--enable-dap-long-tests],
                                 [enable dap long tests])])
test "x$enable_dap_long_tests" = xyes || enable_dap_long_tests=no
if test "x$enable_dap_remote_tests" = "xno" ; then
  enable_dap_long_tests=no
fi
AC_MSG_RESULT([$enable_dap_long_tests])

AM_CONDITIONAL(INTERNAL_OCLIB,[test "x" = "x"])

# Does the user want to do some extra tests?
AC_MSG_CHECKING([whether netCDF extra tests should be run (developers only)])
AC_ARG_ENABLE([extra-tests],
              [AS_HELP_STRING([--enable-extra-tests],
                              [run some extra tests that may not pass because of known issues])])
test "x$enable_extra_tests" = xyes || enable_extra_tests=no
AC_MSG_RESULT($enable_extra_tests)
if test "x$enable_extra_tests" = xyes; then
   AC_DEFINE([EXTRA_TESTS], [1], [if true, run extra tests which may not work yet])
fi
AM_CONDITIONAL(EXTRA_TESTS, [test x$enable_extra_tests = xyes])

# Does the user want to use the ffio module?
AC_MSG_CHECKING([whether FFIO will be used])
AC_ARG_ENABLE([ffio],
              [AS_HELP_STRING([--enable-ffio],
                              [use ffio instead of posixio (ex. on the Cray)])])
test "x$enable_ffio" = xyes || enable_ffio=no
AC_MSG_RESULT($enable_ffio)
if test "x$enable_ffio" = xyes; then
   AC_DEFINE([USE_FFIO], [1], [if true, use ffio instead of posixio])
fi
AM_CONDITIONAL(USE_FFIO, [test x$enable_ffio = xyes])

# Does the user want to enable the user-provided NEC-SX vectorization
# patch.
dnl AC_MSG_CHECKING([whether netCDF NEC-SX vectorization patch is enabled])
dnl AC_ARG_ENABLE([sx-vectorization],
dnl               [AS_HELP_STRING([--enable-sx-vectorization],
dnl                               [enable a user-provided performance patch to allow \	
dnl 			      vectorization of type conversions on NEC SX machines.])])   
dnl test "x$enable_sx_vectorization" = xyes || enable_sx_vectorization=no
dnl AC_MSG_RESULT([$enable_sx_vectorization])
dnl if test "x$enable_sx_vectorization" = xyes; then
dnl    AC_DEFINE([SX_VECTORIZATION], 1, [if true, turn on vectorization patch for NEC SX])
dnl fi

nc_build_c=yes
nc_build_v2=yes
nc_build_utilities=yes
nc_build_tests=yes
nc_build_examples=yes

# Does the user want to build examples?
AC_MSG_CHECKING([whether examples should be built])
AC_ARG_ENABLE([examples],
              [AS_HELP_STRING([--disable-examples],
                              [don't build the netCDF examples during make check \
                              (examples are treated as extra tests by netCDF)])])
test "x$enable_examples" = xno && nc_build_examples=no
AC_MSG_RESULT($nc_build_examples)
AM_CONDITIONAL(BUILD_EXAMPLES, [test x$nc_build_examples = xyes])

# Does the user want to disable the V2 API?
AC_MSG_CHECKING([whether v2 netCDF API should be built])
AC_ARG_ENABLE([v2],
              [AS_HELP_STRING([--disable-v2],
                              [turn off the netCDF version 2 API])])
test "x$enable_v2" = xno && nc_build_v2=no
AC_MSG_RESULT($nc_build_v2)
AM_CONDITIONAL(BUILD_V2, [test x$nc_build_v2 = xyes])
if test "x$nc_build_v2" = xno; then
   AC_DEFINE_UNQUOTED(NO_NETCDF_2, 1, [do not build the netCDF version 2 API])
fi

# Does the user want to disable ncgen/ncdump/nccopy?
AC_MSG_CHECKING([whether the ncgen/ncdump/nccopy should be built])
AC_ARG_ENABLE([utilities],
              [AS_HELP_STRING([--disable-utilities],
                              [don't build netCDF utilities ncgen, ncdump, and nccopy])])
test "x$nc_build_c" = xno && enable_utilities=no
test "x$enable_utilities" = xno && nc_build_utilities=no
AC_MSG_RESULT($nc_build_utilities)
AM_CONDITIONAL(BUILD_UTILITIES, [test x$nc_build_utilities = xyes])

# Does the user want to disable all tests?
AC_MSG_CHECKING([whether test should be built and run])
AC_ARG_ENABLE([testsets],
              [AS_HELP_STRING([--disable-testsets],
                              [don't build or run netCDF tests])])
test "x$enable_testsets" = xno || enable_testsets=yes
nc_build_tests=$enable_testsets
AC_MSG_RESULT($nc_build_tests)
AM_CONDITIONAL(BUILD_TESTSETS, [test x$nc_build_tests = xyes])

# Does the user want to run tests for large files (> 2GiB)?
AC_MSG_CHECKING([whether large file (> 2GB) tests should be run])
AC_ARG_ENABLE([large-file-tests],
              [AS_HELP_STRING([--enable-large-file-tests],
                              [Run tests which create very large data files (~13 GB disk space
                              required, but it will be recovered when tests are complete). See
                              option --with-temp-large to specify temporary directory])])
test "x$enable_large_file_tests" = xyes || enable_large_file_tests=no
AC_MSG_RESULT($enable_large_file_tests)
AM_CONDITIONAL(LARGE_FILE_TESTS, [test x$enable_large_file_tests = xyes])
if test "x$enable_large_file_tests" = xyes; then
   AC_DEFINE([LARGE_FILE_TESTS], [1], [do large file tests])
fi

# Does the user want to run benchmarks?
AC_MSG_CHECKING([whether benchmaks should be run (experimental)])
AC_ARG_ENABLE([benchmarks],
              [AS_HELP_STRING([--enable-benchmarks],
                              [Run benchmarks. This is an experimental feature. You must fetch 
                              sample data files from the Unidata ftp site to use these benchmarks. 
                              The benchmarks are a bunch of extra tests, which are timed. We use these
			      tests to check netCDF performance.])])
test "x$enable_benchmarks" = xyes || enable_benchmarks=no
AC_MSG_RESULT($enable_benchmarks)
AM_CONDITIONAL(BUILD_BENCHMARKS, [test x$enable_benchmarks = xyes])

# Does the user want to use extreme numbers in testing.
AC_MSG_CHECKING([whether extreme numbers should be used in tests])
AC_ARG_ENABLE([extreme-numbers],
              [AS_HELP_STRING([--disable-extreme-numbers],
                              [don't use extreme numbers during testing, such as MAX_INT - 1])])
case "$host_cpu $host_os" in
     *386*solaris*)
        test "x$enable_extreme_numbers" = xyes || enable_extreme_numbers=no
        ;;
     *)
        test "x$enable_extreme_numbers" = xno || enable_extreme_numbers=yes
        ;;
esac     
AC_MSG_RESULT($enable_extreme_numbers)

if test "x$enable_extreme_numbers" = xyes; then
   AC_DEFINE(USE_EXTREME_NUMBERS, 1, [set this to use extreme numbers in tests])
fi

# If the env. variable TEMP_LARGE is set, or if
# --with-temp-large=<directory>, use it as a place for the large
# (i.e. > 2 GiB) files created during the large file testing.
AC_MSG_CHECKING([where to put large temp files if large file tests are run])
AC_ARG_WITH([temp-large],
            [AS_HELP_STRING([--with-temp-large=<directory>],
                            [specify directory where large files (i.e. >2 GB) \
                            will be written, if large files tests are run with
                            --enable-large-file-tests])],
            [TEMP_LARGE=$with_temp_large])
TEMP_LARGE=${TEMP_LARGE-.}
AC_MSG_RESULT($TEMP_LARGE)
#AC_SUBST(TEMP_LARGE)
AC_DEFINE_UNQUOTED([TEMP_LARGE], ["$TEMP_LARGE"], [Place to put very large netCDF test files.])

# According to the autoconf mailing list gurus, we must test for
# compilers unconditionally. That is, we can't skip looking for the
# fortran compilers, just because the user doesn't want fortran. This
# is due to a limitation in autoconf.

# Find the C compiler.
AC_MSG_NOTICE([finding C compiler])

AC_PROG_CC
AM_PROG_CC_C_O
AC_C_CONST

# CURLOPT_KEYPASSWD is not defined until curl version 7.16.4

# Save/restore CFLAGS
SAVECFLAGS="$CFLAGS"
CFLAGS="${curl_cflags}"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
[#include "curl/curl.h"],
[[int x = CURLOPT_KEYPASSWD;]])],
                   [havekeypassword=yes],
                   [havekeypassword=no])
AC_MSG_CHECKING([whether a CURLOPT_KEYPASSWD is defined])
AC_MSG_RESULT([${havekeypassword}])
if test $havekeypassword = yes; then
  AC_DEFINE([HAVE_CURLOPT_KEYPASSWD],[1],[Is CURLOPT_KEYPASSWD defined])
fi
CFLAGS="$SAVECFLAGS"

# Set up libtool.
AC_MSG_NOTICE([setting up libtool])
LT_PREREQ([2.2])
LT_INIT(win32-dll)

# Valgrind tests don't work with shared builds because of some libtool
# weirdness.
if test "x$enable_shared" = xyes; then
   if test $enable_valgrind_tests = yes; then
      AC_MSG_ERROR([No valgrind tests with shared libraries])
   fi
fi

AC_MSG_NOTICE([finding other utilities])

# Is doxygen installed? If so, have configure construct the Doxyfile.
AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN"; then 
   AC_MSG_WARN([Doxygen not found - documentation will not be built])
fi
# If we have doxygen, and it's enabled, then process the file.
if test "x$enable_doxygen" != xno; then
   if test -n "$DOXYGEN"; then
      AC_CONFIG_FILES([man4/Doxyfile])
   fi
# Note: the list of files to input to doxygen
# has been moved to man4/Doxyfile.in so
# that make distcheck works correctly.
# Any new inputs should be inserted into
# man4/Doxyfile.in and possibley man4/Makefile.am
fi

# Find the install program.
AC_PROG_INSTALL

# Check to see if any macros must be set to enable large (>2GB) files.
AC_SYS_LARGEFILE

AC_MSG_NOTICE([displaying some results])

## This next macro just prints some results for debugging
## support issues.
UD_DISPLAY_RESULTS

# For nightly build testing, output CC, FC, etc.
echo "CPPFLAGS=$CPPFLAGS CC=$CC CFLAGS=$CFLAGS LDFLAGS=$LDFLAGS LIBS=$LIBS" >> comps.txt

AC_MSG_NOTICE([checking types, headers, and functions])

AC_CHECK_HEADER(stdlib.h, ,AC_DEFINE([NO_STDLIB_H], [], [no stdlib.h]))
AC_CHECK_HEADER(sys/types.h, ,AC_DEFINE([NO_SYS_TYPES_H], [], [no sys_types.h]))
AC_CHECK_HEADERS([sys/param.h])
#AC_CHECK_HEADERS([locale.h])
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS([locale.h stdio.h stdarg.h errno.h ctype.h fcntl.h malloc.h stdlib.h string.h strings.h unistd.h sys/stat.h getopt.h sys/time.h])
AC_FUNC_VPRINTF

# Do sys/resource.h separately
#AC_CHECK_HEADERS([sys/resource.h],[havesysresource=1],[havesysresource=0])
#if test "x$enable_dll" != xyes ; then
AC_CHECK_HEADERS([sys/resource.h])
#fi

# Check for <stdbool.h> that conforms to C99 requirements
AC_HEADER_STDBOOL

# Check for these functions...
AC_CHECK_FUNCS([strlcat strerror snprintf strchr strrchr strcat strcpy \
                strdup strcasecmp strtod strtoll strtoull strstr \
		mkstemp rand \
		getrlimit gettimeofday fsync MPI_Comm_f2c])

# Does the user want to use NC_DISKLESS?
AC_MSG_CHECKING([whether in-memory files are enabled])
AC_ARG_ENABLE([diskless],
              [AS_HELP_STRING([--disable-diskless],
                              [disable support for in-memory (NC_DISKLESS) files])])
test "x$enable_diskless" = xno || enable_diskless=yes

if test "x$enable_dap" = "xyes" -o "x$enable_cdmremote" = "xyes" -o "x$enable_rpc" = "xyes" ; then
enable_diskless=yes
AC_MSG_NOTICE([--enable-dap requires --enable-diskless])
fi

AC_MSG_RESULT($enable_diskless)


# check for useful, but not essential, memio support
AC_CHECK_FUNCS([memmove getpagesize sysconf])

# Does the user want to allow use of mmap for NC_DISKLESS?
AC_MSG_CHECKING([whether mmap is enabled for in-memory files])
AC_ARG_ENABLE([mmap],
              [AS_HELP_STRING([--enable-mmap],
                              [allow mmap for in-memory files])])
test "x$enable_mmap" = xyes || enable_mmap=no
AC_MSG_RESULT($enable_mmap)

# check for mmap and mremap availability before committing to use mmap
AC_FUNC_MMAP
AC_CHECK_FUNCS([mremap])

if test "x$ac_cv_func_mmap_fixed_mapped" != xyes -o "x$ac_cv_func_mremap" != xyes ; then
  echo "mmap function or mremap function is not available: disabling mmap"
  enable_mmap=no
fi

# Setup the diskless and mmap conditionals
if test "x$enable_diskless" = xyes ; then
  AC_DEFINE([USE_DISKLESS], [1], [if true, include NC_DISKLESS code])
  if test "x$enable_mmap" = xyes; then
    AC_DEFINE([USE_MMAP], [1], [if true, use mmap for in-memory files])
  fi
fi

AC_FUNC_ALLOCA
AC_CHECK_DECLS([isnan, isinf, isfinite, signbit],,,[#include <math.h>])
AC_STRUCT_ST_BLKSIZE
UD_CHECK_IEEE
AC_TYPE_SIZE_T
AC_TYPE_OFF_T
AC_CHECK_TYPES([ssize_t, ptrdiff_t, uchar, longlong])
AC_C_CHAR_UNSIGNED
AC_C_BIGENDIAN
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(float)
AC_CHECK_SIZEOF(double)
AC_CHECK_SIZEOF(off_t)
AC_CHECK_SIZEOF(size_t)

if test "x$enable_netcdf_4" = xyes || test "x$enable_dap" = xyes; then
   AC_SEARCH_LIBS([deflate], [zlibwapi zlibstat zlib zlib1 z], [], [
     AC_MSG_ERROR([Can't find or link to the z library. Turn off netCDF-4 and \
     opendap with --disable-netcdf-4 --disable-dap, or see config.log for errors.])])
fi

# We need the math library
AC_CHECK_LIB([m], [floor], [], 
[AC_MSG_ERROR([Can't find or link to the math library.])])

if test "x$enable_netcdf_4" = xyes; then

   AC_DEFINE([USE_NETCDF4], [1], [if true, build netCDF-4])
   AC_DEFINE([H5_USE_16_API], [1], [use HDF5 1.6 API])

   AC_SEARCH_LIBS([SZ_Compress], [szip sz], [], [])

   # Check for the main hdf5 and hdf5_hl library.

   AC_SEARCH_LIBS([H5Fflush], [hdf5dll hdf5], [], 
   [AC_MSG_ERROR([Can't find or link to the hdf5 library. Use --disable-netcdf-4, or see config.log for errors.])])
   AC_SEARCH_LIBS([H5DSis_scale], [hdf5_hldll hdf5_hl], [], 
   [AC_MSG_ERROR([Can't find or link to the hdf5 high-level. Use --disable-netcdf-4, or see config.log for errors.])])
	
   AC_CHECK_HEADERS([hdf5.h], [], [AC_MSG_ERROR([NetCDF-4 requires HDF5, but hdf5.h cannot be found.])])
   AC_CHECK_FUNCS([H5Pget_fapl_mpiposix H5Pget_fapl_mpio H5Pset_deflate H5Z_SZIP])

   # The user may have parallel HDF5 based on MPI POSIX.
   if test "x$ac_cv_func_H5Pget_fapl_mpiposix" = xyes; then
      AC_DEFINE([USE_PARALLEL_POSIX], [1], [if true, compile in parallel netCDF-4 based on MPI/POSIX])
   fi
   
   # The user may have parallel HDF5 based on MPI mumble mumble.
   if test "x$ac_cv_func_H5Pget_fapl_mpio" = xyes; then
      AC_DEFINE([USE_PARALLEL_MPIO], [1], [if true, compile in parallel netCDF-4 based on MPI/IO])
   fi

   # If parallel is in use, enable it in the C code. Also add some stuff to netcdf.h.
   enable_parallel=no
   if test "x$ac_cv_func_H5Pget_fapl_mpio" = xyes -o "x$ac_cv_func_H5Pget_fapl_mpiposix" = xyes; then
      enable_parallel=yes
      AC_DEFINE([USE_PARALLEL], [1], [if true, parallel netCDF-4 is in use])

      # Using pnetcdf for classic parallel I/O?
      if test "x$enable_pnetcdf" = xyes; then
         AC_CHECK_LIB([pnetcdf], [ncmpi_create], [], 
	 [AC_MSG_ERROR([Cannot link to pnetcdf library, yet --enable-pnetcdf was used.])])

dnl          AC_CHECK_HEADERS([pnetcdf.h], [], [nc4_pnetcdf_h_missing=yes)	   
dnl 	 if test ! "x$HAVE_PNETCDF" = x1; then
dnl             AC_MSG_ERROR([Cannot find pnetcdf header, yet --enable-pnetcdf was used.])
dnl 	 fi
         AC_DEFINE([USE_PNETCDF], [1], [if true, parallel netCDF is used])      	 
      fi
   fi
   
   AC_MSG_CHECKING([whether parallel I/O features are to be included])
   AC_MSG_RESULT([$enable_parallel])         
   
   # The user must have built HDF5 with the ZLIB library.
   if test "x$ac_cv_func_H5Pset_deflate" = xyes; then
      AC_DEFINE([USE_ZLIB], [1], [if true, compile in zlib compression in netCDF-4 variables])
   else		    
      AC_MSG_ERROR([HDF5 must be built with zlib for netCDF-4])
   fi

   # The user may have built HDF5 with the SZLIB library.
   if test "x$ac_cv_func_H5Z_SZIP" = xyes; then
     AC_DEFINE([USE_SZIP], [1], [if true, compile in szip compression in netCDF-4 variables])
   fi

   # If the user wants hdf4 built in, check it out.
   if test "x$enable_hdf4" = xyes; then
      AC_CHECK_HEADERS([mfhdf.h], [], [nc_mfhdf_h_missing=yes])	   
      if test "x$nc_mfhdf_h_missing" = xyes; then
         AC_MSG_ERROR([Cannot find mfhdf.h, yet --enable-hdf4 was used.])
      fi		      
      AC_CHECK_LIB([df], [Hclose], [], [])
      AC_CHECK_LIB([mfhdf], [SDcreate], [], [])
      AC_DEFINE([USE_HDF4], [1], [if true, use HDF4 too])
   fi
fi

# No logging for netcdf-3.
if test "x$enable_netcdf_4" = xno; then
   enable_logging=no
fi
if test "x$enable_logging" = xyes; then
   AC_DEFINE([LOGGING], 1, [If true, turn on logging.])
fi

# Like other libraries, udunits and libcf
# are no long part of the netcdf distribution.

#AC_MSG_CHECKING([whether udunits is to be built])
#AC_ARG_WITH([udunits],
#              [AS_HELP_STRING([--with-udunits],
#                              [Build udunits2 package.])])
#test "x$with_udunits" = xyes || with_udunits=no
#AC_MSG_RESULT($with_udunits)
#AM_CONDITIONAL(BUILD_UDUNITS, [test "x$with_udunits" = xyes])


# Does the user want to also build the libcf library?
#AC_MSG_CHECKING([whether libcf is to be built])
#AC_ARG_WITH([libcf],
#              [AS_HELP_STRING([--with-libcf],
#                              [build and install libcf library, a library for \
#	      handling data in conformance with the Climate and \
#	      Forecast conventions. (Requires netCDF-4 and HDF5)])])
#test "x$with_libcf" = xyes || with_libcf=no
#AC_MSG_RESULT($with_libcf)
#AM_CONDITIONAL(BUILD_LIBCF, [test "x$with_libcf" = xyes])

#AC_CONFIG_SUBDIRS([udunits libcf])


# Automake conditionals need to be called, whether the answer is yes
# or no.
AM_CONDITIONAL(BUILD_PARALLEL, [test x$enable_parallel = xyes])
AM_CONDITIONAL(TEST_PARALLEL, [test "x$enable_parallel" = xyes -a "x$enable_parallel_tests" = xyes])
AM_CONDITIONAL(BUILD_DAP, [test "x$enable_dap" = xyes])
AM_CONDITIONAL(USE_DAP, [test "x$enable_dap" = xyes]) # Alias
AM_CONDITIONAL(ENABLE_DAP_REMOTE_TESTS, [test "x$enable_dap_remote_tests" = xyes])
AM_CONDITIONAL(ENABLE_DAP_LONG_TESTS, [test "x$enable_dap_long_tests" = xyes])
AM_CONDITIONAL(EXTRA_EXAMPLE_TESTS, [test "x$enable_extra_example_tests" = xyes])
AM_CONDITIONAL(USE_SZIP, [test "x$ac_cv_func_H5Z_SZIP" = xyes])
AM_CONDITIONAL(USE_PNETCDF_DIR, [test ! "x$PNETCDFDIR" = x])
AM_CONDITIONAL(USE_LOGGING, [test "x$enable_logging" = xyes])
AM_CONDITIONAL(CROSS_COMPILING, [test "x$cross_compiling" = xyes])
AM_CONDITIONAL(USE_VALGRIND_TESTS, [test "x$enable_valgrind_tests" = xyes])
AM_CONDITIONAL(USE_NETCDF4, [test x$enable_netcdf_4 = xyes])
AM_CONDITIONAL(USE_HDF4, [test x$enable_hdf4 = xyes])
AM_CONDITIONAL(USE_HDF4_FILE_TESTS, [test x$enable_hdf4_file_tests = xyes])
AM_CONDITIONAL(USE_RENAMEV3, [test x$enable_netcdf_4 = xyes -o x$enable_dap = xyes])
AM_CONDITIONAL(USE_PNETCDF, [test x$enable_pnetcdf = xyes])
AM_CONDITIONAL(USE_DISPATCH, [test x$enable_dispatch = xyes])
AM_CONDITIONAL(BUILD_CDMREMOTE, [test "x$enable_cdmremote" = xyes]) # Alias
AM_CONDITIONAL(BUILD_RPC, [test "x$enable_rpc" = xyes])
AM_CONDITIONAL(BUILD_DISKLESS, [test x$enable_diskless = xyes]) 
AM_CONDITIONAL(BUILD_MMAP, [test x$enable_mmap = xyes])

# If the machine doesn't have a long long, and we want netCDF-4, then
# we've got problems!
if test "x$enable_netcdf_4" = xyes; then
   AC_TYPE_LONG_LONG_INT
   AC_TYPE_UNSIGNED_LONG_LONG_INT
dnl    if test ! "x$ac_cv_type_long_long_int" = xyes -o ! "x$ac_cv_type_unsigned_long_long_int" = xyes; then
dnl       AC_MSG_ERROR([This platform does not support long long types. These are required for netCDF-4.])
dnl    fi
fi

# Create the file name for a "make ftpbin" which is used to generate a
# binary distribution. For each release we generate binary releases on
# the thousands of machines in Unidata's vast underground complex at
# an undisclosed location in the Rocky Mountains. The binary
# distributions, along with the 25-foot thick cement slabs and the
# giant springs, will help distribute netCDF even after a catastrophic
# meteor strike.
AC_MSG_CHECKING([what to call the output of the ftpbin target])
BINFILE_NAME=binary-netcdf-$PACKAGE_VERSION
test "x$enable_netcdf_4" = xno && BINFILE_NAME=${BINFILE_NAME}_nc3
BINFILE_NAME=${BINFILE_NAME}.tar
AC_SUBST(BINFILE_NAME)
AC_MSG_RESULT([$BINFILE_NAME $FC $CXX])

#UD_FTPBINDIR

AC_MSG_CHECKING([value of LIBS])
AC_MSG_RESULT([$LIBS])

# Flags for nc-config script; by design $prefix, $includir, $libdir,
# etc.  are left as shell variables in the script so as to facilitate
# relocation
if test "x$with_netcdf_c_lib" = x ; then 
   NC_LIBS="-lnetcdf"
else
   NC_LIBS="$with_netcdf_c_lib"
fi
if test "x$enable_shared" != xyes; then
   NC_LIBS="$LDFLAGS $NC_LIBS $LIBS"
fi	 

case "x$target_os" in
xsolaris*) 
  NEWNCLIBS=""
  for x in $NC_LIBS ; do
    case "$x" in
    -L*) r=`echo "$x" | sed -e 's|^-L|-R|'`
	 NEWNCLIBS="$NEWNCLIBS $x $r"
	 ;;
    *)	 NEWNCLIBS="$NEWNCLIBS $x" ;;
    esac
  done
  NC_LIBS="$NEWNCLIBS"
  ;;
*);;
esac

NC_FLIBS="-lnetcdff $NC_LIBS"

AC_SUBST(NC_LIBS,[$NC_LIBS])
AC_SUBST(HAS_DAP,[$enable_dap])
AC_SUBST(HAS_NC2,[$nc_build_v2])
AC_SUBST(HAS_NC4,[$enable_netcdf_4])
AC_SUBST(HAS_HDF4,[$enable_hdf4])
AC_SUBST(HAS_PNETCDF,[$enable_pnetcdf])
AC_SUBST(HAS_HDF5,[$enable_netcdf_4])
AC_SUBST(HAS_SZLIB,[$nc_has_szlib])

# Access netcdf specific version of config.h
AH_BOTTOM([#include "ncconfigure.h"])

##################################################
# Uncomment this to keep a copy of autoconf defines at this point, for
# debugging purposes.
# cp confdefs.h my_config.h


AC_MSG_NOTICE([generating header files and makefiles])
AC_CONFIG_FILES([Makefile
                 nc-config
                 netcdf.pc
                 include/Makefile
                 h5_test/Makefile
                 man4/Makefile
                 man4/images/Makefile
                 libsrc/Makefile
                 libsrc4/Makefile
                 nctest/Makefile
                 nc_test4/Makefile
                 nc_test/Makefile
                 ncdump/Makefile
                 ncgen3/Makefile
                 ncgen/Makefile
                 examples/Makefile
                 examples/C/Makefile
                 examples/CDL/Makefile
                 oc2/Makefile
                 libdap2/Makefile
                 libcdmr/Makefile
                 librpc/Makefile
                 libdispatch/Makefile
                 liblib/Makefile
                 ncdump/cdl4/Makefile
                 ncdump/expected4/Makefile
                 ncdap_test/Makefile
                 ncdap_test/testdata3/Makefile
                 ncdap_test/expected3/Makefile
                 ncdap_test/expected4/Makefile
                 ncdap_test/expectremote3/Makefile
                 ncdap_test/expectremote4/Makefile
		 ],
                 [test -f  nc-config && chmod 755 nc-config])
AC_OUTPUT()
