dnl $Id$
dnl UD macros for netcdf configure


dnl Convert a string to all uppercase.
dnl
define([uppercase],
[translit($1, abcdefghijklmnopqrstuvwxyz, ABCDEFGHIJKLMNOPQRSTUVWXYZ)])

dnl
dnl Check for an nm(1) utility.
dnl
AC_DEFUN([UD_PROG_NM],
[
    case "${NM-unset}" in
	unset) AC_CHECK_PROGS(NM, nm, nm) ;;
	*) AC_CHECK_PROGS(NM, $NM nm, nm) ;;
    esac
    AC_MSG_CHECKING(nm flags)
    case "${NMFLAGS-unset}" in
	unset) NMFLAGS= ;;
    esac
    AC_MSG_RESULT($NMFLAGS)
    AC_SUBST(NMFLAGS)
])

dnl
dnl Check for an m4(1) preprocessor utility.
dnl
AC_DEFUN([UD_PROG_M4],
[
    case "${M4-unset}" in
	unset) AC_CHECK_PROGS(M4, m4 gm4, m4) ;;
	*) AC_CHECK_PROGS(M4, $M4 m4 gm4, m4) ;;
    esac
    AC_MSG_CHECKING(m4 flags)
    case "${M4FLAGS-unset}" in
	unset) M4FLAGS=-B10000 ;;
    esac
    AC_MSG_RESULT($M4FLAGS)
    AC_SUBST(M4FLAGS)
])

dnl
dnl Set the top-level source-directory.
dnl
AC_DEFUN([UD_SRCDIR],
[
    AC_MSG_CHECKING(for top-level source-directory)
    SRCDIR=`(cd $srcdir && pwd)`
    AC_MSG_RESULT($SRCDIR)
    AC_SUBST(SRCDIR)
])


dnl
dnl like AC_LONG_DOUBLE, except checks for 'long long'
dnl
AC_DEFUN([UD_C_LONG_LONG],
[AC_MSG_CHECKING(for long long)
AC_CACHE_VAL(ac_cv_c_long_long,
[if test "$GCC" = yes; then
  ac_cv_c_long_long=yes
else
AC_TRY_RUN([int main() {
long long foo = 0;
exit(sizeof(long long) < sizeof(long)); }],
ac_cv_c_long_long=yes, ac_cv_c_long_long=no, :)
fi])dnl
AC_MSG_RESULT($ac_cv_c_long_long)
if test "$ac_cv_c_long_long" = yes; then
  AC_DEFINE([HAVE_LONG_LONG], [], [have long long type])
fi
])

dnl UD_CHECK_SIZEOF(TYPE)
AC_DEFUN([UD_CHECK_SIZEOF],
[changequote(<<, >>)dnl
dnl The name to #define.
define(<<AC_TYPE_NAME>>, translit(sizeof_$1, [a-z *], [A-Z_P]))dnl
dnl The cache variable name.
define(<<AC_CV_NAME>>, translit(ac_cv_sizeof_$1, [ *], [_p]))dnl
changequote([, ])dnl
AC_MSG_CHECKING(size of $1)
AC_CACHE_VAL(AC_CV_NAME,
[AC_TRY_RUN([#include <stdio.h>
#include <sys/types.h>
#if STDC_HEADERS
#include <stdlib.h>
#endif
main()
{
  FILE *f=fopen("conftestval", "w");
  if (!f) exit(1);
  fprintf(f, "%d\n", sizeof($1));
  exit(0);
}], AC_CV_NAME=`cat conftestval`, AC_CV_NAME=0, AC_CV_NAME=0)])dnl
AC_MSG_RESULT($AC_CV_NAME)
AC_DEFINE_UNQUOTED(AC_TYPE_NAME, $AC_CV_NAME, [type size])
undefine([AC_TYPE_NAME])dnl
undefine([AC_CV_NAME])dnl
])


