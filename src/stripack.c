/*
 * stripack.c: Translated via f2c then massaged so that f2c include and lib
 * are not required to compile and link the sph supplement.
 *
 * All these functions are local (static) and included into gmt_sph.c where they are used.
 */

/* Common Block Declarations */

static struct {
    doublereal y;
} stcom_;

#define stcom_1 stcom_

GMT_LOCAL doublereal store_(doublereal *x) {
    /* System generated locals */
    doublereal ret_val;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   05/09/92 */

/*   This function forces its argument X to be stored in a */
/* memory location, thus providing a means of determining */
/* floating point number characteristics (such as the machine */
/* precision) when it is necessary to avoid computation in */
/* high precision registers. */


/* On input: */

/*       X = Value to be stored. */

/* X is not altered by this function. */

/* On output: */

/*       STORE = Value of X after it has been stored and */
/*               possibly truncated or rounded to the single */
/*               precision word length. */

/* Modules required by STORE:  None */

/* *********************************************************** */

    stcom_1.y = *x;
    ret_val = stcom_1.y;
    return ret_val;
} /* store_ */

GMT_LOCAL integer jrand_(integer *n, integer *ix, integer *iy, integer *iz) {
    /* System generated locals */
    integer ret_val;

    /* Local variables */
    static doublereal u, x;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/28/98 */

/*   This function returns a uniformly distributed pseudo- */
/* random integer in the range 1 to N. */


/* On input: */

/*       N = Maximum value to be returned. */

/* N is not altered by this function. */

/*       IX,IY,IZ = Integer seeds initialized to values in */
/*                  the range 1 to 30,000 before the first */
/*                  call to JRAND, and not altered between */
/*                  subsequent calls (unless a sequence of */
/*                  random numbers is to be repeated by */
/*                  reinitializing the seeds). */

/* On output: */

/*       IX,IY,IZ = Updated integer seeds. */

/*       JRAND = Random integer in the range 1 to N. */

/* Reference:  B. A. Wichmann and I. D. Hill, "An Efficient */
/*             and Portable Pseudo-random Number Generator", */
/*             Applied Statistics, Vol. 31, No. 2, 1982, */
/*             pp. 188-190. */

/* Modules required by JRAND:  None */

/* Intrinsic functions called by JRAND:  INT, MOD, REAL */

/* *********************************************************** */


/* Local parameters: */

/* U = Pseudo-random number uniformly distributed in the */
/*     interval (0,1). */
/* X = Pseudo-random number in the range 0 to 3 whose frac- */
/*       tional part is U. */

    *ix = *ix * 171 % 30269;
    *iy = *iy * 172 % 30307;
    *iz = *iz * 170 % 30323;
    x = (doublereal) (*ix) / 30269. + (doublereal) (*iy) / 30307. + (
	    doublereal) (*iz) / 30323.;
    u = x - (integer) x;
    ret_val = (integer) ((doublereal) (*n) * u + 1.);
    return ret_val;
} /* jrand_ */

GMT_LOCAL integer lstptr_(integer *lpl, integer *nb, integer *list, integer *lptr) {
    /* System generated locals */
    integer ret_val;

    /* Local variables */
    static integer nd, lp;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/15/96 */

/*   This function returns the index (LIST pointer) of NB in */
/* the adjacency list for N0, where LPL = LEND(N0). */

/*   This function is identical to the similarly named */
/* function in TRIPACK. */


/* On input: */

/*       LPL = LEND(N0) */

/*       NB = Index of the node whose pointer is to be re- */
/*            turned.  NB must be connected to N0. */

/*       LIST,LPTR = Data structure defining the triangula- */
/*                   tion.  Refer to Subroutine TRMESH. */

/* Input parameters are not altered by this function. */

/* On output: */

/*       LSTPTR = Pointer such that LIST(LSTPTR) = NB or */
/*                LIST(LSTPTR) = -NB, unless NB is not a */
/*                neighbor of N0, in which case LSTPTR = LPL. */

/* Modules required by LSTPTR:  None */

/* *********************************************************** */


/* Local parameters: */

/* LP = LIST pointer */
/* ND = Nodal index */

    /* Parameter adjustments */
    --lptr;
    --list;

    /* Function Body */
    lp = lptr[*lpl];
L1:
    nd = list[lp];
    if (nd == *nb) {
	goto L2;
    }
    lp = lptr[lp];
    if (lp != *lpl) {
	goto L1;
    }

L2:
    ret_val = lp;
    return ret_val;
} /* lstptr_ */

GMT_LOCAL integer swap_(integer *in1, integer *in2, integer *io1, integer *io2, integer *list, integer *lptr, integer *lend, integer *lp21) {
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer lp, lph, lpsav;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   06/22/98 */

/*   Given a triangulation of a set of points on the unit */
/* sphere, this subroutine replaces a diagonal arc in a */
/* strictly convex quadrilateral (defined by a pair of adja- */
/* cent triangles) with the other diagonal.  Equivalently, a */
/* pair of adjacent triangles is replaced by another pair */
/* having the same union. */


/* On input: */

/*       IN1,IN2,IO1,IO2 = Nodal indexes of the vertices of */
/*                         the quadrilateral.  IO1-IO2 is re- */
/*                         placed by IN1-IN2.  (IO1,IO2,IN1) */
/*                         and (IO2,IO1,IN2) must be trian- */
/*                         gles on input. */

/* The above parameters are not altered by this routine. */

/*       LIST,LPTR,LEND = Data structure defining the trian- */
/*                        gulation.  Refer to Subroutine */
/*                        TRMESH. */

/* On output: */

/*       LIST,LPTR,LEND = Data structure updated with the */
/*                        swap -- triangles (IO1,IO2,IN1) and */
/*                        (IO2,IO1,IN2) are replaced by */
/*                        (IN1,IN2,IO2) and (IN2,IN1,IO1) */
/*                        unless LP21 = 0. */

/*       LP21 = Index of IN1 as a neighbor of IN2 after the */
/*              swap is performed unless IN1 and IN2 are */
/*              adjacent on input, in which case LP21 = 0. */

/* Module required by SWAP:  LSTPTR */

/* Intrinsic function called by SWAP:  ABS */

/* *********************************************************** */


/* Local parameters: */

/* LP,LPH,LPSAV = LIST pointers */


/* Test for IN1 and IN2 adjacent. */

    /* Parameter adjustments */
    --lend;
    --lptr;
    --list;

    /* Function Body */
    lp = lstptr_(&lend[*in1], in2, &list[1], &lptr[1]);
    if ((i__1 = list[lp], int64_abs(i__1)) == *in2) {
	*lp21 = 0;
	return 0;
    }

/* Delete IO2 as a neighbor of IO1. */

    lp = lstptr_(&lend[*io1], in2, &list[1], &lptr[1]);
    lph = lptr[lp];
    lptr[lp] = lptr[lph];

/* If IO2 is the last neighbor of IO1, make IN2 the */
/*   last neighbor. */

    if (lend[*io1] == lph) {
	lend[*io1] = lp;
    }

/* Insert IN2 as a neighbor of IN1 following IO1 */
/*   using the hole created above. */

    lp = lstptr_(&lend[*in1], io1, &list[1], &lptr[1]);
    lpsav = lptr[lp];
    lptr[lp] = lph;
    list[lph] = *in2;
    lptr[lph] = lpsav;

/* Delete IO1 as a neighbor of IO2. */

    lp = lstptr_(&lend[*io2], in1, &list[1], &lptr[1]);
    lph = lptr[lp];
    lptr[lp] = lptr[lph];

/* If IO1 is the last neighbor of IO2, make IN1 the */
/*   last neighbor. */

    if (lend[*io2] == lph) {
	lend[*io2] = lp;
    }

/* Insert IN1 as a neighbor of IN2 following IO2. */

    lp = lstptr_(&lend[*in2], io2, &list[1], &lptr[1]);
    lpsav = lptr[lp];
    lptr[lp] = lph;
    list[lph] = *in1;
    lptr[lph] = lpsav;
    *lp21 = lph;
    return 0;
} /* swap_ */

GMT_LOCAL integer insert_(integer *k, integer *lp, integer *list, integer *lptr, integer *lnew) {
    static integer lsav;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/17/96 */

/*   This subroutine inserts K as a neighbor of N1 following */
/* N2, where LP is the LIST pointer of N2 as a neighbor of */
/* N1.  Note that, if N2 is the last neighbor of N1, K will */
/* become the first neighbor (even if N1 is a boundary node). */

/*   This routine is identical to the similarly named routine */
/* in TRIPACK. */


/* On input: */

/*       K = Index of the node to be inserted. */

/*       LP = LIST pointer of N2 as a neighbor of N1. */

/* The above parameters are not altered by this routine. */

/*       LIST,LPTR,LNEW = Data structure defining the trian- */
/*                        gulation.  Refer to Subroutine */
/*                        TRMESH. */

/* On output: */

/*       LIST,LPTR,LNEW = Data structure updated with the */
/*                        addition of node K. */

/* Modules required by INSERT:  None */

/* *********************************************************** */


    /* Parameter adjustments */
    --lptr;
    --list;

    /* Function Body */
    lsav = lptr[*lp];
    lptr[*lp] = *lnew;
    list[*lnew] = *k;
    lptr[*lnew] = lsav;
    ++(*lnew);
    return 0;
} /* insert_ */

GMT_LOCAL integer bdyadd_(integer *kk, integer *i1, integer *i2, integer *list, integer *lptr, integer *lend, integer *lnew) {
    static integer k, n1, n2, lp, lsav, nsav, next;

/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/11/96 */

/*   This subroutine adds a boundary node to a triangulation */
/* of a set of KK-1 points on the unit sphere.  The data */
/* structure is updated with the insertion of node KK, but no */
/* optimization is performed. */

/*   This routine is identical to the similarly named routine */
/* in TRIPACK. */


/* On input: */

/*       KK = Index of a node to be connected to the sequence */
/*            of all visible boundary nodes.  KK .GE. 1 and */
/*            KK must not be equal to I1 or I2. */

/*       I1 = First (rightmost as viewed from KK) boundary */
/*            node in the triangulation that is visible from */
/*            node KK (the line segment KK-I1 intersects no */
/*            arcs. */

/*       I2 = Last (leftmost) boundary node that is visible */
/*            from node KK.  I1 and I2 may be determined by */
/*            Subroutine TRFIND. */

/* The above parameters are not altered by this routine. */

/*       LIST,LPTR,LEND,LNEW = Triangulation data structure */
/*                             created by Subroutine TRMESH. */
/*                             Nodes I1 and I2 must be in- */
/*                             cluded in the triangulation. */

/* On output: */

/*       LIST,LPTR,LEND,LNEW = Data structure updated with */
/*                             the addition of node KK.  Node */
/*                             KK is connected to I1, I2, and */
/*                             all boundary nodes in between. */

/* Module required by BDYADD:  INSERT */

/* *********************************************************** */


/* Local parameters: */

/* K =     Local copy of KK */
/* LP =    LIST pointer */
/* LSAV =  LIST pointer */
/* N1,N2 = Local copies of I1 and I2, respectively */
/* NEXT =  Boundary node visible from K */
/* NSAV =  Boundary node visible from K */

    /* Parameter adjustments */
    --lend;
    --lptr;
    --list;

    /* Function Body */
    k = *kk;
    n1 = *i1;
    n2 = *i2;

/* Add K as the last neighbor of N1. */

    lp = lend[n1];
    lsav = lptr[lp];
    lptr[lp] = *lnew;
    list[*lnew] = -k;
    lptr[*lnew] = lsav;
    lend[n1] = *lnew;
    ++(*lnew);
    next = -list[lp];
    list[lp] = next;
    nsav = next;

/* Loop on the remaining boundary nodes between N1 and N2, */
/*   adding K as the first neighbor. */

L1:
    lp = lend[next];
    insert_(&k, &lp, &list[1], &lptr[1], lnew);
    if (next == n2) {
	goto L2;
    }
    next = -list[lp];
    list[lp] = next;
    goto L1;

/* Add the boundary nodes between N1 and N2 as neighbors */
/*   of node K. */

L2:
    lsav = *lnew;
    list[*lnew] = n1;
    lptr[*lnew] = *lnew + 1;
    ++(*lnew);
    next = nsav;

L3:
    if (next == n2) {
	goto L4;
    }
    list[*lnew] = next;
    lptr[*lnew] = *lnew + 1;
    ++(*lnew);
    lp = lend[next];
    next = list[lp];
    goto L3;

L4:
    list[*lnew] = -n2;
    lptr[*lnew] = lsav;
    lend[k] = *lnew;
    ++(*lnew);
    return 0;
} /* bdyadd_ */

