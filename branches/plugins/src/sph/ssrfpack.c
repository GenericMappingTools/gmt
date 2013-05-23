/* $Id$
 * ssrfpack.c: Translated via f2c then massaged so that f2c include and lib
 * are not required to compile and link the sph supplement.
 */

/* Need three functions from stripack.c: */
extern doublereal store_(doublereal *);
extern integer lstptr_(integer *, integer *, integer *, integer *);
extern integer trfind_(integer *, doublereal *, integer *, 
	    doublereal *, doublereal *, doublereal *, integer *, integer *, 
	    integer *, doublereal *, doublereal *, doublereal *, integer *, 
	    integer *, integer *);

double d_sign (doublereal *a, doublereal *b)
{
	double x;
	x = (*a >= 0 ? *a : - *a);
	return (*b >= 0 ? x : -x);
}

/* Table of constant values */

static doublereal c_b23 = 1.0;

/* Subroutine */ integer aplyr_(doublereal *x, doublereal *y, doublereal *z__, 
	doublereal *cx, doublereal *sx, doublereal *cy, doublereal *sy, 
	doublereal *xp, doublereal *yp, doublereal *zp)
{
    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static doublereal t;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   05/09/92 */

/*   This subroutine applies the rotation R defined by Sub- */
/* routine CONSTR to the unit vector (X Y Z)**T, i,e. (X,Y,Z) */
/* is rotated to (XP,YP,ZP).  If (XP,YP,ZP) lies in the */
/* southern hemisphere (ZP < 0), (XP,YP) are set to the */
/* coordinates of the nearest point of the equator, ZP re- */
/* maining unchanged. */

/* On input: */

/*       X,Y,Z = Coordinates of a point on the unit sphere. */

/*       CX,SX,CY,SY = Elements of the rotation defined by */
/*                     Subroutine CONSTR. */

/* Input parameters are not altered except as noted below. */

/* On output: */

/*       XP,YP,ZP = Coordinates of the rotated point on the */
/*                  sphere unless ZP < 0, in which case */
/*                  (XP,YP,0) is the closest point of the */
/*                  equator to the rotated point.  Storage */
/*                  for XP, YP, and ZP may coincide with */
/*                  storage for X, Y, and Z, respectively, */
/*                  if the latter need not be saved. */

/* Modules required by APLYR:  None */

/* Intrinsic function called by APLYR:  SQRT */

/* *********************************************************** */


/* Local parameter: */

/* T = Temporary variable */

    t = *sx * *y + *cx * *z__;
    *yp = *cx * *y - *sx * *z__;
    *zp = *sy * *x + *cy * t;
    *xp = *cy * *x - *sy * t;
    if (*zp >= 0.) {
	return 0;
    }

/* Move (XP,YP,ZP) to the equator. */

    t = sqrt(*xp * *xp + *yp * *yp);
    if (t == 0.) {
	goto L1;
    }
    *xp /= t;
    *yp /= t;
    return 0;

/* Move the south pole to an arbitrary point of the equator. */

L1:
    *xp = 1.;
    *yp = 0.;
    return 0;
} /* aplyr_ */

/* Subroutine */ integer aplyrt_(doublereal *g1p, doublereal *g2p, doublereal *cx,
	 doublereal *sx, doublereal *cy, doublereal *sy, doublereal *g)
{
    static doublereal t;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   05/09/92 */

/*   This subroutine applies the inverse (transpose) of the */
/* rotation defined by Subroutine CONSTR to the vector */
/* (G1P G2P 0)**T, i.e., the gradient (G1P,G2P,0) in the rot- */
/* ated coordinate system is mapped to (G1,G2,G3) in the */
/* original coordinate system. */

/* On input: */

/*       G1P,G2P = X and Y components, respectively, of the */
/*                 gradient in the rotated coordinate system. */

/*       CX,SX,CY,SY = Elements of the rotation R constructed */
/*                     by Subroutine CONSTR. */

/* Input parameters are not altered by this routine. */

/* On output: */

/*       G = X, Y, and Z components (in that order) of the */
/*           inverse rotation applied to (G1P,G2P,0) -- */
/*           gradient in the original coordinate system. */

/* Modules required by APLYRT:  None */

/* *********************************************************** */


/* Local parameters: */

/* T = Temporary variable */

    /* Parameter adjustments */
    --g;

    /* Function Body */
    t = *sy * *g1p;
    g[1] = *cy * *g1p;
    g[2] = *cx * *g2p - *sx * t;
    g[3] = -(*sx) * *g2p - *cx * t;
    return 0;
} /* aplyrt_ */

doublereal arclen_(doublereal *p, doublereal *q)
{
    /* System generated locals */
    doublereal ret_val, d__1;

    /* Builtin functions */
    double atan(doublereal), sqrt(doublereal);

    /* Local variables */
    static doublereal d__;
    static integer i__;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   05/09/92 */

/*   This function computes the arc-length (angle in radians) */
/* between a pair of points on the unit sphere. */

/* On input: */

/*       P,Q = Arrays of length 3 containing the X, Y, and Z */
/*             coordinates (in that order) of points on the */
/*             unit sphere. */

/* Input parameters are not altered by this function. */

/* On output: */

/*       ARCLEN = Angle in radians between the unit vectors */
/*                P and Q.  0 .LE. ARCLEN .LE. PI. */

/* Modules required by ARCLEN:  None */

/* Intrinsic functions called by ARCLEN:  ATAN, SQRT */

/* *********************************************************** */


/* Local parameters: */

/* D = Euclidean norm squared of P+Q */
/* I = DO-loop index */

    /* Parameter adjustments */
    --q;
    --p;

    /* Function Body */
    d__ = 0.;
    for (i__ = 1; i__ <= 3; ++i__) {
/* Computing 2nd power */
	d__1 = p[i__] + q[i__];
	d__ += d__1 * d__1;
/* L1: */
    }
    if (d__ == 0.) {

/* P and Q are separated by 180 degrees. */

	ret_val = atan(1.) * 4.;
    } else if (d__ >= 4.) {

/* P and Q coincide. */

	ret_val = 0.;
    } else {
	ret_val = atan(sqrt((4. - d__) / d__)) * 2.;
    }
    return ret_val;
} /* arclen_ */

/* Subroutine */ integer snhcsh_(doublereal *x, doublereal *sinhm, doublereal *
	coshm, doublereal *coshmm)
{
    /* Initialized data */

    static doublereal c1 = .1666666666659;
    static doublereal c2 = .008333333431546;
    static doublereal c3 = 1.984107350948e-4;
    static doublereal c4 = 2.768286868175e-6;

    /* Builtin functions */
    double exp(doublereal);

    /* Local variables */
    static doublereal f, ax, xc, xs, xsd2, xsd4, expx;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   03/18/90 */

/*   This subroutine computes approximations to the modified */
/* hyperbolic functions defined below with relative error */
/* bounded by 4.7E-12 for a floating point number system with */
/* sufficient precision.  For IEEE standard single precision, */
/* the relative error is less than 1.E-5 for all x. */

/*   Note that the 13-digit constants in the data statements */
/* below may not be acceptable to all compilers. */

/* On input: */

/*       X = Point at which the functions are to be */
/*           evaluated. */

/* X is not altered by this routine. */

/* On output: */

/*       SINHM = sinh(X) - X. */

/*       COSHM = cosh(X) - 1. */

/*       COSHMM = cosh(X) - 1 - X*X/2. */

/* Modules required by SNHCSH:  None */

/* Intrinsic functions called by SNHCSH:  ABS, EXP */

/* *********************************************************** */


    ax = fabs(*x);
    xs = ax * ax;
    if (ax <= .5) {

/* Approximations for small X: */

	xc = *x * xs;
	*sinhm = xc * (((c4 * xs + c3) * xs + c2) * xs + c1);
	xsd4 = xs * .25;
	xsd2 = xsd4 + xsd4;
	f = (((c4 * xsd4 + c3) * xsd4 + c2) * xsd4 + c1) * xsd4;
	*coshmm = xsd2 * f * (f + 2.);
	*coshm = *coshmm + xsd2;
    } else {

/* Approximations for large X: */

	expx = exp(ax);
	*sinhm = -(1. / expx + ax + ax - expx) / 2.;
	if (*x < 0.) {
	    *sinhm = -(*sinhm);
	}
	*coshm = (1. / expx - 2. + expx) / 2.;
	*coshmm = *coshm - xs / 2.;
    }
    return 0;
} /* snhcsh_ */

/* Subroutine */ integer arcint_(doublereal *p, doublereal *p1, doublereal *p2, 
	doublereal *f1, doublereal *f2, doublereal *g1, doublereal *g2, 
	doublereal *sigma, doublereal *f, doublereal *g, doublereal *gn)
{

    /* Builtin functions */
    double sqrt(doublereal), exp(doublereal);

    /* Local variables */
    static doublereal a, e;
    static integer i__;
    static doublereal s, b1, b2, d1, d2, e1, e2, al, cm, gt, sm, tm, un[3], 
	    ts, cm2, sb1, sb2, sm2, tm1, tm2, tp1, tp2, cmm, sig, ems, tau1, 
	    tau2, sinh__, sinh2, dummy, unorm;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   11/21/96 */

/*   Given 3 points P, P1, and P2 lying on a common geodesic */
/* of the unit sphere with P between P1 and P2, along with */
/* data values and gradients at P1 and P2, this subroutine */
/* computes an interpolated value F and a gradient vector G */
/* AT P.  F and the tangential component of G are taken to be */
/* the value and derivative (with respect to arc-length) of */
/* a Hermite interpolatory tension spline defined by the end- */
/* point values and tangential gradient components.  The nor- */
/* mal component of G is obtained by linear interpolation of */
/* the normal components of the gradients at P1 and P2. */

/* On input: */

/*       P = Cartesian coordinates of a point lying on the */
/*           arc defined by P1 and P2.  P(1)**2 + P(2)**2 + */
/*           P(3)**2 = 1. */

/*       P1,P2 = Coordinates of distinct points on the unit */
/*               sphere defining an arc with length less than */
/*               180 degrees. */

/*       F1,F2 = Data values associated with P1 and P2, */
/*               respectively. */

/*       G1,G2 = Gradient vectors associated with P1 and P2. */
/*               G1 and G2 are orthogonal to P1 and P2, */
/*               respectively. */

/*       SIGMA = Tension factor associated with P1-P2. */

/* The above parameters are not altered by this routine. */

/*       G = Array of length 3. */

/* On output: */

/*       F = Interpolated value at P. */

/*       G = Interpolated gradient at P. */

/*       GN = Normal component of G with the direction */
/*            P1 X P2 taken to be positive.  The extrapola- */
/*            tion procedure requires this component. */

/*   For each vector V, V(1), V(2), and V(3) contain X, Y, */
/* and Z components, respectively. */

/* SSRFPACK modules required by ARCINT:  ARCLEN, SNHCSH */

/* Intrinsic functions called by ARCINT:  ABS, EXP, SQRT */

/* *********************************************************** */

    /* Parameter adjustments */
    --g;
    --g2;
    --g1;
    --p2;
    --p1;
    --p;

    /* Function Body */

/* Local parameters: */

/* A =         Angle in radians (arc-length) between P1 and */
/*               P2 */
/* AL =        Arc-length between P1 and P */
/* B1,B2 =     Local coordinates of P with respect to P1-P2 */
/* CM,CMM =    Coshm(SIG) and Coshmm(SIG) -- refer to SNHCSH */
/* CM2 =       Coshm(SB2) */
/* DUMMY =     Dummy parameter for SNHCSH */
/* D1,D2 =     Scaled second differences */
/* E =         CM**2 - SM*Sinh = SIG*SM - 2*CMM (scaled by */
/*               2*EMS if SIG > .5) */
/* EMS =       Exp(-SIG) */
/* E1,E2 =     Exp(-SB1), Exp(-SB2) */
/* GT =        Tangential component of G -- component in the */
/*               direction UN X P */
/* I =         DO-loop index */
/* LUN =       Logical unit for error messages */
/* S =         Slope:  (F2-F1)/A */
/* SB1,SB2 =   SIG*B1, SIG*B2 */
/* SIG =       Abs(SIGMA) */
/* SINH =      Sinh(SIGMA) */
/* SINH2 =     Sinh(SB2) */
/* SM,SM2 =    Sinhm(SIG), Sinhm(SB2) */
/* TAU1,TAU2 = Tangential derivatives (components of G1,G2) */
/*               at P1 and P2 */
/* TM =        1-EMS */
/* TM1,TM2 =   1-E1, 1-E2 */
/* TP1,TP2 =   1+E1, 1+E2 */
/* TS =        TM**2 */
/* UN =        Unit normal to the plane of P, P1, and P2 */
/* UNORM =     Euclidean norm of P1 X P2 -- used to normalize */
/*               UN */


/* Compute unit normal UN. */

    un[0] = p1[2] * p2[3] - p1[3] * p2[2];
    un[1] = p1[3] * p2[1] - p1[1] * p2[3];
    un[2] = p1[1] * p2[2] - p1[2] * p2[1];
    unorm = sqrt(un[0] * un[0] + un[1] * un[1] + un[2] * un[2]);
    if (unorm == 0.) {
	goto L2;
    }

/* Normalize UN. */

    for (i__ = 1; i__ <= 3; ++i__) {
	un[i__ - 1] /= unorm;
/* L1: */
    }

/* Compute tangential derivatives at the endpoints: */
/*   TAU1 = (G1,UN X P1) = (G1,P2)/UNORM and */
/*   TAU2 = (G2,UN X P2) = -(G2,P1)/UNORM. */

    tau1 = (g1[1] * p2[1] + g1[2] * p2[2] + g1[3] * p2[3]) / unorm;
    tau2 = -(g2[1] * p1[1] + g2[2] * p1[2] + g2[3] * p1[3]) / unorm;

/* Compute arc-lengths A, AL. */

    a = arclen_(&p1[1], &p2[1]);
    if (a == 0.) {
	goto L2;
    }
    al = arclen_(&p1[1], &p[1]);

/* Compute local coordinates, slope, and second differences. */

    b2 = al / a;
    b1 = 1. - b2;
    s = (*f2 - *f1) / a;
    d1 = s - tau1;
    d2 = tau2 - s;

/* Test the range of SIGMA. */

    sig = fabs(*sigma);
    if (sig < 1e-9) {

/* Hermite cubic interpolation. */

	*f = *f1 + al * (tau1 + b2 * (d1 + b1 * (d1 - d2)));
	gt = tau1 + b2 * (d1 + d2 + b1 * 3. * (d1 - d2));
    } else if (sig <= .5) {

/* 0 < SIG .LE. .5.  Use approximations designed to avoid */
/*   cancellation error in the hyperbolic functions. */

	sb2 = sig * b2;
	snhcsh_(&sig, &sm, &cm, &cmm);
	snhcsh_(&sb2, &sm2, &cm2, &dummy);
	sinh__ = sm + sig;
	sinh2 = sm2 + sb2;
	e = sig * sm - cmm - cmm;
	*f = *f1 + al * tau1 + a * ((cm * sm2 - sm * cm2) * (d1 + d2) + sig * 
		(cm * cm2 - sinh__ * sm2) * d1) / (sig * e);
	gt = tau1 + ((cm * cm2 - sm * sinh2) * (d1 + d2) + sig * (cm * sinh2 
		- sinh__ * cm2) * d1) / e;
    } else {

/* SIG > .5.  Use negative exponentials in order to avoid */
/*   overflow.  Note that EMS = EXP(-SIG). */

	sb1 = sig * b1;
	sb2 = sig - sb1;
	e1 = exp(-sb1);
	e2 = exp(-sb2);
	ems = e1 * e2;
	tm = 1. - ems;
	ts = tm * tm;
	tm1 = 1. - e1;
	tm2 = 1. - e2;
	e = tm * (sig * (ems + 1.) - tm - tm);
	*f = *f1 + al * s + a * (tm * tm1 * tm2 * (d1 + d2) + sig * ((e2 * 
		tm1 * tm1 - b1 * ts) * d1 + (e1 * tm2 * tm2 - b2 * ts) * d2)) 
		/ (sig * e);
	tp1 = e1 + 1.;
	tp2 = e2 + 1.;
	gt = s + (tm1 * (tm * tp2 - sig * e2 * tp1) * d1 - tm2 * (tm * tp1 - 
		sig * e1 * tp2) * d2) / e;
    }

/* Compute GN. */

    *gn = b1 * (un[0] * g1[1] + un[1] * g1[2] + un[2] * g1[3]) + b2 * (un[0] *
	     g2[1] + un[1] * g2[2] + un[2] * g2[3]);

/* Compute G = GT*(UN X P) + GN*UN. */

    g[1] = gt * (un[1] * p[3] - un[2] * p[2]) + *gn * un[0];
    g[2] = gt * (un[2] * p[1] - un[0] * p[3]) + *gn * un[1];
    g[3] = gt * (un[0] * p[2] - un[1] * p[1]) + *gn * un[2];
    return 0;

/* P1 X P2 = 0.  Print an error message and terminate */
/*   processing. */

/*    2 WRITE (LUN,100) (P1(I),I=1,3), (P2(I),I=1,3) */
/*  100 FORMAT ('1','ERROR IN ARCINT -- P1 = ',2(F9.6,',  '), */
/*     .        F9.6/1X,19X,'P2 = ',2(F9.6,',  '),F9.6) */
L2:
#ifdef SPH_DEBUG
        fprintf (stderr, "ERROR IN ARCINT -- P1 = %9.6f %9.6f %9.6f   P2 = %9.6f %9.6f %9.6f\n", p1[1], p1[2], p1[3], p2[1], p2[2], p2[3]);
#endif
    i__ = 1;
    return 0;
} /* arcint_ */

/* Subroutine */ integer constr_(doublereal *xk, doublereal *yk, doublereal *zk, 
	doublereal *cx, doublereal *sx, doublereal *cy, doublereal *sy)
{
    /* Builtin functions */
    double sqrt(doublereal);


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   05/09/92 */

/*   This subroutine constructs the elements of a 3 by 3 */
/* orthogonal matrix R which rotates a point (XK,YK,ZK) on */
/* the unit sphere to the north pole, i.e., */

/*      (XK)     (CY  0 -SY)   (1   0   0)   (XK)     (0) */
/*  R * (YK)  =  ( 0  1   0) * (0  CX -SX) * (YK)  =  (0) */
/*      (ZK)     (SY  0  CY)   (0  SX  CX)   (ZK)     (1) */

/* On input: */

/*       XK,YK,ZK = Components of a unit vector to be */
/*                  rotated to (0,0,1). */

/* Input parameters are not altered by this routine. */

/* On output: */

/*       CX,SX,CY,SY = Elements of R:  CX,SX define a rota- */
/*                     tion about the X-axis and CY,SY define */
/*                     a rotation about the Y-axis. */

/* Modules required by CONSTR:  None */

/* Intrinsic function called by CONSTR:  SQRT */

/* *********************************************************** */

    *cy = sqrt(*yk * *yk + *zk * *zk);
    *sy = *xk;
    if (*cy != 0.) {
	*cx = *zk / *cy;
	*sx = *yk / *cy;
    } else {

/* (XK,YK,ZK) lies on the X-axis. */

	*cx = 1.;
	*sx = 0.;
    }
    return 0;
} /* constr_ */

doublereal hval_(doublereal *b, doublereal *h1, doublereal *h2, doublereal *
	hp1, doublereal *hp2, doublereal *sigma)
{
    /* System generated locals */
    doublereal ret_val;

    /* Builtin functions */
    double exp(doublereal);

    /* Local variables */
    static doublereal e, s, b1, b2, d1, d2, e1, e2, cm, sm, tm, ts, cm2, sb1, 
	    sb2, sm2, tm1, tm2, cmm, sig, ems, dummy;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   11/21/96 */

/*   Given a line segment P1-P2 containing a point P, along */
/* with values and derivatives at the endpoints, this func- */
/* tion returns the value H(P), where H is the Hermite inter- */
/* polatory tension spline defined by the endpoint data. */

/* On input: */

/*       B = Local coordinate of P with respect to P1-P2: */
/*           P = B*P1 + (1-B)*P2, and thus B = d(P,P2)/ */
/*           d(P1,P2), where d(P1,P2) is the distance between */
/*           P1 and P2.  B < 0 or B > 1 results in extrapola- */
/*           tion. */

/*       H1,H2 = Values interpolated at P1 and P2, respec- */
/*               tively. */

/*       HP1,HP2 = Products of d(P1,P2) with first order der- */
/*                 ivatives at P1 and P2, respectively.  HP1 */
/*                 may, for example, be the scalar product of */
/*                 P2-P1 with a gradient at P1. */

/*       SIGMA = Nonnegative tension factor associated with */
/*               the spline.  SIGMA = 0 corresponds to a */
/*               cubic spline, and H approaches the linear */
/*               interpolant of H1 and H2 as SIGMA increases. */

/* Input parameters are not altered by this function. */

/* On output: */

/*       HVAL = Interpolated value H(P). */

/* SSRFPACK module required by HVAL:  SNHCSH */

/* Intrinsic functions called by HVAL:  ABS, EXP */

/* *********************************************************** */

    b1 = *b;
    b2 = 1. - b1;

/* Compute slope S and second differences D1 and D2 scaled */
/*   by the separation between P1 and P2. */

    s = *h2 - *h1;
    d1 = s - *hp1;
    d2 = *hp2 - s;

/* Test the range of SIGMA. */

    sig = fabs(*sigma);
    if (sig < 1e-9) {

/* Hermite cubic interpolation: */

	ret_val = *h1 + b2 * (*hp1 + b2 * (d1 + b1 * (d1 - d2)));
    } else if (sig <= .5) {

/* 0 < SIG .LE. .5.  Use approximations designed to avoid */
/*   cancellation error in the hyperbolic functions. */

	sb2 = sig * b2;
	snhcsh_(&sig, &sm, &cm, &cmm);
	snhcsh_(&sb2, &sm2, &cm2, &dummy);
	e = sig * sm - cmm - cmm;
	ret_val = *h1 + b2 * *hp1 + ((cm * sm2 - sm * cm2) * (d1 + d2) + sig *
		 (cm * cm2 - (sm + sig) * sm2) * d1) / (sig * e);
    } else {

/* SIG > .5.  Use negative exponentials in order to avoid */
/*   overflow.  Note that EMS = EXP(-SIG). */

	sb1 = sig * b1;
	sb2 = sig - sb1;
	e1 = exp(-sb1);
	e2 = exp(-sb2);
	ems = e1 * e2;
	tm = 1. - ems;
	ts = tm * tm;
	tm1 = 1. - e1;
	tm2 = 1. - e2;
	e = tm * (sig * (ems + 1.) - tm - tm);
	ret_val = *h1 + b2 * s + (tm * tm1 * tm2 * (d1 + d2) + sig * ((e2 * 
		tm1 * tm1 - b1 * ts) * d1 + (e1 * tm2 * tm2 - b2 * ts) * d2)) 
		/ (sig * e);
    }
    return ret_val;
} /* hval_ */

