#!/bin/sh
#	$Id: stripack_prep.sh,v 1.1 2009-05-15 21:17:31 guru Exp $
#
# Removes print and plot subroutines from stripack FORTRAN code,
# then replaces error messages with return of error codes that
# sph can choose to report, then replaces integer/doublereal with
# int and double.  This lets us compile the f2c-converted code
# without requiring f2c.h and libf2c.a

cat << EOF > $$.sed
1993,1995s/^ /C /g
2515,2518s/^ /C /g
2524,2526s/^ /C /g
4926,5118d
5540,6715d
EOF
sed -f $$.sed D772/Fortran77/Src/Sp/src.f > stripack_nowrite.f
rm -f $$.sed
f2c -r8 stripack_nowrite.f
cat << EOF > stripack_raw.c
typedef double doublereal;
typedef int integer;
typedef int logical;
#define FALSE_ 0
#define TRUE_ 1
EOF
grep -v "#include" stripack_nowrite.c >> stripack_raw.c
rm -f stripack_nowrite.[cf]