GMT_LOCAL integer intadd_(integer *kk, integer *i1, integer *i2, integer * i3, integer *list, integer *lptr, integer *lend, integer *lnew) {
    static integer k, n1, n2, n3, lp;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/17/96 */

/*   This subroutine adds an interior node to a triangulation */
/* of a set of points on the unit sphere.  The data structure */
/* is updated with the insertion of node KK into the triangle */
/* whose vertices are I1, I2, and I3.  No optimization of the */
/* triangulation is performed. */

/*   This routine is identical to the similarly named routine */
/* in TRIPACK. */


/* On input: */

/*       KK = Index of the node to be inserted.  KK .GE. 1 */
/*            and KK must not be equal to I1, I2, or I3. */

/*       I1,I2,I3 = Indexes of the counterclockwise-ordered */
/*                  sequence of vertices of a triangle which */
/*                  contains node KK. */

/* The above parameters are not altered by this routine. */

/*       LIST,LPTR,LEND,LNEW = Data structure defining the */
/*                             triangulation.  Refer to Sub- */
/*                             routine TRMESH.  Triangle */
/*                             (I1,I2,I3) must be included */
/*                             in the triangulation. */

/* On output: */

/*       LIST,LPTR,LEND,LNEW = Data structure updated with */
/*                             the addition of node KK.  KK */
/*                             will be connected to nodes I1, */
/*                             I2, and I3. */

/* Modules required by INTADD:  INSERT, LSTPTR */

/* *********************************************************** */


/* Local parameters: */

/* K =        Local copy of KK */
/* LP =       LIST pointer */
/* N1,N2,N3 = Local copies of I1, I2, and I3 */

    /* Parameter adjustments */
    --lend;
    --lptr;
    --list;

    /* Function Body */
    k = *kk;

/* Initialization. */

    n1 = *i1;
    n2 = *i2;
    n3 = *i3;

/* Add K as a neighbor of I1, I2, and I3. */

    lp = lstptr_(&lend[n1], &n2, &list[1], &lptr[1]);
    insert_(&k, &lp, &list[1], &lptr[1], lnew);
    lp = lstptr_(&lend[n2], &n3, &list[1], &lptr[1]);
    insert_(&k, &lp, &list[1], &lptr[1], lnew);
    lp = lstptr_(&lend[n3], &n1, &list[1], &lptr[1]);
    insert_(&k, &lp, &list[1], &lptr[1], lnew);

/* Add I1, I2, and I3 as neighbors of K. */

    list[*lnew] = n1;
    list[*lnew + 1] = n2;
    list[*lnew + 2] = n3;
    lptr[*lnew] = *lnew + 1;
    lptr[*lnew + 1] = *lnew + 2;
    lptr[*lnew + 2] = *lnew;
    lend[k] = *lnew + 2;
    *lnew += 3;
    return 0;
} /* intadd_ */

GMT_LOCAL integer trfind_(integer *nst, doublereal *p, integer *n, doublereal *x, doublereal *y, doublereal *z__, integer *list, integer *lptr, integer *lend, doublereal *b1, doublereal *b2, doublereal *b3, integer *i1, integer *i2, integer *i3) {
    /* Initialized data */

    static integer ix = 1;
    static integer iy = 2;
    static integer iz = 3;

    /* System generated locals */
    integer i__1;
    doublereal d__1, d__2;

    /* Local variables */
    static doublereal q[3];
    static integer n0, n1, n2, n3, n4, nf;
    static doublereal s12;
    static integer nl, lp;
    static doublereal xp, yp, zp;
    static integer n1s, n2s;
    static doublereal eps, tol, ptn1, ptn2;
    static integer next;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   11/30/99 */

/*   This subroutine locates a point P relative to a triangu- */
/* lation created by Subroutine TRMESH.  If P is contained in */
/* a triangle, the three vertex indexes and barycentric coor- */
/* dinates are returned.  Otherwise, the indexes of the */
/* visible boundary nodes are returned. */


/* On input: */

/*       NST = Index of a node at which TRFIND begins its */
/*             search.  Search time depends on the proximity */
/*             of this node to P. */

/*       P = Array of length 3 containing the x, y, and z */
/*           coordinates (in that order) of the point P to be */
/*           located. */

/*       N = Number of nodes in the triangulation.  N .GE. 3. */

/*       X,Y,Z = Arrays of length N containing the Cartesian */
/*               coordinates of the triangulation nodes (unit */
/*               vectors).  (X(I),Y(I),Z(I)) defines node I */
/*               for I = 1 to N. */

/*       LIST,LPTR,LEND = Data structure defining the trian- */
/*                        gulation.  Refer to Subroutine */
/*                        TRMESH. */

/* Input parameters are not altered by this routine. */

/* On output: */

/*       B1,B2,B3 = Unnormalized barycentric coordinates of */
/*                  the central projection of P onto the un- */
/*                  derlying planar triangle if P is in the */
/*                  convex hull of the nodes.  These parame- */
/*                  ters are not altered if I1 = 0. */

/*       I1,I2,I3 = Counterclockwise-ordered vertex indexes */
/*                  of a triangle containing P if P is con- */
/*                  tained in a triangle.  If P is not in the */
/*                  convex hull of the nodes, I1 and I2 are */
/*                  the rightmost and leftmost (boundary) */
/*                  nodes that are visible from P, and */
/*                  I3 = 0.  (If all boundary nodes are vis- */
/*                  ible from P, then I1 and I2 coincide.) */
/*                  I1 = I2 = I3 = 0 if P and all of the */
/*                  nodes are coplanar (lie on a common great */
/*                  circle. */

/* Modules required by TRFIND:  JRAND, LSTPTR, STORE */

/* Intrinsic function called by TRFIND:  ABS */

/* *********************************************************** */


    /* Parameter adjustments */
    --p;
    --lend;
    --z__;
    --y;
    --x;
    --list;
    --lptr;

    /* Function Body */

/* Local parameters: */

/* EPS =      Machine precision */
/* IX,IY,IZ = Integer seeds for JRAND */
/* LP =       LIST pointer */
/* N0,N1,N2 = Nodes in counterclockwise order defining a */
/*              cone (with vertex N0) containing P, or end- */
/*              points of a boundary edge such that P Right */
/*              N1->N2 */
/* N1S,N2S =  Initially-determined values of N1 and N2 */
/* N3,N4 =    Nodes opposite N1->N2 and N2->N1, respectively */
/* NEXT =     Candidate for I1 or I2 when P is exterior */
/* NF,NL =    First and last neighbors of N0, or first */
/*              (rightmost) and last (leftmost) nodes */
/*              visible from P when P is exterior to the */
/*              triangulation */
/* PTN1 =     Scalar product <P,N1> */
/* PTN2 =     Scalar product <P,N2> */
/* Q =        (N2 X N1) X N2  or  N1 X (N2 X N1) -- used in */
/*              the boundary traversal when P is exterior */
/* S12 =      Scalar product <N1,N2> */
/* TOL =      Tolerance (multiple of EPS) defining an upper */
/*              bound on the magnitude of a negative bary- */
/*              centric coordinate (B1 or B2) for P in a */
/*              triangle -- used to avoid an infinite number */
/*              of restarts with 0 <= B3 < EPS and B1 < 0 or */
/*              B2 < 0 but small in magnitude */
/* XP,YP,ZP = Local variables containing P(1), P(2), and P(3) */
/* X0,Y0,Z0 = Dummy arguments for DET */
/* X1,Y1,Z1 = Dummy arguments for DET */
/* X2,Y2,Z2 = Dummy arguments for DET */

/* Statement function: */

/* DET(X1,...,Z0) .GE. 0 if and only if (X0,Y0,Z0) is in the */
/*                       (closed) left hemisphere defined by */
/*                       the plane containing (0,0,0), */
/*                       (X1,Y1,Z1), and (X2,Y2,Z2), where */
/*                       left is defined relative to an ob- */
/*                       server at (X1,Y1,Z1) facing */
/*                       (X2,Y2,Z2). */


/* Initialize variables. */

    xp = p[1];
    yp = p[2];
    zp = p[3];
    n0 = *nst;
    if (n0 < 1 || n0 > *n) {
	n0 = jrand_(n, &ix, &iy, &iz);
    }

/* Compute the relative machine precision EPS and TOL. */

    eps = 1.;
L1:
    eps /= 2.;
    d__1 = eps + 1.;
    if (store_(&d__1) > 1.) {
	goto L1;
    }
    eps *= 2.;
    tol = eps * 100.;

/* Set NF and NL to the first and last neighbors of N0, and */
/*   initialize N1 = NF. */

L2:
    lp = lend[n0];
    nl = list[lp];
    lp = lptr[lp];
    nf = list[lp];
    n1 = nf;

/* Find a pair of adjacent neighbors N1,N2 of N0 that define */
/*   a wedge containing P:  P LEFT N0->N1 and P RIGHT N0->N2. */

    if (nl > 0) {

/*   N0 is an interior node.  Find N1. */

L3:
	if (xp * (y[n0] * z__[n1] - y[n1] * z__[n0]) - yp * (x[n0] * z__[n1]
		- x[n1] * z__[n0]) + zp * (x[n0] * y[n1] - x[n1] * y[n0]) <
		0.) {
	    lp = lptr[lp];
	    n1 = list[lp];
	    if (n1 == nl) {
		goto L6;
	    }
	    goto L3;
	}
    } else {

/*   N0 is a boundary node.  Test for P exterior. */

	nl = -nl;
	if (xp * (y[n0] * z__[nf] - y[nf] * z__[n0]) - yp * (x[n0] * z__[nf]
		- x[nf] * z__[n0]) + zp * (x[n0] * y[nf] - x[nf] * y[n0]) <
		0.) {

/*   P is to the right of the boundary edge N0->NF. */

	    n1 = n0;
	    n2 = nf;
	    goto L9;
	}
	if (xp * (y[nl] * z__[n0] - y[n0] * z__[nl]) - yp * (x[nl] * z__[n0]
		- x[n0] * z__[nl]) + zp * (x[nl] * y[n0] - x[n0] * y[nl]) <
		0.) {

/*   P is to the right of the boundary edge NL->N0. */

	    n1 = nl;
	    n2 = n0;
	    goto L9;
	}
    }

/* P is to the left of arcs N0->N1 and NL->N0.  Set N2 to the */
/*   next neighbor of N0 (following N1). */

L4:
    lp = lptr[lp];
    n2 = (i__1 = list[lp], int64_abs(i__1));
    if (xp * (y[n0] * z__[n2] - y[n2] * z__[n0]) - yp * (x[n0] * z__[n2] - x[
	    n2] * z__[n0]) + zp * (x[n0] * y[n2] - x[n2] * y[n0]) < 0.) {
	goto L7;
    }
    n1 = n2;
    if (n1 != nl) {
	goto L4;
    }
    if (xp * (y[n0] * z__[nf] - y[nf] * z__[n0]) - yp * (x[n0] * z__[nf] - x[
	    nf] * z__[n0]) + zp * (x[n0] * y[nf] - x[nf] * y[n0]) < 0.) {
	goto L6;
    }

/* P is left of or on arcs N0->NB for all neighbors NB */
/*   of N0.  Test for P = +/-N0. */

    d__2 = (d__1 = x[n0] * xp + y[n0] * yp + z__[n0] * zp, fabs(d__1));
    if (store_(&d__2) < 1. - eps * 4.) {

/*   All points are collinear if P Left NB->N0 for all */
/*     neighbors NB of N0.  Search the neighbors of N0. */
/*     Note:  N1 = NL and LP points to NL. */

L5:
	if (xp * (y[n1] * z__[n0] - y[n0] * z__[n1]) - yp * (x[n1] * z__[n0]
		- x[n0] * z__[n1]) + zp * (x[n1] * y[n0] - x[n0] * y[n1]) >=
		0.) {
	    lp = lptr[lp];
	    n1 = (i__1 = list[lp], int64_abs(i__1));
	    if (n1 == nl) {
		goto L14;
	    }
	    goto L5;
	}
    }

/* P is to the right of N1->N0, or P = +/-N0.  Set N0 to N1 */
/*   and start over. */

    n0 = n1;
    goto L2;

/* P is between arcs N0->N1 and N0->NF. */

L6:
    n2 = nf;

/* P is contained in a wedge defined by geodesics N0-N1 and */
/*   N0-N2, where N1 is adjacent to N2.  Save N1 and N2 to */
/*   test for cycling. */

L7:
    n3 = n0;
    n1s = n1;
    n2s = n2;

/* Top of edge-hopping loop: */

L8:
    *b3 = xp * (y[n1] * z__[n2] - y[n2] * z__[n1]) - yp * (x[n1] * z__[n2] -
	    x[n2] * z__[n1]) + zp * (x[n1] * y[n2] - x[n2] * y[n1]);
    if (*b3 < 0.) {

/*   Set N4 to the first neighbor of N2 following N1 (the */
/*     node opposite N2->N1) unless N1->N2 is a boundary arc. */

	lp = lstptr_(&lend[n2], &n1, &list[1], &lptr[1]);
	if (list[lp] < 0) {
	    goto L9;
	}
	lp = lptr[lp];
	n4 = (i__1 = list[lp], int64_abs(i__1));

/*   Define a new arc N1->N2 which intersects the geodesic */
/*     N0-P. */

	if (xp * (y[n0] * z__[n4] - y[n4] * z__[n0]) - yp * (x[n0] * z__[n4]
		- x[n4] * z__[n0]) + zp * (x[n0] * y[n4] - x[n4] * y[n0]) <
		0.) {
	    n3 = n2;
	    n2 = n4;
	    n1s = n1;
	    if (n2 != n2s && n2 != n0) {
		goto L8;
	    }
	} else {
	    n3 = n1;
	    n1 = n4;
	    n2s = n2;
	    if (n1 != n1s && n1 != n0) {
		goto L8;
	    }
	}

/*   The starting node N0 or edge N1-N2 was encountered */
/*     again, implying a cycle (infinite loop).  Restart */
/*     with N0 randomly selected. */

	n0 = jrand_(n, &ix, &iy, &iz);
	goto L2;
    }

/* P is in (N1,N2,N3) unless N0, N1, N2, and P are collinear */
/*   or P is close to -N0. */

    if (*b3 >= eps) {

/*   B3 .NE. 0. */

	*b1 = xp * (y[n2] * z__[n3] - y[n3] * z__[n2]) - yp * (x[n2] * z__[n3]
		 - x[n3] * z__[n2]) + zp * (x[n2] * y[n3] - x[n3] * y[n2]);
	*b2 = xp * (y[n3] * z__[n1] - y[n1] * z__[n3]) - yp * (x[n3] * z__[n1]
		 - x[n1] * z__[n3]) + zp * (x[n3] * y[n1] - x[n1] * y[n3]);
	if (*b1 < -tol || *b2 < -tol) {

/*   Restart with N0 randomly selected. */

	    n0 = jrand_(n, &ix, &iy, &iz);
	    goto L2;
	}
    } else {

/*   B3 = 0 and thus P lies on N1->N2. Compute */
/*     B1 = Det(P,N2 X N1,N2) and B2 = Det(P,N1,N2 X N1). */

	*b3 = 0.;
	s12 = x[n1] * x[n2] + y[n1] * y[n2] + z__[n1] * z__[n2];
	ptn1 = xp * x[n1] + yp * y[n1] + zp * z__[n1];
	ptn2 = xp * x[n2] + yp * y[n2] + zp * z__[n2];
	*b1 = ptn1 - s12 * ptn2;
	*b2 = ptn2 - s12 * ptn1;
	if (*b1 < -tol || *b2 < -tol) {

/*   Restart with N0 randomly selected. */

	    n0 = jrand_(n, &ix, &iy, &iz);
	    goto L2;
	}
    }

/* P is in (N1,N2,N3). */

    *i1 = n1;
    *i2 = n2;
    *i3 = n3;
    if (*b1 < 0.) {
	*b1 = 0.;
    }
    if (*b2 < 0.) {
	*b2 = 0.;
    }
    return 0;

/* P Right N1->N2, where N1->N2 is a boundary edge. */
/*   Save N1 and N2, and set NL = 0 to indicate that */
/*   NL has not yet been found. */

L9:
    n1s = n1;
    n2s = n2;
    nl = 0;

/*           Counterclockwise Boundary Traversal: */

L10:
    lp = lend[n2];
    lp = lptr[lp];
    next = list[lp];
    if (xp * (y[n2] * z__[next] - y[next] * z__[n2]) - yp * (x[n2] * z__[next]
	     - x[next] * z__[n2]) + zp * (x[n2] * y[next] - x[next] * y[n2])
	    >= 0.) {

/*   N2 is the rightmost visible node if P Forward N2->N1 */
/*     or NEXT Forward N2->N1.  Set Q to (N2 X N1) X N2. */

	s12 = x[n1] * x[n2] + y[n1] * y[n2] + z__[n1] * z__[n2];
	q[0] = x[n1] - s12 * x[n2];
	q[1] = y[n1] - s12 * y[n2];
	q[2] = z__[n1] - s12 * z__[n2];
	if (xp * q[0] + yp * q[1] + zp * q[2] >= 0.) {
	    goto L11;
	}
	if (x[next] * q[0] + y[next] * q[1] + z__[next] * q[2] >= 0.) {
	    goto L11;
	}

/*   N1, N2, NEXT, and P are nearly collinear, and N2 is */
/*     the leftmost visible node. */

	nl = n2;
    }

/* Bottom of counterclockwise loop: */

    n1 = n2;
    n2 = next;
    if (n2 != n1s) {
	goto L10;
    }

/* All boundary nodes are visible from P. */

    *i1 = n1s;
    *i2 = n1s;
    *i3 = 0;
    return 0;

/* N2 is the rightmost visible node. */

L11:
    nf = n2;
    if (nl == 0) {

/* Restore initial values of N1 and N2, and begin the search */
/*   for the leftmost visible node. */

	n2 = n2s;
	n1 = n1s;

/*           Clockwise Boundary Traversal: */

L12:
	lp = lend[n1];
	next = -list[lp];
	if (xp * (y[next] * z__[n1] - y[n1] * z__[next]) - yp * (x[next] *
		z__[n1] - x[n1] * z__[next]) + zp * (x[next] * y[n1] - x[n1] *
		 y[next]) >= 0.) {

/*   N1 is the leftmost visible node if P or NEXT is */
/*     forward of N1->N2.  Compute Q = N1 X (N2 X N1). */

	    s12 = x[n1] * x[n2] + y[n1] * y[n2] + z__[n1] * z__[n2];
	    q[0] = x[n2] - s12 * x[n1];
	    q[1] = y[n2] - s12 * y[n1];
	    q[2] = z__[n2] - s12 * z__[n1];
	    if (xp * q[0] + yp * q[1] + zp * q[2] >= 0.) {
		goto L13;
	    }
	    if (x[next] * q[0] + y[next] * q[1] + z__[next] * q[2] >= 0.) {
		goto L13;
	    }

/*   P, NEXT, N1, and N2 are nearly collinear and N1 is the */
/*     rightmost visible node. */

	    nf = n1;
	}

/* Bottom of clockwise loop: */

	n2 = n1;
	n1 = next;
	if (n1 != n1s) {
	    goto L12;
	}

/* All boundary nodes are visible from P. */

	*i1 = n1;
	*i2 = n1;
	*i3 = 0;
	return 0;

/* N1 is the leftmost visible node. */

L13:
	nl = n1;
    }

/* NF and NL have been found. */

    *i1 = nf;
    *i2 = nl;
    *i3 = 0;
    return 0;

/* All points are collinear (coplanar). */

L14:
    *i1 = 0;
    *i2 = 0;
    *i3 = 0;
    return 0;
} /* trfind_ */