dnl 
dnl UD_CHECK_IEEE
dnl If the 'double' is not an IEEE double
dnl or the 'float' is not and IEEE single,
dnl define NO_IEEE_FLOAT
dnl
AC_DEFUN([UD_CHECK_IEEE],
[
AC_MSG_CHECKING(for IEEE floating point format)
AC_TRY_RUN([#ifndef NO_FLOAT_H
#include <float.h>
#endif

#define EXIT_NOTIEEE	1
#define EXIT_MAYBEIEEE	0

int
main()
{
#if	defined(FLT_RADIX)	&& FLT_RADIX != 2
		return EXIT_NOTIEEE;
#elif	defined(DBL_MAX_EXP)	&& DBL_MAX_EXP != 1024
		return EXIT_NOTIEEE;
#elif	defined(DBL_MANT_DIG)	&& DBL_MANT_DIG != 53
		return EXIT_NOTIEEE;
#elif 	defined(FLT_MAX_EXP)	&& !(FLT_MAX_EXP == 1024 || FLT_MAX_EXP == 128)
		return EXIT_NOTIEEE;
#elif	defined(FLT_MANT_DIG)	&& !(FLT_MANT_DIG == 53 || FLT_MANT_DIG == 24)
		return EXIT_NOTIEEE;
#else
	/* (assuming eight bit char) */
	if(sizeof(double) != 8)
		return EXIT_NOTIEEE;
	if(!(sizeof(float) == 4 || sizeof(float) == 8))
		return EXIT_NOTIEEE;

	return EXIT_MAYBEIEEE;
#endif
}],ac_cv_c_ieeefloat=yes, ac_cv_c_ieeefloat=no, :)
AC_MSG_RESULT($ac_cv_c_ieeefloat)
if test "$ac_cv_c_ieeefloat" = no; then
  AC_DEFINE([NO_IEEE_FLOAT], [], [no IEEE float on this platform])
fi
])

dnl Check for utility for generating makefile dependencies.
dnl Should only be used at the UPC.
dnl
AC_DEFUN([UD_PROG_CC_MAKEDEPEND],
[
    AC_MSG_CHECKING(how to make dependencies)
    case `uname -s` in
	IRIX*|OSF1)
	    CC_MAKEDEPEND='cc -M'
	    ;;
	SunOS)
	    case `uname -r` in
		4*)
		    CC_MAKEDEPEND='cc -M'
		    ;;
		5*|*)
		    CC_MAKEDEPEND='cc -xM'
		    ;;
	    esac
	    ;;
	ULTRIX)
	    case `uname -m` in
		RISC)
		    CC_MAKEDEPEND='cc -M'
		    ;;
		VAX)	# Can't handle prototypes in netcdf.h
		    ;;
	    esac
	    ;;
	AIX)	# Writes to .u files rather than standard out
	    ;;
	HP-UX)	# Writes escaped newlines to standard error
	    ;;
    esac
    case "${CC_MAKEDEPEND}" in
	'')
	    CC_MAKEDEPEND=false
	    ;;
    esac
    AC_MSG_RESULT($CC_MAKEDEPEND)
    AC_SUBST(CC_MAKEDEPEND)
])