doublereal fval_(doublereal *b1, doublereal *b2, doublereal *b3, doublereal *
	v1, doublereal *v2, doublereal *v3, doublereal *f1, doublereal *f2, 
	doublereal *f3, doublereal *g1, doublereal *g2, doublereal *g3, 
	doublereal *sig1, doublereal *sig2, doublereal *sig3)
{
    /* System generated locals */
    doublereal ret_val;

    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static doublereal f, g[3];
    static integer i__;
    static doublereal c1, c2, c3, q1[3], q2[3], q3[3], s1, s2, s3, u1[3], u2[
	    3], u3[3], ds, dv, u1n, u2n, u3n, sig, val, dum, sum;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   05/09/92 */

/*   Given data values and gradients at the three vertices of */
/* a spherical triangle containing a point P, this routine */
/* computes the value of F at P where F interpolates the ver- */
/* tex data.  Along the triangle sides, the interpolatory */
/* function F is the Hermite interpolatory tension spline */
/* defined by the values and tangential gradient components */
/* at the endpoints, and the gradient component normal to the */
/* triangle side varies linearly with respect to arc-length */
/* between the normal gradient components at the endpoints. */
/* A first-order C-1 blending method is used on the underly- */
/* ing planar triangle.  Since values and gradients on an arc */
/* depend only on the vertex data, the method results in C-1 */
/* continuity when used to interpolate over a triangulation. */

/*   The blending method consists of taking F(P) to be a */
/* weighted sum of the values at PP of three univariate Her- */
/* mite interpolatory tension splines defined on the line */
/* segments which join the vertices to the opposite sides and */
/* pass through PP:  the central projection of P onto the */
/* underlying planar triangle.  The tension factors for these */
/* splines are obtained by linear interpolation between the */
/* pair of tension factors associated with the triangle sides */
/* which join at the appropriate vertex. */

/*   A tension factor SIGMA associated with a Hermite interp- */
/* olatory tension spline is a nonnegative parameter which */
/* determines the curviness of the spline.  SIGMA = 0 results */
/* in a cubic spline, and the spline approaches the linear */
/* interpolant as SIGMA increases. */

/* On input: */

/*       B1,B2,B3 = Barycentric coordinates of PP with re- */
/*                  spect to the (planar) underlying triangle */
/*                  (V1,V2,V3), where PP is the central */
/*                  projection of P onto this triangle. */

/*       V1,V2,V3 = Cartesian coordinates of the vertices of */
/*                  a spherical triangle containing P.  V3 */
/*                  Left V1->V2. */

/*       F1,F2,F3 = Data values associated with the vertices. */

/*       G1,G2,G3 = Gradients associated with the vertices. */
/*                  Gi is orthogonal to Vi for i = 1,2,3. */

/*       SIG1,SIG2,SIG3 = Tension factors associated with the */
/*                        triangle sides opposite V1, V2, and */
/*                        V3, respectively. */

/* Input parameters are not altered by this function. */

/* On output: */

/*       FVAL = Interpolated value at P. */

/* Each vector V above contains X, Y, and Z components in */
/*   V(1), V(2), and V(3), respectively. */

/* SSRFPACK modules required by FVAL:  ARCINT, ARCLEN, HVAL */

/* Intrinsic function called by FVAL:  SQRT */

/* *********************************************************** */


/* Local parameters: */

/* C1,C2,C3 =    Coefficients (weight functions) of partial */
/*                 interpolants.  C1 = 1 on the edge opposite */
/*                 V1 and C1 = 0 on the other edges.  Simi- */
/*                 larly for C2 and C3.  C1+C2+C3 = 1. */
/* DS =          Directional derivative (scaled by distnace) */
/*                 at U1, U2, or U3:  DS = (G,U1-V1)/U1N = */
/*                 -(G,V1)/U1N on side opposite V1, where G/ */
/*                 U1N (plus an orthogonal component) is the */
/*                 projection of G onto the planar triangle */
/* DUM =         Dummy variable for calls to ARCINT */
/* DV =          Directional derivatives (scaled by distance) */
/*                 at a vertex:  D1 = (G1,U1-V1) = (G1,U1) */
/* F,G =         Value and gradient at Q1 Q2, or Q3 obtained */
/*                 by interpolation along one of the arcs of */
/*                 the spherical triangle */
/* I =           DO-loop index */
/* Q1,Q2,Q3 =    Central projections of U1, U2, and U3 onto */
/*                 the sphere and thus lying on an arc of the */
/*                 spherical triangle */
/* SIG =         Tension factor for a side-vertex (partial) */
/*                 interpolant:  obtained by linear interpo- */
/*                 lation applied to triangle side tensions */
/* SUM =         Quantity used to normalize C1, C2, and C3 */
/* S1,S2,S3 =    Sums of pairs of barycentric coordinates: */
/*                 used to compute U1, U2, U3, and SIG */
/* U1,U2,U3 =    Points on the boundary of the planar trian- */
/*                 gle and lying on the lines containing PP */
/*                 and the vertices.  U1 is opposite V1, etc. */
/* U1N,U2N,U3N = Quantities used to compute Q1, Q2, and Q3 */
/*                 (magnitudes of U1, U2, and U3) */
/* VAL =         Local variable used to accumulate the con- */
/*                 tributions to FVAL */


/* Compute weight functions C1, C2, and C3. */

    /* Parameter adjustments */
    --g3;
    --g2;
    --g1;
    --v3;
    --v2;
    --v1;

    /* Function Body */
    c1 = *b2 * *b3;
    c2 = *b3 * *b1;
    c3 = *b1 * *b2;
    sum = c1 + c2 + c3;
    if (sum <= 0.) {

/* P coincides with a vertex. */

	ret_val = *b1 * *f1 + *b2 * *f2 + *b3 * *f3;
	return ret_val;
    }

/* Normalize C1, C2, and C3. */

    c1 /= sum;
    c2 /= sum;
    c3 /= sum;

/* Compute (S1,S2,S3), (U1,U2,U3) and (U1N,U2N,U3N). */

    s1 = *b2 + *b3;
    s2 = *b3 + *b1;
    s3 = *b1 + *b2;
    u1n = 0.;
    u2n = 0.;
    u3n = 0.;
    for (i__ = 1; i__ <= 3; ++i__) {
	u1[i__ - 1] = (*b2 * v2[i__] + *b3 * v3[i__]) / s1;
	u2[i__ - 1] = (*b3 * v3[i__] + *b1 * v1[i__]) / s2;
	u3[i__ - 1] = (*b1 * v1[i__] + *b2 * v2[i__]) / s3;
	u1n += u1[i__ - 1] * u1[i__ - 1];
	u2n += u2[i__ - 1] * u2[i__ - 1];
	u3n += u3[i__ - 1] * u3[i__ - 1];
/* L1: */
    }

/* Compute Q1, Q2, and Q3. */

    u1n = sqrt(u1n);
    u2n = sqrt(u2n);
    u3n = sqrt(u3n);
    for (i__ = 1; i__ <= 3; ++i__) {
	q1[i__ - 1] = u1[i__ - 1] / u1n;
	q2[i__ - 1] = u2[i__ - 1] / u2n;
	q3[i__ - 1] = u3[i__ - 1] / u3n;
/* L2: */
    }

/* Compute interpolated value (VAL) at P by looping on */
/*   triangle sides. */

    val = 0.;

/* Contribution from side opposite V1: */

/*   Compute value and gradient at Q1 by interpolating */
/*     between V2 and V3. */

    arcint_(q1, &v2[1], &v3[1], f2, f3, &g2[1], &g3[1], sig1, &f, g, &dum);

/*   Add in the contribution. */

    dv = g1[1] * u1[0] + g1[2] * u1[1] + g1[3] * u1[2];
    ds = -(g[0] * v1[1] + g[1] * v1[2] + g[2] * v1[3]) / u1n;
    sig = (*b2 * *sig3 + *b3 * *sig2) / s1;
    val += c1 * hval_(b1, f1, &f, &dv, &ds, &sig);

/* Contribution from side opposite V2: */

/*   Compute value and gradient at Q2 by interpolating */
/*     between V3 and V1. */

    arcint_(q2, &v3[1], &v1[1], f3, f1, &g3[1], &g1[1], sig2, &f, g, &dum);

/*   Add in the contribution. */

    dv = g2[1] * u2[0] + g2[2] * u2[1] + g2[3] * u2[2];
    ds = -(g[0] * v2[1] + g[1] * v2[2] + g[2] * v2[3]) / u2n;
    sig = (*b3 * *sig1 + *b1 * *sig3) / s2;
    val += c2 * hval_(b2, f2, &f, &dv, &ds, &sig);

/* Contribution from side opposite V3: */

/*   Compute interpolated value and gradient at Q3 */
/*     by interpolating between V1 and V2. */

    arcint_(q3, &v1[1], &v2[1], f1, f2, &g1[1], &g2[1], sig3, &f, g, &dum);

/*   Add in the final contribution. */

    dv = g3[1] * u3[0] + g3[2] * u3[1] + g3[3] * u3[2];
    ds = -(g[0] * v3[1] + g[1] * v3[2] + g[2] * v3[3]) / u3n;
    sig = (*b1 * *sig2 + *b2 * *sig1) / s3;
    ret_val = val + c3 * hval_(b3, f3, &f, &dv, &ds, &sig);
    return ret_val;
} /* fval_ */

/* Subroutine */ integer getsig_(integer *n, doublereal *x, doublereal *y, 
	doublereal *z__, doublereal *h__, integer *list, integer *lptr, 
	integer *lend, doublereal *grad, doublereal *tol, doublereal *sigma, 
	doublereal *dsmax, integer *ier)
{
    /* Initialized data */

    static doublereal sbig = 85.;

    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1, d__2;

    /* Builtin functions */
    double sqrt(doublereal), exp(doublereal), d_sign(doublereal *, doublereal 
	    *);

    /* Local variables */
    static doublereal a, e, f, s, t, c1, c2, d0, d1, d2, f0;
    static integer n1, n2;
    static doublereal p1[3], p2[3], s1, s2, t0, t1, t2, al, fp, tm, un[3];
    static integer nm1, lp1, lp2;
    static doublereal tp1, d1d2, scm, dsm, ems, sig;
    static integer lpl;
    static doublereal sgn;
    static integer nit;
    static doublereal ssm, ems2, d1pd2, fneg, dsig, dmax__, fmax;
    static integer icnt;
    static doublereal ftol, rtol, stol, coshm, sigin, sinhm, ssinh;
    static doublereal unorm;
    static doublereal coshmm;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   11/21/96 */

/*   Given a triangulation of a set of nodes on the unit */
/* sphere, along with data values H and gradients GRAD at the */
/* nodes, this subroutine determines, for each triangulation */
/* arc, the smallest (nonnegative) tension factor SIGMA such */
/* that the Hermite interpolatory tension spline H(A), de- */
/* fined by SIGMA and the endpoint values and directional */
/* derivatives, preserves local shape properties of the data. */
/* In order to define the shape properties on an arc, it is */
/* convenient to map the arc to an interval (A1,A2).  Then, */
/* denoting the endpoint data values by H1,H2 and the deriva- */
/* tives (tangential gradient components) by HP1,HP2, and */
/* letting S = (H2-H1)/(A2-A1), the data properties are */

/*       Monotonicity:  S, HP1, and HP2 are nonnegative or */
/*                        nonpositive, */
/*   and */

/*       Convexity:     HP1 .LE. S .LE. HP2  or  HP1 .GE. S */
/*                        .GE. HP2. */

/* The corresponding properties of H are constant sign of the */
/* first and second derivatives, respectively.  Note that, */
/* unless HP1 = S = HP2, infinite tension is required (and H */
/* is linear on the interval) if S = 0 in the case of mono- */
/* tonicity, or if HP1 = S or HP2 = S in the case of */
/* convexity. */

/*   Note that if gradients are to be computed by Subroutine */
/* GRADG or function values and gradients are computed by */
/* SMSURF, it may be desirable to alternate those computa- */
/* tions (which require tension factors) with calls to this */
/* subroutine.  This iterative procedure should terminate */
/* with a call to GETSIG in order to ensure that the shape */
/* properties are preserved, and convergence can be achieved */
/* (at the cost of optimality) by allowing only increases in */
/* tension factors (refer to the parameter descriptions for */
/* SIGMA, DSMAX, and IER). */

/*   Refer to functions SIG0, SIG1, and SIG2 for means of */
/* selecting minimum tension factors to preserve more general */
/* properties. */

/* On input: */

/*       N = Number of nodes in the triangulation.  N .GE. 3. */

/*       X,Y,Z = Arrays of length N containing the Cartesian */
/*               coordinates of the nodes. */

/*       H = Array of length N containing data values at the */
/*           nodes.  H(I) is associated with (X(I),Y(I),Z(I)) */
/*           for I = 1,...,N. */

/*       LIST,LPTR,LEND = Data structure defining the tri- */
/*                        angulation.  Refer to STRIPACK */
/*                        Subroutine TRMESH. */

/*       GRAD = Array dimensioned 3 by N whose columns con- */
/*              tain gradients at the nodes.  GRAD( ,J) must */
/*              be orthogonal to node J:  GRAD(1,J)*X(J) + */
/*              GRAD(2,J)*Y(J) + GRAD(3,J)*Z(J) = 0..  Refer */
/*              to Subroutines GRADG, GRADL, and SMSURF. */

/*       TOL = Tolerance whose magnitude determines how close */
/*             each tension factor is to its optimal value */
/*             when nonzero finite tension is necessary and */
/*             sufficient to satisfy the constraint -- */
/*             abs(TOL) is an upper bound on the magnitude */
/*             of the smallest (nonnegative) or largest (non- */
/*             positive) value of the first or second deriva- */
/*             tive of H in the interval.  Thus, the con- */
/*             straint is satisfied, but possibly with more */
/*             tension than necessary. */

/* The above parameters are not altered by this routine. */

/*       SIGMA = Array of length 2*NA = 6*(N-1)-2*NB, where */
/*               NA and NB are the numbers of arcs and boun- */
/*               dary nodes, respectively, containing minimum */
/*               values of the tension factors.  The tension */
/*               factors are associated with arcs in one-to- */
/*               one correspondence with LIST entries.  Note */
/*               that each arc N1-N2 has two LIST entries and */
/*               thus, the tension factor is stored in both */
/*               SIGMA(I) and SIGMA(J) where LIST(I) = N2 (in */
/*               the adjacency list for N1) and LIST(J) = N1 */
/*               (in the list associated with N2).  SIGMA */
/*               should be set to all zeros if minimal ten- */
/*               sion is desired, and should be unchanged */
/*               from a previous call in order to ensure con- */
/*               vergence of the iterative procedure describ- */
/*               ed in the header comments. */

/* On output: */

/*       SIGMA = Array containing tension factors for which */
/*               H(A) preserves the local data properties on */
/*               each triangulation arc, with the restriction */
/*               that SIGMA(I) .LE. 85 for all I (unless the */
/*               input value is larger).  The factors are as */
/*               small as possible (within the tolerance) but */
/*               not less than their input values.  If infin- */
/*               ite tension is required on an arc, the cor- */
/*               responding factor is SIGMA(I) = 85 (and H */
/*               is an approximation to the linear inter- */
/*               polant on the arc), and if neither property */
/*               is satisfied by the data, then SIGMA(I) = 0 */
/*               (assuming its input value is 0), and thus H */
/*               is cubic on the arc. */

/*       DSMAX = Maximum increase in a component of SIGMA */
/*               from its input value. */

/*       IER = Error indicator and information flag: */
/*             IER = I if no errors were encountered and I */
/*                     components of SIGMA were altered from */
/*                     their input values for I .GE. 0. */
/*             IER = -1 if N < 3.  SIGMA is not altered in */
/*                      this case. */
/*             IER = -2 if duplicate nodes were encountered. */

/* STRIPACK modules required by GETSIG:  LSTPTR, STORE */

/* SSRFPACK modules required by GETSIG:  ARCLEN, SNHCSH */

/* Intrinsic functions called by GETSIG:  ABS, EXP, MAX, MIN, */
/*                                          SIGN, SQRT */

/* *********************************************************** */


    /* Parameter adjustments */
    grad -= 4;
    --lend;
    --h__;
    --z__;
    --y;
    --x;
    --list;
    --lptr;
    --sigma;

    /* Function Body */
    nm1 = *n - 1;
    if (nm1 < 2) {
	goto L11;
    }

/* Compute an absolute tolerance FTOL = abs(TOL) and a */
/*   relative tolerance RTOL = 100*Macheps. */

    ftol = fabs(*tol);
    rtol = 1.;
L1:
    rtol /= 2.;
    d__1 = rtol + 1.;
    if (store_(&d__1) > 1.) {
	goto L1;
    }
    rtol *= 200.;

/* Print a heading. */

/*      IF (LUN .GE. 0) WRITE (LUN,100) N, FTOL */
/*  100 FORMAT ('1',13X,'GETSIG -- N =',I4,', TOL = ',E10.3//) */
#ifdef SPH_DEBUG
    fprintf (stderr, "GETSIG -- N = %4d TOL = %g\n", *n, ftol);
#endif

/* Initialize change counter ICNT and maximum change DSM for */
/*   the loop on arcs. */

    icnt = 0;
    dsm = 0.;

/* Loop on arcs N1-N2 for which N2 > N1.  LPL points to the */
/*   last neighbor of N1. */

    i__1 = nm1;
    for (n1 = 1; n1 <= i__1; ++n1) {
	lpl = lend[n1];
	lp1 = lpl;

/*   Top of loop on neighbors N2 of N1. */

L2:
	lp1 = lptr[lp1];
	n2 = (i__2 = list[lp1], GMT_abs(i__2));
	if (n2 <= n1) {
	    goto L9;
	}

/* Print a message and compute parameters for the arc: */
/*   nodal coordinates P1 and P2, arc-length AL, */
/*   UNORM = magnitude of P1 X P2, and */
/*   SIGIN = input SIGMA value. */

/*        IF (LUN .GE. 0) WRITE (LUN,110) N1, N2 */
/*  110   FORMAT (/1X,'ARC',I4,' -',I4) */
#ifdef SPH_DEBUG
    fprintf (stderr, "ARC %d - %d\n", n1, n2);
#endif
	p1[0] = x[n1];
	p1[1] = y[n1];
	p1[2] = z__[n1];
	p2[0] = x[n2];
	p2[1] = y[n2];
	p2[2] = z__[n2];
	al = arclen_(p1, p2);
	un[0] = p1[1] * p2[2] - p1[2] * p2[1];
	un[1] = p1[2] * p2[0] - p1[0] * p2[2];
	un[2] = p1[0] * p2[1] - p1[1] * p2[0];
	unorm = sqrt(un[0] * un[0] + un[1] * un[1] + un[2] * un[2]);
	if (unorm == 0. || al == 0.) {
	    goto L12;
	}
	sigin = sigma[lp1];
	if (sigin >= sbig) {
	    goto L9;
	}

/* Compute scaled directional derivatives S1,S2 at the end- */
/*   points (for the direction N1->N2), first difference S, */
/*   and second differences D1,D2. */

	s1 = al * (grad[n1 * 3 + 1] * p2[0] + grad[n1 * 3 + 2] * p2[1] + grad[
		n1 * 3 + 3] * p2[2]) / unorm;
	s2 = -al * (grad[n2 * 3 + 1] * p1[0] + grad[n2 * 3 + 2] * p1[1] + 
		grad[n2 * 3 + 3] * p1[2]) / unorm;
	s = h__[n2] - h__[n1];
	d1 = s - s1;
	d2 = s2 - s;
	d1d2 = d1 * d2;

/* Test for infinite tension required to satisfy either */
/*   property. */

	sig = sbig;
if ((d1d2 == 0. && s1 != s2) || (s == 0. && s1 * s2 > 0.)) {
	    goto L8;
	}

/* Test for SIGMA = 0 sufficient.  The data satisfies convex- */
/*   ity iff D1D2 .GE. 0, and D1D2 = 0 implies S1 = S = S2. */

	sig = 0.;
	if (d1d2 < 0.) {
	    goto L4;
	}
	if (d1d2 == 0.) {
	    goto L8;
	}
/* Computing MAX */
	d__1 = d1 / d2, d__2 = d2 / d1;
	t = max(d__1,d__2);
	if (t <= 2.) {
	    goto L8;
	}
	tp1 = t + 1.;

/* Convexity:  find a zero of F(SIG) = SIG*coshm(SIG)/ */
/*   sinhm(SIG) - TP1. */

/*   F(0) = 2-T < 0, F(TP1) .GE. 0, the derivative of F */
/*     vanishes at SIG = 0, and the second derivative of F is */
/*     .2 at SIG = 0.  A quadratic approximation is used to */
/*     obtain a starting point for the Newton method. */

	sig = sqrt(t * 10. - 20.);
	nit = 0;

/*   Top of loop: */

L3:
	if (sig <= .5) {
	    snhcsh_(&sig, &sinhm, &coshm, &coshmm);
	    t1 = coshm / sinhm;
	    fp = t1 + sig * (sig / sinhm - t1 * t1 + 1.);
	} else {

/*   Scale sinhm and coshm by 2*exp(-SIG) in order to avoid */
/*     overflow with large SIG. */

	    ems = exp(-sig);
	    ssm = 1. - ems * (ems + sig + sig);
	    t1 = (1. - ems) * (1. - ems) / ssm;
	    fp = t1 + sig * (sig * 2. * ems / ssm - t1 * t1 + 1.);
	}

	f = sig * t1 - tp1;
/*        IF (LUN .GE. 0) WRITE (LUN,120) SIG, F, FP */
/*  120   FORMAT (1X,'CONVEXITY -- SIG = ',E15.8, */
/*     .          ', F(SIG) = ',E15.8/1X,35X,'FP(SIG) = ', */
/*     .          E15.8) */
#ifdef SPH_DEBUG
    fprintf (stderr, "CONVEXITY -- SIG = %g  F(SIG) = %g  FP(SIG) = %g\n", sig, f, fp);
#endif
	++nit;

/*   Test for convergence. */

	if (fp <= 0.) {
	    goto L8;
	}
	dsig = -f / fp;
if (fabs(dsig) <= rtol * sig || (f >= 0. && f <= ftol) || fabs(f) <= rtol)
		 {
	    goto L8;
	}

/*   Update SIG. */

	sig += dsig;
	goto L3;

/* Convexity cannot be satisfied.  Monotonicity can be satis- */
/*   fied iff S1*S .GE. 0 and S2*S .GE. 0 since S .NE. 0. */

L4:
	if (s1 * s < 0. || s2 * s < 0.) {
	    goto L8;
	}
	t0 = s * 3. - s1 - s2;
	d0 = t0 * t0 - s1 * s2;

/* SIGMA = 0 is sufficient for monotonicity iff S*T0 .GE. 0 */
/*   or D0 .LE. 0. */

	if (d0 <= 0. || s * t0 >= 0.) {
	    goto L8;
	}

/* Monotonicity:  find a zero of F(SIG) = sign(S)*HP(R), */
/*   where HPP(R) = 0 and HP, HPP denote derivatives of H. */
/*   F has a unique zero, F(0) < 0, and F approaches */
/*   abs(S) as SIG increases. */

/*   Initialize parameters for the secant method.  The method */
/*     uses three points:  (SG0,F0), (SIG,F), and */
/*     (SNEG,FNEG), where SG0 and SNEG are defined implicitly */
/*     by DSIG = SIG - SG0 and DMAX = SIG - SNEG. */

	sgn = d_sign(&c_b23, &s);
	sig = sbig;
	fmax = sgn * (sig * s - s1 - s2) / (sig - 2.);
	if (fmax <= 0.) {
	    goto L8;
	}
	stol = rtol * sig;
	f = fmax;
	f0 = sgn * d0 / ((d1 - d2) * 3.);
	fneg = f0;
	dsig = sig;
	dmax__ = sig;
	d1pd2 = d1 + d2;
	nit = 0;

/*   Top of loop:  compute the change in SIG by linear */
/*     interpolation. */

L5:
	dsig = -f * dsig / (f - f0);
/*        IF (LUN .GE. 0) WRITE (LUN,130) DSIG */
/*  130   FORMAT (1X,'MONOTONICITY -- DSIG = ',E15.8) */
#ifdef SPH_DEBUG
    fprintf (stderr, "MONOTONICITY -- DSIG = %g\n", dsig);
#endif
	if (fabs(dsig) > fabs(dmax__) || dsig * dmax__ > 0.) {
	    goto L7;
	}

/*   Restrict the step-size such that abs(DSIG) .GE. STOL/2. */
/*     Note that DSIG and DMAX have opposite signs. */

	if (fabs(dsig) < stol / 2.) {
	    d__1 = stol / 2.;
	    dsig = -d_sign(&d__1, &dmax__);
	}

/*   Update SIG, F0, and F. */

	sig += dsig;
	f0 = f;
	if (sig <= .5) {

/*   Use approximations to the hyperbolic functions designed */
/*     to avoid cancellation error with small SIG. */

	    snhcsh_(&sig, &sinhm, &coshm, &coshmm);
	    c1 = sig * coshm * d2 - sinhm * d1pd2;
	    c2 = sig * (sinhm + sig) * d2 - coshm * d1pd2;
	    a = c2 - c1;
	    e = sig * sinhm - coshmm - coshmm;
	} else {

/*   Scale sinhm and coshm by 2*exp(-SIG) in order to avoid */
/*     overflow with large SIG. */

	    ems = exp(-sig);
	    ems2 = ems + ems;
	    tm = 1. - ems;
	    ssinh = tm * (ems + 1.);
	    ssm = ssinh - sig * ems2;
	    scm = tm * tm;
	    c1 = sig * scm * d2 - ssm * d1pd2;
	    c2 = sig * ssinh * d2 - scm * d1pd2;

/*   R is in (0,1) and well-defined iff HPP(T1)*HPP(T2) < 0. */

	    f = fmax;
	    if (c1 * (sig * scm * d1 - ssm * d1pd2) >= 0.) {
		goto L6;
	    }
	    a = ems2 * (sig * tm * d2 + (tm - sig) * d1pd2);
	    if (a * (c2 + c1) < 0.) {
		goto L6;
	    }
	    e = sig * ssinh - scm - scm;
	}

	f = (sgn * (e * s2 - c2) + sqrt(a * (c2 + c1))) / e;

/*   Update the number of iterations NIT. */

L6:
	++nit;
/*        IF (LUN .GE. 0) WRITE (LUN,140) NIT, SIG, F */
/*  140   FORMAT (1X,11X,I2,' -- SIG = ',E15.8,', F = ', */
/*     .          E15.8) */
#ifdef SPH_DEBUG
    fprintf (stderr, "%d -- SIG = %g  F = %g\n", nit, sig, f);
#endif

/*   Test for convergence. */

	stol = rtol * sig;
if (fabs(dmax__) <= stol || (f >= 0. && f <= ftol) || fabs(f) <= rtol) {
	    goto L8;
	}
	dmax__ += dsig;
	if (f0 * f > 0. && fabs(f) >= fabs(f0)) {
	    goto L7;
	}
	if (f0 * f <= 0.) {

/*   F and F0 have opposite signs.  Update (SNEG,FNEG) to */
/*     (SG0,F0) so that F and FNEG always have opposite */
/*     signs.  If SIG is closer to SNEG than SG0 and abs(F) */
/*     < abs(FNEG), then swap (SNEG,FNEG) with (SG0,F0). */

	    t1 = dmax__;
	    t2 = fneg;
	    dmax__ = dsig;
	    fneg = f0;
	    if (fabs(dsig) > fabs(t1) && fabs(f) < fabs(t2)) {

		dsig = t1;
		f0 = t2;
	    }
	}
	goto L5;

/*   Bottom of loop:  F0*F > 0 and the new estimate would */
/*     be outside of the bracketing interval of length */
/*     abs(DMAX).  Reset (SG0,F0) to (SNEG,FNEG). */

L7:
	dsig = dmax__;
	f0 = fneg;
	goto L5;

/*  Update SIGMA, ICNT, and DSM if necessary. */

L8:
	sig = min(sig,sbig);
	if (sig > sigin) {
	    sigma[lp1] = sig;
	    lp2 = lstptr_(&lend[n2], &n1, &list[1], &lptr[1]);
	    sigma[lp2] = sig;
	    ++icnt;
/* Computing MAX */
	    d__1 = dsm, d__2 = sig - sigin;
	    dsm = max(d__1,d__2);
	}

/* Bottom of loop on neighbors N2 of N1. */

L9:
	if (lp1 != lpl) {
	    goto L2;
	}
/* L10: */
    }

/* No errors encountered. */

    *dsmax = dsm;
    *ier = icnt;
    return 0;

/* N < 3. */

L11:
    *dsmax = 0.;
    *ier = -1;
    return 0;

/* Nodes N1 and N2 coincide. */

L12:
    *dsmax = dsm;
    *ier = -2;
    return 0;
} /* getsig_ */