GMT_LOCAL integer covsph_(integer *kk, integer *n0, integer *list, integer *lptr, integer *lend, integer *lnew) {
    static integer k, lp, nst, lsav, next;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/17/96 */

/*   This subroutine connects an exterior node KK to all */
/* boundary nodes of a triangulation of KK-1 points on the */
/* unit sphere, producing a triangulation that covers the */
/* sphere.  The data structure is updated with the addition */
/* of node KK, but no optimization is performed.  All boun- */
/* dary nodes must be visible from node KK. */


/* On input: */

/*       KK = Index of the node to be connected to the set of */
/*            all boundary nodes.  KK .GE. 4. */

/*       N0 = Index of a boundary node (in the range 1 to */
/*            KK-1).  N0 may be determined by Subroutine */
/*            TRFIND. */

/* The above parameters are not altered by this routine. */

/*       LIST,LPTR,LEND,LNEW = Triangulation data structure */
/*                             created by Subroutine TRMESH. */
/*                             Node N0 must be included in */
/*                             the triangulation. */

/* On output: */

/*       LIST,LPTR,LEND,LNEW = Data structure updated with */
/*                             the addition of node KK as the */
/*                             last entry.  The updated */
/*                             triangulation contains no */
/*                             boundary nodes. */

/* Module required by COVSPH:  INSERT */

/* *********************************************************** */


/* Local parameters: */

/* K =     Local copy of KK */
/* LP =    LIST pointer */
/* LSAV =  LIST pointer */
/* NEXT =  Boundary node visible from K */
/* NST =   Local copy of N0 */

    /* Parameter adjustments */
    --lend;
    --lptr;
    --list;

    /* Function Body */
    k = *kk;
    nst = *n0;

/* Traverse the boundary in clockwise order, inserting K as */
/*   the first neighbor of each boundary node, and converting */
/*   the boundary node to an interior node. */

    next = nst;
L1:
    lp = lend[next];
    insert_(&k, &lp, &list[1], &lptr[1], lnew);
    next = -list[lp];
    list[lp] = next;
    if (next != nst) {
	goto L1;
    }

/* Traverse the boundary again, adding each node to K's */
/*   adjacency list. */

    lsav = *lnew;
L2:
    lp = lend[next];
    list[*lnew] = next;
    lptr[*lnew] = *lnew + 1;
    ++(*lnew);
    next = list[lp];
    if (next != nst) {
	goto L2;
    }

    lptr[*lnew - 1] = lsav;
    lend[k] = *lnew - 1;
    return 0;
} /* covsph_ */

GMT_LOCAL logical swptst_(integer *n1, integer *n2, integer *n3, integer *n4, doublereal *x, doublereal *y, doublereal *z__) {
    /* System generated locals */
    logical ret_val;

    /* Local variables */
    static doublereal x4, y4, z4, dx1, dx2, dx3, dy1, dy2, dy3, dz1, dz2, dz3;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   03/29/91 */

/*   This function decides whether or not to replace a */
/* diagonal arc in a quadrilateral with the other diagonal. */
/* The decision will be to swap (SWPTST = true) if and only */
/* if N4 lies above the plane (in the half-space not contain- */
/* ing the origin) defined by (N1,N2,N3), or equivalently, if */
/* the projection of N4 onto this plane is interior to the */
/* circumcircle of (N1,N2,N3).  The decision will be for no */
/* swap if the quadrilateral is not strictly convex. */


/* On input: */

/*       N1,N2,N3,N4 = Indexes of the four nodes defining the */
/*                     quadrilateral with N1 adjacent to N2, */
/*                     and (N1,N2,N3) in counterclockwise */
/*                     order.  The arc connecting N1 to N2 */
/*                     should be replaced by an arc connec- */
/*                     ting N3 to N4 if SWPTST = true.  Refer */
/*                     to Subroutine SWAP. */

/*       X,Y,Z = Arrays of length N containing the Cartesian */
/*               coordinates of the nodes.  (X(I),Y(I),Z(I)) */
/*               define node I for I = N1, N2, N3, and N4. */

/* Input parameters are not altered by this routine. */

/* On output: */

/*       SWPTST = true if and only if the arc connecting N1 */
/*                and N2 should be swapped for an arc con- */
/*                necting N3 and N4. */

/* Modules required by SWPTST:  None */

/* *********************************************************** */


/* Local parameters: */

/* DX1,DY1,DZ1 = Coordinates of N4->N1 */
/* DX2,DY2,DZ2 = Coordinates of N4->N2 */
/* DX3,DY3,DZ3 = Coordinates of N4->N3 */
/* X4,Y4,Z4 =    Coordinates of N4 */

    /* Parameter adjustments */
    --z__;
    --y;
    --x;

    /* Function Body */
    x4 = x[*n4];
    y4 = y[*n4];
    z4 = z__[*n4];
    dx1 = x[*n1] - x4;
    dx2 = x[*n2] - x4;
    dx3 = x[*n3] - x4;
    dy1 = y[*n1] - y4;
    dy2 = y[*n2] - y4;
    dy3 = y[*n3] - y4;
    dz1 = z__[*n1] - z4;
    dz2 = z__[*n2] - z4;
    dz3 = z__[*n3] - z4;

/* N4 lies above the plane of (N1,N2,N3) if N3 lies above */
/*   the plane of (N2,N1,N4) if Det(N3-N4,N2-N4,N1-N4) = */
/*   (N3-N4,N2-N4 X N1-N4) > 0. */

    ret_val = dx3 * (dy2 * dz1 - dy1 * dz2) - dy3 * (dx2 * dz1 - dx1 * dz2) +
	    dz3 * (dx2 * dy1 - dx1 * dy2) > 0.;
    return ret_val;
} /* swptst_ */

