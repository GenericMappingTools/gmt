#!/bin/sh
#	$Id: stripack_prep.sh,v 1.3 2009-05-16 02:21:32 guru Exp $
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
/* stripack.c: Translated via f2c then massaged so that f2c include and lib
 *   are not required to compile and link with sph.
 */

#include <stdio.h>
#include <stdlib.h>

typedef double doublereal;
typedef int integer;
typedef int logical;
#define FALSE_ 0
#define TRUE_ 1
EOF
tail +13 stripack_nowrite.c | grep -v "#include" >> stripack_raw.c
rm -f stripack_nowrite.[cf]
cat << EOF > $$.sed
2390ifprintf (stderr, "*** Error in OPTIM (called from DELNOD):  NIT = %d, IER = %d ***\\\n", nit, ierr);
3031ifprintf (stderr, "*** Error in EDGE:  Invalid triangulation or null triangles on boundary IN1 = %d IN2 = %d\\\n", *in1, *in2);
3040ifprintf (stderr, "*** Error in OPTIM (called from DELNOD):  NIT = %d, IER = %d ***\\\n", nit, ierr);
EOF

sed -f $$.sed stripack_raw.c > stripack_nof2c.c
rm -f $$.sed