/* Subroutine */ integer givens_(doublereal *a, doublereal *b, doublereal *c__, 
	doublereal *s)
{
    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static doublereal r__, u, v, aa, bb;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   05/09/92 */

/*   This subroutine constructs the Givens plane rotation, */

/*           ( C  S) */
/*       G = (     ) , where C*C + S*S = 1, */
/*           (-S  C) */

/* which zeros the second component of the vector (A,B)**T */
/* (transposed).  Subroutine ROTATE may be called to apply */
/* the transformation to a 2 by N matrix. */

/*   This routine is identical to Subroutine SROTG from the */
/* LINPACK BLAS (Basic Linear Algebra Subroutines). */

/* On input: */

/*       A,B = Components of the vector defining the rota- */
/*             tion.  These are overwritten by values R */
/*             and Z (described below) which define C and S. */

/* On output: */

/*       A = Signed Euclidean norm R of the input vector: */
/*           R = +/-SQRT(A*A + B*B) */

/*       B = Value Z such that: */
/*             C = SQRT(1-Z*Z) and S=Z if ABS(Z) .LE. 1, and */
/*             C = 1/Z and S = SQRT(1-C*C) if ABS(Z) > 1. */

/*       C = +/-(A/R) or 1 if R = 0. */

/*       S = +/-(B/R) or 0 if R = 0. */

/* Modules required by GIVENS:  None */

/* Intrinsic functions called by GIVENS:  ABS, SQRT */

/* *********************************************************** */


/* Local parameters: */

/* AA,BB = Local copies of A and B */
/* R =     C*A + S*B = +/-SQRT(A*A+B*B) */
/* U,V =   Variables used to scale A and B for computing R */

    aa = *a;
    bb = *b;
    if (fabs(aa) > fabs(bb)) {

/* ABS(A) > ABS(B). */

	u = aa + aa;
	v = bb / u;
	r__ = sqrt(v * v + .25) * u;
	*c__ = aa / r__;
	*s = v * (*c__ + *c__);

/* Note that R has the sign of A, C > 0, and S has */
/*   SIGN(A)*SIGN(B). */

	*b = *s;
	*a = r__;
    } else if (bb != 0.) {

/* ABS(A) .LE. ABS(B). */

	u = bb + bb;
	v = aa / u;

/* Store R in A. */

	*a = sqrt(v * v + .25) * u;
	*s = bb / *a;
	*c__ = v * (*s + *s);

/* Note that R has the sign of B, S > 0, and C has */
/*   SIGN(A)*SIGN(B). */

	*b = 1.;
	if (*c__ != 0.) {
	    *b = 1. / *c__;
	}
    } else {

/* A = B = 0. */

	*c__ = 1.;
	*s = 0.;
    }
    return 0;
} /* givens_ */

/* Subroutine */ integer grcoef_(doublereal *sigma, doublereal *d__, doublereal *sd)
{
    /* Builtin functions */
    double exp(doublereal);

    /* Local variables */
    static doublereal e, scm, sig, ems, ssm, coshm, sinhm, ssinh, coshmm;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   11/21/96 */

/*   This subroutine computes factors involved in the linear */
/* systems solved by Subroutines GRADG and SMSGS. */

/* On input: */

/*       SIGMA = Nonnegative tension factor associated with a */
/*               triangulation arc. */

/* SIGMA is not altered by this routine. */

/* On output: */

/*       D = Diagonal factor.  D = SIG*(SIG*Coshm(SIG) - */
/*           Sinhm(SIG))/E where E = SIG*Sinh(SIG) - 2* */
/*           Coshm(SIG).  D > 0, and D = 4 at SIG = 0. */

/*       SD = Off-diagonal factor.  SD = SIG*Sinhm(SIG)/E. */
/*            SD > 0, and SD = 2 at SIG = 0. */

/* SSRFPACK module required by GRCOEF:  SNHCSH */

/* Intrinsic function called by GRCOEF:  EXP */

/* *********************************************************** */

    sig = *sigma;
    if (sig < 1e-9) {

/* Cubic function: */

	*d__ = 4.;
	*sd = 2.;
    } else if (sig <= .5) {

/* 0 < SIG .LE. .5. */

/* Use approximations designed to avoid cancellation error */
/*   in the hyperbolic functions when SIGMA is small. */

	snhcsh_(&sig, &sinhm, &coshm, &coshmm);
	e = sig * sinhm - coshmm - coshmm;
	*d__ = sig * (sig * coshm - sinhm) / e;
	*sd = sig * sinhm / e;
    } else {

/* SIG > .5. */

/* Scale SINHM, COSHM, and E by 2*EXP(-SIG) in order to */
/*   avoid overflow when SIGMA is large. */

	ems = exp(-sig);
	ssinh = 1. - ems * ems;
	ssm = ssinh - sig * 2. * ems;
	scm = (1. - ems) * (1. - ems);
	e = sig * ssinh - scm - scm;
	*d__ = sig * (sig * scm - ssm) / e;
	*sd = sig * ssm / e;
    }
    return 0;
} /* grcoef_ */

/* Subroutine */ integer gradg_(integer *n, doublereal *x, doublereal *y, 
	doublereal *z__, doublereal *f, integer *list, integer *lptr, integer 
	*lend, integer *iflgs, doublereal *sigma, integer *nit, doublereal *
	dgmax, doublereal *grad, integer *ier)
{
    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1, d__2;

    /* Builtin functions */
    double sqrt(doublereal), atan(doublereal);

    /* Local variables */
    static doublereal d__;
    static integer j, k;
    static doublereal t, g1, g2, g3, r1, r2, a11, a12, a22, fk, sd, cx;
    static integer nn;
    static doublereal cy, xj, xk, yk, zk, yj, zj, sx, sy, xs, ys, dg1, dg2, 
	    dgk[3], den;
    static integer ifl;
    static doublereal det, sig;
    static integer lpj, lpl;
    static doublereal tol, alfa, dgmx;
    static integer iter;
    static doublereal sinal;
    static integer maxit;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/24/96 */

/*   Given a triangulation of N nodes on the unit sphere with */
/* data values F at the nodes and tension factors SIGMA asso- */
/* ciated with the arcs, this routine uses a global method */
/* to compute estimated gradients at the nodes.  The method */
/* consists of minimizing a quadratic functional Q(G) over */
/* the N-vector G of gradients, where Q approximates the */
/* linearized curvature of the restriction to arcs of the */
/* interpolatory function F defined by Function FVAL.  The */
/* restriction of F to an arc of the triangulation is the */
/* Hermite interpolatory tension spline defined by the data */
/* values and tangential gradient components at the endpoints */
/* of the arc.  Letting D1F(A) and D2F(A) denote first and */
/* second derivatives of F with respect to a parameter A var- */
/* ying along a triangulation arc, Q is the sum of integrals */
/* over the arcs of D2F(A)**2 + ((SIGMA/L)*(D1F(A)-S))**2, */
/* where L denotes arc-length, SIGMA is the appropriate ten- */
/* sion factor, and S is the slope of the linear function of */
/* A which interpolates the values of F at the endpoints of */
/* the arc. */

/*   Since the gradient at node K lies in the plane tangent */
/* to the sphere surface at K, it is effectively defined by */
/* only two components -- its X and Y components in the coor- */
/* dinate system obtained by rotating K to the north pole. */
/* Thus, the minimization problem corresponds to an order-2N */
/* symmetric positive-definite sparse linear system which is */
/* solved by a block Gauss-Seidel method with 2 by 2 blocks. */

/*   An alternative method, Subroutine GRADL, computes a */
/* local approximation to the gradient at a single node and, */
/* although less efficient when all gradients are needed, was */
/* found to be generally more accurate (in the case of uni- */
/* form zero tension) when the nodal distribution is very */
/* dense, varies greatly, or does not cover the sphere. */
/* GRADG, on the other hand, was found to be slightly more */
/* accurate on a uniform distribution of 514 nodes. */

/* On input: */

/*       N = Number of nodes in the triangulation.  N .GE. 3. */

/*       X,Y,Z = Arrays of length N containing Cartesian */
/*               coordinates of the nodes.  X(I)**2 + Y(I)**2 */
/*               + Z(I)**2 = 1 for I = 1,...,N. */

/*       F = Array of length N containing data values at the */
/*           nodes.  F(I) is associated with (X(I),Y(I),Z(I)) */
/*           for I = 1,...,N. */

/*       LIST,LPTR,LEND = Data structure defining the trian- */
/*                        gulation.  Refer to STRIPACK */
/*                        Subroutine TRMESH. */

/*       IFLGS = Tension factor option: */
/*               IFLGS .LE. 0 if a single uniform tension */
/*                            factor is to be used. */
/*               IFLGS .GE. 1 if variable tension is desired. */

/*       SIGMA = Uniform tension factor (IFLGS .LE. 0), or */
/*               array containing tension factors associated */
/*               with arcs in one-to-one correspondence with */
/*               LIST entries (IFLGS .GE. 1).  Refer to Sub- */
/*               programs GETSIG, SIG0, SIG1, and SIG2. */

/* The above parameters are not altered by this routine. */

/*       NIT = Maximum number of Gauss-Seidel iterations to */
/*             be applied.  This maximum will likely be a- */
/*             chieved if DGMAX is smaller than the machine */
/*             precision.  NIT .GE. 0. */

/*       DGMAX = Nonnegative convergence criterion.  The */
/*               method is terminated when the maximum change */
/*               in a gradient between iterations is at most */
/*               DGMAX.  The change in a gradient is taken to */
/*               be the Euclidean norm of the difference (in */
/*               the rotated coordinate system) relative to 1 */
/*               plus the norm of the old gradient value. */

/*       GRAD = 3 by N array whose columns contain initial */
/*              solution estimates (zero vectors are suffici- */
/*              ent).  GRAD(I,J) contains component I of the */
/*              gradient at node J for I = 1,2,3 (X,Y,Z) and */
/*              J = 1,...,N.  GRAD( ,J) must be orthogonal to */
/*              node J -- GRAD(1,J)*X(J) + GRAD(2,J)*Y(J) + */
/*              GRAD(3,J)*Z(J) = 0. */

/* On output: */

/*       NIT = Number of Gauss-Seidel iterations employed. */

/*       DGMAX = Maximum change in a gradient at the last */
/*               iteration. */

/*       GRAD = Estimated gradients.  See the description */
/*              under input parameters.  GRAD is not changed */
/*              if IER = -1. */

/*       IER = Error indicator: */
/*             IER = 0 if no errors were encountered and the */
/*                     convergence criterion was achieved. */
/*             IER = 1 if no errors were encountered but con- */
/*                     vergence was not achieved within NIT */
/*                     iterations. */
/*             IER = -1 if N or DGMAX is outside its valid */
/*                      range or NIT .LT. 0 on input. */
/*             IER = -2 if all nodes are collinear or the */
/*                      triangulation is invalid. */
/*             IER = -3 if duplicate nodes were encountered. */

/* SSRFPACK modules required by GRADG:  APLYRT, CONSTR, */
/*                                        GRCOEF, SNHCSH */

/* Intrinsic functions called by GRADG:  ATAN, MAX, SQRT */

/* *********************************************************** */


/* Local parameters: */

/* ALFA =        Arc-length between nodes K and J */
/* A11,A12,A22 = Matrix components of the 2 by 2 block A*DG */
/*                 = R where A is symmetric, (DG1,DG2,0) is */
/*                 the change in the gradient at K, and R is */
/*                 the residual */
/* CX,CY =       Components of a rotation mapping K to the */
/*                 north pole (0,0,1) */
/* D =           Function of SIG computed by GRCOEF -- factor */
/*                 in the order-2 system */
/* DEN =         ALFA*SINAL**2 -- factor in the 2 by 2 system */
/* DET =         Determinant of the order-2 matrix */
/* DGK =         Change in GRAD( ,K) from the previous esti- */
/*                 mate in the original coordinate system */
/* DGMX =        Maximum change in a gradient between itera- */
/*                 tions */
/* DG1,DG2 =     Solution of the 2 by 2 system -- first 2 */
/*                 components of DGK in the rotated coordi- */
/*                 nate system */
/* FK =          Data value F(K) */
/* G1,G2,G3 =    Components of GRAD( ,K) */
/* IFL =         Local copy of IFLGS */
/* ITER =        Number of iterations used */
/* J =           Neighbor of K */
/* K =           DO-loop and node index */
/* LPJ =         LIST pointer of node J as a neighbor of K */
/* LPL =         Pointer to the last neighbor of K */
/* MAXIT =       Input value of NIT */
/* NN =          Local copy of N */
/* R1,R2 =       Components of the residual -- derivatives of */
/*                 Q with respect to the components of the */
/*                 gradient at node K */
/* SD =          Function of SIG computed by GRCOEF -- factor */
/*                 in the order-2 system */
/* SIG =         Tension factor associated with ARC K-J */
/* SINAL =       SIN(ALFA) -- magnitude of the vector cross */
/*                 product between nodes K and J */
/* SX,SY =       Components of a rotation mapping K to the */
/*                 north pole (0,0,1) */
/* T =           Temporary storage for factors in the system */
/*                 components */
/* TOL =         Local copy of DGMAX */
/* XK,YK,ZK =    Coordinates of node K -- X(K), Y(K), Z(K) */
/* XJ,YJ,ZJ =    Coordinates of node J in the rotated coor- */
/*                 dinate system */
/* XS,YS =       XJ**2, YJ**2 */

    /* Parameter adjustments */
    grad -= 4;
    --lend;
    --f;
    --z__;
    --y;
    --x;
    --list;
    --lptr;
    --sigma;

    /* Function Body */
    nn = *n;
    ifl = *iflgs;
    maxit = *nit;
    tol = *dgmax;

/* Test for errors in input, and initialize iteration count, */
/*   tension factor, and output value of DGMAX. */

    if (nn < 3 || maxit < 0 || tol < 0.) {
	goto L11;
    }
    iter = 0;
    sig = sigma[1];
    dgmx = 0.;

/* Top of iteration loop. */

L1:
    if (iter == maxit) {
	goto L4;
    }
    dgmx = 0.;

/* Loop on nodes. */

    i__1 = nn;
    for (k = 1; k <= i__1; ++k) {
	xk = x[k];
	yk = y[k];
	zk = z__[k];
	fk = f[k];
	g1 = grad[k * 3 + 1];
	g2 = grad[k * 3 + 2];
	g3 = grad[k * 3 + 3];

/*   Construct the rotation mapping node K to the north pole. */

	constr_(&xk, &yk, &zk, &cx, &sx, &cy, &sy);

/*   Initialize components of the 2 by 2 system for the */
/*     change (DG1,DG2,0) in the K-th solution components */
/*     (symmetric matrix in A and residual in R). */

	a11 = 0.;
	a12 = 0.;
	a22 = 0.;
	r1 = 0.;
	r2 = 0.;

/*   Loop on neighbors J of node K. */

	lpl = lend[k];
	lpj = lpl;
L2:
	lpj = lptr[lpj];
	j = (i__2 = list[lpj], GMT_abs(i__2));

/*   Compute the coordinates of J in the rotated system. */

	t = sx * y[j] + cx * z__[j];
	yj = cx * y[j] - sx * z__[j];
	zj = sy * x[j] + cy * t;
	xj = cy * x[j] - sy * t;

/*   Compute arc-length ALFA between J and K, SINAL = */
/*     SIN(ALFA), and DEN = ALFA*SIN(ALFA)**2. */

	alfa = atan(sqrt((1. - zj) / (zj + 1.))) * 2.;
	xs = xj * xj;
	ys = yj * yj;
	sinal = sqrt(xs + ys);
	den = alfa * (xs + ys);

/*   Test for coincident nodes and compute functions of SIG: */
/*     D = SIG*(SIG*COSHM-SINHM)/E and SD = SIG*SINHM/E for */
/*     E = SIG*SINH-2*COSHM. */

	if (den == 0.) {
	    goto L13;
	}
	if (ifl >= 1) {
	    sig = sigma[lpj];
	}
	grcoef_(&sig, &d__, &sd);

/*   Update the system components for node J. */

	t = d__ / den;
	a11 += t * xs;
	a12 += t * xj * yj;
	a22 += t * ys;
	t = (d__ + sd) * (fk - f[j]) / (alfa * alfa * sinal) + (d__ * (g1 * x[
		j] + g2 * y[j] + g3 * z__[j]) - sd * (grad[j * 3 + 1] * xk + 
		grad[j * 3 + 2] * yk + grad[j * 3 + 3] * zk)) / den;
	r1 -= t * xj;
	r2 -= t * yj;

/*   Bottom of loop on neighbors. */

	if (lpj != lpl) {
	    goto L2;
	}

/*   Solve the 2 by 2 system and update DGMAX. */

	det = a11 * a22 - a12 * a12;
	if (det == 0. || a11 == 0.) {
	    goto L12;
	}
	dg2 = (a11 * r2 - a12 * r1) / det;
	dg1 = (r1 - a12 * dg2) / a11;
/* Computing MAX */
	d__1 = dgmx, d__2 = sqrt(dg1 * dg1 + dg2 * dg2) / (sqrt(g1 * g1 + g2 *
		 g2 + g3 * g3) + 1.);
	dgmx = max(d__1,d__2);

/*   Rotate (DG1,DG2,0) back to the original coordinate */
/*     system and update GRAD( ,K). */

	aplyrt_(&dg1, &dg2, &cx, &sx, &cy, &sy, dgk);
	grad[k * 3 + 1] = g1 + dgk[0];
	grad[k * 3 + 2] = g2 + dgk[1];
	grad[k * 3 + 3] = g3 + dgk[2];
/* L3: */
    }

/*   Increment ITER and test for convergence. */

    ++iter;
    if (dgmx > tol) {
	goto L1;
    }

/* The method converged. */

    *nit = iter;
    *dgmax = dgmx;
    *ier = 0;
    return 0;

/* The method failed to converge within NIT iterations. */

L4:
    *dgmax = dgmx;
    *ier = 1;
    return 0;

/* Invalid input parameter. */

L11:
    *nit = 0;
    *dgmax = 0.;
    *ier = -1;
    return 0;

/* Node K and its neighbors are collinear. */

L12:
    *nit = 0;
    *dgmax = dgmx;
    *ier = -2;
    return 0;

/* Nodes K and J coincide. */

L13:
    *nit = 0;
    *dgmax = dgmx;
    *ier = -3;
    return 0;
} /* gradg_ */

/* Subroutine */ integer getnp_(doublereal *x, doublereal *y, doublereal *z__, 
	integer *list, integer *lptr, integer *lend, integer *l, integer *
	npts, doublereal *df, integer *ier)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer i__, n1;
    static doublereal x1, y1, z1;
    static integer nb, ni, lp, np, lm1;
    static doublereal dnb, dnp;
    static integer lpl;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/28/98 */

/*   Given a Delaunay triangulation of N nodes on the unit */
/* sphere and an array NPTS containing the indexes of L-1 */
/* nodes ordered by angular distance from NPTS(1), this sub- */
/* routine sets NPTS(L) to the index of the next node in the */
/* sequence -- the node, other than NPTS(1),...,NPTS(L-1), */
/* that is closest to NPTS(1).  Thus, the ordered sequence */
/* of K closest nodes to N1 (including N1) may be determined */
/* by K-1 calls to GETNP with NPTS(1) = N1 and L = 2,3,...,K */
/* for K .GE. 2. */

/*   The algorithm uses the property of a Delaunay triangula- */
/* tion that the K-th closest node to N1 is a neighbor of one */
/* of the K-1 closest nodes to N1. */


/* On input: */

/*       X,Y,Z = Arrays of length N containing the Cartesian */
/*               coordinates of the nodes. */

/*       LIST,LPTR,LEND = Triangulation data structure.  Re- */
/*                        fer to Subroutine TRMESH. */

/*       L = Number of nodes in the sequence on output.  2 */
/*           .LE. L .LE. N. */

/* The above parameters are not altered by this routine. */

/*       NPTS = Array of length .GE. L containing the indexes */
/*              of the L-1 closest nodes to NPTS(1) in the */
/*              first L-1 locations. */

/* On output: */

/*       NPTS = Array updated with the index of the L-th */
/*              closest node to NPTS(1) in position L unless */
/*              IER = 1. */

/*       DF = Value of an increasing function (negative cos- */
/*            ine) of the angular distance between NPTS(1) */
/*            and NPTS(L) unless IER = 1. */

/*       IER = Error indicator: */
/*             IER = 0 if no errors were encountered. */
/*             IER = 1 if L < 2. */

/* Modules required by GETNP:  None */

/* Intrinsic function called by GETNP:  ABS */

/* *********************************************************** */


/* Local parameters: */

/* DNB,DNP =  Negative cosines of the angular distances from */
/*              N1 to NB and to NP, respectively */
/* I =        NPTS index and DO-loop index */
/* LM1 =      L-1 */
/* LP =       LIST pointer of a neighbor of NI */
/* LPL =      Pointer to the last neighbor of NI */
/* N1 =       NPTS(1) */
/* NB =       Neighbor of NI and candidate for NP */
/* NI =       NPTS(I) */
/* NP =       Candidate for NPTS(L) */
/* X1,Y1,Z1 = Coordinates of N1 */

    /* Parameter adjustments */
    --x;
    --y;
    --z__;
    --list;
    --lptr;
    --lend;
    --npts;

    /* Function Body */
    lm1 = *l - 1;
    if (lm1 < 1) {
	goto L6;
    }
    *ier = 0;

/* Store N1 = NPTS(1) and mark the elements of NPTS. */

    n1 = npts[1];
    x1 = x[n1];
    y1 = y[n1];
    z1 = z__[n1];
    i__1 = lm1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	ni = npts[i__];
	lend[ni] = -lend[ni];
/* L1: */
    }

/* Candidates for NP = NPTS(L) are the unmarked neighbors */
/*   of nodes in NPTS.  DNP is initially greater than -cos(PI) */
/*   (the maximum distance). */

    dnp = 2.;

/* Loop on nodes NI in NPTS. */

    i__1 = lm1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	ni = npts[i__];
	lpl = -lend[ni];
	lp = lpl;

/* Loop on neighbors NB of NI. */

L2:
	nb = (i__2 = list[lp], GMT_abs(i__2));
	if (lend[nb] < 0) {
	    goto L3;
	}

/* NB is an unmarked neighbor of NI.  Replace NP if NB is */
/*   closer to N1. */

	dnb = -(x[nb] * x1 + y[nb] * y1 + z__[nb] * z1);
	if (dnb >= dnp) {
	    goto L3;
	}
	np = nb;
	dnp = dnb;
