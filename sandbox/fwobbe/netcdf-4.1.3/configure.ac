#                                               -*- Autoconf -*-
## Process this file with autoconf to produce a configure script.

# This is part of Unidata's netCDF package. Copyright 2005-2011, see
# the COPYRIGHT file for more information.

# Recall that ${VAR-exp} expands to $VAR if var is set (even to null),
# and to exp otherwise. 

## This puts the cvs ID tag in the output configure script.
AC_REVISION([$Id$])

# Running autoconf on this file will trigger a warning if 
# autoconf is not at least the specified version.
AC_PREREQ([2.59])

# Initialize with name, version, and support email address. 
AC_INIT([netCDF], [4.1.3], [support-netcdf@unidata.ucar.edu])

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

# We will output a config.h for the C library, and a fortran inc file,
# nfconfig.inc.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_HEADERS([fortran/nfconfig1.inc:fortran/nfconfig.in], 
   [sed '1d;s%^/\* \(.*\) \*/%\1%' fortran/nfconfig1.inc >conftest.tmp && mv conftest.tmp fortran/nfconfig.inc])

# This call is required by automake.
AM_INIT_AUTOMAKE([foreign dist-bzip2 dist-zip dist-tarZ subdir-objects])

# Check for the existance of this file before proceeding.
AC_CONFIG_SRCDIR([libsrc/nc.c])

AC_MSG_NOTICE([checking user options])

# For backward compatibility, check to see if the user has set CXX,
# FC, and/or F90 to null. If so, treat that as the equivilent
# --disable- option to configure. The autoconf manual suggests the
# following to set a var if it's not already set:
# test "${var+set}" = set || var={value}

# Did the user set cxx to null?
AC_MSG_CHECKING([whether CXX is set to ''])
nc_null_cxx=no
test "x${CXX+set}" = xset && test "x$CXX" = x && nc_null_cxx=yes
AC_MSG_RESULT([$nc_null_cxx])

# Did the user set fc to null?
AC_MSG_CHECKING([whether FC is set to ''])
nc_null_fc=no
test "x${FC+set}" = xset && test "x$FC" = x && nc_null_fc=yes
AC_MSG_RESULT([$nc_null_fc])

# Did the user set f90 to null?
AC_MSG_CHECKING([whether F90 is set to ''])
nc_null_f90=no
test "x${F90+set}" = xset && test "x$F90" = x && nc_null_f90=yes
AC_MSG_RESULT([$nc_null_f90])

# Did the user specify a default minimum blocksize (NCIO_MINBLOCKSIZE) for posixio?
AC_MSG_CHECKING([whether a NCIO_MINBLOCKSIZE was specified])
AC_ARG_WITH([minblocksize],
              [AS_HELP_STRING([--with-minblocksize=<integer>],
                              [Specify minimum I/O blocksize for netCDF classic and 64-bit offset format files.])],
            [NCIO_MINBLOCKSIZE=$with_minblocksize], [NCIO_MINBLOCKSIZE=256])
AC_MSG_RESULT([$NCIO_MINBLOCKSIZE])
AC_DEFINE_UNQUOTED([NCIO_MINBLOCKSIZE], [$NCIO_MINBLOCKSIZE], [min blocksize for posixio.])

AC_MSG_CHECKING([whether udunits is to be built])
AC_ARG_WITH([udunits],
              [AS_HELP_STRING([--with-udunits],
                              [Build udunits2 package.])])
test "x$with_udunits" = xyes || with_udunits=no
AC_MSG_RESULT($with_udunits)

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
AC_ARG_ENABLE([netcdf-4], [AS_HELP_STRING([--disable-netcdf-4],
              [build with netcdf-4 (HDF5 and zlib required)])])
test "x$enable_netcdf_4" = xno || enable_netcdf_4=yes
AC_MSG_RESULT([$enable_netcdf_4])

