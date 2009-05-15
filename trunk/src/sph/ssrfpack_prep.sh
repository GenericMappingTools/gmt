#!/bin/sh
#	$Id: ssrfpack_prep.sh,v 1.1 2009-05-15 21:17:31 guru Exp $
#
# Removes print and plot subroutines from ssrfpack FORTRAN code,
# then replaces error messages with return of error codes that
# sph can choose to report, then replaces integer/doublereal with
# int and double.  This lets us compile the f2c-converted code
# without requiring f2c.h and libf2c.a

cat << EOF > $$.sed
328,330s/^ /C /g
331s/  STOP/2 I=1/g
834,835s/^ /C /g
861,862s/^ /C /g
937,940s/^ /C /g
994,995s/^ /C /g
1047,1049s/^ /C /g
3106,3113s/^ /C /g
3246,3248s/^ /C /g
3325,3327s/^ /C /g
3368,3369s/^ /C /g
3540,3547s/^ /C /g
3654,3656s/^ /C /g
3725,3727s/^ /C /g
3765,3766s/^ /C /g
3931,3932s/^ /C /g
4047,4049s/^ /C /g
4558,4560s/^ /C /g
4569,4571s/^ /C /g
4603,4605s/^ /C /g
4638,4639s/^ /C /g
2803,2962d
EOF
sed -f $$.sed D773/Src/Sp/src.f > ssrfpack_nowrite.f
f2c -r8 ssrfpack_nowrite.f
cat << EOF > ssrfpack_raw.c
typedef double doublereal;
typedef int integer;
EOF
grep -v "#include" ssrfpack_nowrite.c >> ssrfpack_raw.c
rm -f ssrfpack_nowrite.[cf]