L3:
	lp = lptr[lp];
	if (lp != lpl) {
	    goto L2;
	}
/* L4: */
    }
    npts[*l] = np;
    *df = dnp;

/* Unmark the elements of NPTS. */

    i__1 = lm1;
    for (i__ = 1; i__ <= i__1; ++i__) {
	ni = npts[i__];
	lend[ni] = -lend[ni];
/* L5: */
    }
    return 0;

/* L is outside its valid range. */

L6:
    *ier = 1;
    return 0;
} /* getnp_ */

/* Subroutine */ integer setup_(doublereal *xi, doublereal *yi, doublereal *wi, 
	doublereal *wk, doublereal *s1, doublereal *s2, doublereal *wt, 
	doublereal *row)
{
    static doublereal w1, w2;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   05/09/92 */

/*   This subroutine sets up the I-th row of an augmented */
/* regression matrix for a weighted least squares fit of a */
/* quadratic function Q(X,Y) to a set of data values Wi, */
/* where Q(0,0) = Wk.  The first 3 columns (quadratic terms) */
/* are scaled by 1/S2 and the fourth and fifth columns (lin- */
/* ear terms) are scaled by 1/S1. */

/* On input: */

/*       XI,YI = Coordinates of node I. */

/*       WI = Data value at node I. */

/*       WK = Data value interpolated by Q at the origin. */

/*       S1,S2 = Inverse scale factors. */

/*       WT = Weight factor corresponding to the I-th */
/*            equation. */

/*       ROW = Array of length 6. */

/* Input parameters are not altered by this routine. */

/* On output: */

/*       ROW = Array containing a row of the augmented re- */
/*             gression matrix. */

/* Modules required by SETUP:  None */

/* *********************************************************** */


/* Local parameters: */

/* W1 = Weighted scale factor for the linear terms */
/* W2 = Weighted scale factor for the quadratic terms */

    /* Parameter adjustments */
    --row;

    /* Function Body */
    w1 = *wt / *s1;
    w2 = *wt / *s2;
    row[1] = *xi * *xi * w2;
    row[2] = *xi * *yi * w2;
    row[3] = *yi * *yi * w2;
    row[4] = *xi * w1;
    row[5] = *yi * w1;
    row[6] = (*wi - *wk) * *wt;
    return 0;
} /* setup_ */

/* Subroutine */ integer rotate_(integer *n, doublereal *c__, doublereal *s, 
	doublereal *x, doublereal *y)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__;
    static doublereal xi, yi;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   09/01/88 */

/*                                                ( C  S) */
/*   This subroutine applies the Givens rotation  (     )  to */
/*                                                (-S  C) */
/*                    (X(1) ... X(N)) */
/* the 2 by N matrix  (             ) . */
/*                    (Y(1) ... Y(N)) */

/*   This routine is identical to Subroutine SROT from the */
/* LINPACK BLAS (Basic Linear Algebra Subroutines). */

/* On input: */

/*       N = Number of columns to be rotated. */

/*       C,S = Elements of the Givens rotation.  Refer to */
/*             Subroutine GIVENS. */

/* The above parameters are not altered by this routine. */

/*       X,Y = Arrays of length .GE. N containing the compo- */
/*             nents of the vectors to be rotated. */

/* On output: */

/*       X,Y = Arrays containing the rotated vectors (not */
/*             altered if N < 1). */

/* Modules required by ROTATE:  None */

/* *********************************************************** */


    /* Parameter adjustments */
    --y;
    --x;

    /* Function Body */
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	xi = x[i__];
	yi = y[i__];
	x[i__] = *c__ * xi + *s * yi;
	y[i__] = -(*s) * xi + *c__ * yi;
/* L1: */
    }
    return 0;
} /* rotate_ */

/* Subroutine */ integer gradl_(integer *n, integer *k, doublereal *x, doublereal 
	*y, doublereal *z__, doublereal *w, integer *list, integer *lptr, 
	integer *lend, doublereal *g, integer *ier)
{
    /* Initialized data */

    static doublereal rtol = 1e-6;
    static doublereal dtol = .01;
    static doublereal sf = 1.;

    /* System generated locals */
    integer i__1;
    doublereal d__1, d__2;

    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static doublereal a[36]	/* was [6][6] */, c__;
    static integer i__, j, l;
    static doublereal s, df;
    static integer kk;
    static doublereal av, rf, cx;
    static integer nn;
    static doublereal cy;
    static integer np;
    static doublereal dx, dy, wk, xp, yp, zp, sx, sy, wt;
    static integer im1, ip1, jp1, lm1;
    static doublereal rin;
    static integer lnp;
    static doublereal sum, dmin__;
    static integer lmin, ierr, lmax;
    static doublereal avsq;
    static integer npts[30];


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/24/96 */

/*   Given a triangulation of a set of nodes on the unit */
/* sphere with their associated data values W, this routine */
/* estimates a gradient vector at node K as follows:  the */
/* coordinate system is rotated so that K becomes the north */
/* pole, node K and a set of nearby nodes are projected */
/* orthogonally onto the X-Y plane (in the new coordinate */
/* system), a quadratic is fitted in a weighted least squares */
/* sense to the data values at the projected nodes such that */
/* the value (associated with K) at (0,0) is interpolated, X */
/* and Y-partial derivative estimates DX and DY are computed */
/* by differentiating the quadratic at (0,0), and the esti- */
/* mated gradient G is obtained by rotating (DX,DY,0) back to */
/* the original coordinate system.  Note that G lies in the */
/* plane tangent to the sphere at node K, i.e., G is orthogo- */
/* nal to the unit vector represented by node K.  A Marquardt */
/* stabilization factor is used if necessary to ensure a */
/* well-conditioned least squares system, and a unique solu- */
/* tion exists unless the nodes are collinear. */

/* On input: */

/*       N = Number of nodes in the triangulation.  N .GE. 7. */

/*       K = Node at which the gradient is sought.  1 .LE. K */
/*           .LE. N. */

/*       X,Y,Z = Arrays containing the Cartesian coordinates */
/*               of the nodes. */

/*       W = Array containing the data values at the nodes. */
/*           W(I) is associated with (X(I),Y(I),Z(I)) for */
/*           I = 1,...,N. */

/*       LIST,LPTR,LEND = Data structure defining the trian- */
/*                        gulation.  Refer to STRIPACK */
/*                        Subroutine TRMESH. */

/* Input parameters are not altered by this routine. */

/* On output: */

/*       G = X, Y, and Z components (in that order) of the */
/*           estimated gradient at node K unless IER < 0. */

/*       IER = Error indicator: */
/*             IER .GE. 6 if no errors were encountered. */
/*                        IER contains the number of nodes */
/*                        (including K) used in the least */
/*                        squares fit. */
/*             IER = -1 if N or K is outside its valid range. */
/*             IER = -2 if the least squares system has no */
/*                      unique solution due to duplicate or */
/*                      collinear nodes. */

/* STRIPACK module required by GRADL:  GETNP */

/* SSRFPACK modules required by GRADL:  APLYR, APLYRT, */
/*                                        CONSTR, GIVENS, */
/*                                        ROTATE, SETUP */

/* Intrinsic functions called by GRADL:  ABS, MIN, REAL, SQRT */

/* *********************************************************** */


    /* Parameter adjustments */
    --lend;
    --w;
    --z__;
    --y;
    --x;
    --list;
    --lptr;
    --g;

    /* Function Body */

/* Local parameters: */

/* A =         Transpose of the (upper triangle of the) aug- */
/*               mented regression matrix */
/* AV =        Root-mean-square distance (in the rotated */
/*               coordinate system) between the origin and */
/*               the nodes (other than K) in the least */
/*               squares fit.  The first 3 columns of A**T */
/*               are scaled by 1/AVSQ, the next 2 by 1/AV. */
/* AVSQ =      AV*AV:  accumulated in SUM */
/* C,S =       Components of the plane rotation used to */
/*               triangularize the regression matrix */
/* CX,SX =     Components of a plane rotation about the X- */
/*               axis which, together with CY and SY, define */
/*               a mapping from node K to the north pole */
/*               (0,0,1) */
/* CY,SY =     Components of a plane rotation about the Y- */
/*               axis */
/* DF =        Negative Z component (in the rotated coordi- */
/*               nate system) of an element NP of NPTS -- */
/*               increasing function of the angular distance */
/*               between K and NP.  DF lies in the interval */
/*               (-1,1). */
/* DMIN =      Minimum of the magnitudes of the diagonal */
/*               elements of the triangularized regression */
/*               matrix */
/* DTOL =      Tolerance for detecting an ill-conditioned */
/*               system (DMIN is required to be at least */
/*               DTOL) */
/* DX,DY =     X and Y components of the estimated gradient */
/*               in the rotated coordinate system */
/* I,J =       Loop indexes */
/* IERR =      Error flag for calls to GETNP (not checked) */
/* IM1,IP1 =   I-1, I+1 */
/* JP1 =       J+1 */
/* KK =        Local copy of K */
/* L =         Number of columns of A**T to which a rotation */
/*               is applied */
/* LM1 =       LMIN-1 */
/* LMIN,LMAX = Min(LMN,N), Min(LMX,N) */
/* LMN,LMX =   Minimum and maximum values of LNP for N */
/*               sufficiently large.  In most cases LMN-1 */
/*               nodes are used in the fit.  7 .LE. LMN .LE. */
/*               LMX. */
/* LNP =       Length of NPTS or LMAX+1 */
/* NN =        Local copy of N */
/* NP =        Element of NPTS to be added to the system */
/* NPTS =      Array containing the indexes of a sequence of */
/*               nodes ordered by angular distance from K. */
/*               NPTS(1)=K and the first LNP-1 elements of */
/*               NPTS are used in the least squares fit. */
/*               unless LNP = LMAX+1, NPTS(LNP) determines R */
/*               (see RIN). */
/* RF =        Value of DF associated with NPTS(LNP) unless */
/*               LNP = LMAX+1 (see RIN) */
/* RIN =       Inverse of a radius of influence R which */
/*               enters into WT:  R = 1+RF unless all ele- */
/*               ments of NPTS are used in the fit (LNP = */
/*               LMAX+1), in which case R is the distance */
/*               function associated with some point more */
/*               distant from K than NPTS(LMAX) */
/* RTOL =      Tolerance for determining LNP (and hence R): */
/*               if the increase in DF between two successive */
/*               elements of NPTS is less than RTOL, they are */
/*               treated as being the same distance from node */
/*               K and an additional node is added */
/* SF =        Marquardt stabilization factor used to damp */
/*               out the first 3 solution components (second */
/*               partials of the quadratic) when the system */
/*               is ill-conditioned.  Increasing SF results */
/*               in more damping (a more nearly linear fit). */
/* SUM =       Sum of squared Euclidean distances (in the */
/*               rotated coordinate system) between the */
/*               origin and the nodes used in the least */
/*               squares fit */
/* WK =        W(K) -- data value at node K */
/* WT =        Weight for the equation coreesponding to NP: */
/*               WT = (R-D)/(R*D) = 1/D - RIN, where D = 1-ZP */
/*               is associated with NP */
/* XP,YP,ZP =  Coordinates of NP in the rotated coordinate */
/*               system unless ZP < 0, in which case */
/*               (XP,YP,0) lies on the equator */

    nn = *n;
    kk = *k;
    wk = w[kk];

/* Check for errors and initialize LMIN, LMAX. */

    if (nn < 7 || kk < 1 || kk > nn) {
	goto L13;
    }
    lmin = min(10,nn);
    lmax = min(30,nn);

/* Compute NPTS, LNP, AVSQ, AV, and R. */
/*   Set NPTS to the closest LMIN-1 nodes to K.  DF contains */
/*   the negative Z component (in the rotated coordinate */
/*   system) of the new node on return from GETNP. */

    sum = 0.;
    npts[0] = kk;
    lm1 = lmin - 1;
    i__1 = lm1;
    for (lnp = 2; lnp <= i__1; ++lnp) {
	getnp_(&x[1], &y[1], &z__[1], &list[1], &lptr[1], &lend[1], &lnp, 
		npts, &df, &ierr);
	sum = sum + 1. - df * df;
/* L1: */
    }

/*   Add additional nodes to NPTS until the increase in */
/*     R = 1+RF is at least RTOL. */

    i__1 = lmax;
    for (lnp = lmin; lnp <= i__1; ++lnp) {
	getnp_(&x[1], &y[1], &z__[1], &list[1], &lptr[1], &lend[1], &lnp, 
		npts, &rf, &ierr);
	if (rf - df >= rtol) {
	    goto L3;
	}
	sum = sum + 1. - rf * rf;
/* L2: */
    }

/*   Use all LMAX nodes in the least squares fit.  R is */
/*     arbitrarily increased by 5 percent. */

    rf = rf * 1.05 + .05;
    lnp = lmax + 1;

/*   There are LNP-2 equations corresponding to nodes */
/*     NPTS(2),...,NPTS(LNP-1). */

L3:
    avsq = sum / (doublereal) (lnp - 2);
    av = sqrt(avsq);
    rin = 1. / (rf + 1.);

/* Construct the rotation. */

    constr_(&x[kk], &y[kk], &z__[kk], &cx, &sx, &cy, &sy);

/* Set up the first 5 equations of the augmented regression */
/*   matrix (transposed) as the columns of A, and zero out */
/*   the lower triangle (upper triangle of A) with Givens */
/*   rotations. */

    for (i__ = 1; i__ <= 5; ++i__) {
	np = npts[i__];
	aplyr_(&x[np], &y[np], &z__[np], &cx, &sx, &cy, &sy, &xp, &yp, &zp);
	wt = 1. / (1. - zp) - rin;
	setup_(&xp, &yp, &w[np], &wk, &av, &avsq, &wt, &a[i__ * 6 - 6]);
	if (i__ == 1) {
	    goto L5;
	}
	im1 = i__ - 1;
	i__1 = im1;
	for (j = 1; j <= i__1; ++j) {
	    jp1 = j + 1;
	    l = 6 - j;
	    givens_(&a[j + j * 6 - 7], &a[j + i__ * 6 - 7], &c__, &s);
	    rotate_(&l, &c__, &s, &a[jp1 + j * 6 - 7], &a[jp1 + i__ * 6 - 7]);
/* L4: */
	}
L5:
	;
    }

/* Add the additional equations to the system using */
/*   the last column of A.  I .LE. LNP. */

    i__ = 7;
L6:
    if (i__ == lnp) {
	goto L8;
    }
    np = npts[i__ - 1];
    aplyr_(&x[np], &y[np], &z__[np], &cx, &sx, &cy, &sy, &xp, &yp, &zp);
    wt = 1. / (1. - zp) - rin;
    setup_(&xp, &yp, &w[np], &wk, &av, &avsq, &wt, &a[30]);
    for (j = 1; j <= 5; ++j) {
	jp1 = j + 1;
	l = 6 - j;
	givens_(&a[j + j * 6 - 7], &a[j + 29], &c__, &s);
	rotate_(&l, &c__, &s, &a[jp1 + j * 6 - 7], &a[jp1 + 29]);
/* L7: */
    }
    ++i__;
    goto L6;

/* Test the system for ill-conditioning. */

L8:
/* Computing MIN */
    d__1 = fabs(a[0]), d__2 = fabs(a[7]), d__1 = min(d__1,d__2), d__2 = fabs(a[
	    14]), d__1 = min(d__1,d__2), d__2 = fabs(a[21]), d__1 = min(d__1,
	    d__2), d__2 = fabs(a[28]);
    dmin__ = min(d__1,d__2);
    if (dmin__ >= dtol) {
	goto L12;
    }
    if (lnp <= lmax) {

/* Add another node to the system and increase R. */
/*   I = LNP. */

	++lnp;
	if (lnp <= lmax) {
	    getnp_(&x[1], &y[1], &z__[1], &list[1], &lptr[1], &lend[1], &lnp, 
		    npts, &rf, &ierr);
	}
	rin = 1. / ((rf + 1.) * 1.05);
	goto L6;
    }

/* Stabilize the system by damping second partials.  Add */
/*   multiples of the first three unit vectors to the first */
/*   three equations. */

    for (i__ = 1; i__ <= 3; ++i__) {
	a[i__ + 29] = sf;
	ip1 = i__ + 1;
	for (j = ip1; j <= 6; ++j) {
	    a[j + 29] = 0.;
/* L9: */
	}
	for (j = i__; j <= 5; ++j) {
	    jp1 = j + 1;
	    l = 6 - j;
	    givens_(&a[j + j * 6 - 7], &a[j + 29], &c__, &s);
	    rotate_(&l, &c__, &s, &a[jp1 + j * 6 - 7], &a[jp1 + 29]);
/* L10: */
	}
/* L11: */
    }

/* Test the linear portion of the stabilized system for */
/*   ill-conditioning. */

/* Computing MIN */
    d__1 = fabs(a[21]), d__2 = fabs(a[28]);
    dmin__ = min(d__1,d__2);
    if (dmin__ < dtol) {
	goto L14;
    }

/* Solve the 2 by 2 triangular system for the estimated */
/*   partial derivatives. */

L12:
    dy = a[29] / a[28];
    dx = (a[23] - a[22] * dy) / a[21] / av;
    dy /= av;

/* Rotate the gradient (DX,DY,0) back into the original */
/*   coordinate system. */

    aplyrt_(&dx, &dy, &cx, &sx, &cy, &sy, &g[1]);
    *ier = lnp - 1;
    return 0;

/* N or K is outside its valid range. */

L13:
    *ier = -1;
    return 0;

/* No unique solution due to collinear nodes. */

L14:
    *ier = -2;
    return 0;
} /* gradl_ */

/* Subroutine */ integer intrc0_(integer *n, doublereal *plat, doublereal *plon, 
	doublereal *x, doublereal *y, doublereal *z__, doublereal *w, integer 
	*list, integer *lptr, integer *lend, integer *ist, doublereal *pw, 
	integer *ier)
{
    /* Builtin functions */
    double cos(doublereal), sin(doublereal);

    /* Local variables */
    static doublereal p[3], b1, b2, b3;
    static integer i1, i2, i3, n1, n2;
    static doublereal s12;
    static integer lp;
    static doublereal cos_plat;
    static doublereal sum, ptn1, ptn2;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/24/96 */

/*   Given a triangulation of a set of nodes on the unit */
/* sphere, along with data values at the nodes, this sub- */
/* routine computes the value at a point P of a continuous */
/* function which interpolates the data values.  The interp- */
/* olatory function is linear on each underlying triangle */
/* (planar triangle with the same vertices as a spherical */
/* triangle).  If P is not contained in a triangle, an ex- */
/* trapolated value is taken to be the interpolated value at */
/* the nearest point of the triangulation boundary. */

/* On input: */

/*       N = Number of nodes in the triangulation.  N .GE. 3. */

/*       PLAT,PLON = Latitude and longitude of P in radians. */

/*       X,Y,Z = Arrays containing Cartesian coordinates of */
/*               the nodes. */

/*       W = Array containing data values at the nodes.  W(I) */
/*           is associated with (X(I),Y(I),Z(I)) for I = */
/*           1,...,N. */

/*       LIST,LPTR,LEND = Data structure defining the trian- */
/*                        gulation.  Refer to STRIPACK */
/*                        Subroutine TRMESH. */

/*       IST = Index of the starting node in the search for a */
/*             triangle containing P.  1 .LE. IST .LE. N. */
/*             The output value of IST from a previous call */
/*             may be a good choice. */

/* Input parameters other than IST are not altered by this */
/*   routine. */

/* On output: */

/*       IST = Index of one of the vertices of the triangle */
/*             containing P (or nearest P) unless IER = -1 */
/*             or IER = -2. */

/*       PW = Value of the interpolatory function at P if */
/*            IER .GE. 0. */

/*       IER = Error indicator: */
/*             IER = 0 if interpolation was performed */
/*                     successfully. */
/*             IER = 1 if extrapolation was performed */
/*                     successfully. */
/*             IER = -1 if N < 3 or IST is outside its valid */
/*                      range. */
/*             IER = -2 if the nodes are collinear. */
/*             IER = -3 if P is not in a triangle and the */
/*                      angle between P and the nearest boun- */
/*                      dary point is at least 90 degrees. */

/* STRIPACK modules required by INTRC0:  JRAND, LSTPTR, */
/*                                         STORE, TRFIND */

/* Intrinsic functions called by INTRC0:  COS, SIN */

/* *********************************************************** */


/* Local parameters: */

/* B1,B2,B3 = Barycentric coordinates of the central projec- */
/*              tion of P onto the underlying planar trian- */
/*              gle, or (B1 and B2) projection of Q onto the */
/*              underlying line segment N1-N2 when P is */
/*              exterior.  Unnormalized coordinates are */
/*              computed by TRFIND when P is in a triangle. */
/* I1,I2,I3 = Vertex indexes returned by TRFIND */
/* LP =       LIST pointer to N1 as a neighbor of N2 or N2 */
/*              as a neighbor of N1 */
/* N1,N2 =    Endpoints of a boundary arc which is visible */
/*              from P when P is not contained in a triangle */
/* P =        Cartesian coordinates of P */
/* PTN1 =     Scalar product (P,N1) */
/* PTN2 =     Scalar product (P,N2) */
/* S12 =      Scalar product (N1,N2) */
/* SUM =      Quantity used to normalize the barycentric */
/*              coordinates */

    /* Parameter adjustments */
    --lend;
    --w;
    --z__;
    --y;
    --x;
    --list;
    --lptr;

    /* Function Body */
    if (*n < 3 || *ist < 1 || *ist > *n) {
	goto L11;
    }

/* Transform (PLAT,PLON) to Cartesian coordinates. */

sincos (*plat, &p[2], &cos_plat);    sincos (*plon, &p[1], &p[0]);    p[0] *= cos_plat;    p[1] *= cos_plat;

/* Find the vertex indexes of a triangle containing P. */

    trfind_(ist, p, n, &x[1], &y[1], &z__[1], &list[1], &lptr[1], &lend[1], &
	    b1, &b2, &b3, &i1, &i2, &i3);
    if (i1 == 0) {
	goto L12;
    }
    *ist = i1;
    if (i3 != 0) {

/* P is contained in the triangle (I1,I2,I3).  Normalize the */
/*   barycentric coordinates. */

	sum = b1 + b2 + b3;
	b1 /= sum;
	b2 /= sum;
	b3 /= sum;
	*pw = b1 * w[i1] + b2 * w[i2] + b3 * w[i3];
	*ier = 0;
	return 0;
    }

/* P is exterior to the triangulation, and I1 and I2 are */
/*   boundary nodes which are visible from P.  Set PW to the */
/*   interpolated value at Q, where Q is the closest boundary */
/*   point to P. */

/* Traverse the boundary starting from the rightmost visible */
/*   node I1. */

    n1 = i1;
    ptn1 = p[0] * x[n1] + p[1] * y[n1] + p[2] * z__[n1];
    if (i1 != i2) {
	goto L2;
    }

/* All boundary nodes are visible from P.  Find a boundary */
/*   arc N1->N2 such that P Left (N2 X N1)->N1. */

/* Counterclockwise boundary traversal: */
/*   Set N2 to the first neighbor of N1. */

L1:
    lp = lend[n1];
    lp = lptr[lp];
    n2 = list[lp];

/* Compute inner products (N1,N2) and (P,N2), and compute */
/*   B2 = DET(P,N1,N2 X N1). */

    s12 = x[n1] * x[n2] + y[n1] * y[n2] + z__[n1] * z__[n2];
    ptn2 = p[0] * x[n2] + p[1] * y[n2] + p[2] * z__[n2];
    b2 = ptn2 - s12 * ptn1;
    if (b2 <= 0.) {
	goto L2;
    }

/* P Right (N2 X N1)->N1 -- Iterate. */

    n1 = n2;
    i1 = n1;
    ptn1 = ptn2;
    goto L1;

/* P Left (N2 X N1)->N1, where N2 is the first neighbor of P1. */
/*   Clockwise boundary traversal: */

L2:
    n2 = n1;
    ptn2 = ptn1;

/* Set N1 to the last neighbor of N2 and test for */
/*   termination. */

    lp = lend[n2];
    n1 = -list[lp];
    if (n1 == i1) {
	goto L13;
    }

/* Compute inner products (N1,N2) and (P,N1). */

    s12 = x[n1] * x[n2] + y[n1] * y[n2] + z__[n1] * z__[n2];
    ptn1 = p[0] * x[n1] + p[1] * y[n1] + p[2] * z__[n1];

/* Compute B2 = DET(P,N1,N2 X N1) = DET(Q,N1,N2 X N1)*(P,Q). */

    b2 = ptn2 - s12 * ptn1;
    if (b2 <= 0.) {
	goto L2;
    }

/* Compute B1 = DET(P,N2 X N1,N2) = DET(Q,N2 X N1,N2)*(P,Q). */

    b1 = ptn1 - s12 * ptn2;
    if (b1 <= 0.) {

/* Q = N2. */

	*pw = w[n2];
    } else {

/* P Strictly Left (N2 X N1)->N2 and P Strictly Left */
/*   N1->(N2 X N1).  Thus Q lies on the interior of N1->N2. */
/*   Normalize the coordinates and compute PW. */

	sum = b1 + b2;
	*pw = (b1 * w[n1] + b2 * w[n2]) / sum;
    }
    *ier = 1;
    return 0;

/* N or IST is outside its valid range. */

L11:
    *ier = -1;
    return 0;

/* Collinear nodes. */

L12:
    *ier = -2;
    return 0;

/* The angular distance between P and the closest boundary */
/*   point to P is at least 90 degrees. */

L13:
    *ier = -3;
    return 0;
} /* intrc0_ */