# Synonym
# --enable-netcdf-4 is the controlling enable switch
if test "x$enable_netcdf_4" = "x" ; then
  AC_ARG_ENABLE([netcdf4], [AS_HELP_STRING([--enable-netcdf4],
              [build with netcdf-4 (HDF5 and zlib required)])])
  enable_netcdf_4="$enable_netcdf4"
else
  enable_netcdf4="$enable_netcdf_4"
fi

# Does the user want to also build the libcf library?
AC_MSG_CHECKING([whether libcf is to be built])
AC_ARG_WITH([libcf],
              [AS_HELP_STRING([--with-libcf],
                              [build and install libcf library, a library for \
	      handling data in conformance with the Climate and \
	      Forecast conventions. (Requires netCDF-4 and HDF5)])])
test "x$with_libcf" = xyes || with_libcf=no
AC_MSG_RESULT($with_libcf)

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

# Does the user want to build cxx-4?
AC_MSG_CHECKING([whether new netCDF-4 C++ API is to be built])
AC_ARG_ENABLE([cxx-4],
              [AS_HELP_STRING([--enable-cxx-4],
                              [build with new netcdf-4 C++ API])])
test "x$enable_cxx_4" = xyes || enable_cxx_4=no
if test "x$enable_netcdf_4" = xno; then
   enable_cxx_4=no
fi
AC_MSG_RESULT($enable_cxx_4)

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

# Capture the state of the --enable-dap flag
AC_MSG_CHECKING([whether DAP client is to be built])
AC_ARG_ENABLE([dap],
                 [AS_HELP_STRING([--disable-dap],
                                 [build without DAP client support.])])
test "x$enable_dap" = xno || enable_dap=yes
AC_MSG_RESULT($enable_dap)

# Curl support is required if these flags are set:
# 1. --enable-dap
# 2. --enable-cdmremote

# If --disable-dap && --disable cdmremote , then libcurl is not required
if test "x$enable_dap" = "xyes" -o "x$enable_cdmremote" = "xyes" ; then
require_curl=yes
else
require_curl=no
fi

# Do a lib test for curl
AC_CHECK_LIB([curl], [curl_easy_setopt])
found_curl=no
if test "x$ac_cv_lib_curl_curl_easy_setopt" = xyes ; then
found_curl=yes
fi

# If curl is required but there is no curl, then complain
if test $require_curl = yes -a $found_curl = no ; then
  AC_MSG_NOTICE([libcurl not found; disabling dap and cdmremote support])
  enable_dap=no
  enable_cdmremote=no
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