GMT_LOCAL integer addnod_(integer *nst, integer *k, doublereal *x, doublereal *y, doublereal *z__, integer *list, integer *lptr, integer *lend, integer *lnew, integer *ier) {
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer l;
    static doublereal p[3], b1, b2, b3;
    static integer i1, i2, i3, kk, lp, in1, io1, io2, km1, lpf, ist, lpo1;
    static integer lpo1s;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/08/99 */

/*   This subroutine adds node K to a triangulation of the */
/* convex hull of nodes 1,...,K-1, producing a triangulation */
/* of the convex hull of nodes 1,...,K. */

/*   The algorithm consists of the following steps:  node K */
/* is located relative to the triangulation (TRFIND), its */
/* index is added to the data structure (INTADD or BDYADD), */
/* and a sequence of swaps (SWPTST and SWAP) are applied to */
/* the arcs opposite K so that all arcs incident on node K */
/* and opposite node K are locally optimal (satisfy the cir- */
/* cumcircle test).  Thus, if a Delaunay triangulation is */
/* input, a Delaunay triangulation will result. */


/* On input: */

/*       NST = Index of a node at which TRFIND begins its */
/*             search.  Search time depends on the proximity */
/*             of this node to K.  If NST < 1, the search is */
/*             begun at node K-1. */

/*       K = Nodal index (index for X, Y, Z, and LEND) of the */
/*           new node to be added.  K .GE. 4. */

/*       X,Y,Z = Arrays of length .GE. K containing Car- */
/*               tesian coordinates of the nodes. */
/*               (X(I),Y(I),Z(I)) defines node I for */
/*               I = 1,...,K. */

/* The above parameters are not altered by this routine. */

/*       LIST,LPTR,LEND,LNEW = Data structure associated with */
/*                             the triangulation of nodes 1 */
/*                             to K-1.  The array lengths are */
/*                             assumed to be large enough to */
/*                             add node K.  Refer to Subrou- */
/*                             tine TRMESH. */

/* On output: */

/*       LIST,LPTR,LEND,LNEW = Data structure updated with */
/*                             the addition of node K as the */
/*                             last entry unless IER .NE. 0 */
/*                             and IER .NE. -3, in which case */
/*                             the arrays are not altered. */

/*       IER = Error indicator: */
/*             IER =  0 if no errors were encountered. */
/*             IER = -1 if K is outside its valid range */
/*                      on input. */
/*             IER = -2 if all nodes (including K) are col- */
/*                      linear (lie on a common geodesic). */
/*             IER =  L if nodes L and K coincide for some */
/*                      L < K. */

/* Modules required by ADDNOD:  BDYADD, COVSPH, INSERT, */
/*                                INTADD, JRAND, LSTPTR, */
/*                                STORE, SWAP, SWPTST, */
/*                                TRFIND */

/* Intrinsic function called by ADDNOD:  ABS */

/* *********************************************************** */


/* Local parameters: */

/* B1,B2,B3 = Unnormalized barycentric coordinates returned */
/*              by TRFIND. */
/* I1,I2,I3 = Vertex indexes of a triangle containing K */
/* IN1 =      Vertex opposite K:  first neighbor of IO2 */
/*              that precedes IO1.  IN1,IO1,IO2 are in */
/*              counterclockwise order. */
/* IO1,IO2 =  Adjacent neighbors of K defining an arc to */
/*              be tested for a swap */
/* IST =      Index of node at which TRFIND begins its search */
/* KK =       Local copy of K */
/* KM1 =      K-1 */
/* L =        Vertex index (I1, I2, or I3) returned in IER */
/*              if node K coincides with a vertex */
/* LP =       LIST pointer */
/* LPF =      LIST pointer to the first neighbor of K */
/* LPO1 =     LIST pointer to IO1 */
/* LPO1S =    Saved value of LPO1 */
/* P =        Cartesian coordinates of node K */

    /* Parameter adjustments */
    --lend;
    --z__;
    --y;
    --x;
    --list;
    --lptr;

    /* Function Body */
    kk = *k;
    if (kk < 4) {
	goto L3;
    }

/* Initialization: */

    km1 = kk - 1;
    ist = *nst;
    if (ist < 1) {
	ist = km1;
    }
    p[0] = x[kk];
    p[1] = y[kk];
    p[2] = z__[kk];

/* Find a triangle (I1,I2,I3) containing K or the rightmost */
/*   (I1) and leftmost (I2) visible boundary nodes as viewed */
/*   from node K. */

    trfind_(&ist, p, &km1, &x[1], &y[1], &z__[1], &list[1], &lptr[1], &lend[1]
	    , &b1, &b2, &b3, &i1, &i2, &i3);

/*   Test for collinear or duplicate nodes. */

    if (i1 == 0) {
	goto L4;
    }
    if (i3 != 0) {
	l = i1;
	if (p[0] == x[l] && p[1] == y[l] && p[2] == z__[l]) {
	    goto L5;
	}
	l = i2;
	if (p[0] == x[l] && p[1] == y[l] && p[2] == z__[l]) {
	    goto L5;
	}
	l = i3;
	if (p[0] == x[l] && p[1] == y[l] && p[2] == z__[l]) {
	    goto L5;
	}
	intadd_(&kk, &i1, &i2, &i3, &list[1], &lptr[1], &lend[1], lnew);
    } else {
	if (i1 != i2) {
	    bdyadd_(&kk, &i1, &i2, &list[1], &lptr[1], &lend[1], lnew);
	} else {
	    covsph_(&kk, &i1, &list[1], &lptr[1], &lend[1], lnew);
	}
    }
    *ier = 0;

/* Initialize variables for optimization of the */
/*   triangulation. */

    lp = lend[kk];
    lpf = lptr[lp];
    io2 = list[lpf];
    lpo1 = lptr[lpf];
    io1 = (i__1 = list[lpo1], int64_abs(i__1));

/* Begin loop:  find the node opposite K. */

L1:
    lp = lstptr_(&lend[io1], &io2, &list[1], &lptr[1]);
    if (list[lp] < 0) {
	goto L2;
    }
    lp = lptr[lp];
    in1 = (i__1 = list[lp], int64_abs(i__1));

/* Swap test:  if a swap occurs, two new arcs are */
/*             opposite K and must be tested. */

    lpo1s = lpo1;
    if (! swptst_(&in1, &kk, &io1, &io2, &x[1], &y[1], &z__[1])) {
	goto L2;
    }
    swap_(&in1, &kk, &io1, &io2, &list[1], &lptr[1], &lend[1], &lpo1);
    if (lpo1 == 0) {

/*   A swap is not possible because KK and IN1 are already */
/*     adjacent.  This error in SWPTST only occurs in the */
/*     neutral case and when there are nearly duplicate */
/*     nodes. */

	lpo1 = lpo1s;
	goto L2;
    }
    io1 = in1;
    goto L1;

/* No swap occurred.  Test for termination and reset */
/*   IO2 and IO1. */

L2:
    if (lpo1 == lpf || list[lpo1] < 0) {
	return 0;
    }
    io2 = io1;
    lpo1 = lptr[lpo1];
    io1 = (i__1 = list[lpo1], int64_abs(i__1));
    goto L1;

/* KK < 4. */

L3:
    *ier = -1;
    return 0;

/* All nodes are collinear. */

L4:
    *ier = -2;
    return 0;

/* Nodes L and K coincide. */

L5:
    *ier = l;
    return 0;
} /* addnod_ */

GMT_LOCAL doublereal areas_(doublereal *v1, doublereal *v2, doublereal *v3) {
    /* System generated locals */
    doublereal ret_val;

    /* Builtin functions */
    double sqrt(doublereal), acos(doublereal);

    /* Local variables */
    static integer i__;
    static doublereal a1, a2, a3, s12, s31, s23, u12[3], u23[3], u31[3], ca1,
	    ca2, ca3, dv1[3], dv2[3], dv3[3];


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   09/18/90 */

/*   This function returns the area of a spherical triangle */
/* on the unit sphere. */


/* On input: */

/*       V1,V2,V3 = Arrays of length 3 containing the Carte- */
/*                  sian coordinates of unit vectors (the */
/*                  three triangle vertices in any order). */
/*                  These vectors, if nonzero, are implicitly */
/*                  scaled to have length 1. */

/* Input parameters are not altered by this function. */

/* On output: */

/*       AREAS = Area of the spherical triangle defined by */
/*               V1, V2, and V3 in the range 0 to 2*PI (the */
/*               area of a hemisphere).  AREAS = 0 (or 2*PI) */
/*               if and only if V1, V2, and V3 lie in (or */
/*               close to) a plane containing the origin. */

/* Modules required by AREAS:  None */

/* Intrinsic functions called by AREAS:  ACOS, DBLE, REAL, */
/*                                         SQRT */

/* *********************************************************** */


/* Local parameters: */

/* A1,A2,A3 =    Interior angles of the spherical triangle */
/* CA1,CA2,CA3 = cos(A1), cos(A2), and cos(A3), respectively */
/* DV1,DV2,DV3 = Double Precision copies of V1, V2, and V3 */
/* I =           DO-loop index and index for Uij */
/* S12,S23,S31 = Sum of squared components of U12, U23, U31 */
/* U12,U23,U31 = Unit normal vectors to the planes defined by */
/*                 pairs of triangle vertices */

    /* Parameter adjustments */
    --v3;
    --v2;
    --v1;

    /* Function Body */
    for (i__ = 1; i__ <= 3; ++i__) {
	dv1[i__ - 1] = v1[i__];
	dv2[i__ - 1] = v2[i__];
	dv3[i__ - 1] = v3[i__];
/* L1: */
    }

/* Compute cross products Uij = Vi X Vj. */

    u12[0] = dv1[1] * dv2[2] - dv1[2] * dv2[1];
    u12[1] = dv1[2] * dv2[0] - dv1[0] * dv2[2];
    u12[2] = dv1[0] * dv2[1] - dv1[1] * dv2[0];

    u23[0] = dv2[1] * dv3[2] - dv2[2] * dv3[1];
    u23[1] = dv2[2] * dv3[0] - dv2[0] * dv3[2];
    u23[2] = dv2[0] * dv3[1] - dv2[1] * dv3[0];

    u31[0] = dv3[1] * dv1[2] - dv3[2] * dv1[1];
    u31[1] = dv3[2] * dv1[0] - dv3[0] * dv1[2];
    u31[2] = dv3[0] * dv1[1] - dv3[1] * dv1[0];

/* Normalize Uij to unit vectors. */

    s12 = 0.;
    s23 = 0.;
    s31 = 0.;
    for (i__ = 1; i__ <= 3; ++i__) {
	s12 += u12[i__ - 1] * u12[i__ - 1];
	s23 += u23[i__ - 1] * u23[i__ - 1];
	s31 += u31[i__ - 1] * u31[i__ - 1];
/* L2: */
    }

/* Test for a degenerate triangle associated with collinear */
/*   vertices. */

    if (s12 == 0. || s23 == 0. || s31 == 0.) {
	ret_val = 0.;
	return ret_val;
    }
    s12 = sqrt(s12);
    s23 = sqrt(s23);
    s31 = sqrt(s31);
    for (i__ = 1; i__ <= 3; ++i__) {
	u12[i__ - 1] /= s12;
	u23[i__ - 1] /= s23;
	u31[i__ - 1] /= s31;
/* L3: */
    }

/* Compute interior angles Ai as the dihedral angles between */
/*   planes: */
/*           CA1 = cos(A1) = -<U12,U31> */
/*           CA2 = cos(A2) = -<U23,U12> */
/*           CA3 = cos(A3) = -<U31,U23> */

    ca1 = -u12[0] * u31[0] - u12[1] * u31[1] - u12[2] * u31[2];
    ca2 = -u23[0] * u12[0] - u23[1] * u12[1] - u23[2] * u12[2];
    ca3 = -u31[0] * u23[0] - u31[1] * u23[1] - u31[2] * u23[2];
    if (ca1 < -1.) {
	ca1 = -1.;
    }
    if (ca1 > 1.) {
	ca1 = 1.;
    }
    if (ca2 < -1.) {
	ca2 = -1.;
    }
    if (ca2 > 1.) {
	ca2 = 1.;
    }
    if (ca3 < -1.) {
	ca3 = -1.;
    }
    if (ca3 > 1.) {
	ca3 = 1.;
    }
    a1 = acos(ca1);
    a2 = acos(ca2);
    a3 = acos(ca3);

/* Compute AREAS = A1 + A2 + A3 - PI. */

    ret_val = a1 + a2 + a3 - acos(-1.);
    if (ret_val < 0.) {
	ret_val = 0.;
    }
    return ret_val;
} /* areas_ */