/* Subroutine */ integer intrc1_(integer *n, doublereal *plat, doublereal *plon, 
	doublereal *x, doublereal *y, doublereal *z__, doublereal *f, integer 
	*list, integer *lptr, integer *lend, integer *iflgs, doublereal *
	sigma, integer *iflgg, doublereal *grad, integer *ist, doublereal *fp,
	 integer *ier)
{
    /* Builtin functions */
    double cos(doublereal), sin(doublereal), sqrt(doublereal);

    /* Local variables */
    static doublereal a;
    static integer i__;
    static doublereal p[3], q[3], b1, b2, b3, g1[3], g2[3];
    static integer i1, i2, i3;
    static doublereal g3[3];
    static integer n1, n2;
    static doublereal p1[3], p2[3], p3[3], s1, s2, s3, fq, gq[3], s12;
    static integer lp, nn;
    static doublereal dum[3], gqn, sum, ptn1, ptn2;
    static integer ierr;
    static doublereal ptgq;
    static doublereal qnorm;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/25/96 */

/*   Given a triangulation of a set of nodes on the unit */
/* sphere, along with data values and gradients at the nodes, */
/* this routine computes a value F(P), where F interpolates */
/* the nodal data and is once-continuously differentiable */
/* over the convex hull of the nodes.  Refer to Function FVAL */
/* for further details.  If P is not contained in a triangle, */
/* an extrapolated value is computed by extending F beyond */
/* the boundary in a continuous fashion. */

/* On input: */

/*       N = Number of nodes in the triangulation.  N .GE. 3 */
/*           and N .GE. 7 if IFLGG .LE. 0. */

/*       PLAT,PLON = Latitude and longitude in radians of the */
/*                   point P at which F is to be evaluated. */

/*       X,Y,Z = Arrays of length N containing Cartesian */
/*               coordinates of the nodes. */

/*       F = Array of length N containing values of F at the */
/*           nodes:  F(I) = F(X(I),Y(I),Z(I)). */

/*       LIST,LPTR,LEND = Data structure defining the trian- */
/*                        gulation.  Refer to STRIPACK */
/*                        Subroutine TRMESH. */

/*       IFLGS = Tension factor option: */
/*               IFLGS .LE. 0 if a single uniform tension */
/*                            factor is to be used. */
/*               IFLGS .GE. 1 if variable tension is desired. */

/*       SIGMA = Uniform tension factor (IFLGS .LE. 0), or */
/*               array containing tension factors associated */
/*               with arcs in one-to-one correspondence with */
/*               LIST entries (IFLGS .GE. 1).  Refer to Sub- */
/*               programs FVAL, GETSIG, SIG0, SIG1, and SIG2. */

/*       IFLGG = Gradient option: */
/*               IFLGG .LE. 0 if INTRC1 is to provide grad- */
/*                            ient estimates as needed (from */
/*                            GRADL). */
/*               IFLGG .GE. 1 if gradients are user-provided */
/*                            in GRAD.  This is more effici- */
/*                            ent if INTRC1 is to be called */
/*                            several times. */

/*       GRAD = 3 by N array whose I-th column contains */
/*              an estimated gradient at node I if IFLGG .GE. */
/*              1, or unused dummy parameter if IFLGG .LE. 0. */
/*              Refer to Subroutines GRADL and GRADG. */

/*       IST = Index of the starting node in the search for a */
/*             triangle containing P.  The output value of */
/*             IST from a previous call may be a good choice. */
/*             1 .LE. IST .LE. N. */

/* Input parameters other than IST are not altered by this */
/*   routine. */

/* On output: */

/*       IST = Index of one of the vertices of the triangle */
/*             containing P (or a boundary node if P is not */
/*             contained in a triangle) unless IER = -1 or */
/*             IER = -2. */

/*       FP = Value of F at P unless IER < 0, in which case */
/*            FP is not defined. */

/*       IER = Error indicator and information flag: */
/*             IER = 0 if no errors were encountered and P is */
/*                     contained in a triangle. */
/*             IER = 1 if no errors were encountered and */
/*                     extrapolation was required. */
/*             IER = -1 if N or IST is outside its valid */
/*                      range. */
/*             IER = -2 if the nodes are collinear. */
/*             IER = -3 if the angular distance between P and */
/*                      the nearest point of the triangula- */
/*                      tion is at least 90 degrees. */

/* STRIPACK modules required by INTRC1: JRAND, LSTPTR, STORE, */
/*                                        TRFIND */
/*                    (and optionally)  GETNP if IFLGG .LE. 0 */

/* SSRFPACK modules required by INTRC1:  ARCINT, ARCLEN, */
/*                                         FVAL, HVAL, SNHCSH */
/*              (and if IFLGG .LE. 0)  APLYR, APLYRT, CONSTR, */
/*                                       GIVENS, GRADL, */
/*                                       ROTATE, SETUP */

/* Intrinsic functions called by INTRC1:  COS, SIN, SQRT */

/* *********************************************************** */


/* Local parameters: */

/* A =        Angular separation between P and Q */
/* B1,B2,B3 = Barycentric coordinates of the central projec- */
/*              tion of P onto the underlying planar triangle, */
/*              or (B1 and B2) projection of Q onto the */
/*              underlying line segment N1-N2 when P is */
/*              exterior.  Unnormalized coordinates are */
/*              computed by TRFIND when P is in a triangle. */
/* DUM =      Dummy parameter for ARCINT */
/* FQ,GQ =    Interpolated value and gradient at Q */
/* GQN =      Negative of the component of GQ in the direction */
/*              Q->P */
/* G1,G2,G3 = Gradients at I1, I2, and I3, or (G1 and G2) at */
/*              N1 and N2 */
/* I =        DO-loop index */
/* IERR =     Error flag for calls to GRADL */
/* I1,I2,I3 = Vertex indexes returned by TRFIND */
/* LP =       LIST pointer */
/* N1,N2 =    Indexes of the endpoints of a boundary arc when */
/*              P is exterior (not contained in a triangle) */
/* NN =       Local copy of N */
/* P =        Cartesian coordinates of P */
/* P1,P2,P3 = Cartesian coordinates of the vertices I1, I2, */
/*              and I3, or (P1 and P2) coordinates of N1 and */
/*              N2 if P is exterior */
/* PTGQ =     Scalar product (P,GQ) -- factor of the component */
/*              of GQ in the direction Q->P */
/* PTN1 =     Scalar product (P,N1) -- factor of B1 and B2 */
/* PTN2 =     Scalar product (P,N2) -- factor of B1 and B2 */
/* Q =        Closest boundary point to P when P is exterior */
/* QNORM =    Factor used to normalize Q */
/* S1,S2,S3 = Tension factors associated with the triangle */
/*              sides opposite I1, I2, and I3, or (S1) the */
/*              boundary arc N1-N2 */
/* S12 =      Scalar product (N1,N2) -- factor of B1 and B2 */
/* SUM =      Quantity used to normalize the barycentric */
/*              coordinates */

    /* Parameter adjustments */
    grad -= 4;
    --lend;
    --f;
    --z__;
    --y;
    --x;
    --list;
    --lptr;
    --sigma;

    /* Function Body */
    nn = *n;
    if (nn < 3 || (*iflgg <= 0 && nn < 7) || *ist < 1 || *ist > nn) {
	goto L11;
    }

/* Transform (PLAT,PLON) to Cartesian coordinates. */

    p[0] = cos(*plat) * cos(*plon);
    p[1] = cos(*plat) * sin(*plon);
    p[2] = sin(*plat);

/* Locate P with respect to the triangulation. */

    trfind_(ist, p, &nn, &x[1], &y[1], &z__[1], &list[1], &lptr[1], &lend[1], 
	    &b1, &b2, &b3, &i1, &i2, &i3);
    if (i1 == 0) {
	goto L12;
    }
    *ist = i1;
    if (i3 != 0) {

/* P is contained in the triangle (I1,I2,I3).  Store the */
/*   vertex coordinates, gradients, and tension factors in */
/*   local variables. */

	p1[0] = x[i1];
	p1[1] = y[i1];
	p1[2] = z__[i1];
	p2[0] = x[i2];
	p2[1] = y[i2];
	p2[2] = z__[i2];
	p3[0] = x[i3];
	p3[1] = y[i3];
	p3[2] = z__[i3];
	if (*iflgg > 0) {

/*   Gradients are user-provided. */

	    for (i__ = 1; i__ <= 3; ++i__) {
		g1[i__ - 1] = grad[i__ + i1 * 3];
		g2[i__ - 1] = grad[i__ + i2 * 3];
		g3[i__ - 1] = grad[i__ + i3 * 3];
/* L1: */
	    }
	} else {

/*   Compute gradient estimates at the vertices. */

	    gradl_(&nn, &i1, &x[1], &y[1], &z__[1], &f[1], &list[1], &lptr[1],
		     &lend[1], g1, &ierr);
	    if (ierr < 0) {
		goto L12;
	    }
	    gradl_(&nn, &i2, &x[1], &y[1], &z__[1], &f[1], &list[1], &lptr[1],
		     &lend[1], g2, &ierr);
	    if (ierr < 0) {
		goto L12;
	    }
	    gradl_(&nn, &i3, &x[1], &y[1], &z__[1], &f[1], &list[1], &lptr[1],
		     &lend[1], g3, &ierr);
	    if (ierr < 0) {
		goto L12;
	    }
	}

	if (*iflgs > 0) {

/*   Variable tension: */

	    lp = lstptr_(&lend[i2], &i3, &list[1], &lptr[1]);
	    s1 = sigma[lp];
	    lp = lstptr_(&lend[i3], &i1, &list[1], &lptr[1]);
	    s2 = sigma[lp];
	    lp = lstptr_(&lend[i1], &i2, &list[1], &lptr[1]);
	    s3 = sigma[lp];
	} else {

/*   Uniform tension: */

	    s1 = sigma[1];
	    s2 = s1;
	    s3 = s1;
	}

/* Normalize the coordinates. */

	sum = b1 + b2 + b3;
	b1 /= sum;
	b2 /= sum;
	b3 /= sum;
	*fp = fval_(&b1, &b2, &b3, p1, p2, p3, &f[i1], &f[i2], &f[i3], g1, g2,
		 g3, &s1, &s2, &s3);
	*ier = 0;
	return 0;
    }

/* P is exterior to the triangulation, and I1 and I2 are */
/*   boundary nodes which are visible from P.  Extrapolate to */
/*   P by linear (with respect to arc-length) interpolation */
/*   of the value and directional derivative (gradient comp- */
/*   onent in the direction Q->P) of the interpolatory */
/*   surface at Q where Q is the closest boundary point to P. */

/* Determine Q by traversing the boundary starting from I1. */

    n1 = i1;
    ptn1 = p[0] * x[n1] + p[1] * y[n1] + p[2] * z__[n1];
    if (i1 != i2) {
	goto L3;
    }

/* All boundary nodes are visible from P.  Find a boundary */
/*   arc N1->N2 such that P Left (N2 X N1)->N1. */

/* Counterclockwise boundary traversal: */
/*   Set N2 to the first neighbor of N1. */

L2:
    lp = lend[n1];
    lp = lptr[lp];
    n2 = list[lp];

/* Compute inner products (N1,N2) and (P,N2), and compute */
/*   B2 = Det(P,N1,N2 X N1). */

    s12 = x[n1] * x[n2] + y[n1] * y[n2] + z__[n1] * z__[n2];
    ptn2 = p[0] * x[n2] + p[1] * y[n2] + p[2] * z__[n2];
    b2 = ptn2 - s12 * ptn1;
    if (b2 <= 0.) {
	goto L3;
    }

/* P Right (N2 X N1)->N1:  iterate. */

    n1 = n2;
    i1 = n1;
    ptn1 = ptn2;
    goto L2;

/* P Left (N2 X N1)->N1 where N2 is the first neighbor of N1. */
/*   Clockwise boundary traversal: */

L3:
    n2 = n1;
    ptn2 = ptn1;

/* Set N1 to the last neighbor of N2 and test for */
/*   termination. */

    lp = lend[n2];
    n1 = -list[lp];
    if (n1 == i1) {
	goto L13;
    }

/* Compute inner products (N1,N2) and (P,N1). */

    s12 = x[n1] * x[n2] + y[n1] * y[n2] + z__[n1] * z__[n2];
    ptn1 = p[0] * x[n1] + p[1] * y[n1] + p[2] * z__[n1];

/* Compute B2 = Det(P,N1,N2 X N1) = Det(Q,N1,N2 X N1)*(P,Q). */

    b2 = ptn2 - s12 * ptn1;
    if (b2 <= 0.) {
	goto L3;
    }

/* Compute B1 = Det(P,N2 X N1,N2) = Det(Q,N2 X N1,N2)*(P,Q). */

    b1 = ptn1 - s12 * ptn2;
    if (b1 <= 0.) {

/* Q = N2.  Store value, coordinates, and gradient at Q. */

	fq = f[n2];
	q[0] = x[n2];
	q[1] = y[n2];
	q[2] = z__[n2];
	if (*iflgg > 0) {
	    for (i__ = 1; i__ <= 3; ++i__) {
		gq[i__ - 1] = grad[i__ + n2 * 3];
/* L4: */
	    }
	} else {
	    gradl_(&nn, &n2, &x[1], &y[1], &z__[1], &f[1], &list[1], &lptr[1],
		     &lend[1], gq, &ierr);
	    if (ierr < 0) {
		goto L12;
	    }
	}

/* Extrapolate to P:  FP = FQ + A*(GQ,Q X (PXQ)/SIN(A)), */
/*   where A is the angular separation between Q and P, */
/*   and Sin(A) is the magnitude of P X Q. */

	a = arclen_(q, p);
	ptgq = p[0] * gq[0] + p[1] * gq[1] + p[2] * gq[2];
	*fp = fq;
	if (a != 0.) {
	    *fp += ptgq * a / sin(a);
	}
	*ier = 1;
	return 0;
    }

/* P Strictly Left (N2 X N1)->N2 and P Strictly Left */
/*   N1->(N2 X N1).  Thus Q lies on the interior of N1->N2. */
/*   Store coordinates of N1 and N2 in local variables. */

    p1[0] = x[n1];
    p1[1] = y[n1];
    p1[2] = z__[n1];
    p2[0] = x[n2];
    p2[1] = y[n2];
    p2[2] = z__[n2];

/* Compute the central projection of Q onto P2-P1 and */
/*   normalize to obtain Q. */

    qnorm = 0.;
    for (i__ = 1; i__ <= 3; ++i__) {
	q[i__ - 1] = b1 * p1[i__ - 1] + b2 * p2[i__ - 1];
	qnorm += q[i__ - 1] * q[i__ - 1];
/* L5: */
    }
    qnorm = sqrt(qnorm);
    for (i__ = 1; i__ <= 3; ++i__) {
	q[i__ - 1] /= qnorm;
/* L6: */
    }

/* Store gradients at N1 and N2 and tension factor S1. */

    if (*iflgg > 0) {
	for (i__ = 1; i__ <= 3; ++i__) {
	    g1[i__ - 1] = grad[i__ + n1 * 3];
	    g2[i__ - 1] = grad[i__ + n2 * 3];
/* L7: */
	}
    } else {
	gradl_(&nn, &n1, &x[1], &y[1], &z__[1], &f[1], &list[1], &lptr[1], &
		lend[1], g1, &ierr);
	if (ierr < 0) {
	    goto L12;
	}
	gradl_(&nn, &n2, &x[1], &y[1], &z__[1], &f[1], &list[1], &lptr[1], &
		lend[1], g2, &ierr);
	if (ierr < 0) {
	    goto L12;
	}
    }

    if (*iflgs <= 0) {
	s1 = sigma[1];
    }
    if (*iflgs >= 1) {
	s1 = sigma[lp];
    }

/* Compute an interpolated value and normal gradient */
/*   component at Q. */

    arcint_(q, p1, p2, &f[n1], &f[n2], g1, g2, &s1, &fq, dum, &gqn);

/* Extrapolate to P:  the normal gradient component GQN is */
/*   the negative of the component in the direction Q->P. */

    *fp = fq - gqn * arclen_(q, p);
    *ier = 1;
    return 0;

/* N or IST is outside its valid range. */

L11:
    *ier = -1;
    return 0;

/* Collinear nodes encountered. */

L12:
    *ier = -2;
    return 0;

/* The distance between P and the closest boundary point */
/*   is at least 90 degrees. */

L13:
    *ier = -3;
    return 0;
} /* intrc1_ */

