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
AC_COMPILE_IFELSE([module conftest_module
   contains
   subroutine conftest_routine
   write(*,'(a)') 'gotcha!'
   end subroutine conftest_routine
   end module conftest_module
  ],[],[])
cd ..
ax_cv_f90_modflag="not found"
for ax_flag in "-I" "-M" "-p"; do
  if test "$ax_cv_f90_modflag" = "not found" ; then
    ax_save_FCFLAGS="$FCFLAGS"
    FCFLAGS="$ax_save_FCFLAGS ${ax_flag}tmpdir_$i"
    AC_COMPILE_IFELSE([program conftest_program
       use conftest_module
       call conftest_routine
       end program conftest_program
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
