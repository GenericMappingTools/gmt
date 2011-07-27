#!/bin/sh
#	$Id$
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
cat << EOF > $$.sed
391iif (dbg_verbose) fprintf (stderr, "ERROR IN ARCINT -- P1 = %9.6f %9.6f %9.6f   P2 = %9.6f %9.6f %9.6f\\\n", p1[1], p1[2], p1[3], p2[1], p2[2], p2[3]);
987iif (dbg_verbose) fprintf (stderr, "GETSIG -- N = %4d TOL = %g\\\n", *n, ftol);
1018iif (dbg_verbose) fprintf (stderr, "ARC %d - %d\\\n", n1, n2);
1110iif (dbg_verbose) fprintf (stderr, "CONVEXITY -- SIG = %g  F(SIG) = %g  FP(SIG) = %g\\\n", sig, f, fp);
1177iif (dbg_verbose) fprintf (stderr, "MONOTONICITY -- DSIG = %g\\\n", dsig);
1239iif (dbg_verbose) fprintf (stderr, "%d -- SIG = %g  F = %g\\\n", nit, sig, f);
3503iif (dbg_verbose) if (rf < 0.0) fprintf (stderr, "SIG0 -- N1 = %d  N2 = %d  LOWER BOUND = %g\\\n", *n1, *n2, bnd);
3503iif (dbg_verbose) if (rf > 0.0) fprintf (stderr, "SIG0 -- N1 = %d  N2 = %d  UPPER BOUND = %g\\\n", *n1, *n2, bnd);
3675iif (dbg_verbose) fprintf (stderr, "SIG = %g  SNEG = %g F0 = %g FMAX = %g\\\n", sig, sneg, f0, fmax);
3768iif (dbg_verbose) fprintf (stderr, "%d -- SIG = %g  F = %g\\\n", nit, sig, f);
3814iif (dbg_verbose) fprintf (stderr, "DSIG = %g\\\n", dsig);
4032iif (dbg_verbose) if (rf < 0.0) fprintf (stderr, "SIG1 -- N1 = %d  N2 = %d  LOWER BOUND = %g\\\n", *n1, *n2, bnd);
4032iif (dbg_verbose) if (rf > 0.0) fprintf (stderr, "SIG1 -- N1 = %d  N2 = %d  UPPER BOUND = %g\\\n", *n1, *n2, bnd);
4170iif (dbg_verbose) fprintf (stderr, "F0 = %g FMAX = %g SIG = %g\\\n", f0, fmax, sig);
4249iif (dbg_verbose) fprintf (stderr, "%d -- SIG = %g  F = %g\\\n", nit, sig, f);
4292iif (dbg_verbose) fprintf (stderr, "DSIG = %g\\\n", dsig);
4495iif (dbg_verbose) fprintf (stderr, "SIG2 -- N1 = %d  N2 = %d\\\n", *n1, *n2);
4648iif (dbg_verbose) fprintf (stderr, "%d -- SIG = %g  F = %g  FP = %g\\\n", nit, sig, f, fp);
5273iif (dbg_verbose) fprintf (stderr, "SMSURF -- THE CONSTRAINT IS NOT ACTIVE AND THE FITTING FCN IS CONSTANT\\\n");
5284iif (dbg_verbose) fprintf (stderr, "SMSURF -- SM = %g  GSTOL = %g  NITMAX = %d  G(0) = %g\\\n", *sm, tol, nitmax, g0);
5330iif (dbg_verbose) fprintf (stderr, " %d -- P = %g  G = %g  NIT = %d  DFMAX = %g\\\n", iter, p, g, nit, dfmax);
5367iif (dbg_verbose) fprintf (stderr, "DP = %g\\\n", dp);
1054c\
	if ((d1d2 == 0. && s1 != s2) || (s == 0. && s1 * s2 > 0.)) {
1118c\
	if (abs(dsig) <= rtol * sig || (f >= 0. && f <= ftol) || abs(f) <= rtol)
1243c\
	if (abs(dmax__) <= stol || (f >= 0. && f <= ftol) || abs(f) <= rtol) {
2894c\
    if (nn < 3 || (*iflgg <= 0 && nn < 7) || *ist < 1 || *ist > nn) {
3573c\
    if ((rf < 0. && min(h1,h2) < bnd) || (rf > 0. && bnd < max(h1,h2))) {
3587c\
    if ((h1 == bnd && rf * s1 > 0.) || (h2 == bnd && rf * s2 < 0.)) {
3788c\
    if (abs(dmax__) <= stol || (f >= 0. && f <= ftol) || abs(f) <= rtol) {
4113c\
    if ((rf < 0. && min(d__1,s) < bnd) || (rf > 0. && bnd < max(d__2,s))) {
4270c\
    if (abs(dmax__) <= stol || (f >= 0. && f <= ftol) || abs(f) <= rtol) {
4655c\
    if (abs(dsig) <= rtol * sig || (f >= 0. && f <= ftol) || abs(f) <= rtol) {
2444i\
    static doublereal cos_plat;
2560,2562c\
    sincos (*plat, &p[2], &cos_plat);\
    sincos (*plon, &p[1], &p[0]);\
    p[0] *= cos_plat;\
    p[1] *= cos_plat;
EOF

cat << EOF > ssrfpack.c
/* \$Id\$
 * ssrfpack.c: Translated via f2c then massaged so that f2c include and lib
 * are not required to compile and link the sph supplement.
 */

double d_sign (doublereal *a, doublereal *b)
{
	double x;
	x = (*a >= 0 ? *a : - *a);
	return (*b >= 0 ? x : -x);
}
EOF
sed -f $$.sed ssrfpack_nowrite.c | tail +13 | grep -v "#include" >> ssrfpack.c

rm -f $$.sed ssrfpack_nowrite.[cf]