doublereal sig0_(integer *n1, integer *n2, integer *n, doublereal *x, 
	doublereal *y, doublereal *z__, doublereal *h__, integer *list, 
	integer *lptr, integer *lend, doublereal *grad, integer *iflgb, 
	doublereal *hbnd, doublereal *tol, integer *iflgs, doublereal *sigma, 
	integer *ier)
{
    /* Initialized data */

    static doublereal sbig = 85.;

    /* System generated locals */
    integer i__1;
    doublereal ret_val, d__1, d__2, d__3, d__4, d__5, d__6;

    /* Builtin functions */
    double sqrt(doublereal), d_sign(doublereal *, doublereal *), exp(
	    doublereal), log(doublereal);

    /* Local variables */
    static doublereal a, b, c__, d__, e, f, r__, s, t, a0, b0, c1, c2, d0, d2,
	     f0, h1, h2, p1[3], p2[3], s1, s2, t0, t1, t2, aa, al, rf, tm, un[
	    3];
    static integer lp1, lp2;
    static doublereal bnd, scm, sig, ems;
    static integer lpl, nit;
    static doublereal ssm, d1pd2, fneg, dsig, dmax__, fmax, ftol, rsig, 
	    rtol, stol, coshm, sinhm, ssinh;
    static doublereal unorm;
    static doublereal coshmm;
#ifdef SPH_DEBUG
    static sneg;
#endif

/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   11/21/96 */

/*   Given a triangulation of a set of nodes on the unit */
/* sphere, along with data values H and gradients GRAD at the */
/* nodes, this function determines the smallest tension fac- */
/* tor SIG0 such that the Hermite interpolatory tension */
/* spline H(A), defined by SIG0 and the endpoint values and */
/* directional derivatives associated with an arc N1-N2, is */
/* bounded (either above or below) by HBND for all A in */
/* (A1,A2), where (A1,A2) denotes an interval corresponding */
/* to the arc and A is the arc-length. */

/* On input: */

/*       N1,N2 = Nodal indexes of the endpoints of an arc for */
/*               which the tension factor is to be computed. */
/*               The indexes must be distinct and lie in the */
/*               range 1 to N, and if IFLGS .GE. 1, they must */
/*               correspond to adjacent nodes in the triangu- */
/*               lation. */

/*       N = Number of nodes in the triangulation.  N .GE. 3. */

/*       X,Y,Z = Arrays of length N containing coordinates of */
/*               the nodes.  X(I)**2 + Y(I)**2 + Z(I)**2 = 1. */

/*       H = Array of length N containing data values at the */
/*           nodes.  H(I) is associated with (X(I),Y(I),Z(I)) */
/*           for I = 1 to N. */

/*       LIST,LPTR,LEND = Data structure defining the trian- */
/*                        gulation.  Refer to STRIPACK */
/*                        Subroutine TRMESH. */

/*       GRAD = Array dimensioned 3 by N whose columns con- */
/*              tain gradients at the nodes.  GRAD( ,J) must */
/*              be orthogonal to node J:  GRAD(1,J)*X(J) + */
/*              GRAD(2,J)*Y(J) + GRAD(3,J)*Z(J) = 0.  Refer */
/*              to Subroutines GRADG, GRADL, and SMSURF. */

/*       IFLGB = Bound option indicator: */
/*               IFLGB = -1 if HBND is a lower bound on H. */
/*               IFLGB = 1 if HBND is an upper bound on H. */

/*       HBND = Bound on H.  HBND .LE. min(H1,H2) if IFLGB = */
/*              -1 and HBND .GE. max(H1,H2) if IFLGB = 1, */
/*              where H1 and H2 are the data values at the */
/*              endpoints of the arc N1-N2. */

/*       TOL = Tolerance whose magnitude determines how close */
/*             SIG0 is to its optimal value when nonzero */
/*             finite tension is necessary and sufficient to */
/*             satisfy the constraint.  For a lower bound, */
/*             SIG0 is chosen so that HBND .LE. HMIN .LE. */
/*             HBND + abs(TOL), where HMIN is the minimum */
/*             value of H on the arc, and for an upper bound, */
/*             the maximum of H satisfies HBND - abs(TOL) */
/*             .LE. HMAX .LE. HBND.  Thus, the constraint is */
/*             satisfied but possibly with more tension than */
/*             necessary. */

/*       IFLGS = Tension array option indicator: */
/*               IFLGS .LE. 0 if SIGMA is not to be used. */
/*               IFLGS .GE. 1 if SIGMA is to be updated by */
/*                            storing SIG0 in the appropriate */
/*                            locations. */

/* The above parameters are not altered by this function. */

/*       SIGMA = Dummy parameter (IFLGS .LE. 0) or array con- */
/*               taining tension factors associated with arcs */
/*               in one-to-one correspondence with LIST */
/*               entries (IFLGS .GE. 1).  Refer to Subroutine */
/*               GETSIG. */

/* On output: */

/*       SIGMA = Tension factor array updated with the new */
/*               value if and only if IFLGS .GE. 1 and IER */
/*               .GE. 0. */

/*       IER = Error indicator: */
/*             IER = 0 if no errors were encountered and the */
/*                     constraint can be satisfied with fin- */
/*                     ite tension. */
/*             IER = 1 if no errors were encountered but in- */
/*                     finite tension is required to satisfy */
/*                     the constraint (e.g., IFLGB = -1, HBND */
/*                     = H(A1), and the directional deriva- */
/*                     tive of H at A1 is negative). */
/*             IER = -1 if N1, N2, N, or IFLGB is outside its */
/*                      valid range. */
/*             IER = -2 if nodes N1 and N2 coincide or IFLGS */
/*                      .GE. 1 and the nodes are not adja- */
/*                      cent. */
/*             IER = -3 if HBND is outside its valid range. */

/*       SIG0 = Minimum tension factor defined above unless */
/*              IER < 0, in which case SIG0 = -1.  If IER */
/*              = 1, SIG0 is set to 85, resulting in an */
/*              approximation to the linear interpolant of */
/*              the endpoint values. */

/* STRIPACK module required by SIG0:  STORE */

/* SSRFPACK modules required by SIG0:  ARCLEN, SNHCSH */

/* Intrinsic functions called by SIG0:  ABS, EXP, LOG, MAX, */
/*                                        MIN, REAL, SIGN, */
/*                                        SQRT */

/* *********************************************************** */


    /* Parameter adjustments */
    grad -= 4;
    --lend;
    --h__;
    --z__;
    --y;
    --x;
    --list;
    --lptr;
    --sigma;

    /* Function Body */
    rf = (doublereal) (*iflgb);
    bnd = *hbnd;

/* Print a heading. */

/*      IF (LUN .GE. 0  .AND.  RF .LT. 0.) WRITE (LUN,100) N1, */
/*     .                                   N2, BND */
/*      IF (LUN .GE. 0  .AND.  RF .GT. 0.) WRITE (LUN,110) N1, */
/*     .                                   N2, BND */
/*  100 FORMAT (//1X,'SIG0 -- N1 =',I4,', N2 =',I4, */
/*     .        ', LOWER BOUND = ',E15.8) */
/*  110 FORMAT (//1X,'SIG0 -- N1 =',I4,', N2 =',I4, */
/*     .        ', UPPER BOUND = ',E15.8) */
#ifdef SPH_DEBUG
    if (rf < 0.0) fprintf (stderr, "SIG0 -- N1 = %d  N2 = %d  LOWER BOUND = %g\n", *n1, *n2, bnd);
    if (rf > 0.0) fprintf (stderr, "SIG0 -- N1 = %d  N2 = %d  UPPER BOUND = %g\n", *n1, *n2, bnd);
#endif

/* Test for errors and store local parameters. */

    *ier = -1;
/* Computing MAX */
    i__1 = max(*n1,*n2);
    if (min(*n1,*n2) < 1 || *n1 == *n2 || max(i__1,3) > *n || fabs(rf) != 1.) {
	goto L11;
    }
    *ier = -2;
    if (*iflgs > 0) {

/*   Set LP1 and LP2 to the pointers to N2 as a neighbor of */
/*     N1 and N1 as a neighbor of N2, respectively. */

	lpl = lend[*n1];
	lp1 = lptr[lpl];
L1:
	if (list[lp1] == *n2) {
	    goto L2;
	}
	lp1 = lptr[lp1];
	if (lp1 != lpl) {
	    goto L1;
	}
	if ((i__1 = list[lp1], GMT_abs(i__1)) != *n2) {
	    goto L11;
	}

L2:
	lpl = lend[*n2];
	lp2 = lptr[lpl];
L3:
	if (list[lp2] == *n1) {
	    goto L4;
	}
	lp2 = lptr[lp2];
	if (lp2 != lpl) {
	    goto L3;
	}
	if ((i__1 = list[lp2], GMT_abs(i__1)) != *n1) {
	    goto L11;
	}
    }

/* Store nodal coordinates P1 and P2, compute arc-length AL */
/*   and unit normal UN = (P1 X P2)/UNORM, and test for */
/*   coincident nodes. */

L4:
    p1[0] = x[*n1];
    p1[1] = y[*n1];
    p1[2] = z__[*n1];
    p2[0] = x[*n2];
    p2[1] = y[*n2];
    p2[2] = z__[*n2];
    al = arclen_(p1, p2);
    un[0] = p1[1] * p2[2] - p1[2] * p2[1];
    un[1] = p1[2] * p2[0] - p1[0] * p2[2];
    un[2] = p1[0] * p2[1] - p1[1] * p2[0];
    unorm = sqrt(un[0] * un[0] + un[1] * un[1] + un[2] * un[2]);
    if (unorm == 0. || al == 0.) {
	goto L11;
    }

/* Store endpoint data values and test for valid constraint. */

    h1 = h__[*n1];
    h2 = h__[*n2];
    *ier = -3;
    if ((rf < 0. && min(h1,h2) < bnd) || (rf > 0. && bnd < max(h1,h2))) {
	goto L11;
    }

/* Compute scaled directional derivatives S1,S2 at the end- */
/*   points (for the direction N1->N2) and test for infinite */
/*   tension required. */

    s1 = al * (grad[*n1 * 3 + 1] * p2[0] + grad[*n1 * 3 + 2] * p2[1] + grad[*
	    n1 * 3 + 3] * p2[2]) / unorm;
    s2 = -al * (grad[*n2 * 3 + 1] * p1[0] + grad[*n2 * 3 + 2] * p1[1] + grad[*
	    n2 * 3 + 3] * p1[2]) / unorm;
    *ier = 1;
    sig = sbig;
    if ((h1 == bnd && rf * s1 > 0.) || (h2 == bnd && rf * s2 < 0.)) {
	goto L10;
    }

/* Test for SIG = 0 sufficient. */

    *ier = 0;
    sig = 0.;
    if (rf * s1 <= 0. && rf * s2 >= 0.) {
	goto L10;
    }

/*   Compute first difference S and coefficients A0 and B0 */
/*     of the Hermite cubic interpolant H0(A) = H2 - (S2*R + */
/*     B0*R**2 + (A0/3)*R**3), where R(A) = (A2-A)/AL. */

    s = h2 - h1;
    t0 = s * 3. - s1 - s2;
    a0 = (s - t0) * 3.;
    b0 = t0 - s2;
    d0 = t0 * t0 - s1 * s2;

/*   H0 has local extrema in (A1,A2) iff S1*S2 < 0 or */
/*     (T0*(S1+S2) < 0 and D0 .GE. 0). */

    if (s1 * s2 >= 0. && (t0 * (s1 + s2) >= 0. || d0 < 0.)) {
	goto L10;
    }
    if (a0 == 0.) {

/*   H0 is quadratic and has an extremum at R = -S2/(2*B0). */
/*     H0(R) = H2 + S2**2/(4*B0).  Note that A0 = 0 implies */
/*     2*B0 = S1-S2, and S1*S2 < 0 implies B0 .NE. 0. */
/*     Also, the extremum is a min iff HBND is a lower bound. */

	f0 = (bnd - h2 - s2 * s2 / (b0 * 4.)) * rf;
    } else {

/*   A0 .NE. 0 and H0 has extrema at R = (-B0 +/- SQRT(D0))/ */
/*     A0 = S2/(-B0 -/+ SQRT(D0)), where the negative root */
/*     corresponds to a min.  The expression for R is chosen */
/*     to avoid cancellation error.  H0(R) = H2 + (S2*B0 + */
/*     2*D0*R)/(3*A0). */

	d__1 = sqrt(d0);
	t = -b0 - d_sign(&d__1, &b0);
	r__ = t / a0;
	if (rf * b0 > 0.) {
	    r__ = s2 / t;
	}
	f0 = (bnd - h2 - (s2 * b0 + d0 * 2. * r__) / (a0 * 3.)) * rf;
    }

/*   F0 .GE. 0 iff SIG = 0 is sufficient to satisfy the */
/*     constraint. */

    if (f0 >= 0.) {
	goto L10;
    }

/* Find a zero of F(SIG) = (BND-H(R))*RF where the derivative */
/*   of H, HP, vanishes at R.  F is a nondecreasing function, */
/*   F(0) < 0, and F = FMAX for SIG sufficiently large. */

/* Initialize parameters for the secant method.  The method */
/*   uses three points:  (SG0,F0), (SIG,F), and (SNEG,FNEG), */
/*   where SG0 and SNEG are defined implicitly by DSIG = SIG */
/*   - SG0 and DMAX = SIG - SNEG.  SG0 is initially zero and */
/*   SNEG is initialized to a sufficiently large value that */
/*   FNEG > 0.  This value is used only if the initial value */
/*   of F is negative. */

/* Computing MAX */
/* Computing MIN */
    d__5 = (d__1 = h1 - bnd, fabs(d__1)), d__6 = (d__2 = h2 - bnd, fabs(d__2));
    d__3 = .001, d__4 = min(d__5,d__6);
    fmax = max(d__3,d__4);
/* Computing MAX */
    d__3 = (d__1 = h1 - bnd, fabs(d__1)), d__4 = (d__2 = h2 - bnd, fabs(d__2));
    t = max(d__3,d__4);
/* Computing MAX */
    d__1 = fabs(s1), d__2 = fabs(s2);
    sig = max(d__1,d__2) / t;
    dmax__ = sig * (1. - t / fmax);
/*      IF (LUN .GE. 0) WRITE (LUN,120) SIG, SNEG, F0, FMAX */
/*  120 FORMAT (1X,8X,'SIG = ',E15.8,', SNEG = ',E15.8/ */
/*     .        1X,9X,'F0 = ',E15.8,', FMAX = ',E15.8/) */
#ifdef SPH_DEBUG
    sneg = sig - dmax__;
    fprintf (stderr, "SIG = %g  SNEG = %g F0 = %g FMAX = %g\n", sig, sneg, f0, fmax);
#endif
    dsig = sig;
    fneg = fmax;
    d2 = s2 - s;
    d1pd2 = s2 - s1;
    nit = 0;

/* Compute an absolute tolerance FTOL = abs(TOL) and a */
/*   relative tolerance RTOL = 100*Macheps. */

    ftol = fabs(*tol);
    rtol = 1.;
L5:
    rtol /= 2.;
    d__1 = rtol + 1.;
    if (store_(&d__1) > 1.) {
	goto L5;
    }
    rtol *= 200.;

/* Top of loop:  compute F. */

L6:
    ems = exp(-sig);
    if (sig <= .5) {

/*   Use approximations designed to avoid cancellation error */
/*     (associated with small SIG) in the modified hyperbolic */
/*     functions. */

	snhcsh_(&sig, &sinhm, &coshm, &coshmm);
	c1 = sig * coshm * d2 - sinhm * d1pd2;
	c2 = sig * (sinhm + sig) * d2 - coshm * d1pd2;
	a = c2 - c1;
	aa = a / ems;
	e = sig * sinhm - coshmm - coshmm;
    } else {

/*   Scale SINHM and COSHM by 2*exp(-SIG) in order to avoid */
/*     overflow. */

	tm = 1. - ems;
	ssinh = tm * (ems + 1.);
	ssm = ssinh - sig * 2. * ems;
	scm = tm * tm;
	c1 = sig * scm * d2 - ssm * d1pd2;
	c2 = sig * ssinh * d2 - scm * d1pd2;
	aa = (sig * tm * d2 + (tm - sig) * d1pd2) * 2.;
	a = ems * aa;
	e = sig * ssinh - scm - scm;
    }

/*   HP(R) = (S2 - (C1*sinh(SIG*R) - C2*coshm(SIG*R))/E)/DT */
/*     = 0 for ESR = (-B +/- sqrt(D))/A = C/(-B -/+ sqrt(D)) */
/*     where ESR = exp(SIG*R), A = C2-C1, D = B**2 - A*C, and */
/*     B and C are defined below. */

    b = e * s2 - c2;
    c__ = c2 + c1;
    d__ = b * b - a * c__;
    f = 0.;
    if (aa * c__ == 0. && b == 0.) {
	goto L7;
    }
    f = fmax;
    if (d__ < 0.) {
	goto L7;
    }
    t1 = sqrt(d__);
    t = -b - d_sign(&t1, &b);
    rsig = 0.;
    if (rf * b < 0. && aa != 0.) {
	if (t / aa > 0.) {
	    rsig = sig + log(t / aa);
	}
    }
    if ((rf * b > 0. || aa == 0.) && c__ / t > 0.) {
	rsig = log(c__ / t);
    }
    if ((rsig <= 0. || rsig >= sig) && b != 0.) {
	goto L7;
    }

/*   H(R) = H2 - (B*SIG*R + C1 + RF*sqrt(D))/(SIG*E). */

    f = (bnd - h2 + (b * rsig + c1 + rf * t1) / (sig * e)) * rf;

/*   Update the number of iterations NIT. */

L7:
    ++nit;
/*      IF (LUN .GE. 0) WRITE (LUN,130) NIT, SIG, F */
/*  130 FORMAT (1X,3X,I2,' -- SIG = ',E15.8,', F = ', */
/*     .        E15.8) */
#ifdef SPH_DEBUG
    fprintf (stderr, "%d -- SIG = %g  F = %g\n", nit, sig, f);
#endif
    if (f0 * f < 0.) {

/*   F0*F < 0.  Update (SNEG,FNEG) to (SG0,F0) so that F and */
/*     FNEG always have opposite signs.  If SIG is closer to */
/*     SNEG than SG0, then swap (SNEG,FNEG) with (SG0,F0). */

	t1 = dmax__;
	t2 = fneg;
	dmax__ = dsig;
	fneg = f0;
	if (fabs(dsig) > fabs(t1)) {

	    dsig = t1;
	    f0 = t2;
	}
    }

/*   Test for convergence. */

    stol = rtol * sig;
if (fabs(dmax__) <= stol || (f >= 0. && f <= ftol) || fabs(f) <= rtol) {
	goto L10;
    }

/*   Test for F0 = F = FMAX or F < 0 on the first iteration. */

    if (f0 != f && (nit > 1 || f > 0.)) {
	goto L9;
    }

/*   F*F0 > 0 and either the new estimate would be outside */
/*     of the bracketing interval of length abs(DMAX) or */
/*     F < 0 on the first iteration.  Reset (SG0,F0) to */
/*     (SNEG,FNEG). */

L8:
    dsig = dmax__;
    f0 = fneg;

/*   Compute the change in SIG by linear interpolation */
/*     between (SG0,F0) and (SIG,F). */

L9:
    dsig = -f * dsig / (f - f0);
/*      IF (LUN .GE. 0) WRITE (LUN,140) DSIG */
/*  140 FORMAT (1X,8X,'DSIG = ',E15.8) */
#ifdef SPH_DEBUG
    fprintf (stderr, "DSIG = %g\n", dsig);
#endif
    if (fabs(dsig) > fabs(dmax__) || dsig * dmax__ > 0.) {
	goto L8;
    }

/*   Restrict the step-size such that abs(DSIG) .GE. STOL/2. */
/*     Note that DSIG and DMAX have opposite signs. */

    if (fabs(dsig) < stol / 2.) {
	d__1 = stol / 2.;
	dsig = -d_sign(&d__1, &dmax__);
    }

/*   Bottom of loop:  Update SIG, DMAX, and F0. */

    sig += dsig;
    dmax__ += dsig;
    f0 = f;
    goto L6;

/* No errors encountered. */

L10:
    ret_val = sig;
    if (*iflgs <= 0) {
	return ret_val;
    }
    sigma[lp1] = sig;
    sigma[lp2] = sig;
    return ret_val;

/* Error termination. */

L11:
    ret_val = -1.;
    return ret_val;
} /* sig0_ */

doublereal sig1_(integer *n1, integer *n2, integer *n, doublereal *x, 
	doublereal *y, doublereal *z__, doublereal *h__, integer *list, 
	integer *lptr, integer *lend, doublereal *grad, integer *iflgb, 
	doublereal *hpbnd, doublereal *tol, integer *iflgs, doublereal *sigma,
	 integer *ier)
{
    /* Initialized data */

    static doublereal sbig = 85.;

    /* System generated locals */
    integer i__1;
    doublereal ret_val, d__1, d__2;

    /* Builtin functions */
    double sqrt(doublereal), exp(doublereal), d_sign(doublereal *, doublereal 
	    *);

    /* Local variables */
    static doublereal a, e, f, s, a0, b0, c0, c1, c2, d0, d1, d2, f0, p1[3], 
	    p2[3], s1, s2, t0, t1, t2, al, rf, tm, un[3];
    static integer lp1, lp2;
    static doublereal bnd, sig, ems;
    static integer lpl, nit;
    static doublereal ems2, d1pd2, fneg, dsig, dmax__, fmax, sinh__, ftol, 
	    rtol, stol, coshm, sinhm;
    static doublereal unorm;
    static doublereal coshmm;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   11/21/96 */

/*   Given a triangulation of a set of nodes on the unit */
/* sphere, along with data values H and gradients GRAD at the */
/* nodes, this function determines the smallest tension fac- */
/* tor SIG1 such that the first derivative HP(A) of the */
/* Hermite interpolatory tension spline H(A), defined by SIG1 */
/* and the endpoint values and directional derivatives asso- */
/* ciated with an arc N1-N2, is bounded (either above or */
/* below) by HPBND for all A in (A1,A2), where (A1,A2) de- */
/* notes an interval corresponding to the arc and A denotes */
/* arc-length. */

/* On input: */

/*       N1,N2 = Nodal indexes of the endpoints of an arc for */
/*               which the tension factor is to be computed. */
/*               The indexes must be distinct and lie in the */
/*               range 1 to N, and if IFLGS .GE. 1, they must */
/*               correspond to adjacent nodes in the triangu- */
/*               lation. */

/*       N = Number of nodes in the triangulation.  N .GE. 3. */

/*       X,Y,Z = Arrays of length N containing coordinates of */
/*               the nodes.  X(I)**2 + Y(I)**2 + Z(I)**2 = 1. */

/*       H = Array of length N containing data values at the */
/*           nodes.  H(I) is associated with (X(I),Y(I),Z(I)) */
/*           for I = 1 to N. */

/*       LIST,LPTR,LEND = Data structure defining the trian- */
/*                        gulation.  Refer to STRIPACK */
/*                        Subroutine TRMESH. */

/*       GRAD = Array dimensioned 3 by N whose columns con- */
/*              gradients at the nodes.  GRAD( ,J) must be */
/*              orthogonal to node J:  GRAD(1,J)*X(J) + */
/*              GRAD(2,J)*Y(J) + GRAD(3,J)*Z(J) = 0.  Refer */
/*              to Subroutines GRADG, GRADL, and SMSURF. */

/*       IFLGB = Bound option indicator: */
/*               IFLGB = -1 if HPBND is a lower bound on HP. */
/*               IFLGB = 1 if HPBND is an upper bound on HP. */

/*       HPBND = Bound on HP.  HPBND .LE. min(HP1,HP2,S) if */
/*               IFLGB = -1 and HPBND .GE. max(HP1,HP2,S) if */
/*               IFLGB = 1, where HP1 and HP2 are the direc- */
/*               tional derivatives at the endpoints of the */
/*               arc N1-N2, and S is the slope of the linear */
/*               interpolant of the endpoint data values. */

/*       TOL = Tolerance whose magnitude determines how close */
/*             SIG1 is to its optimal value when nonzero */
/*             finite tension is necessary and sufficient to */
/*             satisfy the constraint.  For a lower bound, */
/*             SIG1 is chosen so that HPBND .LE. HPMIN .LE. */
/*             HPBND + abs(TOL), where HPMIN is the minimum */
/*             value of HP on the arc.  For an upper bound, */
/*             the maximum of HP satisfies HPBND - abs(TOL) */
/*             .LE. HPMAX .LE. HPBND.  Thus, the constraint */
/*             is satisfied but possibly with more tension */
/*             than necessary. */

/*       IFLGS = Tension array option indicator: */
/*               IFLGS .LE. 0 if SIGMA is not to be used. */
/*               IFLGS .GE. 1 if SIGMA is to be updated by */
/*                            storing SIG1 in the appropriate */
/*                            locations. */

/* The above parameters are not altered by this function. */

/*       SIGMA = Dummy parameter (IFLGS .LE. 0) or array */
/*               containing tension factors associated with */
/*               arcs in one-to-one correspondence with LIST */
/*               entries (IFLGS .GE. 1).  Refer to Subroutine */
/*               GETSIG. */

/* On output: */

/*       SIGMA = Tension factor array updated with the new */
/*               value if and only if IFLGS .GE. 1 and IER */
/*               .GE. 0. */

/*       IER = Error indicator: */
/*             IER = 0 if no errors were encountered and the */
/*                     constraint can be satisfied with fin- */
/*                     ite tension. */
/*             IER = 1 if no errors were encountered but in- */
/*                     finite tension is required to satisfy */
/*                     the constraint (e.g., IFLGB = -1, */
/*                     HPBND = S, and HP1 > S). */
/*             IER = -1 if N1, N2, N, or IFLGB is outside its */
/*                      valid range. */
/*             IER = -2 if nodes N1 and N2 coincide or IFLGS */
/*                      .GE. 1 and the nodes are not adja- */
/*                      cent. */
/*             IER = -3 if HPBND is outside its valid range. */

/*       SIG1 = Minimum tension factor defined above unless */
/*              IER < 0, in which case SIG1 = -1.  If IER */
/*              = 1, SIG1 is set to 85, resulting in an */
/*              approximation to the linear interpolant of */
/*              the endpoint values. */

/* STRIPACK module required by SIG1:  STORE */

/* SSRFPACK modules required by SIG1:  ARCLEN, SNHCSH */

/* Intrinsic functions called by SIG1:   ABS, EXP, MAX, MIN, */
/*                                         REAL, SIGN, SQRT */

/* *********************************************************** */


    /* Parameter adjustments */
    grad -= 4;
    --lend;
    --h__;
    --z__;
    --y;
    --x;
    --list;
    --lptr;
    --sigma;

    /* Function Body */
    rf = (doublereal) (*iflgb);
    bnd = *hpbnd;

/* Print a heading. */

/*      IF (LUN .GE. 0  .AND.  RF .LT. 0.) WRITE (LUN,100) N1, */
/*     .                                   N2, BND */
/*      IF (LUN .GE. 0  .AND.  RF .GT. 0.) WRITE (LUN,110) N1, */
/*     .                                   N2, BND */
/*  100 FORMAT (//1X,'SIG1 -- N1 =',I4,', N2 =',I4, */
/*     .        ', LOWER BOUND = ',E15.8) */
/*  110 FORMAT (//1X,'SIG1 -- N1 =',I4,', N2 =',I4, */
/*     .        ', UPPER BOUND = ',E15.8) */
#ifdef SPH_DEBUG
    if (rf < 0.0) fprintf (stderr, "SIG1 -- N1 = %d  N2 = %d  LOWER BOUND = %g\n", *n1, *n2, bnd);
    if (rf > 0.0) fprintf (stderr, "SIG1 -- N1 = %d  N2 = %d  UPPER BOUND = %g\n", *n1, *n2, bnd);
#endif

/* Test for errors and store local parameters. */

    *ier = -1;
/* Computing MAX */
    i__1 = max(*n1,*n2);
    if (min(*n1,*n2) < 1 || *n1 == *n2 || max(i__1,3) > *n || fabs(rf) != 1.) {
	goto L11;
    }
    *ier = -2;
    if (*iflgs > 0) {

/*   Set LP1 and LP2 to the pointers to N2 as a neighbor of */
/*     N1 and N1 as a neighbor of N2, respectively. */

	lpl = lend[*n1];
	lp1 = lptr[lpl];
L1:
	if (list[lp1] == *n2) {
	    goto L2;
	}
	lp1 = lptr[lp1];
	if (lp1 != lpl) {
	    goto L1;
	}
	if ((i__1 = list[lp1], GMT_abs(i__1)) != *n2) {
	    goto L11;
	}

L2:
	lpl = lend[*n2];
	lp2 = lptr[lpl];
L3:
	if (list[lp2] == *n1) {
	    goto L4;
	}
	lp2 = lptr[lp2];
	if (lp2 != lpl) {
	    goto L3;
	}
	if ((i__1 = list[lp2], GMT_abs(i__1)) != *n1) {
	    goto L11;
	}
    }

/* Store nodal coordinates P1 and P2, compute arc-length AL */
/*   and unit normal UN = (P1 X P2)/UNORM, and test for */
/*   coincident nodes. */

L4:
    p1[0] = x[*n1];
    p1[1] = y[*n1];
    p1[2] = z__[*n1];
    p2[0] = x[*n2];
    p2[1] = y[*n2];
    p2[2] = z__[*n2];
    al = arclen_(p1, p2);
    un[0] = p1[1] * p2[2] - p1[2] * p2[1];
    un[1] = p1[2] * p2[0] - p1[0] * p2[2];
    un[2] = p1[0] * p2[1] - p1[1] * p2[0];
    unorm = sqrt(un[0] * un[0] + un[1] * un[1] + un[2] * un[2]);
    if (unorm == 0. || al == 0.) {
	goto L11;
    }

/* Compute first difference S and scaled directional deriva- */
/*   tives S1,S2 at the endpoints (for the direction N1->N2). */

    s = h__[*n2] - h__[*n1];
    s1 = al * (grad[*n1 * 3 + 1] * p2[0] + grad[*n1 * 3 + 2] * p2[1] + grad[*
	    n1 * 3 + 3] * p2[2]) / unorm;
    s2 = -al * (grad[*n2 * 3 + 1] * p1[0] + grad[*n2 * 3 + 2] * p1[1] + grad[*
	    n2 * 3 + 3] * p1[2]) / unorm;

/* Test for a valid constraint. */

    *ier = -3;
/* Computing MIN */
    d__1 = min(s1,s2);
/* Computing MAX */
    d__2 = max(s1,s2);
if ((rf < 0. && min(d__1,s) < bnd) || (rf > 0. && bnd < max(d__2,s))) {
	goto L11;
    }

/* Test for infinite tension required. */

    *ier = 1;
    sig = sbig;
    if (s == bnd && (s1 != s || s2 != s)) {
	goto L10;
    }

/* Test for SIG = 0 sufficient.  The Hermite cubic interpo- */
/*   lant H0 has derivative HP0(T) = (S2 + 2*B0*R + A0*R**2)/ */
/*   AL, where R = (T2-T)/AL. */

    *ier = 0;
    sig = 0.;
    t0 = s * 3. - s1 - s2;
    b0 = t0 - s2;
    c0 = t0 - s1;
    a0 = -b0 - c0;

/*   HP0(R) has an extremum (at R = -B0/A0) in (0,1) iff */
/*     B0*C0 > 0 and the third derivative of H0 has the */
/*     sign of A0. */

    if (b0 * c0 <= 0. || a0 * rf > 0.) {
	goto L10;
    }

/*   A0*RF < 0 and HP0(R) = -D0/(DT*A0) at R = -B0/A0. */

    d0 = t0 * t0 - s1 * s2;
    f0 = (bnd + d0 / (a0 * al)) * rf;
    if (f0 >= 0.) {
	goto L10;
    }

/* Find a zero of F(SIG) = (BND-HP(R))*RF, where HP has an */
/*   extremum at R.  F has a unique zero, F(0) = F0 < 0, and */
/*   F = (BND-S)*RF > 0 for SIG sufficiently large. */

/* Initialize parameters for the secant method.  The method */
/*   uses three points:  (SG0,F0), (SIG,F), and (SNEG,FNEG), */
/*   where SG0 and SNEG are defined implicitly by DSIG = SIG */
/*   - SG0 and DMAX = SIG - SNEG.  SG0 is initially zero and */
/*   SIG is initialized to the zero of (BND - (SIG*S-S1-S2)/ */
/*   (AL*(SIG-2.)))*RF -- a value for which F(SIG) .GE. 0 and */
/*   F(SIG) = 0 for SIG sufficiently large that 2*SIG is in- */
/*   significant relative to exp(SIG). */

    fmax = (bnd - s / al) * rf;
    sig = 2. - a0 / ((al * bnd - s) * 3.);
/*      IF (LUN .GE. 0) WRITE (LUN,120) F0, FMAX, SIG */
/*  120 FORMAT (1X,9X,'F0 = ',E15.8,', FMAX = ',E15.8/ */
/*     .        1X,8X,'SIG = ',E15.8/) */
#ifdef SPH_DEBUG
    fprintf (stderr, "F0 = %g FMAX = %g SIG = %g\n", f0, fmax, sig);
#endif
    d__1 = sig * exp(-sig) + .5;
    if (store_(&d__1) == .5) {
	goto L10;
    }
    dsig = sig;
    dmax__ = sig * -2.;
    fneg = fmax;
    d1 = s - s1;
    d2 = s2 - s;
    d1pd2 = d1 + d2;
    nit = 0;

/* Compute an absolute tolerance FTOL = abs(TOL), and a */
/*   relative tolerance RTOL = 100*Macheps. */

    ftol = fabs(*tol);
    rtol = 1.;
L5:
    rtol /= 2.;
    d__1 = rtol + 1.;
    if (store_(&d__1) > 1.) {
	goto L5;
    }
    rtol *= 200.;

/* Top of loop:  compute F. */

L6:
    if (sig <= .5) {

/*   Use approximations designed to avoid cancellation */
/*     error (associated with small SIG) in the modified */
/*     hyperbolic functions. */

	snhcsh_(&sig, &sinhm, &coshm, &coshmm);
	c1 = sig * coshm * d2 - sinhm * d1pd2;
	c2 = sig * (sinhm + sig) * d2 - coshm * d1pd2;
	a = c2 - c1;
	e = sig * sinhm - coshmm - coshmm;
    } else {

/*   Scale SINHM and COSHM by 2*exp(-SIG) in order to avoid */
/*     overflow. */

	ems = exp(-sig);
	ems2 = ems + ems;
	tm = 1. - ems;
	sinh__ = tm * (ems + 1.);
	sinhm = sinh__ - sig * ems2;
	coshm = tm * tm;
	c1 = sig * coshm * d2 - sinhm * d1pd2;
	c2 = sig * sinh__ * d2 - coshm * d1pd2;
	a = ems2 * (sig * tm * d2 + (tm - sig) * d1pd2);
	e = sig * sinh__ - coshm - coshm;
    }

/*   The second derivative HPP of H(R) has a zero at exp(SIG* */
/*     R) = SQRT((C2+C1)/A) and R is in (0,1) and well- */
/*     defined iff HPP(T1)*HPP(T2) < 0. */

    f = fmax;
    t1 = a * (c2 + c1);
    if (t1 >= 0.) {
	if (c1 * (sig * coshm * d1 - sinhm * d1pd2) < 0.) {

/*   HP(R) = (B+SIGN(A)*SQRT(A*C))/(AL*E) at the critical */
/*     value of R, where A = C2-C1, B = E*S2-C2, and C = C2 + */
/*     C1.  Note that RF*A < 0. */

	    f = (bnd - (e * s2 - c2 - rf * sqrt(t1)) / (al * e)) * rf;
	}
    }

/*   Update the number of iterations NIT. */

    ++nit;
/*      IF (LUN .GE. 0) WRITE (LUN,130) NIT, SIG, F */
/*  130 FORMAT (1X,3X,I2,' -- SIG = ',E15.8,', F = ', */
/*     .        E15.8) */
#ifdef SPH_DEBUG
    fprintf (stderr, "%d -- SIG = %g  F = %g\n", nit, sig, f);
#endif
    if (f0 * f < 0.) {

/*   F0*F < 0.  Update (SNEG,FNEG) to (SG0,F0) so that F */
/*     and FNEG always have opposite signs.  If SIG is closer */
/*     to SNEG than SG0 and abs(F) < abs(FNEG), then swap */
/*     (SNEG,FNEG) with (SG0,F0). */

	t1 = dmax__;
	t2 = fneg;
	dmax__ = dsig;
	fneg = f0;
	if (fabs(dsig) > fabs(t1) && fabs(f) < fabs(t2)) {

	    dsig = t1;
	    f0 = t2;
	}
    }

/*   Test for convergence. */

    stol = rtol * sig;
if (fabs(dmax__) <= stol || (f >= 0. && f <= ftol) || fabs(f) <= rtol) {
	goto L10;
    }
    if (f0 * f < 0. || fabs(f) < fabs(f0)) {
	goto L8;
    }

/*   F*F0 > 0 and the new estimate would be outside of the */
/*     bracketing interval of length abs(DMAX).  Reset */
/*     (SG0,F0) to (SNEG,FNEG). */

L7:
    dsig = dmax__;
    f0 = fneg;

/*   Compute the change in SIG by linear interpolation */
/*     between (SG0,F0) and (SIG,F). */

L8:
    dsig = -f * dsig / (f - f0);
/*      IF (LUN .GE. 0) WRITE (LUN,140) DSIG */
/*  140 FORMAT (1X,8X,'DSIG = ',E15.8) */
#ifdef SPH_DEBUG
    fprintf (stderr, "DSIG = %g\n", dsig);
#endif
    if (fabs(dsig) > fabs(dmax__) || dsig * dmax__ > 0.) {
	goto L7;
    }

/*   Restrict the step-size such that abs(DSIG) .GE. STOL/2. */
/*     Note that DSIG and DMAX have opposite signs. */

    if (fabs(dsig) < stol / 2.) {
	d__1 = stol / 2.;
	dsig = -d_sign(&d__1, &dmax__);
    }

/*   Bottom of loop:  update SIG, DMAX, and F0. */

    sig += dsig;
    dmax__ += dsig;
    f0 = f;
    goto L6;

/* No errors encountered. */

L10:
    ret_val = sig;
    if (*iflgs <= 0) {
	return ret_val;
    }
    sigma[lp1] = sig;
    sigma[lp2] = sig;
    return ret_val;

/* Error termination. */

L11:
    ret_val = -1.;
    return ret_val;
} /* sig1_ */

