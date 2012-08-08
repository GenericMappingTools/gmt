#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# Copyright 2008, 2009 University Corporation for Atmospheric Research
#
# This file is part of the UDUNITS-2 package.  See the file LICENSE
# in the top-level source-directory of the package for terms and
# conditions.
#

AC_PREREQ(2.59)
AC_INIT(UDUNITS, 2.1.21, support-udunits@unidata.ucar.edu)
AC_CONFIG_SRCDIR([lib/converter.c])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_HEADERS([config.h expat/expat_config.h])

CFLAGS_COVERAGE=''
LIBS_COVERAGE=''

AC_ARG_ENABLE([coverage],
[AS_HELP_STRING([--enable-coverage],[Turn on code-coverage support])],
[case "${enableval}" in
  yes) CFLAGS_COVERAGE='--coverage'
       LIBS_COVERAGE=-lgcov
       coverage_enabled=true;;
  no) ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-coverage]) ;;
esac])
AC_SUBST(CFLAGS_COVERAGE)
AC_SUBST(LIBS_COVERAGE)

AC_ARG_ENABLE([debug],
[AS_HELP_STRING([--enable-debug],[Turn on debugging support])],
[case "${enableval}" in
  yes)
    CFLAGS="-g${CFLAGS:+ $CFLAGS}"
    debug=true ;;
  no)
    CFLAGS="-O${CFLAGS:+ $CFLAGS}"
    debug=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[if test "$coverage_enabled" = true; then
    CFLAGS="-g${CFLAGS:+ $CFLAGS}"
    debug=true
else
    debug=false
fi
])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

AM_CONDITIONAL([ENABLE_UDUNITS_1], [true])
AC_ARG_ENABLE([udunits-1],
    [AS_HELP_STRING([--disable-udunits-1],
        [Turn off support for the UDUNITS-1 API [default=enabled]])],
    [case "${enableval}" in
      no)   AM_CONDITIONAL([ENABLE_UDUNITS_1], [false]) ;;
      yes)  ;;
      *)    AC_MSG_ERROR([bad value ${enableval} for --enable-udunits-1]) ;;
    esac])

# Ensure that compilation is optimized and with assertions disabled by default.
CFLAGS=${CFLAGS:--O}
CPPFLAGS=${CPPFLAGS:--DNDEBUG}

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
#if test "$ac_cv_prog_cc_${ac_cc}_c_o" = yes; then
#    case "$AM_CFLAGS" in
#	"-g") ;;
#	*) AM_CFLAGS="${AM_CFLAGS:+$AM_CFLAGS }-g";;
#    esac
#fi
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_YACC
AM_PROG_LEX

# Checks for libraries.
AC_SEARCH_LIBS([dirname], [gen], , 
    AC_MSG_ERROR([cannot find function dirname]))
AC_SEARCH_LIBS([log10], [m], , 
    AC_MSG_ERROR([cannot find function log10]))
AC_SEARCH_LIBS([XML_StopParser], [expat], [need_expat=no], 
    [need_expat=yes])
AM_CONDITIONAL([COND_EXPAT], test "$need_expat" = yes)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h inttypes.h stddef.h stdlib.h string.h strings.h])

# Checks for the CUNIT unit-testing package
LD_CUNIT=
AC_MSG_NOTICE([Checking for the CUNIT unit-testing package.])
AC_CHECK_HEADER([CUnit/CUnit.h],
    [AC_CHECK_LIB([cunit], [CU_initialize_registry], [LD_CUNIT=-lcunit])])
AC_SUBST([LD_CUNIT])
if test "$LD_CUNIT"; then
    AC_MSG_NOTICE([CUNIT found.  Enabling unit-tests.])
else
    AC_MSG_NOTICE([CUNIT not found.  Disabling unit-tests.])
fi
AM_CONDITIONAL([HAVE_CUNIT], [test "$LD_CUNIT"])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Needed for the Expat subpackage:
AC_C_BIGENDIAN([byteorder=4321], [byteorder=1234])
AC_DEFINE_UNQUOTED([BYTEORDER], [$byteorder],
    [Define to 4321 for big-endian and 1234 for little-endian])
AC_DEFINE([XML_CONTEXT_BYTES], 1024,
    [How much context to retain around the current parse point.])

# Checks for library functions.
AC_CHECK_FUNCS([floor memmove memset modf pow strcasecmp strdup strpbrk])

AC_PROG_LIBTOOL

AC_CONFIG_FILES([Makefile
		 expat/Makefile
                 lib/Makefile
                 lib/xmlFailures/Makefile
                 lib/xmlSuccesses/Makefile
		 prog/Makefile
		 test/Makefile])
AC_OUTPUT
