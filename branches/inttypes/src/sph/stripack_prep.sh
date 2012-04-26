#!/bin/sh
#	$Id$
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
cat << EOF > $$.sed
2392ifprintf (stderr, "*** Error in OPTIM (called from DELNOD):  NIT = %d, IER = %d ***\\\n", nit, ierr);
3033ifprintf (stderr, "*** Error in EDGE:  Invalid triangulation or null triangles on boundary IN1 = %d IN2 = %d\\\n", *in1, *in2);
3042ifprintf (stderr, "*** Error in OPTIM (called from DELNOD):  NIT = %d, IER = %d ***\\\n", nit, ierr);
660c\
    *nt = (nn - 2) << 1;
2826,2827c\
	iwk[((i__ - 1) << 1) + 1] = iwk[(i__ << 1) + 1];\
	iwk[((i__ - 1) << 1) + 2] = iwk[(i__ << 1) + 2];
2883,2884c\
	iwk[((i__ + 1) << 1) + 1] = iwk[(i__ << 1) + 1];\
	iwk[((i__ + 1) << 1) + 2] = iwk[(i__ << 1) + 2];
2946,2947c\
	iwk[(i__ << 1) + 1] = iwk[((i__ - 1) << 1) + 1];\
	iwk[(i__ << 1) + 2] = iwk[((i__ - 1) << 1) + 2];
2972c\
	nit = (iwc - 1) << 2;
2987c\
	nit = (iwend - iwc) << 2;
2990c\
		nit, &iwk[((iwc + 1) << 1) + 1], &ierr);
5719c\
    if (*n < 3 || (*nrow != 6 && *nrow != 9)) {
EOF

cat << EOF > stripack.c
/* \$Id\$
 * stripack.c: Translated via f2c then massaged so that f2c include and lib
 * are not required to compile and link the sph supplement.
 */

typedef int logical;
EOF
sed -f $$.sed stripack_nowrite.c | tail +13 | grep -v "#include" >> stripack.c

rm -f $$.sed stripack_nowrite.[cf] 
