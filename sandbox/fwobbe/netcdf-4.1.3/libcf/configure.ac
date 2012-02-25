#                                               -*- Autoconf -*-
## Process this file with autoconf to produce a configure script.

# This is part of Unidata's NetCDF CF Library. package. Copyright
# 2006, see the COPYRIGHT file for more information.

## This puts the cvs ID tag in the output configure script.
AC_REVISION([$Id$])

## Running autoconf on this file will trigger a warning if 
## autoconf is not at least the specified version.
AC_PREREQ([2.59])

## Initialize with name, version, and support email address. 
AC_INIT([libcf], [1.0-alpha5], [support@unidata.ucar.edu])
AC_CONFIG_MACRO_DIR([m4])

AC_MSG_NOTICE([libcf AC_PACKAGE_VERSION])

# Find out about the host we're building on.
AC_CANONICAL_HOST

# We will output a config.h for the C library, and a fortran inc file,
# nfconfig.inc.
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_HEADERS([nfconfig.inc:nfconfig.in], 
   [sed '1d;s%^/\* \(.*\) \*/%\1%' nfconfig.inc >conftest.tmp && mv conftest.tmp nfconfig.inc])

# This call is required by automake.
AM_INIT_AUTOMAKE([foreign no-installinfo dist-bzip2 subdir-objects -Wall])

# Check for the existance of this file before proceeding.
AC_CONFIG_SRCDIR([src/nccoord.c])

AC_MSG_NOTICE([checking user options])

# Turn off shared libraries by default
AM_DISABLE_SHARED

# The autoconf manual suggests the following to set a var if it's not
# already set: test "${var+set}" = set || var={value}

# For backward compatibility, check to see if the user has set FC to
# null. If so, treat that as the --disable-fortran option to
# configure.

# Did the user set fc to null?
AC_MSG_CHECKING([whether FC is set to ''])
nc_null_fc=no
test "x${FC+set}" = xset && test "x$FC" = x && nc_null_fc=yes
AC_MSG_RESULT([$nc_null_fc])

# Does the user want to build documentation?
AC_MSG_CHECKING([whether documentation is to be built])
AC_ARG_ENABLE([docs],
              [AS_HELP_STRING([--enable-docs],
                              [build the libcf documentation (a built version is already \
			      included with the distrubution)])])
test "x$enable_docs" = xyes || enable_docs=no
AC_MSG_RESULT($enable_docs)
AM_CONDITIONAL(BUILD_DOCS, [test x$enable_docs = xyes])

# Does the user want to build gridspec tools?
AC_MSG_CHECKING([whether gridspec tools are to be built])
AC_ARG_ENABLE([gridspec],
              [AS_HELP_STRING([--enable-gridspec],
                              [build the gridspec tools])])
test "x$enable_gridspec" = xyes || enable_gridspec=no
AC_MSG_RESULT($enable_gridspec)
AM_CONDITIONAL(BUILD_GRIDSPEC, [test x$enable_gridspec = xyes])

# Does the user want to fetch test data and run extra gridspec tests?
AC_MSG_CHECKING([whether gridspec test data should be fetched from Unidata FTP site])
AC_ARG_ENABLE([gridspec-file-tests],
              [AS_HELP_STRING([--enable-gridspec-file-tests],
                              [get GRIDSPEC test data from Unidata FTP server and run extra tests])])
test "x$enable_gridspec_file_tests" = xyes || enable_gridspec_file_tests=no
AC_MSG_RESULT($enable_gridspec_file_tests)
AM_CONDITIONAL(BUILD_GRIDSPEC_FILE_TESTS, [test x$enable_gridspec_file_tests = xyes])