GMT_LOCAL integer circum_(doublereal *v1, doublereal *v2, doublereal *v3, doublereal *c__, integer *ier) {
    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static integer i__;
    static doublereal e1[3], e2[3], cu[3], cnorm;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   06/29/95 */

/*   This subroutine returns the circumcenter of a spherical */
/* triangle on the unit sphere:  the point on the sphere sur- */
/* face that is equally distant from the three triangle */
/* vertices and lies in the same hemisphere, where distance */
/* is taken to be arc-length on the sphere surface. */


/* On input: */

/*       V1,V2,V3 = Arrays of length 3 containing the Carte- */
/*                  sian coordinates of the three triangle */
/*                  vertices (unit vectors) in CCW order. */

/* The above parameters are not altered by this routine. */

/*       C = Array of length 3. */

/* On output: */

/*       C = Cartesian coordinates of the circumcenter unless */
/*           IER > 0, in which case C is not defined.  C = */
/*           (V2-V1) X (V3-V1) normalized to a unit vector. */

/*       IER = Error indicator: */
/*             IER = 0 if no errors were encountered. */
/*             IER = 1 if V1, V2, and V3 lie on a common */
/*                     line:  (V2-V1) X (V3-V1) = 0. */
/*             (The vertices are not tested for validity.) */

/* Modules required by CIRCUM:  None */

/* Intrinsic function called by CIRCUM:  SQRT */

/* *********************************************************** */


/* Local parameters: */

/* CNORM = Norm of CU:  used to compute C */
/* CU =    Scalar multiple of C:  E1 X E2 */
/* E1,E2 = Edges of the underlying planar triangle: */
/*           V2-V1 and V3-V1, respectively */
/* I =     DO-loop index */

    /* Parameter adjustments */
    --c__;
    --v3;
    --v2;
    --v1;

    /* Function Body */
    for (i__ = 1; i__ <= 3; ++i__) {
	e1[i__ - 1] = v2[i__] - v1[i__];
	e2[i__ - 1] = v3[i__] - v1[i__];
/* L1: */
    }

/* Compute CU = E1 X E2 and CNORM**2. */

    cu[0] = e1[1] * e2[2] - e1[2] * e2[1];
    cu[1] = e1[2] * e2[0] - e1[0] * e2[2];
    cu[2] = e1[0] * e2[1] - e1[1] * e2[0];
    cnorm = cu[0] * cu[0] + cu[1] * cu[1] + cu[2] * cu[2];

/* The vertices lie on a common line if and only if CU is */
/*   the zero vector. */

    if (cnorm != 0.) {

/*   No error:  compute C. */

	cnorm = sqrt(cnorm);
	for (i__ = 1; i__ <= 3; ++i__) {
	    c__[i__] = cu[i__ - 1] / cnorm;
/* L2: */
	}
	*ier = 0;
    } else {

/*   CU = 0. */

	*ier = 1;
    }
    return 0;
} /* circum_ */