dnl Check for a Fortran type equivalent to a netCDF type.
dnl
dnl UD_CHECK_FORTRAN_NCTYPE(forttype, possibs, nctype)
dnl
AC_DEFUN([UD_CHECK_FORTRAN_NCTYPE],
[
    AC_MSG_CHECKING(for Fortran-equivalent to netCDF \"$3\")
    for type in $2; do
	cat >conftest.f <<EOF
               $type foo
               end
EOF
	doit='$FC -c ${FFLAGS} conftest.f'
	if AC_TRY_EVAL(doit); then
	    break;
	fi
    done
    rm -f conftest.f conftest.o
    AC_DEFINE_UNQUOTED($1, $type, [type definition])
    AC_MSG_RESULT($type)
    $1=$type
])


dnl Check for a Fortran type equivalent to a C type.
dnl
dnl UD_CHECK_FORTRAN_CTYPE(v3forttype, v2forttype, ctype, min, max)
dnl
AC_DEFUN([UD_CHECK_FORTRAN_CTYPE],
[
    AC_MSG_CHECKING(for Fortran-equivalent to C \"$3\")
    cat >conftest.f <<EOF
        subroutine sub(values, minval, maxval)
        implicit        none
        $2              values(5), minval, maxval
        minval = values(2)
        maxval = values(4)
        if (values(2) .ge. values(4)) then
            minval = values(4)
            maxval = values(2)
        endif
        end
EOF
    doit='$FC -c ${FFLAGS conftest.f'
    if AC_TRY_EVAL(doit); then
	mv conftest.o conftestf.o
	cat >conftest.c <<EOF
#include <limits.h>
#include <float.h>
void main()
{
$3		values[[]] = {0, $4, 0, $5, 0};
$3		minval, maxval;
void	$FCALLSCSUB($3*, $3*, $3*);
$FCALLSCSUB(values, &minval, &maxval);
exit(!(minval == $4 && maxval == $5));
}
EOF
	doit='$CC -o conftest ${CPPFLAGS} ${CFLAGS} ${LDFLAGS} conftest.c conftestf.o ${LIBS}'
	if AC_TRY_EVAL(doit); then
	    doit=./conftest
	    if AC_TRY_EVAL(doit); then
		AC_MSG_RESULT($2)
		$1=$2
		AC_DEFINE_UNQUOTED($1,$2, [take a guess])
	    else
		AC_MSG_RESULT(no equivalent type)
		unset $1
	    fi
	else
	    AC_MSG_ERROR(Could not compile-and-link conftest.c and conftestf.o)
	fi
    else
	AC_MSG_ERROR(Could not compile conftest.f)
    fi
    rm -f conftest*
])


dnl Check for a Fortran data type.
dnl
dnl UD_CHECK_FORTRAN_TYPE(varname, ftypes)
dnl
AC_DEFUN([UD_CHECK_FORTRAN_TYPE],
[
    for ftype in $2; do
	AC_MSG_CHECKING(for Fortran \"$ftype\")
	cat >conftest.f <<EOF
      subroutine sub(value)
      $ftype value
      end
EOF
	doit='$FC -c ${FFLAGS} conftest.f'
	if AC_TRY_EVAL(doit); then
	    AC_MSG_RESULT(yes)
	    $1=$ftype
	    AC_DEFINE_UNQUOTED($1, $ftype, [type thing])
	    break
	else
	    AC_MSG_RESULT(no)
	fi
    done
    rm -f conftest*
])


dnl Check for the name format of a Fortran-callable C routine.
dnl
dnl UD_CHECK_FCALLSCSUB
AC_DEFUN([UD_CHECK_FCALLSCSUB],
[
#    AC_REQUIRE([UD_PROG_FC])
    case "$FC" in
	'') ;;
	*)
	    AC_REQUIRE([UD_PROG_NM])
	    AC_BEFORE([UD_CHECK_FORTRAN_CTYPE])
	    AC_BEFORE([UD_CHECK_CTYPE_FORTRAN])
	    AC_MSG_CHECKING(for C-equivalent to Fortran routine \"SUB\")
	    cat >conftest.f <<\EOF
              call sub()
              end
EOF
	    doit='$FC -c ${FFLAGS} conftest.f'
	    if AC_TRY_EVAL(doit); then
		FCALLSCSUB=`$NM $NMFLAGS conftest.o | awk '
		    /SUB_/{print "SUB_";exit}
		    /SUB/ {print "SUB"; exit}
		    /sub_/{print "sub_";exit}
		    /sub/ {print "sub"; exit}'`
		case "$FCALLSCSUB" in
		    '') AC_MSG_ERROR(not found)
			;;
		    *)  AC_MSG_RESULT($FCALLSCSUB)
			;;
		esac
	    else
		AC_MSG_ERROR(Could not compile conftest.f)
	    fi
	    rm -f conftest*
	    ;;
    esac
])


