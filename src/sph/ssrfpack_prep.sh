#!/bin/sh
#	$Id: ssrfpack_prep.sh,v 1.5 2009-05-16 03:01:51 guru Exp $
#
# Removes print and plot subroutines from ssrfpack FORTRAN code,
# then replaces error messages with return of error codes that
# sph can choose to report, then replaces integer/doublereal with
# int and double.  This lets us compile the f2c-converted code
# without requiring f2c.h and libf2c.a
#2879s/^ /C /g
#2911,2912s/^ /C /g
#2915s/^ /C /g
#2928s/^ /C /g
#2932s/^ /C /g
#2938s/^ /C /g
#2943s/^ /C /g
#2944s/  RETURN/4 RETURN/g
#2948,2961s/^ /C /g

cat << EOF > $$.sed
328,330s/^ /C /g
331s/  STOP/2 I=1/g
834,835s/^ /C /g
861,862s/^ /C /g
937,940s/^ /C /g
994,995s/^ /C /g
1047,1049s/^ /C /g
2803,2962d
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
EOF
sed -f $$.sed D773/Src/Sp/src.f > ssrfpack_nowrite.f
f2c -r8 ssrfpack_nowrite.f
cat << EOF > ssrfpack_raw.c
/* ssrfpack.c: Translated via f2c then massaged so that f2c include and lib
 *   are not required to compile and link with sph.
 */

#include <stdio.h>
#include <stdlib.h>

typedef double doublereal;
typedef int integer;

#define min(x, y) (((x) < (y)) ? (x) : (y))
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define abs(x) ((x) >= 0 ? (x) : -(x))
int speak = 0;
double d_sign (doublereal *a, doublereal *b)
{
	double x;
	x = (*a >= 0 ? *a : - *a);
	return (*b >= 0 ? x : -x);
}
EOF
tail +13 ssrfpack_nowrite.c | grep -v "#include" >> ssrfpack_raw.c

cat << EOF > $$.sed
384iif (speak) fprintf (stderr, "ERROR IN ARCINT -- P1 = %9.6f %9.6f %9.6f   P2 = %9.6f %9.6f %9.6f\\\n", p1[1], p1[2], p1[3], p2[1], p2[2], p2[3]);
980iif (speak) fprintf (stderr, "GETSIG -- N = %4d TOL = %g\\\n", *n, ftol);
1011iif (speak) fprintf (stderr, "ARC %d - %d\\\n", n1, n2);
1103iif (speak) fprintf (stderr, "CONVEXITY -- SIG = %g  F(SIG) = %g  FP(SIG) = %g\\\n", sig, f, fp);
1170iif (speak) fprintf (stderr, "MONOTONICITY -- DSIG = %g\\\n", dsig);
1232iif (speak) fprintf (stderr, "%d -- SIG = %g  F = %g\\\n", nit, sig, f);
3496iif (speak) if (rf < 0.0) fprintf (stderr, "SIG0 -- N1 = %d  N2 = %d  LOWER BOUND = %g\\\n", *n1, *n2, bnd);
3496iif (speak) if (rf > 0.0) fprintf (stderr, "SIG0 -- N1 = %d  N2 = %d  UPPER BOUND = %g\\\n", *n1, *n2, bnd);
3668iif (speak) fprintf (stderr, "SIG = %g  SNEG = %g F0 = %g FMAX = %g\\\n", sig, sneg, f0, fmax);
3761iif (speak) fprintf (stderr, "%d -- SIG = %g  F = %g\\\n", nit, sig, f);
3807iif (speak) fprintf (stderr, "DSIG = %g\\\n", dsig);
4025iif (speak) if (rf < 0.0) fprintf (stderr, "SIG1 -- N1 = %d  N2 = %d  LOWER BOUND = %g\\\n", *n1, *n2, bnd);
4025iif (speak) if (rf > 0.0) fprintf (stderr, "SIG1 -- N1 = %d  N2 = %d  UPPER BOUND = %g\\\n", *n1, *n2, bnd);
4163iif (speak) fprintf (stderr, "F0 = %g FMAX = %g SIG = %g\\\n", f0, fmax, sig);
4242iif (speak) fprintf (stderr, "%d -- SIG = %g  F = %g\\\n", nit, sig, f);
4285iif (speak) fprintf (stderr, "DSIG = %g\\\n", dsig);
4488iif (speak) fprintf (stderr, "SIG2 -- N1 = %d  N2 = %d\\\n", *n1, *n2);
4641iif (speak) fprintf (stderr, "%d -- SIG = %g  F = %g  FP = %g\\\n", nit, sig, f, fp);
5266iif (speak) fprintf (stderr, "SMSURF -- THE CONSTRAINT IS NOT ACTIVE AND THE FITTING FCN IS CONSTANT\\\n");
5277iif (speak) fprintf (stderr, "SMSURF -- SM = %g  GSTOL = %g  NITMAX = %d  G(0) = %g\\\n", *sm, tol, nitmax, g0);
5323iif (speak) fprintf (stderr, " %d -- P = %g  G = %g  NIT = %d  DFMAX = %g\\\n", iter, p, g, nit, dfmax);
5360iif (speak) fprintf (stderr, "DP = %g\\\n", dp);
EOF

sed -f $$.sed ssrfpack_raw.c > ssrfpack_nof2c.c
rm -f $$.sed ssrfpack_raw.c rm -f ssrfpack_nowrite.[cf]