# Does the user want to build cfcheck?
AC_MSG_CHECKING([whether cfcheck is to be built])
AC_ARG_ENABLE([cfcheck],
              [AS_HELP_STRING([--disable-cfcheck],
                              [don't build the cfcheck application])])
test "x$enable_cfcheck" = xno || enable_cfcheck=yes
AC_MSG_RESULT($enable_cfcheck)
AM_CONDITIONAL(BUILD_CFCHECK, [test x$enable_cfcheck = xyes])

# Does the user want to enable logging?
AC_MSG_CHECKING([whether logging is enabled (useful to CF developers, mainly])
AC_ARG_ENABLE([logging],
              [AS_HELP_STRING([--enable-logging],
                              [enable logging capability. This debugging features \
			      is only of interest to libcf developers.])])
test "x$enable_logging" = "xyes" || enable_logging=no
AC_MSG_RESULT([$enable_logging])
if test "x$enable_logging" = xyes; then
   AC_DEFINE([LOGGING], 1, [if true, turn on logging])
fi

# Does the user want to use netcdf-4?
AC_MSG_CHECKING([whether netCDF-4 is to be used])
AC_ARG_ENABLE([netcdf-4],
              [AS_HELP_STRING([--enable-netcdf-4],
                              [build with netcdf-4 (HDF5 is required)])])

# If hdf5 dir is set, turn on enable-netcdf-4.
if test "x$with_hdf5" != x; then
   enable_netcdf_4=yes
else
   test "x$enable_netcdf_4" = xyes || enable_netcdf_4=no
fi
AC_MSG_RESULT($enable_netcdf_4)
AM_CONDITIONAL(USE_NETCDF4, [test x$enable_netcdf_4 = xyes])
if test "x$enable_netcdf_4" = xyes; then
   AC_DEFINE([USE_NETCDF4], 1, [if true, use netCDF-4])
fi

# Does the user want to turn on HDF4 read ability?
AC_ARG_ENABLE([hdf4], [AS_HELP_STRING([--enable-hdf4],
              [build netcdf-4 with HDF4 read capability (HDF4, HDF5 and zlib required)])])

# Does the user want extra tests on CF sample files?
AC_MSG_CHECKING([whether extra tests on sample data should be performed])
AC_ARG_ENABLE([extra-data-tests],
              [AS_HELP_STRING([--enable-extra-data-tests],
                              [Perform extra tests using CF example files from Unidata web site \
			      (example files must be donloaded separately). This option should \
			      only be used by those with excessive interest in testing libcf.])])
test "x$enable_extra_data_tests" = xyes || enable_extra_data_tests=no
AC_MSG_RESULT($enable_extra_data_tests)
AM_CONDITIONAL(EXTRA_DATA_TESTS, [test x$enable_extra_data_tests = xyes])

# Did the user specify a location for the HDF5 library?
AC_MSG_CHECKING([whether a location for the HDF5 library was specified])
AC_ARG_WITH([hdf5],
              [AS_HELP_STRING([--with-hdf5=<directory>],
                              [Specify location of HDF5 library. Configure will expect \
			      to find subdirs include and lib.])],
            [HDF5DIR=$with_hdf5])
AC_MSG_RESULT([$HDF5DIR])
AM_CONDITIONAL(USE_HDF5_DIR, [test ! "x$HDF5DIR" = x])
AC_SUBST(HDF5DIR, [$HDF5DIR])
if test ! "x$HDF5DIR" = x; then
   LDFLAGS="${LDFLAGS} -L$HDF5DIR/lib"
fi

# Did the user specify a location for the HDF4 library?
AC_MSG_CHECKING([whether a location for the HDF4 library was specified])
AC_ARG_WITH([hdf4],
              [AS_HELP_STRING([--with-hdf4=<directory>],
                              [Specify location of HDF4 library. Configure will \
			      expect to find subdirs include and lib.])],
            [HDF4DIR=$with_hdf4])
AC_MSG_RESULT([$HDF4DIR])
AM_CONDITIONAL(USE_HDF4_DIR, [test ! "x$HDF4DIR" = x])
AC_SUBST(HDF4DIR, [$HDF4DIR])
AM_CONDITIONAL(USE_HDF4, [test x$enable_hdf4 = xyes])

# Did the user specify a location for the ZLIB library?
AC_MSG_CHECKING([whether a location for the ZLIB library was specified])
AC_ARG_WITH([zlib],
              [AS_HELP_STRING([--with-zlib=<directory>],
                              [Specify location of ZLIB library. ZLIB is required \
			      for netCDF-4. Configure will expect to find subdirs \
			      include and lib.])],
            [ZLIBDIR=$with_zlib])
AC_MSG_RESULT([$ZLIBDIR])
AC_SUBST(ZLIBDIR, [$ZLIBDIR])

# Did the user specify a location for the SZLIB library?
AC_MSG_CHECKING([whether a location for the SZLIB library was specified])
AC_ARG_WITH([szlib],
              [AS_HELP_STRING([--with-szlib=<directory>],
                              [Specify location of SZLIB library. SZLIB is not \
			      required for netCDF-4, but you must specify its location \
			      if HDF5 was built with SZLIB. Configure will expect to \
			      find subdirs include and lib.])],
            [SZLIBDIR=$with_szlib])
AC_MSG_RESULT([$SZLIBDIR])
AC_SUBST(SZLIBDIR, [$SZLIBDIR])

AM_CONDITIONAL(USE_ZLIB_DIR, [test ! "x$ZLIBDIR" = x])
AM_CONDITIONAL(USE_SZLIB_DIR, [test ! "x$SZLIBDIR" = x])

# Do we want to build the fortran API? Check for --disable-f90
nc_build_f90=yes
AC_MSG_CHECKING([whether Fortran 90 API is desired])
AC_ARG_ENABLE([f90],
              [AS_HELP_STRING([--disable-f90],
                              [don't try to build Fortran 90 API])])
test "x$enable_f90" = "xno" && nc_build_f90=no
test "x$nc_null_fc" = xyes && nc_build_f90=no
AC_MSG_RESULT([$nc_build_f90])
AM_CONDITIONAL(BUILD_F90, [test "x$nc_build_f90" = xyes])

# Did the user specify a location for the netcdf library?
AC_MSG_CHECKING([whether a location for the netCDF library was specified])
AC_ARG_WITH([netcdf],
              [AS_HELP_STRING([--with-netcdf=<directory>],
                              [Specify location of netCDF library. Configure will expect to find \
			      subdirs include and lib.])], [], [with_netcdf=no])
AC_MSG_RESULT([$with_netcdf])
AM_CONDITIONAL([USE_NETCDF_DIR], [test ! x$with_netcdf = xno])
if test ! x$with_netcdf = xno; then
   AC_SUBST(NETCDFDIR, [$with_netcdf])
   AC_SUBST(NCDUMPDIR, [$with_netcdf/bin])
   LDFLAGS="${LDFLAGS} -L$NETCDFDIR/lib"
else
   AC_SUBST(NCDUMPDIR, [../../../../ncdump])
fi

# According to the autoconf mailing list gurus, we must test for
# compilers unconditionally. That is, we can't skip looking for the
# fortran compilers, just because the user doesn't want fortran. This
# is due to a limitation in autoconf.

# Find the C compiler.
AC_MSG_NOTICE([finding C compiler])
AC_PROG_CC(mpcc_r xlc cc c89 acc gcc)
AM_PROG_CC_C_O

# Find fortran compiler. If FC isn't set after AC_PROG_FC, we couldn't
# find a fortran compiler. If the user requested the fortran API, then
# bail out right here.
AC_MSG_NOTICE([finding Fortran compiler (required, even if Fortran API is not desired)])

# On solaris, f95 comes before gfortran.
case "$host" in
  *solaris*) 
             F77S="xlf fort xlf95 ifort ifc efc pgf77 lf95 f95 f90 g95 gfortran f77 frt pgf95 fort77 fl32 af77 f90 xlf90 pgf90 epcf90 g77"
             F90S="xlf90 fort xlf95 ifort ifc efc pgf95 lf95 f95 f90 g95 gfortran xlf90 pgf90 epcf90"
             ;;
  *) 
             F77S="xlf fort xlf95 ifort ifc efc pgf77 lf95 g95 gfortran f77 frt pgf95 f95 f90 fort77 fl32 af77 f90 xlf90 pgf90 epcf90 g77"
             F90S="xlf90 fort xlf95 ifort ifc efc pgf95 lf95 g95 gfortran f95 f90 f90 xlf90 pgf90 epcf90"
             ;;
esac

# Find a fortran 90 compiler, if possible.
# MingW has a wierd problem. If arguments are passed to AC_PROG_FC,
# the link of tests won't work (you get a multiply defined atexit
# function.) This is annoying, but there it is.
if test "$host" = "i686-pc-mingw32"; then
   AC_PROG_FC
else
   AC_PROG_FC($F90S)
fi

# If we couldn't find an F90 compiler, deal with it.
if test "${FC+set}" = set && test "x$FC" = x; then
   AC_MSG_WARN([No F90 compiler found. F90 API will not be built.])
   nc_build_f90=no
fi

# We must call AC_PROG_F77 here, otherwise the LIBTOOL macro below
# will call it, and reset F77 to g77.
AC_PROG_F77($F77S)
if test "${F77+set}" = set && test "x$F77" = x; then
   AC_MSG_WARN([No F77 compiler found. F77 API will not be built.])
   nc_build_f77=no
fi

if test "x$nc_build_f90" = xno; then
   F90=
   # If F77 is set, and FC isn't, then set FC to whatever F77 was set to.
   FC=$F77
fi

ac_cv_prog_f90_uppercase_mod=no
if test "x$nc_build_f90" = xyes; then
   # Some f90 compilers change the case of the mod file names. Others
   # require special flags to be set to deal with .f90 files. Find out
   # about this compiler.
   AC_LANG_PUSH(Fortran)
   AC_FC_SRCEXT(f90)
   AC_LANG_POP(Fortran)
   AX_F90_MODULE_FLAG
   AC_SUBST(MOD_FLAG, ${ax_cv_f90_modflag})
   AC_PROG_FC_UPPERCASE_MOD	
fi
AM_CONDITIONAL(UPPER_CASE_MOD, [test "x$ac_cv_prog_f90_uppercase_mod" = xyes])

AC_MSG_NOTICE([finding other utilities])

AC_PROG_INSTALL

AC_MSG_NOTICE([setting up libtool])

# Produce a windows DLL. This macro must be before AM_PROG_LIBTOOL.
AC_LIBTOOL_WIN32_DLL

# Use libtool.
AC_PROG_LIBTOOL

AC_MSG_NOTICE([checking for large file support])

AC_SYS_LARGEFILE

AC_MSG_NOTICE([checking types, headers, and functions])

AC_CHECK_HEADER(stdlib.h, ,AC_DEFINE([NO_STDLIB_H], [], [no stdlib.h]))
AC_CHECK_HEADER(sys/types.h, ,AC_DEFINE([NO_SYS_TYPES_H], [], [no sys_types.h]))
AC_CHECK_HEADER([unistd.h])
AC_CHECK_FUNC(strerror, ,AC_DEFINE([NO_STRERROR], [], [no strerror.h]))
AC_CHECK_FUNC(strlcat,AC_DEFINE([HAVE_STRLCAT],1,[Define this if you have strlcat()]))
AC_FUNC_ALLOCA
AC_STRUCT_ST_BLKSIZE
AC_TYPE_SIZE_T
AC_TYPE_OFF_T
AC_CHECK_TYPES([ssize_t, ptrdiff_t, uchar])
AC_C_CHAR_UNSIGNED
AC_C_BIGENDIAN
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(float)
AC_CHECK_SIZEOF(double)
AC_CHECK_SIZEOF(off_t)
AC_CHECK_SIZEOF(size_t)

AC_SEARCH_LIBS([floor], [m])

AC_MSG_NOTICE([generating header files and makefiles])


# The following flags are defined
# Define the -L flags and libs for external libraries
EXTERN_LDFLAGS=""
# Define the -I flags for external libraries
EXTERN_CFLAGS=""

# Add all libraries even if shared is enabled
if test "x$enable_netcdf_4" = xyes ; then 
  if test "x$HDF5DIR" != x ; then 
    EXTERN_LDFLAGS="${EXTERN_LDFLAGS} -L$HDF5DIR/lib"
    EXTERN_CFLAGS="${EXTERN_CFLAGS} -I$HDF5DIR/include"
  fi         
  EXTERN_LDFLAGS="${EXTERN_LDFLAGS} -lhdf5_hl -lhdf5"
  if test "x$enable_hdf4" = xyes; then
    if test "x$HDF4DIR" != x ; then 
      EXTERN_LDFLAGS="${EXTERN_LDFLAGS} -L$HDF4DIR/lib"
      EXTERN_CFLAGS="${EXTERN_CFLAGS} -I$HDF4DIR/include"
    fi      
    EXTERN_LDFLAGS="${EXTERN_LDFLAGS} -lmfhdf -ldf"
  fi
fi # netcdf-4
# Do zlib and szlib no matter-what
if test "x$ZLIBDIR" != x ; then 
  EXTERN_LDFLAGS="${EXTERN_LDFLAGS} -L$ZLIBDIR/lib"
  EXTERN_CFLAGS="${EXTERN_CFLAGS} -I$ZLIBDIR/include"
fi
EXTERN_LDFLAGS="${EXTERN_LDFLAGS} -lz"
if test "x$SZLIBDIR" != x ; then 
  EXTERN_LDFLAGS="${EXTERN_LDFLAGS} -L$SZLIBDIR/lib -lsz"
  EXTERN_CFLAGS="${EXTERN_CFLAGS} -I$SZLIBDIR/include"
fi

if test "x$enable_dap" = xyes ; then 
  if test "x$curl_libs" != x ; then
    EXTERN_LDFLAGS="${EXTERN_LDFLAGS} $curl_libs"
    EXTERN_CFLAGS="${EXTERN_CFLAGS} $curl_cflags"
  fi
  # If we are building with dap support check to see if we need to add
  # external oc library
  if test "x$with_oclib" != x ; then
    EXTERN_LDFLAGS="${EXTERN_LDFLAGS} -L$OCLIBDIR/lib -loc"
    EXTERN_CFLAGS="${EXTERN_CFLAGS} -I$OCLIBDIR/include"
  fi
fi

# If we are running on Solaris, then we need to add -R flags as well

case "x$target_os" in
xsolaris*) 
  NEWLDFLAGS=""
  for x in $EXTERN_LDFLAGS ; do
    case "$x" in
    -L*) r=`echo "$x" | sed -e 's|^-L|-R|'`
	 NEWLDFLAGS="$NEWLDFLAGS $x $r"
	 ;;
    *)	 NEWLDFLAGS="$NEWLDFLAGS $x" ;;
    esac
  done
  EXTERN_LDFLAGS="$NEWLDFLAGS"
  ;;
*);;
esac
AC_SUBST(EXTERN_CFLAGS,[$EXTERN_CFLAGS])
AC_SUBST(EXTERN_LDFLAGS,[$EXTERN_LDFLAGS])
AC_SUBST(EXTERN_CPPFLAGS,[$EXTERN_CFLAGS])

##################################################



AC_CONFIG_FILES([gridspec/tools/make_hgrid/run_tests], 
	           [chmod +x gridspec/tools/make_hgrid/run_tests])
AC_CONFIG_FILES([gridspec/tools/make_vgrid/run_tests], 
	           [chmod +x gridspec/tools/make_vgrid/run_tests])
AC_CONFIG_FILES([gridspec/tools/make_topog/run_tests], 
	           [chmod +x gridspec/tools/make_topog/run_tests])

AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 cfcheck/Makefile
                 src/Makefile])
dnl 		 gridspec/Makefile
dnl 		 gridspec/shared/Makefile
dnl 		 gridspec/shared/mosaic/Makefile
dnl 		 gridspec/tools/Makefile
dnl 		 gridspec/tools/shared/Makefile
dnl 		 gridspec/tools/make_hgrid/Makefile
dnl 		 gridspec/tools/make_solo_mosaic/Makefile
dnl 		 gridspec/tools/make_coupler_mosaic/Makefile
dnl 		 gridspec/tools/fregrid/Makefile
dnl 		 gridspec/tools/make_topog/Makefile
dnl 		 gridspec/tools/make_vgrid/Makefile
dnl 		 gridspec/tools/river_regrid/Makefile
dnl 		 gridspec/tools/transfer_to_mosaic_grid/Makefile])

AC_OUTPUT()