dnl Check for a C type equivalent to a Fortran type.
dnl
dnl UD_CHECK_CTYPE_FORTRAN(ftype, ctypes, fmacro_root)
dnl
AC_DEFUN([UD_CHECK_CTYPE_FORTRAN],
[
    cat >conftestf.f <<EOF
           $1 values(4)
           data values /-1, -2, -3, -4/
           call sub(values)
           end
EOF
    for ctype in $2; do
	AC_MSG_CHECKING(if Fortran \"$1\" is C \"$ctype\")
	cat >conftest.c <<EOF
	    void $FCALLSCSUB(values)
		$ctype values[[4]];
	    {
		exit(values[[1]] != -2 || values[[2]] != -3);
	    }
EOF
	doit='$CC -c ${CPPFLAGS} ${CFLAGS} conftest.c'
	if AC_TRY_EVAL(doit); then
	    doit='$FC ${FFLAGS} -c conftestf.f'
	    if AC_TRY_EVAL(doit); then
	        doit='$FC -o conftest ${FFLAGS} ${LDFLAGS} conftestf.o conftest.o ${FLIBS} ${LIBS}'
	        if AC_TRY_EVAL(doit); then
		    doit=./conftest
		    if AC_TRY_EVAL(doit); then
		        AC_MSG_RESULT(yes)
		        cname=`echo $ctype | tr ' abcdefghijklmnopqrstuvwxyz' \
			    _ABCDEFGHIJKLMNOPQRSTUVWXYZ`
		        AC_DEFINE_UNQUOTED(NF_$3[]_IS_C_$cname, [1], [fortran to c conversion])
		        break
		    else
		        AC_MSG_RESULT(no)
		    fi
	        else
		    AC_MSG_ERROR(Could not link conftestf.o and conftest.o)
	        fi
	    else
		AC_MSG_ERROR(Could not compile conftestf.f)
	    fi
	else
	    AC_MSG_ERROR(Could not compile conftest.c)
	fi
    done
    rm -f conftest*
])


dnl Get information about Fortran data types.
dnl
AC_DEFUN([UD_FORTRAN_TYPES],
[
#    AC_REQUIRE([UD_PROG_FC])
    case "$FC" in
    '')
	;;
    *)
	AC_REQUIRE([UD_CHECK_FCALLSCSUB])
dnl	UD_CHECK_FORTRAN_TYPE(NF_INT1_T, byte integer*1 "integer(kind(1))")
dnl	UD_CHECK_FORTRAN_TYPE(NF_INT2_T, integer*2 "integer(kind(2))")
	UD_CHECK_FORTRAN_TYPE(NF_INT1_T, byte integer*1 "integer(kind=1)" "integer(selected_int_kind(2))")
	UD_CHECK_FORTRAN_TYPE(NF_INT2_T, integer*2 "integer(kind=2)" "integer(selected_int_kind(4))")

	case "${NF_INT1_T}" in
	    '') ;;
	    *)  UD_CHECK_CTYPE_FORTRAN($NF_INT1_T, "signed char", INT1)
		UD_CHECK_CTYPE_FORTRAN($NF_INT1_T, "short", INT1)
		UD_CHECK_CTYPE_FORTRAN($NF_INT1_T, "int", INT1)
		UD_CHECK_CTYPE_FORTRAN($NF_INT1_T, "long", INT1)
		;;
	esac
	case "${NF_INT2_T}" in
	    '') ;;
	    *)  UD_CHECK_CTYPE_FORTRAN($NF_INT2_T, short, INT2)
		UD_CHECK_CTYPE_FORTRAN($NF_INT2_T, int, INT2)
		UD_CHECK_CTYPE_FORTRAN($NF_INT2_T, long, INT2)
		;;
	esac
	UD_CHECK_CTYPE_FORTRAN(integer, int long, INT)
	UD_CHECK_CTYPE_FORTRAN(real, float double, REAL)
	UD_CHECK_CTYPE_FORTRAN(doubleprecision, double float, DOUBLEPRECISION)

dnl	UD_CHECK_FORTRAN_NCTYPE(NCBYTE_T, byte integer*1 integer, byte)
	UD_CHECK_FORTRAN_NCTYPE(NCBYTE_T, byte integer*1 "integer(kind=1)" "integer(selected_int_kind(2))" integer, byte)

dnl	UD_CHECK_FORTRAN_NCTYPE(NCSHORT_T, integer*2 integer, short)
	UD_CHECK_FORTRAN_NCTYPE(NCSHORT_T, integer*2 "integer(kind=2)" "integer(selected_int_kind(4))" integer, short)