doublereal sig2_(integer *n1, integer *n2, integer *n, doublereal *x, 
	doublereal *y, doublereal *z__, doublereal *h__, integer *list, 
	integer *lptr, integer *lend, doublereal *grad, doublereal *tol, 
	integer *iflgs, doublereal *sigma, integer *ier)
{
    /* Initialized data */

    static doublereal sbig = 85.;

    /* System generated locals */
    integer i__1;
    doublereal ret_val, d__1, d__2;

    /* Builtin functions */
    double sqrt(doublereal), exp(doublereal);

    /* Local variables */
    static doublereal f, s, t, d1, d2, p1[3], p2[3], t1, al, fp, un[3];
    static integer lp1, lp2;
    static doublereal tp1, d1d2, sig, ems;
    static integer lpl, nit;
    static doublereal ssm, dsig, ftol, rtol, coshm, sinhm, dummy;
    static doublereal unorm;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/25/96 */

/*   Given a triangulation of a set of nodes on the unit */
/* sphere, along with data values H and gradients GRAD at the */
/* nodes, this function determines the smallest tension fac- */
/* tor SIG2 such that the Hermite interpolatory tension */
/* spline H(A), defined by SIG2 and the endpoint values and */
/* directional derivatives associated with an arc N1-N2, */
/* preserves convexity (or concavity) of the data: */

/*   HP1 .LE. S .LE. HP2 implies HPP(A) .GE. 0, and */
/*   HP1 .GE. S .GE. HP2 implies HPP(A) .LE. 0 */

/* for all A in the open interval (A1,A2) corresponding to */
/* the arc, where HP1 and HP2 are the derivative values of H */
/* at the endpoints, S is the slope of the linear interpolant */
/* of the endpoint data values, HPP denotes the second deriv- */
/* ative of H, and A is arc-length.  Note, however, that */
/* infinite tension is required if HP1 = S or HP2 = S (unless */
/* HP1 = HP2 = S). */

/* On input: */

/*       N1,N2 = Nodal indexes of the endpoints of an arc for */
/*               which the tension factor is to be computed. */
/*               The indexes must be distinct and lie in the */
/*               range 1 to N, and if IFLGS .GE. 1, they must */
/*               correspond to adjacent nodes in the triangu- */
/*               lation. */

/*       N = Number of nodes in the triangulation.  N .GE. 3. */

/*       X,Y,Z = Arrays of length N containing coordinates of */
/*               the nodes.  X(I)**2 + Y(I)**2 + Z(I)**2 = 1. */

/*       H = Array of length N containing data values at the */
/*           nodes.  H(I) is associated with (X(I),Y(I),Z(I)) */
/*           for I = 1 to N. */

/*       LIST,LPTR,LEND = Data structure defining the trian- */
/*                        gulation.  Refer to STRIPACK */
/*                        Subroutine TRMESH. */

/*       GRAD = Array dimensioned 3 by N whose columns con- */
/*              gradients at the nodes.  GRAD( ,J) must be */
/*              orthogonal to node J:  GRAD(1,J)*X(J) + */
/*              GRAD(2,J)*Y(J) + GRAD(3,J)*Z(J) = 0.  Refer */
/*              to Subroutines GRADG, GRADL, and SMSURF. */

/*       TOL = Tolerance whose magnitude determines how close */
/*             SIG2 is to its optimal value when nonzero */
/*             finite tension is necessary and sufficient to */
/*             satisfy convexity or concavity.  In the case */
/*             convexity, SIG2 is chosen so that 0 .LE. */
/*             HPPMIN .LE. abs(TOL), where HPPMIN is the */
/*             minimum value of HPP on the arc.  In the case */
/*             of concavity, the maximum value of HPP satis- */
/*             fies -abs(TOL) .LE. HPPMAX .LE. 0.  Thus, the */
/*             constraint is satisfied but possibly with more */
/*             tension than necessary. */

/*       IFLGS = Tension array option indicator: */
/*               IFLGS .LE. 0 if SIGMA is not to be used. */
/*               IFLGS .GE. 1 if SIGMA is to be updated by */
/*                            storing SIG2 in the appropriate */
/*                            locations. */

/* The above parameters are not altered by this function. */

/*       SIGMA = Dummy parameter (IFLGS .LE. 0) or array */
/*               containing tension factors associated with */
/*               arcs in one-to-one correspondence with LIST */
/*               entries (IFLGS .GE. 1).  Refer to Subroutine */
/*               GETSIG. */

/* On output: */

/*       SIGMA = Tension factor array updated with the new */
/*               value if and only if IFLGS .GE. 1 and IER */
/*               .GE. 0. */

/*       IER = Error indicator: */
/*             IER = 0 if no errors were encountered and fin- */
/*                     ite tension is sufficient to satisfy */
/*                     convexity (or concavity). */
/*             IER = 1 if no errors were encountered but in- */
/*                     finite tension is required to satisfy */
/*                     convexity. */
/*             IER = 2 if the data does not satisfy convexity */
/*                     or concavity. */
/*             IER = -1 if N1, N2, or N is outside its valid */
/*                      range. */
/*             IER = -2 if nodes N1 and N2 coincide or IFLGS */
/*                      .GE. 1 and the nodes are not adja- */
/*                      cent. */

/*       SIG2 = Minimum tension factor defined above unless */
/*              IER < 0, in which case SIG2 = -1.  If IER */
/*              = 1, SIG2 is set to 85, resulting in an */
/*              approximation to the linear interpolant of */
/*              the endpoint values.  If IER = 2, SIG2 = 0, */
/*              resulting in the Hermite cubic interpolant. */

/* STRIPACK module required by SIG2:  STORE */

/* SSRFPACK modules required by SIG2:  ARCLEN, SNHCSH */

/* Intrinsic functions called by SIG2:  ABS, EXP, MAX, MIN, */
/*                                        SQRT */

/* *********************************************************** */


    /* Parameter adjustments */
    grad -= 4;
    --lend;
    --h__;
    --z__;
    --y;
    --x;
    --list;
    --lptr;
    --sigma;

    /* Function Body */

/* Print a heading. */

/*      IF (LUN .GE. 0) WRITE (LUN,100) N1, N2 */
/*  100 FORMAT (//1X,'SIG2 -- N1 =',I4,', N2 =',I4) */
#ifdef SPH_DEBUG
    fprintf (stderr, "SIG2 -- N1 = %d  N2 = %d\n", *n1, *n2);
#endif

/* Test for errors and set local parameters. */

    *ier = -1;
/* Computing MAX */
    i__1 = max(*n1,*n2);
    if (min(*n1,*n2) < 1 || *n1 == *n2 || max(i__1,3) > *n) {
	goto L11;
    }
    *ier = -2;
    if (*iflgs > 0) {

/*   Set LP1 and LP2 to the pointers to N2 as a neighbor of */
/*     N1 and N1 as a neighbor of N2, respectively. */

	lpl = lend[*n1];
	lp1 = lptr[lpl];
L1:
	if (list[lp1] == *n2) {
	    goto L2;
	}
	lp1 = lptr[lp1];
	if (lp1 != lpl) {
	    goto L1;
	}
	if ((i__1 = list[lp1], GMT_abs(i__1)) != *n2) {
	    goto L11;
	}

L2:
	lpl = lend[*n2];
	lp2 = lptr[lpl];
L3:
	if (list[lp2] == *n1) {
	    goto L4;
	}
	lp2 = lptr[lp2];
	if (lp2 != lpl) {
	    goto L3;
	}
	if ((i__1 = list[lp2], GMT_abs(i__1)) != *n1) {
	    goto L11;
	}
    }

/* Store nodal coordinates P1 and P2, compute arc-length AL */
/*   and unit normal UN = (P1 X P2)/UNORM, and test for */
/*   coincident nodes. */

L4:
    p1[0] = x[*n1];
    p1[1] = y[*n1];
    p1[2] = z__[*n1];
    p2[0] = x[*n2];
    p2[1] = y[*n2];
    p2[2] = z__[*n2];
    al = arclen_(p1, p2);
    un[0] = p1[1] * p2[2] - p1[2] * p2[1];
    un[1] = p1[2] * p2[0] - p1[0] * p2[2];
    un[2] = p1[0] * p2[1] - p1[1] * p2[0];
    unorm = sqrt(un[0] * un[0] + un[1] * un[1] + un[2] * un[2]);
    if (unorm == 0. || al == 0.) {
	goto L11;
    }

/* Compute first and second differences and test for infinite */
/*   tension required. */

    s = h__[*n2] - h__[*n1];
    d1 = s - al * (grad[*n1 * 3 + 1] * p2[0] + grad[*n1 * 3 + 2] * p2[1] + 
	    grad[*n1 * 3 + 3] * p2[2]) / unorm;
    d2 = -al * (grad[*n2 * 3 + 1] * p1[0] + grad[*n2 * 3 + 2] * p1[1] + grad[*
	    n2 * 3 + 3] * p1[2]) / unorm - s;
    d1d2 = d1 * d2;
    *ier = 1;
    sig = sbig;
    if (d1d2 == 0. && d1 != d2) {
	goto L10;
    }

/* Test for a valid constraint. */

    *ier = 2;
    sig = 0.;
    if (d1d2 < 0.) {
	goto L10;
    }

/* Test for SIG = 0 sufficient. */

    *ier = 0;
    if (d1d2 == 0.) {
	goto L10;
    }
/* Computing MAX */
    d__1 = d1 / d2, d__2 = d2 / d1;
    t = max(d__1,d__2);
    if (t <= 2.) {
	goto L10;
    }

/* Find a zero of F(SIG) = SIG*COSHM(SIG)/SINHM(SIG) - (T+1). */
/*   Since the derivative of F vanishes at the origin, a */
/*   quadratic approximation is used to obtain an initial */
/*   estimate for the Newton method. */

    tp1 = t + 1.;
    sig = sqrt(t * 10. - 20.);
    nit = 0;

/*   Compute an absolute tolerance FTOL = abs(TOL) and a */
/*     relative tolerance RTOL = 100*Macheps. */

    ftol = fabs(*tol);
    rtol = 1.;
L5:
    rtol /= 2.;
    d__1 = rtol + 1.;
    if (store_(&d__1) > 1.) {
	goto L5;
    }
    rtol *= 200.;

/* Top of loop:  evaluate F and its derivative FP. */

L6:
    if (sig <= .5) {

/*   Use approximations designed to avoid cancellation error */
/*     in the hyperbolic functions. */

	snhcsh_(&sig, &sinhm, &coshm, &dummy);
	t1 = coshm / sinhm;
	fp = t1 + sig * (sig / sinhm - t1 * t1 + 1.);
    } else {

/*   Scale SINHM and COSHM by 2*exp(-SIG) in order to avoid */
/*     overflow. */

	ems = exp(-sig);
	ssm = 1. - ems * (ems + sig + sig);
	t1 = (1. - ems) * (1. - ems) / ssm;
	fp = t1 + sig * (sig * 2. * ems / ssm - t1 * t1 + 1.);
    }

    f = sig * t1 - tp1;

/*   Update the number of iterations NIT. */

    ++nit;
/*      IF (LUN .GE. 0) WRITE (LUN,110) NIT, SIG, F, FP */
/*  110 FORMAT (1X,3X,I2,' -- SIG = ',E15.8,', F = ', */
/*     .        E15.8/1X,31X,'FP = ',E15.8) */
#ifdef SPH_DEBUG
    fprintf (stderr, "%d -- SIG = %g  F = %g  FP = %g\n", nit, sig, f, fp);
#endif

/*   Test for convergence. */

    if (fp <= 0.) {
	goto L10;
    }
    dsig = -f / fp;
if (fabs(dsig) <= rtol * sig || (f >= 0. && f <= ftol) || fabs(f) <= rtol) {
	goto L10;
    }

/*   Bottom of loop:  update SIG. */

    sig += dsig;
    goto L6;

/* No errors encountered. */

L10:
    ret_val = sig;
    if (*iflgs <= 0) {
	return ret_val;
    }
    sigma[lp1] = sig;
    sigma[lp2] = sig;
    return ret_val;

/* Error termination. */

L11:
    ret_val = -1.;
    return ret_val;
} /* sig2_ */

/* Subroutine */ integer smsgs_(integer *n, doublereal *x, doublereal *y, 
	doublereal *z__, doublereal *u, integer *list, integer *lptr, integer 
	*lend, integer *iflgs, doublereal *sigma, doublereal *w, doublereal *
	p, integer *nit, doublereal *dfmax, doublereal *f, doublereal *grad, 
	integer *ier)
{
    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1, d__2;

    /* Builtin functions */
    double sqrt(doublereal), atan(doublereal);

    /* Local variables */
    static integer j, k;
    static doublereal t, g1, g2, g3, r1, r2, r3, t1, t2, t3, t4, t5, t6, c11, 
	    c12, c13, c22, c23, c33, df, fk, cx;
    static integer nn;
    static doublereal cy, pp, xj, xk, yj, yk, zj, zk, sx, sy, xs, ys, rr2, 
	    rr3, cc22, cc23, cc33, dgk[3];
    static integer ifl;
    static doublereal gjk, det, gkj, dgx, dgy, sig;
    static integer lpj, lpl;
    static doublereal tol, den1, den2, alfa, dfmx;
    static integer iter;
    static doublereal alfsq, sinal;
    static integer itmax;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/25/96 */

/*   This subroutine solves the symmetric positive definite */
/* linear system associated with minimizing the quadratic */
/* functional Q(F,FX,FY,FZ) described in Subroutine SMSURF. */
/* Since the gradient at node K lies in the plane tangent to */
/* the sphere surface at K, it is effectively defined by only */
/* two components -- its X and Y components in the coordinate */
/* system obtained by rotating K to the north pole.  Thus, */
/* the minimization problem corresponds to an order-3N system */
/* which is solved by the block Gauss-Seidel method with 3 by */
/* 3 blocks. */

/* On input: */

/*       N,X,Y,Z,U,LIST,LPTR,LEND,IFLGS,SIGMA,W = Parameters */
/*           as described in Subroutine SMSURF. */

/*       P = Positive smoothing parameter defining Q. */

/* The above parameters are not altered by this routine. */

/*       NIT = Maximum number of iterations to be used.  This */
/*             maximum will likely be achieved if DFMAX is */
/*             smaller than the machine precision.  NIT .GE. */
/*             0. */

/*       DFMAX = Nonnegative convergence criterion.  The */
/*               method is terminated when the maximum */
/*               change in a solution F-component between */
/*               iterations is at most DFMAX.  The change in */
/*               a component is taken to be the absolute */
/*               difference relative to 1 plus the old value. */

/*       F = Initial estimate of the first N solution compo- */
/*           nents. */

/*       GRAD = 3 by N array containing initial estimates of */
/*              the last 3N solution components (the gradi- */
/*              ent with FX, FY, and FZ in rows 1, 2, and 3, */
/*              respectively). */

/* On output: */

/*       NIT = Number of Gauss-Seidel iterations employed. */

/*       DFMAX = Maximum relative change in a solution F- */
/*               component at the last iteration. */

/*       F = First N solution components -- function values */
/*           at the nodes. */

/*       GRAD = Last 3N solution components -- gradients at */
/*              the nodes. */

/*       IER = Error indicator: */
/*             IER = 0 if no errors were encountered and the */
/*                     convergence criterion was achieved. */
/*             IER = 1 if no errors were encountered but con- */
/*                     vergence was not achieved within NIT */
/*                     iterations. */
/*             IER = -1 if N, P, NIT, or DFMAX is outside its */
/*                      valid range on input.  F and GRAD are */
/*                      not altered in this case. */
/*             IER = -2 if all nodes are collinear or the */
/*                      triangulation is invalid. */
/*             IER = -3 if duplicate nodes were encountered. */

/* SSRFPACK modules required by SMSGS:  APLYRT, CONSTR, */
/*                                        GRCOEF, SNHCSH */

/* Intrinsic functions called by SMSGS:  ABS, ATAN, MAX, SQRT */

/* *********************************************************** */


    /* Parameter adjustments */
    grad -= 4;
    --f;
    --w;
    --lend;
    --u;
    --z__;
    --y;
    --x;
    --list;
    --lptr;
    --sigma;

    /* Function Body */
    nn = *n;
    ifl = *iflgs;
    pp = *p;
    itmax = *nit;
    tol = *dfmax;

/* Test for errors in input and initialize iteration count, */
/*   tension factor, and output value of DFMAX. */

    if (nn < 3 || pp <= 0. || itmax < 0 || tol < 0.) {
	goto L5;
    }
    iter = 0;
    sig = sigma[1];
    dfmx = 0.;

/* Top of iteration loop. */

L1:
    if (iter == itmax) {
	goto L4;
    }
    dfmx = 0.;

/*   Loop on nodes. */

    i__1 = nn;
    for (k = 1; k <= i__1; ++k) {
	xk = x[k];
	yk = y[k];
	zk = z__[k];
	fk = f[k];
	g1 = grad[k * 3 + 1];
	g2 = grad[k * 3 + 2];
	g3 = grad[k * 3 + 3];

/*   Construct the rotation mapping node K to the north pole. */

	constr_(&xk, &yk, &zk, &cx, &sx, &cy, &sy);

/*   Initialize components of the order-3 system for the */
/*     change (DF,DGX,DGY) in the K-th solution components. */

	c11 = pp * w[k];
	c12 = 0.;
	c13 = 0.;
	c22 = 0.;
	c23 = 0.;
	c33 = 0.;
	r1 = c11 * (u[k] - fk);
	r2 = 0.;
	r3 = 0.;

/*   Loop on neighbors J of node K. */

	lpl = lend[k];
	lpj = lpl;
L2:
	lpj = lptr[lpj];
	j = (i__2 = list[lpj], GMT_abs(i__2));

/*   Compute the coordinates of J in the rotated system. */

	t = sx * y[j] + cx * z__[j];
	yj = cx * y[j] - sx * z__[j];
	zj = sy * x[j] + cy * t;
	xj = cy * x[j] - sy * t;

/*   Compute arc-length ALFA between K and J, ALFSQ = ALFA* */
/*     ALFA, SINAL = SIN(ALFA), DEN1 = ALFA*SIN(ALFA)**2, and */
/*     DEN2 = ALFSQ*SINAL. */

	alfa = atan(sqrt((1. - zj) / (zj + 1.))) * 2.;
	alfsq = alfa * alfa;
	xs = xj * xj;
	ys = yj * yj;
	sinal = sqrt(xs + ys);
	den1 = alfa * (xs + ys);
	den2 = alfsq * sinal;

/*   Test for coincident nodes and compute functions of SIG: */
/*     T1 = SIG*SIG*COSHM/E, T2 = SIG*SINHM/E, and T3 = SIG* */
/*     (SIG*COSHM-SINHM)/E for E = SIG*SINH - 2*COSHM. */

	if (den1 == 0.) {
	    goto L7;
	}
	if (ifl >= 1) {
	    sig = sigma[lpj];
	}
	grcoef_(&sig, &t3, &t2);
	t1 = t2 + t3;

/*   Update system components for node J. */

	t4 = t1 * 2. / (alfa * alfsq);
	t5 = t1 / den2;
	t6 = t3 / den1;
	c11 += t4;
	c12 += t5 * xj;
	c13 += t5 * yj;
	c22 += t6 * xs;
	c23 += t6 * xj * yj;
	c33 += t6 * ys;
	gkj = g1 * x[j] + g2 * y[j] + g3 * z__[j];
	gjk = grad[j * 3 + 1] * xk + grad[j * 3 + 2] * yk + grad[j * 3 + 3] * 
		zk;
	r1 = r1 + t4 * (f[j] - fk) + t5 * (gjk - gkj);
	t = t5 * (f[j] - fk) - t6 * gkj + t2 * gjk / den1;
	r2 += t * xj;
	r3 += t * yj;

/*   Bottom of loop on neighbors. */

	if (lpj != lpl) {
	    goto L2;
	}

/*   Solve the system associated with the K-th block. */

	cc22 = c11 * c22 - c12 * c12;
	cc23 = c11 * c23 - c12 * c13;
	cc33 = c11 * c33 - c13 * c13;
	rr2 = c11 * r2 - c12 * r1;
	rr3 = c11 * r3 - c13 * r1;
	det = cc22 * cc33 - cc23 * cc23;
	if (det == 0. || cc22 == 0. || c11 == 0.) {
	    goto L6;
	}
	dgy = (cc22 * rr3 - cc23 * rr2) / det;
	dgx = (rr2 - cc23 * dgy) / cc22;
	df = (r1 - c12 * dgx - c13 * dgy) / c11;

/*   Rotate (DGX,DGY,0) back to the original coordinate */
/*     system, and update GRAD( ,K), F(K), and DFMX. */

	aplyrt_(&dgx, &dgy, &cx, &sx, &cy, &sy, dgk);
	grad[k * 3 + 1] = g1 + dgk[0];
	grad[k * 3 + 2] = g2 + dgk[1];
	grad[k * 3 + 3] = g3 + dgk[2];
	f[k] = fk + df;
/* Computing MAX */
	d__1 = dfmx, d__2 = fabs(df) / (fabs(fk) + 1.);
	dfmx = max(d__1,d__2);
/* L3: */
    }

/*   Increment ITER and test for convergence. */

    ++iter;
    if (dfmx > tol) {
	goto L1;
    }

/* The method converged. */

    *nit = iter;
    *dfmax = dfmx;
    *ier = 0;
    return 0;

/* The method failed to converge within NIT iterations. */

L4:
    *dfmax = dfmx;
    *ier = 1;
    return 0;

/* Invalid input parameter. */

L5:
    *nit = 0;
    *dfmax = 0.;
    *ier = -1;
    return 0;

/* Node K and its neighbors are collinear. */

L6:
    *nit = 0;
    *dfmax = dfmx;
    *ier = -2;
    return 0;

/* Nodes J and K coincide. */

L7:
    *nit = 0;
    *dfmax = dfmx;
    *ier = -3;
    return 0;
} /* smsgs_ */