# Do we want to recover from a bad C++, F77, or F90 compiler?
nc_compiler_recover=yes
AC_MSG_CHECKING([whether to skip C++, F77, or F90 APIs if compiler is broken])
AC_ARG_ENABLE([compiler-recover],
              [AS_HELP_STRING([--disable-compiler-recover],
                              [don't continue if a desired C++, F77, or F90 \
                              compiler is missing; this is useful for testing \
                              netCDF, but is not very useful for most netCDF \
                              users])])
test "x$enable_compiler_recover" = "xno" || enable_compiler_recover=yes
AC_MSG_RESULT([$enable_compiler_recover])

nc_build_c=yes
nc_build_f77=yes
nc_build_f90=yes
nc_build_cxx=yes
nc_build_v2=yes
nc_build_utilities=yes
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

# Do we want to build the fortran 77 API? Check for --disable-f77.
AC_MSG_CHECKING([whether F77 API is desired])
AC_ARG_ENABLE([f77],
              [AS_HELP_STRING([--disable-f77],
                              [don't try to build the netCDF Fortran 77 (or \   
                              Fortran 90) API])])
test "x$enable_f77" = "xno" && nc_build_f77=no
test "x$nc_null_fc" = xyes && nc_build_f77=no
AC_MSG_RESULT([$nc_build_f77])

# Do we want to build the fortran 77 API? Check for --disable-fortran
# for Russ, who can never remember --disable-f77..
AC_MSG_CHECKING([whether any Fortran API is desired])
AC_ARG_ENABLE([fortran],
              [AS_HELP_STRING([--disable-fortran],
                              [don't try to build the netCDF Fortran 77 or \   
                              Fortran 90 APIs (same effect as --disable-f77)])])
test "x$enable_fortran" = "xno" && nc_build_f77=no
AC_MSG_RESULT([$nc_build_f77])

# Do we want to build the fortran 90 API?
AC_MSG_CHECKING([whether F90 API is desired])
AC_ARG_ENABLE([f90],
              [AS_HELP_STRING([--disable-f90],
                              [don't try to build the netCDF Fortran 90 API])])
test "x$enable_f90" = xno && nc_build_f90=no
test "x$nc_null_fc" = xyes || test "x$nc_null_f90" = xyes || \
     test "x$nc_build_f77" = xno && nc_build_f90=no
AC_MSG_RESULT([$nc_build_f90])

# Does the user want to check into fortran type information?
AC_MSG_CHECKING([whether fortran type sizes should be checked])
AC_ARG_ENABLE([fortran-type-check],
              [AS_HELP_STRING([--disable-fortran-type-check],
                              [cause the Fortran type sizes checks to be skipped])])
test "x$enable_fortran_type_check" = xno || enable_fortran_type_check=yes
AC_MSG_RESULT($enable_fortran_type_check)

# Do we want to build the C++ API? Check for --disable-cxx as a
# configure argument.
AC_MSG_CHECKING([whether CXX API is desired])
AC_ARG_ENABLE([cxx],
              [AS_HELP_STRING([--disable-cxx],
                              [don't try to build C++ API])])
test "x$enable_cxx" = xno && nc_build_cxx=no
test "x$nc_null_cxx" = xyes && nc_build_cxx=no
AC_MSG_RESULT([$nc_build_cxx])

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

# If --enable-dll is specified the DLL will be built. This only works
# on mingw.
AC_MSG_CHECKING([whether a win32 DLL is desired])
AC_ARG_ENABLE([dll],
              [AS_HELP_STRING([--enable-dll],
                              [build a win32 DLL (only works on mingw)])])
test "x$enable_dll" = xyes || enable_dll=no
AC_MSG_RESULT([$enable_dll])
AM_CONDITIONAL(BUILD_DLL, [test x$enable_dll = xyes])
if test "x$enable_dll" = xyes; then
   AC_DEFINE(DLL_NETCDF, 1, [set this only when building a DLL under MinGW])
fi

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

# If the user set environment var FC, but not F77, then set F77=FC
if test "x${FC+set}" = xset -a "x${F77+set}" != xset; then
   F77=${FC}
fi

# If F90 is set, and FC isn't, then set FC to whatever F90 was set to.
if test "${F90+set}" = set -a "${FC+set}" != set; then
   FC=$F90
fi

# If F77 is set, and FC (still) isn't, then set FC to whatever F77 was
# set to.
if test "${F77+set}" = set -a "${FC+set}" != set; then
   FC=$F77
fi

# Find fortran compiler. If FC isn't set after AC_PROG_FC, we couldn't
# find a fortran compiler. If the user requested the fortran API, and
# has told us not to recover from a missing compiler, then bail out
# right here.
AC_MSG_NOTICE([finding Fortran compiler (will not be used if Fortran API is not desired)])
AC_PROG_FC

# If we couldn't find an F90 compiler, deal with it.
if test "${FC+set}" = set && test "x$FC" = x; then
   if test "x$enable_compiler_recover" = xno && test "x$nc_build_f90" = xyes; then
      AC_MSG_ERROR([Can't find F90 compiler, and compiler recovery disabled.])
   fi
   AC_MSG_WARN([No F90 compiler found. F90 API will not be built.])
   nc_build_f90=no
fi

AC_PROG_F77($FC)

# Handle missing f77 compiler.
if test "${F77+set}" = set && test "x$F77" = x; then
   if test "x$enable_compiler_recover" = xno && test "x$nc_build_f77" = xyes; then
      AC_MSG_ERROR([Can't find F77 compiler, and compiler recovery disabled.])
   fi
   AC_MSG_WARN([No F77 compiler found. F77 API will not be built.])
   nc_build_f77=no
   nc_build_f90=no
fi

# See if the fortran 90 build is desired. If so, set some stuff
if test "x$nc_build_f90" = xyes; then
   AC_MSG_NOTICE([setting up Fortran 90])
   if test "${F90+set}" != set; then 
      F90=$FC 
   fi
   AC_MSG_CHECKING([what FC is set to])
   AC_MSG_RESULT([$FC])   

   # Some f90 compilers change the case of the mod file names. Others
   # require special flags to be set to dea with .f90 files. Find out
   # about this compiler.
   AC_LANG_PUSH(Fortran)
   AC_FC_SRCEXT(f90)
   AC_LANG_POP(Fortran)
   AX_F90_MODULE_FLAG
   AC_SUBST(MOD_FLAG, ${ax_cv_f90_modflag})

   # Set the FCLIBS flag to help with shared libraries.
#   AC_FC_LIBRARY_LDFLAGS
#   AC_F77_LIBRARY_LDFLAGS      
fi

AC_MSG_NOTICE([finding C++ compiler (will not be used if C++ API is not desired)])
AC_PROG_CXX

# Set these conditionals for automake. For some reason, it can't keep
# it's SUBDIR order straight if I use BUILD_F77 to attach the nf_test
# directory, so also define an automake conditional TEST_F77 every
# time BUILD_F77 is defined, and set it to the same value.
AM_CONDITIONAL(BUILD_C, [test "x$nc_build_c" = xyes])
AM_CONDITIONAL(BUILD_F77, [test "x$nc_build_f77" = xyes])
AM_CONDITIONAL(TEST_F77, [test "x$nc_build_f77" = xyes])
AM_CONDITIONAL(BUILD_F90, [test "x$nc_build_f90" = xyes])
AM_CONDITIONAL(BUILD_CXX, [test "x$nc_build_cxx" = xyes])

# Set up libtool.
AC_MSG_NOTICE([setting up libtool])
LT_PREREQ([2.4])
LT_INIT(win32-dll)

# Valgrind tests don't work with shared builds because of some libtool
# weirdness.
if test "x$enable_shared" = xyes; then
   if test $enable_valgrind_tests = yes; then
      AC_MSG_ERROR([No valgrind tests with shared libraries])
   fi
fi

AC_MSG_NOTICE([finding other utilities])

AC_PROG_INSTALL

if test "x$nc_build_f77" = xyes; then
   AC_MSG_NOTICE([trying to set fortran flags for this platform])
   # Set fortran flag if the user has not already set it in CPPFLAGS.
   case "$CPPFLAGS" in
    *pgiFortran*|*NAGf90Fortran*|*f2cFortran*|*hpuxFortran*|*apolloFortran*|*sunFortran*|*IBMR2Fortran*|*CRAYFortran*|*PATHSCALE_COMPILER*|*gFortran*|*mipsFortran*|*DECFortran*|*vmsFortran*|*CONVEXFortran*|*PowerStationFortran*|*AbsoftUNIXFortran*|*AbsoftProFortran*|*SXFortran*)
       ;;
      *)
         case "$FC" in
            pgf95|pgf90|pgf77|ifort*|lf90|lf95)
               AC_DEFINE(pgiFortran, [1], [Turned on by netCDF configure.])
               ;;
             *gfortran) 
                if (gfortran --version | grep '(GCC 4.1.' || gfortran --version | grep '(GCC 4.0.'); then
                   AC_DEFINE(gFortran, [1], [Turned on by netCDF configure.])    
                else
                   AC_DEFINE(pgiFortran, [1], [Turned on by netCDF configure.])    
                fi
                ;;
             *g77) 
                AC_DEFINE(f2cFortran, [1], [Turned on by netCDF configure.])
                ;;
             *)
                # If it's a gnu compiler, guess f2c.
                if test "x$ac_cv_fc_compiler_gnu" = xyes; then
                   AC_DEFINE(f2cFortran, [1], [Turned on by netCDF configure.])
                fi
                ;;
         esac
   esac
fi 

# Check to see if any macros must be set to enable large (>2GB) files.
AC_SYS_LARGEFILE

AC_MSG_NOTICE([displaying some results])

## This next macro just prints some results for debugging
## support issues.
UD_DISPLAY_RESULTS

# For nightly build testing, output CC, FC, etc.
echo "CPPFLAGS=$CPPFLAGS CC=$CC CFLAGS=$CFLAGS CXX=$CXX CXXFLAGS=$CXXFLAGS FC=$FC FCFLAGS=$FCFLAGS F77=$F77 FFLAGS=$FFLAGS LDFLAGS=$LDFLAGS LIBS=$LIBS FLIBS=$FLIBS F90LIBS=$F90LIBS" >> comps.txt

ac_cv_prog_f90_uppercase_mod=no

# This will test the f77 compiler, as well as check some fortran types.
if test "x$nc_build_f77" = xyes; then

   if test "$enable_fortran_type_check" = yes; then
      UD_FORTRAN_TYPES
      if test "x$ud_fc_failed" = xyes -a "x$enable_compiler_recover" = xno -a "x$nc_build_f77" = xyes; then
         AC_MSG_ERROR([F77 compiler doesn't work, and compiler recovery disabled.])
      fi
   else
      AC_DEFINE(NCBYTE_T, byte)
      AC_DEFINE(NCSHORT_T, integer*2)
      AC_DEFINE(NF_INT1_T, byte)
      AC_DEFINE(NF_INT2_T, integer*2)
      AC_DEFINE(NF_INT1_IS_C_SIGNED_CHAR, 1, [default])
      AC_DEFINE(NF_INT2_IS_C_SHORT, 1, [default])
      AC_DEFINE(NF_INT_IS_C_INT, 1, [default])
      AC_DEFINE(NF_REAL_IS_C_FLOAT, 1, [default])
      AC_DEFINE(NF_DOUBLEPRECISION_IS_C_DOUBLE, 1, [default])
   fi

   # Test whether F90 module names get capitalized.
   if test "x$nc_build_f90" = xyes; then
      AC_PROG_FC_UPPERCASE_MOD
   fi
fi
AM_CONDITIONAL(UPPER_CASE_MOD, [test "x$ac_cv_prog_f90_uppercase_mod" = xyes])

AC_MSG_NOTICE([checking types, headers, and functions])

AC_CHECK_HEADER(stdlib.h, ,AC_DEFINE([NO_STDLIB_H], [], [no stdlib.h]))
AC_CHECK_HEADER(sys/types.h, ,AC_DEFINE([NO_SYS_TYPES_H], [], [no sys_types.h]))
#AC_CHECK_HEADERS([locale.h])
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS([locale.h stdio.h stdarg.h errno.h ctype.h fcntl.h malloc.h stdlib.h string.h \
			   strings.h unistd.h sys/stat.h getopt.h sys/time.h sys/resource.h])
AC_FUNC_VPRINTF

# Check for <stdbool.h> that conforms to C99 requirements
AC_HEADER_STDBOOL

# Check for these functions...
AC_CHECK_FUNCS([strlcat strerror snprintf strchr strrchr mktemp strcat strcpy strdup \
			strcasecmp strtod strtoll strtoull getrlimit gettimeofday fsync MPI_Comm_f2c])
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
AC_CHECK_SIZEOF(float)
AC_CHECK_SIZEOF(double)
AC_CHECK_SIZEOF(off_t)
AC_CHECK_SIZEOF(size_t)

if test "x$enable_netcdf_4" = xyes || test "x$enable_dap" = xyes; then
   AC_SEARCH_LIBS([deflate], [zlib1 z], [], [
     AC_MSG_ERROR([Can't find or link to the z library. Turn off netCDF-4 and \
     opendap with --disable-netcdf-4 --disable-dap, or see config.log for errors.])])
fi

# We need the math library.
AC_CHECK_LIB([m], [floor], [], 
[AC_MSG_ERROR([Can't find or link to the math library.])])

if test "x$enable_netcdf_4" = xyes; then
   AC_DEFINE([USE_NETCDF4], [1], [if true, build netCDF-4])
   AC_DEFINE([H5_USE_16_API], [1], [use HDF5 1.6 API])

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
      AC_SEARCH_LIBS([SZ_Compress], [szip], [], 
      [AC_MSG_ERROR([Can't find or link to the szlib library. Use --disable-netcdf-4, or see config.log for errors.])])
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

# Automake conditionals need to be called, whether the answer is yes
# or no.
AM_CONDITIONAL(BUILD_PARALLEL, [test x$enable_parallel = xyes])
AM_CONDITIONAL(TEST_PARALLEL, [test "x$enable_parallel" = xyes -a "x$enable_parallel_tests" = xyes])
AM_CONDITIONAL(BUILD_DAP, [test "x$enable_dap" = xyes])
AM_CONDITIONAL(USE_DAP, [test "x$enable_dap" = xyes]) # Alias
AM_CONDITIONAL(ENABLE_DAP_REMOTE_TESTS, [test "x$enable_dap_remote_tests" = xyes])
AM_CONDITIONAL(ENABLE_DAP_LONG_TESTS, [test "x$enable_dap_long_tests" = xyes])
AM_CONDITIONAL(BUILD_CXX4, [test x$enable_cxx_4 = xyes])
AM_CONDITIONAL(EXTRA_EXAMPLE_TESTS, [test "x$enable_extra_example_tests" = xyes])
AM_CONDITIONAL(USE_SZIP, [test "x$ac_cv_func_H5Z_SZIP" = xyes])
AM_CONDITIONAL(USE_PNETCDF_DIR, [test ! "x$PNETCDFDIR" = x])
AM_CONDITIONAL(USE_LOGGING, [test "x$enable_logging" = xyes])
AM_CONDITIONAL(BUILD_UDUNITS, [test "x$with_udunits" = xyes])
AM_CONDITIONAL(BUILD_LIBCF, [test "x$with_libcf" = xyes])
AM_CONDITIONAL(CROSS_COMPILING, [test "x$cross_compiling" = xyes])
AM_CONDITIONAL(USE_VALGRIND_TESTS, [test "x$enable_valgrind_tests" = xyes])
AM_CONDITIONAL(USE_NETCDF4, [test x$enable_netcdf_4 = xyes])
AM_CONDITIONAL(USE_HDF4, [test x$enable_hdf4 = xyes])
AM_CONDITIONAL(USE_HDF4_FILE_TESTS, [test x$enable_hdf4_file_tests = xyes])
AM_CONDITIONAL(USE_RENAMEV3, [test x$enable_netcdf_4 = xyes -o x$enable_dap = xyes])
AM_CONDITIONAL(USE_PNETCDF, [test x$enable_pnetcdf = xyes])
AM_CONDITIONAL(USE_DISPATCH, [test x$enable_dispatch = xyes])
AM_CONDITIONAL(BUILD_CDMREMOTE, [test "x$enable_cdmremote" = xyes]) # Alias

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
test "x$nc_build_f77" = xyes && BINFILE_NAME=${BINFILE_NAME}_${F77}
test "x$nc_build_f90" = xyes && BINFILE_NAME=${BINFILE_NAME}_${F90}
test "x$nc_build_cxx" = xyes && BINFILE_NAME=${BINFILE_NAME}_${CXX}
BINFILE_NAME=${BINFILE_NAME}.tar
AC_SUBST(BINFILE_NAME)
AC_MSG_RESULT([$BINFILE_NAME $FC $CXX])

UD_FTPBINDIR

##################################################
# Require some version of xdr/rpc if --enable-dap is set.
# See if autoconf can find it; check libc, librpc, librpcsvc and libnsl
if test "x$enable_dap" = xyes; then
   AC_SEARCH_LIBS([xdr_void],[c rpc nsl rpcsvc],
   		  [dap_xdrlib=$ac_res],[dap_xdrlib=])
  if test -z "$dap_xdrlib" -a "x$enable_dap" = "xyes"; then
    AC_MSG_ERROR(Cannot locate library containing xdr functions.)
  else
  if test "$dap_xdrlib" = "lc" ; then
    AC_MSG_NOTICE(XDR Functions appear to be in libc.)	
  elif test "$dap_xdrlib" = "none required" ; then
    AC_MSG_NOTICE(XDR Functions appear to be in libc.)	
  else
    dap_xdrlib=`echo $dap_xdrlib | sed -e s/-l//g`
    AC_MSG_NOTICE(XDR Functions appear to be in lib${dap_xdrlib}.)	
    # Add to library list
    AC_CHECK_LIB($dap_xdrlib,xdr_void)
  fi
  # Fix to get working on OS X (thanks to Fedor Baart)
  AC_CHECK_HEADERS([rpc/types.h rpc/xdr.h], [
       AC_DEFINE([HAVE_RPC_TYPES_H], [], [no rpc/types.h])
       AC_DEFINE([HAVE_RPC_XDR_H], [], [no rpc/xdr.h])
       ], [], [[#ifdef HAVE_RPC_TYPES_H
  # include <rpc/types.h>
  #endif]])
fi
fi
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
AC_SUBST(NC_FLIBS,[$NC_FLIBS])
AC_SUBST(HAS_DAP,[$enable_dap])
AC_SUBST(HAS_NC2,[$nc_build_v2])
AC_SUBST(HAS_NC4,[$enable_netcdf_4])
AC_SUBST(HAS_HDF4,[$enable_hdf4])
AC_SUBST(HAS_PNETCDF,[$enable_pnetcdf])
AC_SUBST(HAS_HDF5,[$enable_netcdf_4])
AC_SUBST(HAS_F77,[$nc_build_f77])
AC_SUBST(HAS_F90,[$nc_build_f90])
AC_SUBST(HAS_CXX,[$nc_build_cxx])
AC_SUBST(HAS_SZLIB,[$nc_has_szlib])

AC_CONFIG_SUBDIRS([udunits libcf])

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
                 fortran/Makefile
                 libsrc/Makefile
                 libsrc4/Makefile
                 nctest/Makefile
                 nc_test4/Makefile
                 nc_test/Makefile
                 ncdump/Makefile
                 ncgen3/Makefile
                 ncgen/Makefile
                 nf_test/Makefile
                 cxx/Makefile
                 cxx4/Makefile
                 f90/Makefile
                 examples/Makefile
                 examples/C/Makefile
                 examples/CDL/Makefile
                 examples/CXX/Makefile
                 examples/CXX4/Makefile
                 examples/F90/Makefile
                 examples/F77/Makefile
                  oc/Makefile
                  libdap2/Makefile
                  libcdmr/Makefile
                  libdispatch/Makefile
                  liblib/Makefile
                  ncdap_test/Makefile
                  ncdap_test/testdata3/Makefile
                  ncdap_test/expected3/Makefile
                  ncdap_test/expected4/Makefile
                  ncdap_test/expectremote3/Makefile
                  ncdap_test/expectremote4/Makefile
                  ncdump/cdl4/Makefile
                  ncdump/expected4/Makefile],
                 [test -f  nc-config && chmod 755 nc-config ])

AC_OUTPUT()