dnl	UD_CHECK_FORTRAN_CTYPE(NF_SHORT_T, $NCSHORT_T, short, SHRT_MIN, SHRT_MAX)

dnl	UD_CHECK_FORTRAN_NCTYPE(NCLONG_T, integer*4 integer, long)
dnl	UD_CHECK_FORTRAN_CTYPE(NF_INT_T, integer, int, INT_MIN, INT_MAX)

dnl	UD_CHECK_FORTRAN_NCTYPE(NCFLOAT_T, real*4 real, float)
dnl	UD_CHECK_FORTRAN_CTYPE(NF_FLOAT_T, $NCFLOAT_T, float, FLT_MIN, FLT_MAX)

dnl	UD_CHECK_FORTRAN_NCTYPE(NCDOUBLE_T, real*8 doubleprecision real, double)
dnl	UD_CHECK_FORTRAN_CTYPE(NF_DOUBLE_T, $NCDOUBLE_T, double, DBL_MIN, DBL_MAX)
	;;
    esac
])


dnl Setup for making a manual-page database.
dnl
AC_DEFUN([UD_MAKEWHATIS],
[
    #
    # NB: We always want to define WHATIS to prevent the
    # $(MANDIR)/$(WHATIS) make(1) target from being just $(MANDIR)/ and
    # conflicting with the (directory creation) target with the same name.
    #
    WHATIS=whatis
    case `uname -sr` in
	BSD/OS*|FreeBSD*)
	    # Can't generate a user-database -- only /usr/share/man/whatis.db.
	    MAKEWHATIS_CMD=
	    ;;
	'IRIX64 6.5'|'IRIX 6.5')
	    MAKEWHATIS_CMD='/usr/lib/makewhatis -M $(MANDIR) $(MANDIR)/whatis'
	    ;;
	'IRIX 6'*)
	    # Can't generate a user-database.
	    MAKEWHATIS_CMD=
	    ;;
	HP-UX*)
	    # Can't generate a user-database -- only /usr/lib/whatis.
	    MAKEWHATIS_CMD=
	    ;;
	'Linux '*)
	    # /usr/sbin/makewhatis doesn't work
	    MAKEWHATIS_CMD=
	    ;;
	ULTRIX*)
	    # Can't generate a user-database -- only /usr/lib/whatis.
	    MAKEWHATIS_CMD=
	    ;;
	*)
	    if test -r /usr/man/windex; then
		WHATIS=windex
	    fi
	    AC_CHECK_PROGS(prog, catman makewhatis /usr/lib/makewhatis)
	    case "$prog" in
		*catman*)
		    MAKEWHATIS_CMD=$prog' -w -M $(MANDIR)'
		    ;;
		*makewhatis*)
		    MAKEWHATIS_CMD=$prog' $(MANDIR)'
		    ;;
	    esac
	    ;;
    esac
    AC_SUBST(WHATIS)
    AC_SUBST(MAKEWHATIS_CMD)
    AC_MSG_CHECKING(for manual-page index command)
    AC_MSG_RESULT($MAKEWHATIS_CMD)
])


dnl Check for the math library.
dnl
AC_DEFUN([UD_CHECK_LIB_MATH],
[
    case "${MATHLIB}" in
	'')
	    AC_CHECK_LIB(c, tanh, MATHLIB=,
		    AC_CHECK_LIB(m, tanh, MATHLIB=-lm, MATHLIB=))
	    ;;
	*)
	    ;;
    esac 
   AC_SUBST(MATHLIB)
])


dnl Set the binary distribution directory.
dnl
AC_DEFUN([UD_FTPBINDIR], [dnl
    AC_MSG_CHECKING([binary distribution directory])
    case ${FTPBINDIR-unset} in
	unset)
	    system=`(system) 2>/dev/null || echo dummy_system`
	    FTPBINDIR=${FTPDIR-/home/ftp}/pub/binary/$system
	    ;;
    esac
    AC_SUBST(FTPBINDIR)dnl
    AC_MSG_RESULT($FTPBINDIR)
])