/* Subroutine */ integer smsurf_(integer *n, doublereal *x, doublereal *y, 
	doublereal *z__, doublereal *u, integer *list, integer *lptr, integer 
	*lend, integer *iflgs, doublereal *sigma, doublereal *w, doublereal *
	sm, doublereal *smtol, doublereal *gstol, integer *lprnt, doublereal *
	f, doublereal *grad, integer *ier)
{
    /* Initialized data */

    static integer itmax = 50;
    static integer nitmax = 40;

    /* System generated locals */
    integer i__1;
    doublereal d__1;

    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static doublereal c__, g;
    static integer i__;
    static doublereal p, s, g0, q2, dp;
    static integer nn;
    static doublereal wi;
    static integer nit, lun;
    static doublereal tol, gneg, dmax__;
    static integer ierr, iter;
    static doublereal sumw, q2min, q2max, dfmax;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/21/98 */

/*   Given a triangulation of N nodes on the unit sphere with */
/* data values U at the nodes and tension factors SIGMA */
/* associated with the arcs, this routine determines a set of */
/* nodal function values F and gradients GRAD = (FX,FY,FZ) */
/* such that a quadratic functional Q1(F,GRAD) is minimized */
/* subject to the constraint Q2(F) .LE. SM for Q2(F) = */
/* (U-F)**T*W*(U-F), where W is a diagonal matrix of positive */
/* weights.  The functional Q1 is an approximation to the */
/* linearized curvature over the triangulation of a C-1 fun- */
/* ction F(V), V a unit vector, which interpolates the nodal */
/* values and gradients.  Subroutines INTRC1 and UNIF may be */
/* called to evaluate F at arbitrary points. */

/*   The smoothing procedure is an extension of the method */
/* for cubic spline smoothing due to C. Reinsch -- Numer. */
/* Math., 10 (1967) and 16 (1971).  Refer to Function FVAL */
/* for a further description of the interpolant F.  Letting */
/* D1F(T) and D2F(T) denote first and second derivatives of F */
/* with respect to a parameter T varying along a triangula- */
/* tion arc, Q1 is the sum of integrals over the arcs of */
/* D2F(T)**2 + ((SIGMA/L)*(D1F(T)-S))**2 where L denotes arc- */
/* length, SIGMA is the appropriate tension factor, and S is */
/* the slope of the linear function of T which interpolates */
/* the values of F at the endpoints of the arc.  Introducing */
/* a smoothing parameter P, and assuming the constraint is */
/* active, the problem is equivalent to minimizing Q(P,F, */
/* GRAD) = Q1(F,GRAD) + P*(Q2(F)-SM).  The secant method is */
/* used to find a zero of G(P) = 1/SQRT(Q2) - 1/SQRT(SM) */
/* where F(P) satisfies the order-3N symmetric positive def- */
/* inite linear system obtained by setting the gradient of Q */
/* (treated as a function of F and GRAD with GRAD tangent to */
/* the sphere surface) to zero.  The linear system is solved */
/* by the block Gauss-Seidel method (refer to SMSGS). */

/*   Note that the method can also be used to select grad- */
/* ients for the interpolation problem (F = U, SM = 0, and P */
/* infinite).  This is achieved by a call to Subroutine */
/* GRADG. */

/* On input: */

/*       N = Number of nodes in the triangulation.  N .GE. 3. */

/*       X,Y,Z = Arrays of length N containing Cartesian */
/*               coordinates of the nodes. */

/*       U = Array of length N containing data values at the */
/*           nodes. */

/*       LIST,LPTR,LEND = Data structure defining the trian- */
/*                        gulation.  Refer to STRIPACK */
/*                        Subroutine TRMESH. */

/*       IFLGS = Tension factor option: */
/*               IFLGS .LE. 0 if a single uniform tension */
/*                            factor is to be used. */
/*               IFLGS .GE. 1 if variable tension is desired. */

/*       SIGMA = Uniform tension factor (IFLGS .LE. 0), or */
/*               array containing tension factors associated */
/*               with arcs in one-to-one correspondence with */
/*               LIST entries (IFLGS .GE. 1).  Refer to Sub- */
/*               programs GETSIG, SIG0, SIG1, and SIG2. */

/*       W = Array of length N containing positive weights */
/*           associated with the data values.  The recommend- */
/*           ed value of W(I) is 1/DU**2 where DU is the */
/*           standard deviation associated with U(I).  DU**2 */
/*           is the expected value of the squared error in */
/*           the measurement of U(I).  (The mean error is */
/*           assumed to be zero.) */

/*       SM = Positive parameter specifying an upper bound on */
/*            Q2(F).  Note that F is constant (and Q2(F) */
/*            is minimized) if SM is sufficiently large that */
/*            the constraint is not active.  It is recommend- */
/*            ed that SM satisfy N-SQRT(2N) .LE. SM .LE. N+ */
/*            SQRT(2N). */

/*       SMTOL = Parameter in the open interval (0,1) speci- */
/*               fying the relative error allowed in satisfy- */
/*               ing the constraint -- the constraint is */
/*               assumed to be satisfied if SM*(1-SMTOL) .LE. */
/*               Q2 .LE. SM*(1+SMTOL).  A reasonable value */
/*               for SMTOL is SQRT(2/N). */

/*       GSTOL = Nonnegative tolerance defining the conver- */
/*               gence criterion for the Gauss-Seidel method. */
/*               Refer to parameter DFMAX in Subroutine */
/*               SMSGS.  A recommended value is .05*DU**2, */
/*               where DU is an average standard deviation */
/*               in the data values. */

/*       LPRNT = Logical unit on which diagnostic messages */
/*               are printed, or negative integer specifying */
/*               no diagnostics.  For each secant iteration, */
/*               the following values are printed:  P, G(P), */
/*               NIT, DFMAX, and DP, where NIT denotes the */
/*               number of Gauss-Seidel iterations used in */
/*               the computation of G, DFMAX denotes the max- */
/*               imum relative change in a solution component */
/*               in the last Gauss-Seidel iteration, and DP */
/*               is the change in P computed by linear inter- */
/*               polation between the current point (P,G) and */
/*               a previous point. */

/* Input parameters are not altered by this routine. */

/* On output: */

/*       F = Array of length N containing nodal function val- */
/*           ues unless IER < 0. */

/*       GRAD = 3 by N array whose columns contain gradients */
/*              of F at the nodes unless IER < 0. */

/*       IER = Error indicator and information flag: */
/*             IER = 0 if no errors were encountered and the */
/*                     constraint is active -- Q2(F) is ap- */
/*                     proximately equal to SM. */
/*             IER = 1 if no errors were encountered but the */
/*                     constraint is not active -- F and GRAD */
/*                     are the values and gradients of a con- */
/*                     stant function which minimizes Q2(F), */
/*                     and Q1 = 0. */
/*             IER = 2 if the constraint could not be satis- */
/*                     fied to within SMTOL due to */
/*                     ill-conditioned linear systems. */
/*             IER = -1 if N, W, SM, SMTOL, or GSTOL is out- */
/*                      side its valid range on input. */
/*             IER = -2 if all nodes are collinear or the */
/*                      triangulation is invalid. */
/*             IER = -3 if duplicate nodes were encountered. */

/* SSRFPACK modules required by SMSURF:  APLYRT, CONSTR, */
/*                                         GRCOEF, SMSGS, */
/*                                         SNHCSH */

/* Intrinsic functions called by SMSURF:  ABS, SQRT */

/* *********************************************************** */


/* Local parameters: */

/* ITMAX = Maximum number of secant iterations. */
/* LUN = Local copy of LPRNT. */
/* NITMAX = Maximum number of Gauss-Seidel iterations for */
/*          each secant iteration. */
/* NN = Local copy of N. */
/* TOL = Local copy of GSTOL. */

    /* Parameter adjustments */
    grad -= 4;
    --f;
    --w;
    --lend;
    --u;
    --z__;
    --y;
    --x;
    --list;
    --lptr;
    --sigma;

    /* Function Body */

    nn = *n;
    tol = *gstol;
    lun = *lprnt;
    if (lun > 99) {
	lun = -1;
    }

/* Test for errors and initialize F to the weighted least */
/*   squares fit of a constant function to the data. */

    *ier = -1;
    if (nn < 3 || *sm <= 0. || *smtol <= 0. || *smtol >= 1. || tol <= 0.) {
	return 0;
    }
    c__ = 0.;
    sumw = 0.;
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	wi = w[i__];
	if (wi <= 0.) {
	    return 0;
	}
	c__ += wi * u[i__];
	sumw += wi;
/* L1: */
    }
    c__ /= sumw;

/* Compute nodal values and gradients, and accumulate Q2 = */
/*   (U-F)**T*W*(U-F). */

    q2 = 0.;
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
	f[i__] = c__;
	grad[i__ * 3 + 1] = 0.;
	grad[i__ * 3 + 2] = 0.;
	grad[i__ * 3 + 3] = 0.;
/* Computing 2nd power */
	d__1 = u[i__] - f[i__];
	q2 += w[i__] * (d__1 * d__1);
/* L2: */
    }

/* Compute bounds on Q2 defined by SMTOL, and test for the */
/*   constraint satisfied by the constant fit. */

    q2min = *sm * (1. - *smtol);
    q2max = *sm * (*smtol + 1.);
    if (q2 <= q2max) {

/* The constraint is satisfied by a constant function. */

	*ier = 1;
/*        IF (LUN .GE. 0) WRITE (LUN,100) */
/*  100   FORMAT (///1X,'SMSURF -- THE CONSTRAINT IS NOT ', */
/*     .          'ACTIVE AND THE FITTING FCN IS CONSTANT.') */
#ifdef SPH_DEBUG
    fprintf (stderr, "SMSURF -- THE CONSTRAINT IS NOT ACTIVE AND THE FITTING FCN IS CONSTANT\n");
#endif
	return 0;
    }

/* Compute G0 = G(0) and print a heading. */

    *ier = 0;
    s = 1. / sqrt(*sm);
    g0 = 1. / sqrt(q2) - s;
/*      IF (LUN .GE. 0) WRITE (LUN,110) SM, TOL, NITMAX, G0 */
/*  110 FORMAT (///1X,'SMSURF -- SM = ',E10.4,', GSTOL = ', */
/*     .        E7.1,', NITMAX = ',I2,', G(0) = ',E15.8) */
#ifdef SPH_DEBUG
    fprintf (stderr, "SMSURF -- SM = %g  GSTOL = %g  NITMAX = %d  G(0) = %g\n", *sm, tol, nitmax, g0);
#endif

/* G(P) is strictly increasing and concave, and G(0) .LT. 0. */
/*   Initialize parameters for the secant method.  The method */
/*   uses three points -- (P0,G0), (P,G), and (PNEG,GNEG) */
/*   where P0 and PNEG are defined implicitly by DP = P - P0 */
/*   and DMAX = P - PNEG. */

    p = *sm * 10.;
    dp = p;
    dmax__ = 0.;
    iter = 0;

/* Top of loop -- compute G. */

L3:
    nit = nitmax;
    dfmax = tol;
    smsgs_(&nn, &x[1], &y[1], &z__[1], &u[1], &list[1], &lptr[1], &lend[1], 
	    iflgs, &sigma[1], &w[1], &p, &nit, &dfmax, &f[1], &grad[4], &ierr)
	    ;
    if (ierr < 0) {
	*ier = ierr;
    }

/*   IERR = -1 in SMSGS could be caused by P = 0 as a result */
/*     of inaccurate solutions to ill-conditioned systems. */

    if (ierr == -1) {
	*ier = 2;
    }
    if (ierr < 0) {
	return 0;
    }
    q2 = 0.;
    i__1 = nn;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* Computing 2nd power */
	d__1 = u[i__] - f[i__];
	q2 += w[i__] * (d__1 * d__1);
/* L4: */
    }
    g = 1. / sqrt(q2) - s;
    ++iter;
/*      IF (LUN .GE. 0) WRITE (LUN,120) ITER, P, G, NIT, DFMAX */
/*  120 FORMAT (/1X,I2,' -- P = ',E15.8,', G = ',E15.8, */
/*     .        ', NIT = ',I2,', DFMAX = ',E12.6) */
#ifdef SPH_DEBUG
    fprintf (stderr, " %d -- P = %g  G = %g  NIT = %d  DFMAX = %g\n", iter, p, g, nit, dfmax);
#endif

/*   Test for convergence. */

    if (q2min <= q2 && q2 <= q2max) {
	return 0;
    }
    if (iter >= itmax) {
	*ier = 2;
	return 0;
    }
    if (dmax__ == 0. && g <= 0.) {

/*   Increase P until G(P) > 0. */

	p *= 10.;
	dp = p;
	goto L3;
    }

/*   A bracketing interval [P0,P] has been found. */

    if (g0 * g <= 0.) {

/*   G0*G < 0.  Update (PNEG,GNEG) to (P0,G0) so that G */
/*     and GNEG always have opposite signs. */

	dmax__ = dp;
	gneg = g0;
    }

/*   Compute the change in P by linear interpolation between */
/*     (P0,G0) and (P,G). */

L5:
    dp = -g * dp / (g - g0);
/*      IF (LUN .GE. 0) WRITE (LUN,130) DP */
/*  130 FORMAT (1X,5X,'DP = ',E15.8) */
#ifdef SPH_DEBUG
    fprintf (stderr, "DP = %g\n", dp);
#endif
    if (fabs(dp) > fabs(dmax__)) {

/*   G0*G .GT. 0 and the new estimate would be outside of the */
/*     bracketing interval of length ABS(DMAX).  Reset */
/*     (P0,G0) to (PNEG,GNEG). */

	dp = dmax__;
	g0 = gneg;
	goto L5;
    }

/*   Bottom of loop -- update P, DMAX, and G0. */

    p += dp;
    dmax__ += dp;
    g0 = g;
    goto L3;
} /* smsurf_ */

/* Subroutine */ integer unif_(integer *n, doublereal *x, doublereal *y, 
	doublereal *z__, doublereal *f, integer *list, integer *lptr, integer 
	*lend, integer *iflgs, doublereal *sigma, integer *nrow, integer *ni, 
	integer *nj, doublereal *plat, doublereal *plon, integer *iflgg, 
	doublereal *grad, doublereal *ff, integer *ier)
{
    /* Initialized data */

    static integer nst = 1;

    /* System generated locals */
    integer ff_dim1, ff_offset, i__1, i__2;

    /* Local variables */
    static integer i__, j, nn, nx, ny, ifl, nex, ist, ierr;


/* *********************************************************** */

/*                                              From SSRFPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/25/96 */

/*   Given a Delaunay triangulation of a set of nodes on the */
/* unit sphere, along with data values and tension factors */
/* associated with the triangulation arcs, this routine */
/* interpolates the data values to a uniform grid for such */
/* applications as contouring.  The interpolant is once con- */
/* tinuously differentiable.  Extrapolation is performed at */
/* grid points exterior to the triangulation when the nodes */
/* do not cover the entire sphere. */

/* On input: */

/*       N = Number of nodes.  N .GE. 3 and N .GE. 7 if */
/*           IFLAG .NE. 1. */

/*       X,Y,Z = Arrays containing Cartesian coordinates of */
/*               the nodes.  X(I)**2 + Y(I)**2 + Z(I)**2 = 1 */
/*               for I = 1 to N. */

/*       F = Array containing data values.  F(I) is associ- */
/*           ated with (X(I),Y(I),Z(I)). */

/*       LIST,LPTR,LEND = Data structure defining the trian- */
/*                        gulation.  Refer to STRIPACK */
/*                        Subroutine TRMESH. */

/*       IFLGS = Tension factor option: */
/*               IFLGS .LE. 0 if a single uniform tension */
/*                            factor is to be used. */
/*               IFLGS .GE. 1 if variable tension is desired. */

/*       SIGMA = Uniform tension factor (IFLGS .LE. 0), or */
/*               array containing tension factors associated */
/*               with arcs in one-to-one correspondence with */
/*               LIST entries (IFLGS .GE. 1).  Refer to Sub- */
/*               programs GETSIG, SIG0, SIG1, and SIG2. */

/*       NROW = Number of rows in the dimension statement of */
/*              FF. */

/*       NI,NJ = Number of rows and columns in the uniform */
/*               grid.  1 .LE. NI .LE. NROW and 1 .LE. NJ. */

/*       PLAT,PLON = Arrays of length NI and NJ, respective- */
/*                   ly, containing the latitudes and */
/*                   longitudes of the grid lines. */

/*       IFLGG = Option indicator: */
/*               IFLGG = 0 if gradient estimates at the ver- */
/*                         tices of a triangle are to be */
/*                         recomputed for each grid point in */
/*                         the triangle and not saved. */
/*               IFLGG = 1 if gradient estimates are input in */
/*                         GRAD. */
/*               IFLGG = 2 if gradient estimates are to be */
/*                         computed once for each node (by */
/*                         GRADL) and saved in GRAD. */

/* The above parameters are not altered by this routine. */

/*       GRAD = 3 by N array whose columns contain the X, Y, */
/*              and Z components (in that order) of the grad- */
/*              ients at the nodes if IFLGG = 1, array of */
/*              sufficient size if IFLGG = 2, or dummy para- */
/*              meter if IFLGG = 0. */

/* Gradient estimates may be computed by Subroutines GRADL or */
/*   GRADG if IFLGG = 1. */

/*       FF = NROW by NCOL array with NROW .GE. NI and NCOL */
/*            .GE. NJ. */

/* On output: */

/*       GRAD = Array containing estimated gradients as de- */
/*              fined above if IFLGG = 2 and IER .GE. 0. */
/*              GRAD is not altered if IFLGG .NE. 2. */

/*       FF = Interpolated values at the grid points if IER */
/*            .GE. 0.  FF(I,J) = F(PLAT(I),PLON(J)) for I = */
/*            1,...,NI and J = 1,...,NJ. */

/*       IER = Error indicator: */
/*             IER = K if no errors were encountered and K */
/*                     grid points required extrapolation for */
/*                     K .GE. 0. */
/*             IER = -1 if N, NI, NJ, or IFLGG is outside its */
/*                      valid range. */
/*             IER = -2 if the nodes are collinear. */
/*             IER = -3 if extrapolation failed due to the */
/*                      uniform grid extending too far beyond */
/*                      the triangulation boundary. */

/* STRIPACK modules required by UNIF:  GETNP, JRAND, LSTPTR, */
/*                                       STORE, TRFIND */

/* SSRFPACK modules required by UNIF:  APLYR, APLYRT, ARCINT, */
/*                                       ARCLEN, CONSTR, */
/*                                       FVAL, GIVENS, GRADL, */
/*                                       HVAL, INTRC1, */
/*                                       ROTATE, SETUP, */
/*                                       SNHCSH */

/* *********************************************************** */

    /* Parameter adjustments */
    grad -= 4;
    --lend;
    --f;
    --z__;
    --y;
    --x;
    --list;
    --lptr;
    --sigma;
    --plat;
    ff_dim1 = *nrow;
    ff_offset = 1 + ff_dim1;
    ff -= ff_offset;
    --plon;

    /* Function Body */

/* Local parameters: */

/* I,J =   DO-loop indexes */
/* IERR =  Error flag for calls to GRADL and INTRC1 */
/* IFL =   Local copy of IFLGG */
/* IST =   Parameter for INTRC1 */
/* NEX =   Number of grid points exterior to the triangula- */
/*           tion boundary (number of extrapolated values) */
/* NN =    Local copy of N */
/* NST =   Initial value for IST */
/* NX,NY = Local copies of NI and NJ */

    nn = *n;
    nx = *ni;
    ny = *nj;
    ifl = *iflgg;
    if (nx < 1 || nx > *nrow || ny < 1 || ifl < 0 || ifl > 2) {
	goto L4;
    }
    ist = nst;
    if (ifl == 2) {

/* Compute gradient estimates at the nodes. */

	i__1 = nn;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    gradl_(&nn, &i__, &x[1], &y[1], &z__[1], &f[1], &list[1], &lptr[1]
		    , &lend[1], &grad[i__ * 3 + 1], &ierr);
	    if (ierr < 0) {
		goto L5;
	    }
/* L1: */
	}
	ifl = 1;
    }

/* Compute uniform grid points and interpolated values. */

    nex = 0;
    i__1 = ny;
    for (j = 1; j <= i__1; ++j) {
	i__2 = nx;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    intrc1_(&nn, &plat[i__], &plon[j], &x[1], &y[1], &z__[1], &f[1], &
		    list[1], &lptr[1], &lend[1], iflgs, &sigma[1], &ifl, &
		    grad[4], &ist, &ff[i__ + j * ff_dim1], &ierr);
	    if (ierr < 0) {
		goto L5;
	    }
	    nex += ierr;
/* L2: */
	}
/* L3: */
    }
    *ier = nex;
    return 0;

/* NI, NJ, or IFLGG is outside its valid range. */

L4:
    *ier = -1;
    return 0;

/* Error in GRADL or INTRC1. */

L5:
    *ier = ierr;
    return 0;
} /* unif_ */