GMT_LOCAL integer crlist_(integer *n, integer *ncol, doublereal *x, doublereal *y, doublereal *z__, integer *list, integer *lend, integer *lptr, integer *lnew, integer *ltri, integer *listc, integer *nb, doublereal *xc, doublereal *yc, doublereal *zc, doublereal *rc, integer *ier) {
    /* System generated locals */
    integer i__1, i__2;

    /* Builtin functions */
    double acos(doublereal);

    /* Local variables */
    static doublereal c__[3], t;
    static integer i1, i2, i3, i4, n0, n1, n2, n3, n4;
    static doublereal v1[3], v2[3], v3[3];
    static integer lp, kt, nn, nt, nm2, kt1, kt2, kt11, kt12, kt21, kt22, lpl,
	     lpn;
    static logical swp;
    static integer ierr;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/05/98 */

/*   Given a Delaunay triangulation of nodes on the surface */
/* of the unit sphere, this subroutine returns the set of */
/* triangle circumcenters corresponding to Voronoi vertices, */
/* along with the circumradii and a list of triangle indexes */
/* LISTC stored in one-to-one correspondence with LIST/LPTR */
/* entries. */

/*   A triangle circumcenter is the point (unit vector) lying */
/* at the same angular distance from the three vertices and */
/* contained in the same hemisphere as the vertices.  (Note */
/* that the negative of a circumcenter is also equidistant */
/* from the vertices.)  If the triangulation covers the sur- */
/* face, the Voronoi vertices are the circumcenters of the */
/* triangles in the Delaunay triangulation.  LPTR, LEND, and */
/* LNEW are not altered in this case. */

/*   On the other hand, if the nodes are contained in a sin- */
/* gle hemisphere, the triangulation is implicitly extended */
/* to the entire surface by adding pseudo-arcs (of length */
/* greater than 180 degrees) between boundary nodes forming */
/* pseudo-triangles whose 'circumcenters' are included in the */
/* list.  This extension to the triangulation actually con- */
/* sists of a triangulation of the set of boundary nodes in */
/* which the swap test is reversed (a non-empty circumcircle */
/* test).  The negative circumcenters are stored as the */
/* pseudo-triangle 'circumcenters'.  LISTC, LPTR, LEND, and */
/* LNEW contain a data structure corresponding to the ex- */
/* tended triangulation (Voronoi diagram), but LIST is not */
/* altered in this case.  Thus, if it is necessary to retain */
/* the original (unextended) triangulation data structure, */
/* copies of LPTR and LNEW must be saved before calling this */
/* routine. */


/* On input: */

/*       N = Number of nodes in the triangulation.  N .GE. 3. */
/*           Note that, if N = 3, there are only two Voronoi */
/*           vertices separated by 180 degrees, and the */
/*           Voronoi regions are not well defined. */

/*       NCOL = Number of columns reserved for LTRI.  This */
/*              must be at least NB-2, where NB is the number */
/*              of boundary nodes. */

/*       X,Y,Z = Arrays of length N containing the Cartesian */
/*               coordinates of the nodes (unit vectors). */

/*       LIST = Integer array containing the set of adjacency */
/*              lists.  Refer to Subroutine TRMESH. */

/*       LEND = Set of pointers to ends of adjacency lists. */
/*              Refer to Subroutine TRMESH. */

/* The above parameters are not altered by this routine. */

/*       LPTR = Array of pointers associated with LIST.  Re- */
/*              fer to Subroutine TRMESH. */

/*       LNEW = Pointer to the first empty location in LIST */
/*              and LPTR (list length plus one). */

/*       LTRI = Integer work space array dimensioned 6 by */
/*              NCOL, or unused dummy parameter if NB = 0. */

/*       LISTC = Integer array of length at least 3*NT, where */
/*               NT = 2*N-4 is the number of triangles in the */
/*               triangulation (after extending it to cover */
/*               the entire surface if necessary). */

/*       XC,YC,ZC,RC = Arrays of length NT = 2*N-4. */

/* On output: */

/*       LPTR = Array of pointers associated with LISTC: */
/*              updated for the addition of pseudo-triangles */
/*              if the original triangulation contains */
/*              boundary nodes (NB > 0). */

/*       LNEW = Pointer to the first empty location in LISTC */
/*              and LPTR (list length plus one).  LNEW is not */
/*              altered if NB = 0. */

/*       LTRI = Triangle list whose first NB-2 columns con- */
/*              tain the indexes of a clockwise-ordered */
/*              sequence of vertices (first three rows) */
/*              followed by the LTRI column indexes of the */
/*              triangles opposite the vertices (or 0 */
/*              denoting the exterior region) in the last */
/*              three rows.  This array is not generally of */
/*              any use. */

/*       LISTC = Array containing triangle indexes (indexes */
/*               to XC, YC, ZC, and RC) stored in 1-1 corres- */
/*               pondence with LIST/LPTR entries (or entries */
/*               that would be stored in LIST for the */
/*               extended triangulation):  the index of tri- */
/*               angle (N1,N2,N3) is stored in LISTC(K), */
/*               LISTC(L), and LISTC(M), where LIST(K), */
/*               LIST(L), and LIST(M) are the indexes of N2 */
/*               as a neighbor of N1, N3 as a neighbor of N2, */
/*               and N1 as a neighbor of N3.  The Voronoi */
/*               region associated with a node is defined by */
/*               the CCW-ordered sequence of circumcenters in */
/*               one-to-one correspondence with its adjacency */
/*               list (in the extended triangulation). */

/*       NB = Number of boundary nodes unless IER = 1. */

/*       XC,YC,ZC = Arrays containing the Cartesian coordi- */
/*                  nates of the triangle circumcenters */
/*                  (Voronoi vertices).  XC(I)**2 + YC(I)**2 */
/*                  + ZC(I)**2 = 1.  The first NB-2 entries */
/*                  correspond to pseudo-triangles if NB > 0. */

/*       RC = Array containing circumradii (the arc lengths */
/*            or angles between the circumcenters and associ- */
/*            ated triangle vertices) in 1-1 correspondence */
/*            with circumcenters. */

/*       IER = Error indicator: */
/*             IER = 0 if no errors were encountered. */
/*             IER = 1 if N < 3. */
/*             IER = 2 if NCOL < NB-2. */
/*             IER = 3 if a triangle is degenerate (has ver- */
/*                     tices lying on a common geodesic). */

/* Modules required by CRLIST:  CIRCUM, LSTPTR, SWPTST */

/* Intrinsic functions called by CRLIST:  ABS, ACOS */

/* *********************************************************** */


/* Local parameters: */

/* C =         Circumcenter returned by Subroutine CIRCUM */
/* I1,I2,I3 =  Permutation of (1,2,3):  LTRI row indexes */
/* I4 =        LTRI row index in the range 1 to 3 */
/* IERR =      Error flag for calls to CIRCUM */
/* KT =        Triangle index */
/* KT1,KT2 =   Indexes of a pair of adjacent pseudo-triangles */
/* KT11,KT12 = Indexes of the pseudo-triangles opposite N1 */
/*               and N2 as vertices of KT1 */
/* KT21,KT22 = Indexes of the pseudo-triangles opposite N1 */
/*               and N2 as vertices of KT2 */
/* LP,LPN =    LIST pointers */
/* LPL =       LIST pointer of the last neighbor of N1 */
/* N0 =        Index of the first boundary node (initial */
/*               value of N1) in the loop on boundary nodes */
/*               used to store the pseudo-triangle indexes */
/*               in LISTC */
/* N1,N2,N3 =  Nodal indexes defining a triangle (CCW order) */
/*               or pseudo-triangle (clockwise order) */
/* N4 =        Index of the node opposite N2 -> N1 */
/* NM2 =       N-2 */
/* NN =        Local copy of N */
/* NT =        Number of pseudo-triangles:  NB-2 */
/* SWP =       Logical variable set to true in each optimiza- */
/*               tion loop (loop on pseudo-arcs) if a swap */
/*               is performed */
/* V1,V2,V3 =  Vertices of triangle KT = (N1,N2,N3) sent to */
/*               Subroutine CIRCUM */

    /* Parameter adjustments */
    --lend;
    --z__;
    --y;
    --x;
    ltri -= 7;
    --list;
    --lptr;
    --listc;
    --xc;
    --yc;
    --zc;
    --rc;

    /* Function Body */
    nn = *n;
    *nb = 0;
    nt = 0;
    if (nn < 3) {
	goto L21;
    }

/* Search for a boundary node N1. */

    i__1 = nn;
    for (n1 = 1; n1 <= i__1; ++n1) {
	lp = lend[n1];
	if (list[lp] < 0) {
	    goto L2;
	}
/* L1: */
    }

/* The triangulation already covers the sphere. */

    goto L9;

/* There are NB .GE. 3 boundary nodes.  Add NB-2 pseudo- */
/*   triangles (N1,N2,N3) by connecting N3 to the NB-3 */
/*   boundary nodes to which it is not already adjacent. */

/*   Set N3 and N2 to the first and last neighbors, */
/*     respectively, of N1. */

L2:
    n2 = -list[lp];
    lp = lptr[lp];
    n3 = list[lp];

/*   Loop on boundary arcs N1 -> N2 in clockwise order, */
/*     storing triangles (N1,N2,N3) in column NT of LTRI */
/*     along with the indexes of the triangles opposite */
/*     the vertices. */

L3:
    ++nt;
    if (nt <= *ncol) {
	ltri[nt * 6 + 1] = n1;
	ltri[nt * 6 + 2] = n2;
	ltri[nt * 6 + 3] = n3;
	ltri[nt * 6 + 4] = nt + 1;
	ltri[nt * 6 + 5] = nt - 1;
	ltri[nt * 6 + 6] = 0;
    }
    n1 = n2;
    lp = lend[n1];
    n2 = -list[lp];
    if (n2 != n3) {
	goto L3;
    }

    *nb = nt + 2;
    if (*ncol < nt) {
	goto L22;
    }
    ltri[nt * 6 + 4] = 0;
    if (nt == 1) {
	goto L7;
    }

/* Optimize the exterior triangulation (set of pseudo- */
/*   triangles) by applying swaps to the pseudo-arcs N1-N2 */
/*   (pairs of adjacent pseudo-triangles KT1 and KT2 > KT1). */
/*   The loop on pseudo-arcs is repeated until no swaps are */
/*   performed. */

L4:
    swp = FALSE_;
    i__1 = nt - 1;
    for (kt1 = 1; kt1 <= i__1; ++kt1) {
	for (i3 = 1; i3 <= 3; ++i3) {
	    kt2 = ltri[i3 + 3 + kt1 * 6];
	    if (kt2 <= kt1) {
		goto L5;
	    }

/*   The LTRI row indexes (I1,I2,I3) of triangle KT1 = */
/*     (N1,N2,N3) are a cyclical permutation of (1,2,3). */

	    if (i3 == 1) {
		i1 = 2;
		i2 = 3;
	    } else if (i3 == 2) {
		i1 = 3;
		i2 = 1;
	    } else {
		i1 = 1;
		i2 = 2;
	    }
	    n1 = ltri[i1 + kt1 * 6];
	    n2 = ltri[i2 + kt1 * 6];
	    n3 = ltri[i3 + kt1 * 6];

/*   KT2 = (N2,N1,N4) for N4 = LTRI(I,KT2), where */
/*     LTRI(I+3,KT2) = KT1. */

	    if (ltri[kt2 * 6 + 4] == kt1) {
		i4 = 1;
	    } else if (ltri[kt2 * 6 + 5] == kt1) {
		i4 = 2;
	    } else {
		i4 = 3;
	    }
	    n4 = ltri[i4 + kt2 * 6];

/*   The empty circumcircle test is reversed for the pseudo- */
/*     triangles.  The reversal is implicit in the clockwise */
/*     ordering of the vertices. */

	    if (! swptst_(&n1, &n2, &n3, &n4, &x[1], &y[1], &z__[1])) {
		goto L5;
	    }

/*   Swap arc N1-N2 for N3-N4.  KTij is the triangle opposite */
/*     Nj as a vertex of KTi. */

	    swp = TRUE_;
	    kt11 = ltri[i1 + 3 + kt1 * 6];
	    kt12 = ltri[i2 + 3 + kt1 * 6];
	    if (i4 == 1) {
		i2 = 2;
		i1 = 3;
	    } else if (i4 == 2) {
		i2 = 3;
		i1 = 1;
	    } else {
		i2 = 1;
		i1 = 2;
	    }
	    kt21 = ltri[i1 + 3 + kt2 * 6];
	    kt22 = ltri[i2 + 3 + kt2 * 6];
	    ltri[kt1 * 6 + 1] = n4;
	    ltri[kt1 * 6 + 2] = n3;
	    ltri[kt1 * 6 + 3] = n1;
	    ltri[kt1 * 6 + 4] = kt12;
	    ltri[kt1 * 6 + 5] = kt22;
	    ltri[kt1 * 6 + 6] = kt2;
	    ltri[kt2 * 6 + 1] = n3;
	    ltri[kt2 * 6 + 2] = n4;
	    ltri[kt2 * 6 + 3] = n2;
	    ltri[kt2 * 6 + 4] = kt21;
	    ltri[kt2 * 6 + 5] = kt11;
	    ltri[kt2 * 6 + 6] = kt1;

/*   Correct the KT11 and KT22 entries that changed. */

	    if (kt11 != 0) {
		i4 = 4;
		if (ltri[kt11 * 6 + 4] != kt1) {
		    i4 = 5;
		    if (ltri[kt11 * 6 + 5] != kt1) {
			i4 = 6;
		    }
		}
		ltri[i4 + kt11 * 6] = kt2;
	    }
	    if (kt22 != 0) {
		i4 = 4;
		if (ltri[kt22 * 6 + 4] != kt2) {
		    i4 = 5;
		    if (ltri[kt22 * 6 + 5] != kt2) {
			i4 = 6;
		    }
		}
		ltri[i4 + kt22 * 6] = kt1;
	    }
L5:
	    ;
	}
/* L6: */
    }
    if (swp) {
	goto L4;
    }

/* Compute and store the negative circumcenters and radii of */
/*   the pseudo-triangles in the first NT positions. */

L7:
    i__1 = nt;
    for (kt = 1; kt <= i__1; ++kt) {
	n1 = ltri[kt * 6 + 1];
	n2 = ltri[kt * 6 + 2];
	n3 = ltri[kt * 6 + 3];
	v1[0] = x[n1];
	v1[1] = y[n1];
	v1[2] = z__[n1];
	v2[0] = x[n2];
	v2[1] = y[n2];
	v2[2] = z__[n2];
	v3[0] = x[n3];
	v3[1] = y[n3];
	v3[2] = z__[n3];
	circum_(v1, v2, v3, c__, &ierr);
	if (ierr != 0) {
	    goto L23;
	}

/*   Store the negative circumcenter and radius (computed */
/*     from <V1,C>). */

	xc[kt] = c__[0];
	yc[kt] = c__[1];
	zc[kt] = c__[2];
	t = v1[0] * c__[0] + v1[1] * c__[1] + v1[2] * c__[2];
	if (t < -1.) {
	    t = -1.;
	}
	if (t > 1.) {
	    t = 1.;
	}
	rc[kt] = acos(t);
/* L8: */
    }

/* Compute and store the circumcenters and radii of the */
/*   actual triangles in positions KT = NT+1, NT+2, ... */
/*   Also, store the triangle indexes KT in the appropriate */
/*   LISTC positions. */

L9:
    kt = nt;

/*   Loop on nodes N1. */

    nm2 = nn - 2;
    i__1 = nm2;
    for (n1 = 1; n1 <= i__1; ++n1) {
	lpl = lend[n1];
	lp = lpl;
	n3 = list[lp];

/*   Loop on adjacent neighbors N2,N3 of N1 for which N2 > N1 */
/*     and N3 > N1. */

L10:
	lp = lptr[lp];
	n2 = n3;
	n3 = (i__2 = list[lp], int64_abs(i__2));
	if (n2 <= n1 || n3 <= n1) {
	    goto L11;
	}
	++kt;

/*   Compute the circumcenter C of triangle KT = (N1,N2,N3). */

	v1[0] = x[n1];
	v1[1] = y[n1];
	v1[2] = z__[n1];
	v2[0] = x[n2];
	v2[1] = y[n2];
	v2[2] = z__[n2];
	v3[0] = x[n3];
	v3[1] = y[n3];
	v3[2] = z__[n3];
	circum_(v1, v2, v3, c__, &ierr);
	if (ierr != 0) {
	    goto L23;
	}

/*   Store the circumcenter, radius and triangle index. */

	xc[kt] = c__[0];
	yc[kt] = c__[1];
	zc[kt] = c__[2];
	t = v1[0] * c__[0] + v1[1] * c__[1] + v1[2] * c__[2];
	if (t < -1.) {
	    t = -1.;
	}
	if (t > 1.) {
	    t = 1.;
	}
	rc[kt] = acos(t);

/*   Store KT in LISTC(LPN), where Abs(LIST(LPN)) is the */
/*     index of N2 as a neighbor of N1, N3 as a neighbor */
/*     of N2, and N1 as a neighbor of N3. */

	lpn = lstptr_(&lpl, &n2, &list[1], &lptr[1]);
	listc[lpn] = kt;
	lpn = lstptr_(&lend[n2], &n3, &list[1], &lptr[1]);
	listc[lpn] = kt;
	lpn = lstptr_(&lend[n3], &n1, &list[1], &lptr[1]);
	listc[lpn] = kt;
L11:
	if (lp != lpl) {
	    goto L10;
	}
/* L12: */
    }
    if (nt == 0) {
	goto L20;
    }

/* Store the first NT triangle indexes in LISTC. */

/*   Find a boundary triangle KT1 = (N1,N2,N3) with a */
/*     boundary arc opposite N3. */

    kt1 = 0;
L13:
    ++kt1;
    if (ltri[kt1 * 6 + 4] == 0) {
	i1 = 2;
	i2 = 3;
	i3 = 1;
	goto L14;
    } else if (ltri[kt1 * 6 + 5] == 0) {
	i1 = 3;
	i2 = 1;
	i3 = 2;
	goto L14;
    } else if (ltri[kt1 * 6 + 6] == 0) {
	i1 = 1;
	i2 = 2;
	i3 = 3;
	goto L14;
    }
    goto L13;
L14:
    n1 = ltri[i1 + kt1 * 6];
    n0 = n1;

/*   Loop on boundary nodes N1 in CCW order, storing the */
/*     indexes of the clockwise-ordered sequence of triangles */
/*     that contain N1.  The first triangle overwrites the */
/*     last neighbor position, and the remaining triangles, */
/*     if any, are appended to N1's adjacency list. */

/*   A pointer to the first neighbor of N1 is saved in LPN. */

L15:
    lp = lend[n1];
    lpn = lptr[lp];
    listc[lp] = kt1;

/*   Loop on triangles KT2 containing N1. */

L16:
    kt2 = ltri[i2 + 3 + kt1 * 6];
    if (kt2 != 0) {

/*   Append KT2 to N1's triangle list. */

	lptr[lp] = *lnew;
	lp = *lnew;
	listc[lp] = kt2;
	++(*lnew);

/*   Set KT1 to KT2 and update (I1,I2,I3) such that */
/*     LTRI(I1,KT1) = N1. */

	kt1 = kt2;
	if (ltri[kt1 * 6 + 1] == n1) {
	    i1 = 1;
	    i2 = 2;
	    i3 = 3;
	} else if (ltri[kt1 * 6 + 2] == n1) {
	    i1 = 2;
	    i2 = 3;
	    i3 = 1;
	} else {
	    i1 = 3;
	    i2 = 1;
	    i3 = 2;
	}
	goto L16;
    }

/*   Store the saved first-triangle pointer in LPTR(LP), set */
/*     N1 to the next boundary node, test for termination, */
/*     and permute the indexes:  the last triangle containing */
/*     a boundary node is the first triangle containing the */
/*     next boundary node. */

    lptr[lp] = lpn;
    n1 = ltri[i3 + kt1 * 6];
    if (n1 != n0) {
	i4 = i3;
	i3 = i2;
	i2 = i1;
	i1 = i4;
	goto L15;
    }

/* No errors encountered. */

L20:
    *ier = 0;
    return 0;

/* N < 3. */

L21:
    *ier = 1;
    return 0;

/* Insufficient space reserved for LTRI. */

L22:
    *ier = 2;
    return 0;

/* Error flag returned by CIRCUM: KT indexes a null triangle. */

L23:
    *ier = 3;
    return 0;
} /* crlist_ */