dnl
dnl
dnl

dnl
dnl These headers won't get C style comments
dnl
dnl UD_CONFIG_HEADER(HEADER-TO-CREATE ...)
AC_DEFUN([UD_CONFIG_HEADER],
[define(UD_LIST_HEADER, $1)])


dnl
dnl Print which compilers are going to be used, the flags, and their
dnl locations. This is all to assist in debugging, and help with
dnl support questions.
dnl
AC_DEFUN([UD_DISPLAY_RESULTS],
[
AC_MSG_CHECKING(CPPFLAGS)
AC_MSG_RESULT($CPPFLAGS)
AC_MSG_CHECKING(CC CFLAGS)
AC_MSG_RESULT($CC $CFLAGS)
ud_type_cc=`type $CC`
AC_MSG_CHECKING(type $CC)
AC_MSG_RESULT($ud_type_cc)

AC_MSG_CHECKING(CXX)
if test -n "$CXX"; then
	AC_MSG_RESULT($CXX)
	AC_MSG_CHECKING(CXXFLAGS)
	AC_MSG_RESULT($CXXFLAGS)
	ud_type_CXX=`type $CXX`
	AC_MSG_CHECKING(type $CXX)
	AC_MSG_RESULT($ud_type_CXX)
else
	AC_MSG_RESULT(unset)
fi

AC_MSG_CHECKING(FC)
if test -n "$FC"; then
	AC_MSG_RESULT($FC)
	AC_MSG_CHECKING(FFLAGS)
	AC_MSG_RESULT($FFLAGS)
	ud_type_fc=`type $FC`
	AC_MSG_CHECKING(type $FC)
	AC_MSG_RESULT($ud_type_fc)
else
	AC_MSG_RESULT(unset)
fi

AC_MSG_CHECKING(F90)
if test -n "$F90"; then
	AC_MSG_RESULT($F90)
	AC_MSG_CHECKING(FCFLAGS)
	AC_MSG_RESULT($FCFLAGS)
	ud_type_F90=`type $F90`
	AC_MSG_CHECKING(type $F90)
	AC_MSG_RESULT($ud_type_F90)
else
	AC_MSG_RESULT(unset)
fi

AC_MSG_CHECKING(AR)
if test -n "$AR"; then
	AC_MSG_RESULT($AR)
	AC_MSG_CHECKING(AR_FLAGS)
	AC_MSG_RESULT($AR_FLAGS)
	ud_type_AR=`type $AR`
	AC_MSG_CHECKING(type $AR)
	AC_MSG_RESULT($ud_type_AR)
else
	AC_MSG_RESULT(unset)
fi

AC_MSG_CHECKING(NM)
if test -n "$NM"; then
	AC_MSG_RESULT($NM)
	AC_MSG_CHECKING(NMFLAGS)
	AC_MSG_RESULT($NMFLAGS)
#	ud_type_NM=`type $NM`
#	AC_MSG_CHECKING(type $NM)
#	AC_MSG_RESULT($ud_type_NM)
else
	AC_MSG_RESULT(unset)
fi

])

# AC_PROG_FC_MOD
# ---------------
AC_DEFUN([AC_PROG_FC_UPPERCASE_MOD],
[
AC_LANG_PUSH(Fortran)
AC_MSG_CHECKING([if Fortran 90 compiler capitalizes .mod filenames])
		    cat <<EOF >conftest.f90
		      module conftest
		      end module conftest
EOF
ac_try='$F90 ${F90FLAGS} conftest.f90 ${F90LIBS}>&AS_MESSAGE_LOG_FD'
AC_TRY_EVAL(ac_try)
if test -f CONFTEST.mod ; then
   ac_cv_prog_f90_uppercase_mod=yes
   rm -f CONFTEST.mod
else
   ac_cv_prog_f90_uppercase_mod=no
fi
AC_MSG_RESULT($ac_cv_prog_f90_uppercase_mod)
#rm -f conftest*
AC_LANG_POP(Fortran)
])

# [AC_LANG_CONFTEST([AC_LANG_SOURCE([      module conftest
#       end module])])
# ac_try='$F90 -o conftest ${F90FLAGS} conftest.f90 ${F90LIBS}>&AS_MESSAGE_LOG_FD'
# if AC_TRY_EVAL(ac_try) &&
#    test -f CONFTEST.mod ; then
#         ac_cv_prog_f90_uppercase_mod=yes
# 	rm -f CONFTEST.mod
# else
#   ac_cv_prog_f90_uppercase_mod=no
# fi
# rm -f conftest*


AC_DEFUN([AX_F90_MODULE_FLAG],[
AC_CACHE_CHECK([fortran 90 modules inclusion flag],
ax_cv_f90_modflag,
[AC_LANG_PUSH(Fortran)
i=0
while test \( -f tmpdir_$i \) -o \( -d tmpdir_$i \) ; do
  i=`expr $i + 1`
done
mkdir tmpdir_$i
cd tmpdir_$i
AC_COMPILE_IFELSE([AC_LANG_SOURCE([module conftest_module
   contains
   subroutine conftest_routine
   write(*,'(a)') 'gotcha!'
   end subroutine conftest_routine
   end module conftest_module])
  ],[],[])
cd ..
ax_cv_f90_modflag="not found"
for ax_flag in "-I" "-M" "-p"; do
  if test "$ax_cv_f90_modflag" = "not found" ; then
    ax_save_FCFLAGS="$FCFLAGS"
    FCFLAGS="$ax_save_FCFLAGS ${ax_flag}tmpdir_$i"
    AC_COMPILE_IFELSE([AC_LANG_SOURCE([program conftest_program
       use conftest_module
       call conftest_routine
       end program conftest_program])
      ],[ax_cv_f90_modflag="$ax_flag"],[])
    FCFLAGS="$ax_save_FCFLAGS"
  fi
done
rm -fr tmpdir_$i
if test "$ax_flag" = "not found" ; then
  AC_MSG_ERROR([unable to find compiler flag for modules inclusion])
fi
AC_LANG_POP(Fortran)
])])

# AX_C_FLOAT_WORDS_BIGENDIAN
# added by:
#   Warren Turkal <wt@penguintechs.org>
#
# Copyright © 2006 Daniel Amelang <dan@amelang.net>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and
# this notice are preserved.
#
# This macro will detect if double variables are words packed in big endian
# order while the bits in the words are arranged in little endian order. This
# macro was added to support the ARM architecture. The FLOAT_WORDS_BIGENDIAN
# macro will be set to 1 if the word order is big endian. If the word order is
# not big endian, FLOAT_WORDS_BIGENDIAN will be not be set.
AC_DEFUN([AX_C_FLOAT_WORDS_BIGENDIAN],
  [AC_CACHE_CHECK(whether float word ordering is bigendian,
                  ax_cv_c_float_words_bigendian, [

ax_cv_c_float_words_bigendian=unknown
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[

double d = 90904234967036810337470478905505011476211692735615632014797120844053488865816695273723469097858056257517020191247487429516932130503560650002327564517570778480236724525140520121371739201496540132640109977779420565776568942592.0;

]])], [

if grep noonsees conftest.$ac_objext >/dev/null ; then
  ax_cv_c_float_words_bigendian=yes
fi
if grep seesnoon conftest.$ac_objext >/dev/null ; then
  if test "$ax_cv_c_float_words_bigendian" = unknown; then
    ax_cv_c_float_words_bigendian=no
  else
    ax_cv_c_float_words_bigendian=unknown
  fi
fi

])])

case $ax_cv_c_float_words_bigendian in
  yes)
    m4_default([$1],
      [AC_DEFINE([FLOAT_WORDS_BIGENDIAN], 1,
                 [Define to 1 if your system stores words within floats
                  with the most significant word first])]) ;;
  no)
    $2 ;;
  *)
    m4_default([$3],
      [AC_MSG_ERROR([

Unknown float word ordering. You need to manually preset
ax_cv_c_float_words_bigendian=no (or yes) according to your system.

    ])]) ;;
esac

])# AX_C_FLOAT_WORDS_BIGENDIAN