GMT_LOCAL logical left_(doublereal *x1, doublereal *y1, doublereal *z1, doublereal *x2, doublereal *y2, doublereal *z2, doublereal *x0, doublereal *y0, doublereal *z0) {
    /* System generated locals */
    logical ret_val;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/15/96 */

/*   This function determines whether node N0 is in the */
/* (closed) left hemisphere defined by the plane containing */
/* N1, N2, and the origin, where left is defined relative to */
/* an observer at N1 facing N2. */


/* On input: */

/*       X1,Y1,Z1 = Coordinates of N1. */

/*       X2,Y2,Z2 = Coordinates of N2. */

/*       X0,Y0,Z0 = Coordinates of N0. */

/* Input parameters are not altered by this function. */

/* On output: */

/*       LEFT = true if and only if N0 is in the closed */
/*              left hemisphere. */

/* Modules required by LEFT:  None */

/* *********************************************************** */

/* LEFT = true if <N0,N1 X N2> = det(N0,N1,N2) .GE. 0. */

    ret_val = *x0 * (*y1 * *z2 - *y2 * *z1) - *y0 * (*x1 * *z2 - *x2 * *z1) +
	    *z0 * (*x1 * *y2 - *x2 * *y1) >= 0.;
    return ret_val;
} /* left_ */

GMT_LOCAL integer trlist_(integer *n, integer *list, integer *lptr, integer *lend, integer *nrow, integer *nt, integer *ltri, integer *ier) {
    /* System generated locals */
    integer ltri_dim1, ltri_offset, i__1, i__2;

    /* Local variables */
    static integer i__, j, i1, i2, i3, n1, n2, n3, ka, kn, lp, kt, nm2, lp2,
	    lpl, isv;
    static logical arcs;
    static integer lpln1;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/20/96 */

/*   This subroutine converts a triangulation data structure */
/* from the linked list created by Subroutine TRMESH to a */
/* triangle list. */

/* On input: */

/*       N = Number of nodes in the triangulation.  N .GE. 3. */

/*       LIST,LPTR,LEND = Linked list data structure defin- */
/*                        ing the triangulation.  Refer to */
/*                        Subroutine TRMESH. */

/*       NROW = Number of rows (entries per triangle) re- */
/*              served for the triangle list LTRI.  The value */
/*              must be 6 if only the vertex indexes and */
/*              neighboring triangle indexes are to be */
/*              stored, or 9 if arc indexes are also to be */
/*              assigned and stored.  Refer to LTRI. */

/* The above parameters are not altered by this routine. */

/*       LTRI = Integer array of length at least NROW*NT, */
/*              where NT is at most 2N-4.  (A sufficient */
/*              length is 12N if NROW=6 or 18N if NROW=9.) */

/* On output: */

/*       NT = Number of triangles in the triangulation unless */
/*            IER .NE. 0, in which case NT = 0.  NT = 2N-NB-2 */
/*            if NB .GE. 3 or 2N-4 if NB = 0, where NB is the */
/*            number of boundary nodes. */

/*       LTRI = NROW by NT array whose J-th column contains */
/*              the vertex nodal indexes (first three rows), */
/*              neighboring triangle indexes (second three */
/*              rows), and, if NROW = 9, arc indexes (last */
/*              three rows) associated with triangle J for */
/*              J = 1,...,NT.  The vertices are ordered */
/*              counterclockwise with the first vertex taken */
/*              to be the one with smallest index.  Thus, */
/*              LTRI(2,J) and LTRI(3,J) are larger than */
/*              LTRI(1,J) and index adjacent neighbors of */
/*              node LTRI(1,J).  For I = 1,2,3, LTRI(I+3,J) */
/*              and LTRI(I+6,J) index the triangle and arc, */
/*              respectively, which are opposite (not shared */
/*              by) node LTRI(I,J), with LTRI(I+3,J) = 0 if */
/*              LTRI(I+6,J) indexes a boundary arc.  Vertex */
/*              indexes range from 1 to N, triangle indexes */
/*              from 0 to NT, and, if included, arc indexes */
/*              from 1 to NA, where NA = 3N-NB-3 if NB .GE. 3 */
/*              or 3N-6 if NB = 0.  The triangles are or- */
/*              dered on first (smallest) vertex indexes. */

/*       IER = Error indicator. */
/*             IER = 0 if no errors were encountered. */
/*             IER = 1 if N or NROW is outside its valid */
/*                     range on input. */
/*             IER = 2 if the triangulation data structure */
/*                     (LIST,LPTR,LEND) is invalid.  Note, */
/*                     however, that these arrays are not */
/*                     completely tested for validity. */

/* Modules required by TRLIST:  None */

/* Intrinsic function called by TRLIST:  ABS */

/* *********************************************************** */


/* Local parameters: */

/* ARCS =     Logical variable with value true if are */
/*              indexes are to be stored */
/* I,J =      LTRI row indexes (1 to 3) associated with */
/*              triangles KT and KN, respectively */
/* I1,I2,I3 = Nodal indexes of triangle KN */
/* ISV =      Variable used to permute indexes I1,I2,I3 */
/* KA =       Arc index and number of currently stored arcs */
/* KN =       Index of the triangle that shares arc I1-I2 */
/*              with KT */
/* KT =       Triangle index and number of currently stored */
/*              triangles */
/* LP =       LIST pointer */
/* LP2 =      Pointer to N2 as a neighbor of N1 */
/* LPL =      Pointer to the last neighbor of I1 */
/* LPLN1 =    Pointer to the last neighbor of N1 */
/* N1,N2,N3 = Nodal indexes of triangle KT */
/* NM2 =      N-2 */


/* Test for invalid input parameters. */

    /* Parameter adjustments */
    --lend;
    --list;
    --lptr;
    ltri_dim1 = *nrow;
    ltri_offset = 1 + ltri_dim1;
    ltri -= ltri_offset;

    /* Function Body */
if (*n < 3 || (*nrow != 6 && *nrow != 9)) {
	goto L11;
    }

/* Initialize parameters for loop on triangles KT = (N1,N2, */
/*   N3), where N1 < N2 and N1 < N3. */

/*   ARCS = true if arc indexes are to be stored. */
/*   KA,KT = Numbers of currently stored arcs and triangles. */
/*   NM2 = Upper bound on candidates for N1. */

    arcs = *nrow == 9;
    ka = 0;
    kt = 0;
    nm2 = *n - 2;

/* Loop on nodes N1. */

    i__1 = nm2;
    for (n1 = 1; n1 <= i__1; ++n1) {

/* Loop on pairs of adjacent neighbors (N2,N3).  LPLN1 points */
/*   to the last neighbor of N1, and LP2 points to N2. */

	lpln1 = lend[n1];
	lp2 = lpln1;
L1:
	lp2 = lptr[lp2];
	n2 = list[lp2];
	lp = lptr[lp2];
	n3 = (i__2 = list[lp], int64_abs(i__2));
	if (n2 < n1 || n3 < n1) {
	    goto L8;
	}

/* Add a new triangle KT = (N1,N2,N3). */

	++kt;
	ltri[kt * ltri_dim1 + 1] = n1;
	ltri[kt * ltri_dim1 + 2] = n2;
	ltri[kt * ltri_dim1 + 3] = n3;

/* Loop on triangle sides (I2,I1) with neighboring triangles */
/*   KN = (I1,I2,I3). */

	for (i__ = 1; i__ <= 3; ++i__) {
	    if (i__ == 1) {
		i1 = n3;
		i2 = n2;
	    } else if (i__ == 2) {
		i1 = n1;
		i2 = n3;
	    } else {
		i1 = n2;
		i2 = n1;
	    }

/* Set I3 to the neighbor of I1 that follows I2 unless */
/*   I2->I1 is a boundary arc. */

	    lpl = lend[i1];
	    lp = lptr[lpl];
L2:
	    if (list[lp] == i2) {
		goto L3;
	    }
	    lp = lptr[lp];
	    if (lp != lpl) {
		goto L2;
	    }

/*   I2 is the last neighbor of I1 unless the data structure */
/*     is invalid.  Bypass the search for a neighboring */
/*     triangle if I2->I1 is a boundary arc. */

	    if ((i__2 = list[lp], int64_abs(i__2)) != i2) {
		goto L12;
	    }
	    kn = 0;
	    if (list[lp] < 0) {
		goto L6;
	    }

/*   I2->I1 is not a boundary arc, and LP points to I2 as */
/*     a neighbor of I1. */

L3:
	    lp = lptr[lp];
	    i3 = (i__2 = list[lp], int64_abs(i__2));

/* Find J such that LTRI(J,KN) = I3 (not used if KN > KT), */
/*   and permute the vertex indexes of KN so that I1 is */
/*   smallest. */

	    if (i1 < i2 && i1 < i3) {
		j = 3;
	    } else if (i2 < i3) {
		j = 2;
		isv = i1;
		i1 = i2;
		i2 = i3;
		i3 = isv;
	    } else {
		j = 1;
		isv = i1;
		i1 = i3;
		i3 = i2;
		i2 = isv;
	    }

/* Test for KN > KT (triangle index not yet assigned). */

	    if (i1 > n1) {
		goto L7;
	    }

/* Find KN, if it exists, by searching the triangle list in */
/*   reverse order. */

	    for (kn = kt - 1; kn >= 1; --kn) {
		if (ltri[kn * ltri_dim1 + 1] == i1 && ltri[kn * ltri_dim1 + 2]
			 == i2 && ltri[kn * ltri_dim1 + 3] == i3) {
		    goto L5;
		}
/* L4: */
	    }
	    goto L7;

/* Store KT as a neighbor of KN. */

L5:
	    ltri[j + 3 + kn * ltri_dim1] = kt;

/* Store KN as a neighbor of KT, and add a new arc KA. */

L6:
	    ltri[i__ + 3 + kt * ltri_dim1] = kn;
	    if (arcs) {
		++ka;
		ltri[i__ + 6 + kt * ltri_dim1] = ka;
		if (kn != 0) {
		    ltri[j + 6 + kn * ltri_dim1] = ka;
		}
	    }
L7:
	    ;
	}

/* Bottom of loop on triangles. */

L8:
	if (lp2 != lpln1) {
	    goto L1;
	}
/* L9: */
    }

/* No errors encountered. */

    *nt = kt;
    *ier = 0;
    return 0;

/* Invalid input parameter. */

L11:
    *nt = 0;
    *ier = 1;
    return 0;

/* Invalid triangulation data structure:  I1 is a neighbor of */
/*   I2, but I2 is not a neighbor of I1. */

L12:
    *nt = 0;
    *ier = 2;
    return 0;
} /* trlist_ */

GMT_LOCAL integer trmesh_(integer *n, doublereal *x, doublereal *y, doublereal *z__, integer *list, integer *lptr, integer *lend, integer *lnew, integer *near__, integer *next, doublereal *dist, integer *ier) {
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static doublereal d__;
    static integer i__, j, k;
    static doublereal d1, d2, d3;
    static integer i0, lp, nn, lpl;
    static integer nexti;


/* *********************************************************** */

/*                                              From STRIPACK */
/*                                            Robert J. Renka */
/*                                  Dept. of Computer Science */
/*                                       Univ. of North Texas */
/*                                           renka@cs.unt.edu */
/*                                                   07/08/99 */

/*   This subroutine creates a Delaunay triangulation of a */
/* set of N arbitrarily distributed points, referred to as */
/* nodes, on the surface of the unit sphere.  The Delaunay */
/* triangulation is defined as a set of (spherical) triangles */
/* with the following five properties: */

/*  1)  The triangle vertices are nodes. */
/*  2)  No triangle contains a node other than its vertices. */
/*  3)  The interiors of the triangles are pairwise disjoint. */
/*  4)  The union of triangles is the convex hull of the set */
/*        of nodes (the smallest convex set that contains */
/*        the nodes).  If the nodes are not contained in a */
/*        single hemisphere, their convex hull is the en- */
/*        tire sphere and there are no boundary nodes. */
/*        Otherwise, there are at least three boundary nodes. */
/*  5)  The interior of the circumcircle of each triangle */
/*        contains no node. */

/* The first four properties define a triangulation, and the */
/* last property results in a triangulation which is as close */
/* as possible to equiangular in a certain sense and which is */
/* uniquely defined unless four or more nodes lie in a common */
/* plane.  This property makes the triangulation well-suited */
/* for solving closest-point problems and for triangle-based */
/* interpolation. */

/*   Provided the nodes are randomly ordered, the algorithm */
/* has expected time complexity O(N*log(N)) for most nodal */
/* distributions.  Note, however, that the complexity may be */
/* as high as O(N**2) if, for example, the nodes are ordered */
/* on increasing latitude. */

/*   Spherical coordinates (latitude and longitude) may be */
/* converted to Cartesian coordinates by Subroutine TRANS. */

/*   The following is a list of the software package modules */
/* which a user may wish to call directly: */

/*  ADDNOD - Updates the triangulation by appending a new */
/*             node. */

/*  AREAS  - Returns the area of a spherical triangle. */

/*  BNODES - Returns an array containing the indexes of the */
/*             boundary nodes (if any) in counterclockwise */
/*             order.  Counts of boundary nodes, triangles, */
/*             and arcs are also returned. */

/*  CIRCUM - Returns the circumcenter of a spherical trian- */
/*             gle. */

/*  CRLIST - Returns the set of triangle circumcenters */
/*             (Voronoi vertices) and circumradii associated */
/*             with a triangulation. */

/*  DELARC - Deletes a boundary arc from a triangulation. */

/*  DELNOD - Updates the triangulation with a nodal deletion. */

/*  EDGE   - Forces an arbitrary pair of nodes to be connec- */
/*             ted by an arc in the triangulation. */

/*  GETNP  - Determines the ordered sequence of L closest */
/*             nodes to a given node, along with the associ- */
/*             ated distances. */

/*  INSIDE - Locates a point relative to a polygon on the */
/*             surface of the sphere. */

/*  INTRSC - Returns the point of intersection between a */
/*             pair of great circle arcs. */

/*  JRAND  - Generates a uniformly distributed pseudo-random */
/*             integer. */

/*  LEFT   - Locates a point relative to a great circle. */

/*  NEARND - Returns the index of the nearest node to an */
/*             arbitrary point, along with its squared */
/*             distance. */

/*  SCOORD - Converts a point from Cartesian coordinates to */
/*             spherical coordinates. */

/*  STORE  - Forces a value to be stored in main memory so */
/*             that the precision of floating point numbers */
/*             in memory locations rather than registers is */
/*             computed. */

/*  TRANS  - Transforms spherical coordinates into Cartesian */
/*             coordinates on the unit sphere for input to */
/*             Subroutine TRMESH. */

/*  TRLIST - Converts the triangulation data structure to a */
/*             triangle list more suitable for use in a fin- */
/*             ite element code. */

/*  TRLPRT - Prints the triangle list created by Subroutine */
/*             TRLIST. */

/*  TRMESH - Creates a Delaunay triangulation of a set of */
/*             nodes. */

/*  TRPLOT - Creates a level-2 Encapsulated Postscript (EPS) */
/*             file containing a triangulation plot. */

/*  TRPRNT - Prints the triangulation data structure and, */
/*             optionally, the nodal coordinates. */

/*  VRPLOT - Creates a level-2 Encapsulated Postscript (EPS) */
/*             file containing a Voronoi diagram plot. */


/* On input: */

/*       N = Number of nodes in the triangulation.  N .GE. 3. */

/*       X,Y,Z = Arrays of length N containing the Cartesian */
/*               coordinates of distinct nodes.  (X(K),Y(K), */
/*               Z(K)) is referred to as node K, and K is re- */
/*               ferred to as a nodal index.  It is required */
/*               that X(K)**2 + Y(K)**2 + Z(K)**2 = 1 for all */
/*               K.  The first three nodes must not be col- */
/*               linear (lie on a common great circle). */

/* The above parameters are not altered by this routine. */

/*       LIST,LPTR = Arrays of length at least 6N-12. */

/*       LEND = Array of length at least N. */

/*       NEAR,NEXT,DIST = Work space arrays of length at */
/*                        least N.  The space is used to */
/*                        efficiently determine the nearest */
/*                        triangulation node to each un- */
/*                        processed node for use by ADDNOD. */

/* On output: */

/*       LIST = Set of nodal indexes which, along with LPTR, */
/*              LEND, and LNEW, define the triangulation as a */
/*              set of N adjacency lists -- counterclockwise- */
/*              ordered sequences of neighboring nodes such */
/*              that the first and last neighbors of a bound- */
/*              ary node are boundary nodes (the first neigh- */
/*              bor of an interior node is arbitrary).  In */
/*              order to distinguish between interior and */
/*              boundary nodes, the last neighbor of each */
/*              boundary node is represented by the negative */
/*              of its index. */

/*       LPTR = Set of pointers (LIST indexes) in one-to-one */
/*              correspondence with the elements of LIST. */
/*              LIST(LPTR(I)) indexes the node which follows */
/*              LIST(I) in cyclical counterclockwise order */
/*              (the first neighbor follows the last neigh- */
/*              bor). */

/*       LEND = Set of pointers to adjacency lists.  LEND(K) */
/*              points to the last neighbor of node K for */
/*              K = 1,...,N.  Thus, LIST(LEND(K)) < 0 if and */
/*              only if K is a boundary node. */

/*       LNEW = Pointer to the first empty location in LIST */
/*              and LPTR (list length plus one).  LIST, LPTR, */
/*              LEND, and LNEW are not altered if IER < 0, */
/*              and are incomplete if IER > 0. */

/*       NEAR,NEXT,DIST = Garbage. */

/*       IER = Error indicator: */
/*             IER =  0 if no errors were encountered. */
/*             IER = -1 if N < 3 on input. */
/*             IER = -2 if the first three nodes are */
/*                      collinear. */
/*             IER =  L if nodes L and M coincide for some */
/*                      M > L.  The data structure represents */
/*                      a triangulation of nodes 1 to M-1 in */
/*                      this case. */

/* Modules required by TRMESH:  ADDNOD, BDYADD, COVSPH, */
/*                                INSERT, INTADD, JRAND, */
/*                                LEFT, LSTPTR, STORE, SWAP, */
/*                                SWPTST, TRFIND */

/* Intrinsic function called by TRMESH:  ABS */

/* *********************************************************** */


/* Local parameters: */

/* D =        (Negative cosine of) distance from node K to */
/*              node I */
/* D1,D2,D3 = Distances from node K to nodes 1, 2, and 3, */
/*              respectively */
/* I,J =      Nodal indexes */
/* I0 =       Index of the node preceding I in a sequence of */
/*              unprocessed nodes:  I = NEXT(I0) */
/* K =        Index of node to be added and DO-loop index: */
/*              K > 3 */
/* LP =       LIST index (pointer) of a neighbor of K */
/* LPL =      Pointer to the last neighbor of K */
/* NEXTI =    NEXT(I) */
/* NN =       Local copy of N */

    /* Parameter adjustments */
    --dist;
    --next;
    --near__;
    --lend;
    --z__;
    --y;
    --x;
    --list;
    --lptr;

    /* Function Body */
    nn = *n;
    if (nn < 3) {
	*ier = -1;
	return 0;
    }

/* Store the first triangle in the linked list. */

    if (! left_(&x[1], &y[1], &z__[1], &x[2], &y[2], &z__[2], &x[3], &y[3], &
	    z__[3])) {

/*   The first triangle is (3,2,1) = (2,1,3) = (1,3,2). */

	list[1] = 3;
	lptr[1] = 2;
	list[2] = -2;
	lptr[2] = 1;
	lend[1] = 2;

	list[3] = 1;
	lptr[3] = 4;
	list[4] = -3;
	lptr[4] = 3;
	lend[2] = 4;

	list[5] = 2;
	lptr[5] = 6;
	list[6] = -1;
	lptr[6] = 5;
	lend[3] = 6;

    } else if (! left_(&x[2], &y[2], &z__[2], &x[1], &y[1], &z__[1], &x[3], &
	    y[3], &z__[3])) {

/*   The first triangle is (1,2,3):  3 Strictly Left 1->2, */
/*     i.e., node 3 lies in the left hemisphere defined by */
/*     arc 1->2. */

	list[1] = 2;
	lptr[1] = 2;
	list[2] = -3;
	lptr[2] = 1;
	lend[1] = 2;

	list[3] = 3;
	lptr[3] = 4;
	list[4] = -1;
	lptr[4] = 3;
	lend[2] = 4;

	list[5] = 1;
	lptr[5] = 6;
	list[6] = -2;
	lptr[6] = 5;
	lend[3] = 6;

    } else {

/*   The first three nodes are collinear. */

	*ier = -2;
	return 0;
    }

/* Initialize LNEW and test for N = 3. */

    *lnew = 7;
    if (nn == 3) {
	*ier = 0;
	return 0;
    }

/* A nearest-node data structure (NEAR, NEXT, and DIST) is */
/*   used to obtain an expected-time (N*log(N)) incremental */
/*   algorithm by enabling constant search time for locating */
/*   each new node in the triangulation. */

/* For each unprocessed node K, NEAR(K) is the index of the */
/*   triangulation node closest to K (used as the starting */
/*   point for the search in Subroutine TRFIND) and DIST(K) */
/*   is an increasing function of the arc length (angular */
/*   distance) between nodes K and NEAR(K):  -Cos(a) for arc */
/*   length a. */

/* Since it is necessary to efficiently find the subset of */
/*   unprocessed nodes associated with each triangulation */
/*   node J (those that have J as their NEAR entries), the */
/*   subsets are stored in NEAR and NEXT as follows:  for */
/*   each node J in the triangulation, I = NEAR(J) is the */
/*   first unprocessed node in J's set (with I = 0 if the */
/*   set is empty), L = NEXT(I) (if I > 0) is the second, */
/*   NEXT(L) (if L > 0) is the third, etc.  The nodes in each */
/*   set are initially ordered by increasing indexes (which */
/*   maximizes efficiency) but that ordering is not main- */
/*   tained as the data structure is updated. */

/* Initialize the data structure for the single triangle. */

    near__[1] = 0;
    near__[2] = 0;
    near__[3] = 0;
    for (k = nn; k >= 4; --k) {
	d1 = -(x[k] * x[1] + y[k] * y[1] + z__[k] * z__[1]);
	d2 = -(x[k] * x[2] + y[k] * y[2] + z__[k] * z__[2]);
	d3 = -(x[k] * x[3] + y[k] * y[3] + z__[k] * z__[3]);
	if (d1 <= d2 && d1 <= d3) {
	    near__[k] = 1;
	    dist[k] = d1;
	    next[k] = near__[1];
	    near__[1] = k;
	} else if (d2 <= d1 && d2 <= d3) {
	    near__[k] = 2;
	    dist[k] = d2;
	    next[k] = near__[2];
	    near__[2] = k;
	} else {
	    near__[k] = 3;
	    dist[k] = d3;
	    next[k] = near__[3];
	    near__[3] = k;
	}
/* L1: */
    }

/* Add the remaining nodes */

    i__1 = nn;
    for (k = 4; k <= i__1; ++k) {
	addnod_(&near__[k], &k, &x[1], &y[1], &z__[1], &list[1], &lptr[1], &
		lend[1], lnew, ier);
	if (*ier != 0) {
	    return 0;
	}

/* Remove K from the set of unprocessed nodes associated */
/*   with NEAR(K). */

	i__ = near__[k];
	if (near__[i__] == k) {
	    near__[i__] = next[k];
	} else {
	    i__ = near__[i__];
L2:
	    i0 = i__;
	    i__ = next[i0];
	    if (i__ != k) {
		goto L2;
	    }
	    next[i0] = next[k];
	}
	near__[k] = 0;

/* Loop on neighbors J of node K. */

	lpl = lend[k];
	lp = lpl;
L3:
	lp = lptr[lp];
	j = (i__2 = list[lp], int64_abs(i__2));

/* Loop on elements I in the sequence of unprocessed nodes */
/*   associated with J:  K is a candidate for replacing J */
/*   as the nearest triangulation node to I.  The next value */
/*   of I in the sequence, NEXT(I), must be saved before I */
/*   is moved because it is altered by adding I to K's set. */

	i__ = near__[j];
L4:
	if (i__ == 0) {
	    goto L5;
	}
	nexti = next[i__];

/* Test for the distance from I to K less than the distance */
/*   from I to J. */

	d__ = -(x[i__] * x[k] + y[i__] * y[k] + z__[i__] * z__[k]);
	if (d__ < dist[i__]) {

/* Replace J by K as the nearest triangulation node to I: */
/*   update NEAR(I) and DIST(I), and remove I from J's set */
/*   of unprocessed nodes and add it to K's set. */

	    near__[i__] = k;
	    dist[i__] = d__;
	    if (i__ == near__[j]) {
		near__[j] = nexti;
	    } else {
		next[i0] = nexti;
	    }
	    next[i__] = near__[k];
	    near__[k] = i__;
	} else {
	    i0 = i__;
	}

/* Bottom of loop on I. */

	i__ = nexti;
	goto L4;

/* Bottom of loop on neighbors J. */

L5:
	if (lp != lpl) {
	    goto L3;
	}
/* L6: */
    }
    return 0;
} /* trmesh_ */
