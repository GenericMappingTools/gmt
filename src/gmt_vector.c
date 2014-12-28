/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

/*
 * Author:	Walter H.F. Smith
 * Date:	1-JAN-2010
 * Version:	5.x
 */
 
#include "gmt_dev.h"
#include "gmt_internals.h"

#define MAX_SWEEPS 50

struct GMT_SINGULAR_VALUE {	/* Used for sorting of eigenvalues in the SVD functions */
	double value;
	unsigned int order;
};

int GMT_jacobi (struct GMT_CTRL *GMT, double *a, unsigned int n, unsigned int m, double *d, double *v, double *b, double *z, unsigned int *nrots) {
/*
 *
 * Find eigenvalues & eigenvectors of a square symmetric matrix by Jacobi's
 * method.  Given A, find V and D such that A = V * D * V-transpose, with
 * V an orthogonal matrix and D a diagonal matrix.  The eigenvalues of A
 * are on diag(D), and the j-th column of V is the eigenvector corresponding
 * to the j-th diagonal element of D.  Returns 0 if OK, -1 if it fails to
 * converge in MAX_SWEEPS.
 *
 * a is sent as a square symmetric matrix, of size n, and row dimension m.
 * Only the diagonal and super-diagonal elements of a will be used, so the
 * sub-diagonal elements could be used to preserve a, or could have been
 * destroyed by an earlier attempt to form the Cholesky decomposition of a.
 * On return, the super-diagonal elements are destroyed.  The diagonal and
 * sub-diagonal elements are unchanged.
 * d is returned as an n-vector containing the eigenvalues of a, sorted 
 * so that d[i] >= d[j] when i < j.  d = diag(D).
 * v is returned as an n by n matrix, V, with row dimension m, and the
 * columns of v are the eigenvectors corresponding to the values in d.
 * b is an n-vector of workspace, used to keep a copy of the diagonal
 * elements which is updated only after a full sweep.  
 * z is an n-vector of workspace, used to accumulate the updates to
 * the diagonal values of a during each sweep.  This reduces round-
 * off problems.
 * nrots is the number of rotations performed.  Bounds on round-off error
 * can be estimated from this if desired.
 *
 * Numerical Details:
 * The basic algorithms is in many textbooks.  The idea is to make an
 * infinite series (which turns out to be at quadratically convergent)
 * of steps, in each of which A_new = P-transpose * A_old * P, where P is
 * a plane-rotation matrix in the p,q plane, through an angle chosen to
 * zero A_new(p,q) and A_new(q,p).  The sum of the diagonal elements
 * of A is unchanged by these operations, but the sum of squares of 
 * diagonal elements of a is increased by 2 * |A_old(p,q)| at each step.
 * Although later steps make non-zero again the previously zeroed entries,
 * the sum of squares of diagonal elements increases with each rotation,
 * while the sum of squares of off-diagonals keeps decreasing, so that
 * eventually A_new is diagonal to machine precision.  This should 
 * happen in a few (3 to 7) sweeps.
 *
 * If only the eigenvalues are wanted then there are faster methods, but
 * if all eigenvalues and eigenvectors are needed, then this method is
 * only somewhat slower than the fastest method (Householder tri-
 * diagonalization followed by symmetric QR iterations), and this method
 * is numerically extremely stable.
 *
 * C G J Jacobi ("Ueber ein leichtes Vefahren, die in der Theorie der 
 * Saekularstoerungen vorkommenden Gelichungen numerisch aufzuloesen",
 * Crelle's Journal, v. 30, pp. 51--94, 1846) originally searched the
 * entire (half) matrix for the largest |A(p,q)| to select each step.
 * When the method was developed for machine computation (R T Gregory,
 * "Computing eigenvalues and eigenvectors of a symmetric matrix on
 * the ILLIAC", Math. Tab. and other Aids to Comp., v. 7, pp. 215--220,
 * 1953) it was done with a series of "sweeps" through the upper triangle,
 * visiting all p,q in turn.  Later, D A Pope and C Tompkins ("Maximizing
 * functions of rotations - experiments concerning speed of diagonalization
 * of symmetric matrices using Jacobi's method", J Assoc. Comput. Mach.
 * v. 4, pp. 459--466, 1957) introduced a variant that skips small
 * elements on the first few sweeps.  The algorithm here was given by 
 * Heinz Rutishauser (1918--1970) and published in Numer. Math. v. 9, 
 * pp 1--10, 1966, and in Linear Algebra (the Handbook for Automatic 
 * Computation, v. II), by James Hardy Wilkinson and C. Reinsch (Springer-
 * Verlag, 1971).  It also appears in Numerical Recipes.
 *
 * This algorithm takes care to avoid round-off error in several ways.
 * First, although there are four values of theta in (-pi, pi] that
 * would zero A(p,q), there is only one with magnitude <= pi/4.
 * This one is used.  This is most stable, and also has the effect
 * that, if A_old(p,p) >= A_old(q,q) then A_new(p,p) > A_new(q,q).
 * Two copies of the diagonal elements are maintained in d[] and b[].
 * d[] is updated immediately in each rotation, and each new rotation
 * is computed based on d[], so that each rotation gets the benefit
 * of the previous ones.  However, z[] is also used to accumulate
 * the sum of all the changes in the diagonal elements during one sweep, 
 * and z[] is used to update b[] after each sweep.  Then b is copied
 * to d.  In this way, at the end of each sweep, d is reset to avoid
 * accumulating round-off.
 *
 * This routine determines whether y is small compared to x by testing
 * if (fabs(y) + fabs(x) == fabs(x) ).  It is assumed that the
 * underflow which may occur here is nevertheless going to allow this
 * expression to be evaluated as true or false and execution to 
 * continue.  If the run environment doesn't allow this, the routine
 * won't work properly.
 *
 * programmer:	W. H. F. Smith, 7 June, 1991.
 * Revised:	PW: 12-MAR-1998 for GMT 3.1
 * Revision by WHF Smith, March 03, 2000, to speed up loop indexes.
 */
	unsigned int p, q, pp, pq, mp1, pm, qm, nsweeps, j, jm, i, k;
	double sum, threshold, g, h, t, theta, c, s, tau;

	/* Begin by initializing v, b, d, and z.  v = identity matrix,
		b = d = diag(a), and z = 0:  */

	GMT_memset (v, m*n, double);
	GMT_memset (z, n, double);

	mp1 = m + 1;

	for (p = 0, pp = 0; p < n; p++, pp+=mp1) {
		v[pp] = 1.0;
		b[p] = a[pp];
		d[p] = b[p];
	}

	/* End of initializations.  Set counters and begin:  */

	(*nrots) = 0;
	nsweeps = 0;

	while (nsweeps < MAX_SWEEPS) {

		/* Sum off-diagonal elements of upper triangle.  */
		sum = 0.0;
		for (q = 1, qm = m; q < n; q++, qm += m ) {
			for (p = 0, pq = qm; p < q; p++, pq++) sum += fabs(a[pq]);
		}

		/* Exit this loop (converged) when sum == 0.0  */
		if (sum == 0.0) break;

		/* If (nsweeps < 3) do only bigger elements;  else all  */
		threshold =  (nsweeps < 3) ? 0.2 * sum / ( n * n ) : 0.0;

		/* Now sweep whole upper triangle doing Givens rotations:  */
		for (q = 1, qm = m; q < n; q++, qm += m ) {
			for (p = 0, pm = 0, pq = qm; p < q; p++, pm += m, pq++) {
				/* In 3/2000 I swapped order of these loops,
					to allow simple incrementing of pq  */

				if (a[pq] == 0.0) continue;	/* New 3/2000  */

				g = 100.0 * fabs (a[pq]);

				/* After four sweeps, if g is small relative
					to a(p,p) and a(q,q), skip the 
					rotation and set a(p,q) to zero.  */

				if ((nsweeps > 3) && ((fabs (d[p])+g) == fabs (d[p])) && ((fabs (d[q])+g) == fabs (d[q]))) {
					a[pq] = 0.0;
				}
				else if (fabs (a[pq]) > threshold) {
					h = d[q] - d[p];

					if (h == 0.0)
						t = 1.0;	/* This if block is new 3/2000  */
					else if (fabs (h) + g == fabs (h))
						t = a[pq] / h;
					else {
						theta = 0.5 * h / a[pq];
						t = 1.0 / (fabs (theta) + sqrt (1.0 + theta*theta) );
						if (theta < 0.0) t = -t;
					}

					c = 1.0 / sqrt (1.0 + t*t);
					s = t * c;
					tau = s / (1.0 + c);

					h = t * a[pq];
					z[p] -= h;
					z[q] += h;
					d[p] -= h;
					d[q] += h;
					a[pq] = 0.0;

					for (j = 0; j < p; j++) {
						g = a[j + pm];
						h = a[j + qm];
						a[j + pm] = g - s * (h + g * tau);
						a[j + qm] = h + s * (g - h * tau);
					}
					for (j = p+1, jm = m*(p+1); j < q; j++, jm += m ) {
						g = a[p + jm];
						h = a[j + qm];
						a[p + jm] = g - s * (h + g * tau);
						a[j + qm] = h + s * (g - h * tau);
					}
					for (j = q+1, jm = m*(q+1); j < n; j++, jm += m ) {
						g = a[p + jm];
						h = a[q + jm];
						a[p + jm] = g - s * (h + g * tau);
						a[q + jm] = h + s * (g - h * tau);
					}

					for (j = 0; j < n; j++) {
						g = v[j + pm];
						h = v[j + qm];
						v[j + pm] = g - s * (h + g * tau);
						v[j + qm] = h + s * (g - h * tau);
					}

					(*nrots)++;
				}
			}
		}

		/* End of one sweep of the upper triangle.  */

		nsweeps++;

		for (p = 0; p < n; p++) {
			b[p] += z[p];	/* Update the b copy of diagonal  */
			d[p] = b[p];	/* Replace d with b to reduce round-off error  */
			z[p] = 0.0;	/* Clear z.  */
		}
	}

	/* Get here via break when converged, or when nsweeps == MAX_SWEEPS.
		Sort eigenvalues by insertion:  */

	for (i = 0; i < n-1; i++) {
		k = i;
		g = d[i];
		for (j = i+1; j < n; j++) {  /* Find max location  */
			if (d[j] >= g) {
				k = j;
				g = d[j];
			}
		}
		if (k != i) {  /*  Need to swap value and vector  */
			d[k] = d[i];
			d[i] = g;
			p = i * m;
			q = k * m;
			for (j = 0; j < n; j++) {
				g = v[j + p];
				v[j + p] = v[j + q];
				v[j + q] = g;
			}
		}
	}

	/* Return 0 if converged; else print warning and return -1:  */

	if (nsweeps == MAX_SWEEPS) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_jacobi failed to converge in %d sweeps\n", nsweeps);
		return(-1);
	}
	return(0);
}

int GMT_gauss (struct GMT_CTRL *GMT, double *a, double *vec, unsigned int n, unsigned int nstore, bool itriag)
{

/* subroutine gauss, by william menke */
/* july 1978 (modified feb 1983, nov 85) */

/* a subroutine to solve a system of n linear equations in n unknowns
 * gaussian reduction with partial pivoting is used
 *      a	(sent, destroyed)	n by n matrix
 *      vec	(sent, overwritten)	n vector, replaced w/ solution
 *      nstore	(sent)			dimension of a
 *      ierror	(returned)		zero on no error
 *      itriag	(sent)			matrix triangularized only
 *					on true useful when solving
 *					multiple systems with same a
 */
	static unsigned int l1;
	unsigned int *line = NULL, i = 0, j, k, l, j1, j2, *isub = NULL;
	int iet, ieb;
	size_t n_alloc = 0;
	double big, testa, b, sum;

	iet = 0;  /* initial error flags, one for triagularization*/
	ieb = 0;  /* one for backsolving */
	GMT_malloc2 (GMT, line, isub, n, &n_alloc, unsigned int);

/* triangularize the matrix a */
/* replacing the zero elements of the triangularized matrix */
/* with the coefficients needed to transform the vector vec */

	if (itriag) {   /* triangularize matrix */

		for (j = 0; j < n; j++) {      /*line is an array of flags*/
			line[j] = 0;
			/* elements of a are not moved during pivoting*/
			/* line=0 flags unused lines */
		}

		for (j=0; j<n-1; j++) {
			/*  triangularize matrix by partial pivoting */
			big = 0.0; /* find biggest element in j-th column*/
				  /* of unused portion of matrix*/
			for (l1=0; l1<n; l1++) {
				if (line[l1]==0) {
					testa = fabs ((*(a+l1*nstore+j)));
					if (testa>big) {
						i=l1;
						big=testa;
					}
				}
			}
			if (big<=DBL_EPSILON) iet=1;   /* test for div by 0 */
 
			line[i]=1;  /* selected unused line becomes used line */
			isub[j]=i;  /* isub points to j-th row of tri. matrix */
 
			sum=1.0/(*(a+i*nstore+j)); 

			/*reduce matrix towards triangle */
			for (k=0; k<n; k++) {
				if (line[k]==0) {
					b=(*(a+k*nstore+j))*sum;
					for (l=j+1; l<n; l++) *(a+k*nstore+l) -= (b*(*(a+i*nstore+l)));
					*(a+k*nstore+j)=b;
				}
			}
		}

		for( j=0; j<n; j++ ) {
			/* find last unused row and set its pointer */
			/* this row contains the apex of the triangle */
			if (line[j]==0) {
				l1=j;   /* apex of triangle */
				isub[n-1]=j;
				break;
			}
		}
 
	}
		
	/* start backsolving */
	
	for (i=0; i<n; i++) line[isub[i]] = i;  /* invert pointers. line(i) now gives row no in triang matrix of i-th row of actual matrix */
 
	for (j=0; j<n-1; j++) { /* transform the vector to match triang. matrix */
		b=vec[isub[j]];
		for( k=0; k<n; k++ ) {
			if (line[k]>j) vec[k] -= (b*(*(a+k*nstore+j)));  /* skip elements outside of triangle */
		}
	}
 
	b = *(a+l1*nstore+(n-1));   /* apex of triangle */
	if (fabs(b)<=DBL_EPSILON) ieb=2; /* check for div by zero in backsolving */
	vec[isub[n-1]]=vec[isub[n-1]]/b;
 
	for (j1=n-1; j1>0; j1--) { /* backsolve rest of triangle*/
		j = j1 - 1;
		sum=vec[isub[j]];
		for (j2=j+1; j2<n; j2++) sum -= (vec[isub[j2]] * (*(a+isub[j]*nstore+j2)));
		b = *(a+isub[j]*nstore+j);
		if (fabs(b)<=DBL_EPSILON) ieb=2; /* test for div by 0 in backsolving */
		vec[isub[j]]=sum/b;   /* solution returned in vec */
	}

	/* put the solution vector into the proper order */

	for (i=0; i<n; i++) {    /* reorder solution */
		for (k=i; k<n; k++) {  /* search for i-th solution element */
			if (line[k]==i) {
				j=k;
				break;
			}
		}
		b = vec[j];	/* swap solution and pointer elements*/
		vec[j] = vec[i];
		vec[i] = b;
		line[j] = line[i];
	}
 
	GMT_free (GMT, isub);
	GMT_free (GMT, line);
	return (iet + ieb);   /* Return final error flag*/
}

/* Modified from similar function in Numerical Recipes */

int GMT_gaussjordan (struct GMT_CTRL *GMT, double *a, unsigned int n_in, unsigned int ndim, double *b, unsigned int m_in, unsigned int mdim)
{
	int i, j, k, l, ll, *ipiv = NULL, *indxc = NULL, *indxr = NULL, irow = 0, icol = 0;
	int n = n_in, m = m_in;
	double big, dum, pivinv;
	
	ipiv  = GMT_memory (GMT, NULL, n, int);
	indxc = GMT_memory (GMT, NULL, n, int);
	indxr = GMT_memory (GMT, NULL, n, int);
	
	for (i = 0; i < n; i++) {
		big = 0.0;
		for (j = 0; j < n; j++) {
			if (ipiv[j] != 1) {
				for (k = 0; k < n; k++) {
					if (ipiv[k] == 0) {
						if ((dum = fabs (a[j*ndim+k])) >= big) {
							big = dum;
							irow = j;
							icol = k;
						}
					}
					else if (ipiv[k] > 1) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_gaussjordan: Singular matrix!\n");
						GMT_free (GMT, ipiv);
						GMT_free (GMT, indxc);
						GMT_free (GMT, indxr);
						return (1);
					}
				}
			}
		}
		ipiv[icol]++;
		
		if (irow != icol) {
			for (l = 0; l < n; l++) {
				dum = a[irow*ndim+l];
				a[irow*ndim+l] = a[icol*ndim+l];
				a[icol*ndim+l] = dum;
			}
			for (l = 0; l < m; l++) {
				dum = b[irow*mdim+l];
				b[irow*mdim+l] = b[icol*mdim+l];
				b[icol*mdim+l] = dum;
			}
		}
		
		indxr[i] = irow;
		indxc[i] = icol;
		if (a[icol*ndim+icol] == 0.0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_gaussjordan: Singular matrix!\n");
			GMT_free (GMT, ipiv);
			GMT_free (GMT, indxc);
			GMT_free (GMT, indxr);
			return (1);
		}
		pivinv = 1.0 / a[icol*ndim+icol];
		a[icol*ndim+icol] = 1.0;
		for (l = 0; l < n; l++) a[icol*ndim+l] *= pivinv;
		for (l = 0; l < m; l++)  b[icol*mdim+l] *= pivinv;
		for (ll = 0; ll < n; ll++) {
			if (ll != icol) {
				dum = a[ll*ndim+icol];
				a[ll*ndim+icol] = 0.0;
				for (l = 0; l < n; l++) a[ll*ndim+l] -= a[icol*ndim+l] * dum;
				for (l = 0; l < m; l++) b[ll*mdim+l] -= b[icol*mdim+l] * dum;
			}
		}
	}
	for (l = n-1; l >= 0; l--) {
		if (indxr[l] != indxc[l]) {
			for (k = 0; k < n; k++) {
				dum = a[k*ndim+indxr[l]];
				a[k*ndim+indxr[l]] = a[k*ndim+indxc[l]];
				a[k*ndim+indxc[l]] = dum;
			}
		}
	}
	
	GMT_free (GMT, ipiv);
	GMT_free (GMT, indxc);
	GMT_free (GMT, indxr);
	
	return (0);
}

/* Given a matrix a[0..m-1][0...n-1], this routine computes its singular
	value decomposition, A=UWVt.  The matrix U replaces a on output.
	The diagonal matrix of singular values W is output as a vector
	w[0...n-1].  The matrix V (Not V transpose) is output as
	v[0...n-1][0....n-1].  m must be greater than or equal to n; if it is
	smaller, then a should be filled up to square with zero rows.
	
	Modified from Numerical Recipes -> page 68.
*/

#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))

int GMT_svdcmp (struct GMT_CTRL *GMT, double *a, unsigned int m_in, unsigned int n_in, double *w, double *v)
{
	/* void svdcmp(double *a,int m,int n,double *w,double *v) */
	
	int flag,i,its,j,jj,k,l=0,nm = 0, n = n_in, m = m_in;
	double c,f,h,s,x,y,z;
	double anorm=0.0,tnorm, g=0.0,scale=0.0;
	double *rv1 = NULL;
	
	if (m < n) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in GMT_svdcmp: m < n augment A with additional rows\n");
		return (EXIT_FAILURE);
	}
	
	/* allocate work space */
		
	rv1 = GMT_memory (GMT, NULL, n, double);
	
	/* do householder reduction to bidiagonal form */
		
	for (i=0;i<n;i++) {
		l=i+1;
		rv1[i]=scale*g;
		g=s=scale=0.0;
		if (i < m) {
			for (k=i;k<m;k++) scale += fabs (a[k*n+i]);		/* a[k][i] */
			if (scale) {
				for (k=i;k<m;k++) {
					a[k*n+i] /= scale;	/* a[k][i] */
					s += a[k*n+i]*a[k*n+i];	/* a[k][i] */
				}
				f=a[i*n+i];	/* a[i][i] */
				g= -1.0*SIGN(sqrt(s),f);
				h=f*g-s;
				a[i*n+i]=f-g;	/* a[i][i] */
				if (i != n-1) {
					for (j=l;j<n;j++) {
						for (s=0.0,k=i;k<m;k++) s += a[k*n+i]*a[k*n+j];	/* a[k][i] a[k][j] */
						f=s/h;
						for (k=i;k<m;k++) a[k*n+j] += f*a[k*n+i];	/* a[k][j] a[k][i] */
					}
				}
				for (k=i;k<m;k++) a[k*n+i] *= scale;	/* a[k][i] */
			}
		}
		w[i]=scale*g;
		g=s=scale=0.0;
		if (i <= m-1 && i != n-1) {
			for (k=l;k<n;k++) scale += fabs (a[i*n+k]);	/* a[i][k] */
			if (scale) {
				for (k=l;k<n;k++) {
					a[i*n+k] /= scale;	/* a[i][k] */
					s += a[i*n+k]*a[i*n+k];	/* a[i][k] */
				}
				f=a[i*n+l];	/* a[i][l] */
				g = -1.0*SIGN(sqrt(s),f);
				h=f*g-s;
				a[i*n+l]=f-g;	/* a[i][l] */
				for (k=l;k<n;k++) rv1[k]=a[i*n+k]/h;	/* a[i][k] */
				if (i != m-1) {
					for (j=l;j<m;j++) {
						for (s=0.0,k=l;k<n;k++) s += a[j*n+k]*a[i*n+k];	/*a[j][k] a[i][k] */
						for (k=l;k<n;k++) a[j*n+k] += s*rv1[k];	/* a[j][k] */
					}
				}
				for (k=l;k<n;k++) a[i*n+k] *= scale;	/* a[i][k] */
			}
		}
		tnorm=fabs (w[i])+fabs (rv1[i]);
		anorm=MAX(anorm,tnorm);
	}
						
	/* accumulation of right-hand transforms */
		
	for (i=n-1;i>=0;i--) {
		if (i < n-1) {
			if (g) {
				for (j=l;j<n;j++) v[j*n+i]=(a[i*n+j]/a[i*n+l])/g;	/* v[j][i] a[i][j] a[i][l] */
				for (j=l;j<n;j++) {
					for (s=0.0,k=l;k<n;k++) s += a[i*n+k]*v[k*n+j];	/* a[i][k] v[k][j] */
					for (k=l;k<n;k++) v[k*n+j] += s*v[k*n+i];	/* v[k][j] v[k][i] */
				}
			}
			for (j=l;j<n;j++) v[i*n+j]=v[j*n+i]=0.0;	/* v[i][j] v[j][i] */
		}
		v[i*n+i]=1.0;	/* v[i][i] */
		g=rv1[i];
		l=i;
	}
	
	/* accumulation of left-hand transforms */
		
	for (i=n-1;i>=0;i--) {
		l=i+1;
		g=w[i];
		if (i < n-1) for (j=l;j<n;j++) a[i*n+j]=0.0;	/* a[i][j] */
		if (g) {
			g=1.0/g;
			if (i != n-1) {
				for (j=l;j<n;j++) {
					for (s=0.0,k=l;k<m;k++) s += a[k*n+i]*a[k*n+j];	/* a[k][i] a[k][j] */
					f=(s/a[i*n+i])*g;	/* a[i][i] */
					for (k=i;k<m;k++) a[k*n+j] += f*a[k*n+i];	/* a[k][j] a[k][i] */
				}
			}
			for (j=i;j<m;j++) a[j*n+i] *= g;	/* a[j][i] */
		}
		else {
			for (j=i;j<m;j++) a[j*n+i]=0.0;	/* a[j][i] */
		}
		++a[i*n+i];	/* a[i][i] */
	}
	
	/* diagonalization of the bidiagonal form */
		
	for (k=n-1;k>=0;k--) {			/* loop over singular values */
		for (its=1;its<=30;its++) {	/* loop over allowed iterations */
			flag=1;
			for (l=k;l>=0;l--) {		/* test for splitting */
				nm=l-1;
				if (fabs(rv1[l])+anorm == anorm) {
					flag=0;
					break;
				}
				if (fabs (w[nm])+anorm == anorm) break;
			}
			if (flag) {
				c=0.0;			/* cancellation of rv1[l] if l > 1 */
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i];
					if (fabs (f)+anorm != anorm) {
						g=w[i];
						h=hypot (f,g);
						w[i]=h;
						h=1.0/h;
						c=g*h;
						s=(-1.0*f*h);
						for (j=0;j<m;j++) {
							y=a[j*n+nm];	/* a[j][nm] */
							z=a[j*n+i];	/* a[j][i] */
							a[j*n+nm]=(y*c)+(z*s);	/* a[j][nm] */
							a[j*n+i]=(z*c)-(y*s);	/* a[j][i] */
						}
					}
				}
			}
			z=w[k];
			if (l == k) {		/* convergence */
				if (z < 0.0) {	/* singular value is made positive */
					w[k]= -1.0*z;
					for (j=0;j<n;j++) v[j*n+k] *= (-1.0);	/* v[j][k] */
				}
				break;
			}
			if (its == 30) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in GMT_svdcmp: No convergence in 30 iterations\n");
				return (EXIT_FAILURE);
			}
			x=w[l];		/* shift from bottom 2-by-2 minor */
			nm=k-1;
			y=w[nm];
			g=rv1[nm];
			h=rv1[k];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g=hypot (f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			
				/* next QR transformation */
			
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w[i];
				h=s*g;
				g=c*g;
				z=hypot(f,h);
				rv1[j]=z;
				c=f/z;
				s=h/z;
				f=(x*c)+(g*s);
				g=(g*c)-(x*s);
				h=y*s;
				y=y*c;
				for (jj=0;jj<n;jj++) {
					x=v[jj*n+j];	/* v[jj][j] */
					z=v[jj*n+i];	/* v[jj][i] */
					v[jj*n+j]=(x*c)+(z*s);	/* v[jj][j] */
					v[jj*n+i]=(z*c)-(x*s);	/* v[jj][i] */
				}
				z=hypot(f,h);
				w[j]=z;		/* rotation can be arbitrary if z=0 */
				if (z) {
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=(c*g)+(s*y);
				x=(c*y)-(s*g);
				for (jj=0;jj<m;jj++) {
					y=a[jj*n+j];	/* a[jj][j] */
					z=a[jj*n+i];	/* a[jj][i] */
					a[jj*n+j]=(y*c)+(z*s);	/* a[jj][j] */
					a[jj*n+i]=(z*c)-(y*s);	/* a[jj][i] */
				}
			}
			rv1[l]=0.0;
			rv1[k]=f;
			w[k]=x;
		}
	}
	GMT_free (GMT, rv1);
	return (GMT_NOERROR);
}

void gmt_mat_trans (double a[], unsigned int mrow, unsigned int ncol, double at[])
{
	/* Return the transpose of a */
	unsigned int i, j;
	for (i = 0; i < ncol; i++) for (j = 0; j < mrow; j++) at[mrow*i+j] = a[ncol*j+i];
}

void gmt_mat_mult (double a[], unsigned int mrow, unsigned int ncol, double b[], unsigned int kcol, double c[])
{
	/* Matrix multiplication a * b = c */
	
	unsigned int i, j, k, ij;
	
	for (i = 0; i < kcol; i++) {
		for (j = 0; j < mrow; j++) {
			ij = j * kcol + i;
			c[ij] = 0.0;
			for (k = 0; k < ncol; k++) c[ij] += a[j * ncol + k] * b[k * kcol + i];
		}
	}
}

/* Given the singular value decomposition of a matrix a[0...m-1][0...n-1]
	solve the system of equations ax=b for x.  Input the matrices 
	U[0....m-1][0...n-1],w[0...n-1], and V[0...n-1][0...n-1] determined from
	svdcmp.  Also input the matrix b[0...m-1][0....k-1] and the solution vector
	x[0....k-1][0....n-1] is output. Singular values whose ratio to the maximum
	singular value are smaller than cutoff are zeroed out. The matrix U is
	overwritten.
	
*/

int compare_singular_values (const void *point_1v, const void *point_2v)
{
	/*  Routine for qsort to sort struct GMT_SINGULAR_VALUE on decreasing eigenvalues
	 * keeping track of the original order before sorting.
	 */
	const struct GMT_SINGULAR_VALUE *E_1 = point_1v, *E_2 = point_2v;

	if (E_1->value < E_2->value) return (+1);
	if (E_1->value > E_2->value) return (-1);
	return (0);
}

int GMT_solve_svd (struct GMT_CTRL *GMT, double *u, unsigned int m, unsigned int n, double *v, double *w, double *b, unsigned int k, double *x, double *cutoff, unsigned int mode)
{
	double *ut = NULL, sing_max, total_variance, variance = 0.0, limit;
	unsigned int i, j, n_use = 0;

	/* allocate work space */
	
	ut = GMT_memory (GMT, NULL, n*m, double);	/* space for the transpose */
	
	/* find maximum singular value and total variance */
	
	sing_max = w[0];
	total_variance = w[0];
	for (i = 1; i < n; i++) {
		sing_max = MAX (sing_max, w[i]);
		total_variance += w[i];
	}
	
	if (mode) {
		/* mode = 1: Find the m largest singular values needed to explain the specified variance level.
		 * mode = 2: Find the m largest singular values, with m = limit.
		 * Either case requires sorted singular values so we need to do some work first.
		 * It also assumes that the matrix passed is a squared normal equation kind of matrix
		 * so that the singular values are the individual variace contributions. */
		struct GMT_SINGULAR_VALUE {
			double value;
			unsigned int order;
		} *eigen;
		unsigned int n_eigen = 0;
		eigen = GMT_memory (GMT, NULL, n, struct GMT_SINGULAR_VALUE);
		for (i = 0; i < n; i++) {	/* Load in original order from w */
			eigen[i].value = w[i];
			eigen[i].order = i;
		}
		qsort (eigen, n, sizeof (struct GMT_SINGULAR_VALUE), compare_singular_values);
		/* Need desired variance level in % */
		limit = (*cutoff) * total_variance * 0.01;
		if (mode == 2) n_eigen = (unsigned int)lrint (*cutoff);
		for (i = 0; i < n; i++) {	/* Visit all singular values in decreasing magnitude */
			if ((mode == 1 && variance <= limit) || (mode == 2 && i < n_eigen)) {	/* Still within specified limit so we add this singular value */
				variance += eigen[i].value;
				w[eigen[i].order] = 1.0 / w[eigen[i].order];
				n_use++;
			}
			else	/* Sorry, we're letting you go */
				w[eigen[i].order] = 0.0;
		}
		GMT_free (GMT, eigen);
	}
	else {	/* Loop through singular values removing small ones (if limit is nonzero) */
		limit = *cutoff;
		for (i = 0; i < n; i++) {
			if ((w[i]/sing_max) > limit) {
				variance += w[i];
				w[i] = 1.0 / w[i];
				n_use++;
			}
			else
				w[i] = 0.0;
		}
	}		
	*cutoff = (100.0 * variance / total_variance);	/* Actual explained variance level in % */
	if (mode == 0)
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "GMT_solve_svd: Ratio limit %g gave %d singular values that explain %g %% of total variance %g\n", limit, n_use, *cutoff, total_variance);
	if (mode == 1)
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "GMT_solve_svd: Found %d singular values needed to explain %g %% of total variance %g\n", n_use, *cutoff, total_variance);
	else
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "GMT_solve_svd: Selected %d singular values that explain %g %% of total variance %g\n", n_use, *cutoff, total_variance);
	
	/* multiply V by 1/w */
	
	for (i = 0; i < n; i++) for (j = 0; j < n; j++) v[j*n+i] *= w[i];
			
	/* get transpose of U */
		
	gmt_mat_trans (u, m, n, ut);
	
	/* multiply v(1/w)ut  -> this overwrites the matrix U */
		
	gmt_mat_mult (v, n, n, ut, m, u);
	
	/* multiply this result by b to get x */
		
	gmt_mat_mult (u, n, m, b, k, x);

	/* free work space */

	GMT_free (GMT, ut);
	
	return (n_use);
}

double GMT_dot3v (struct GMT_CTRL *GMT_UNUSED(GMT), double *a, double *b)
{
	return (a[GMT_X]*b[GMT_X] + a[GMT_Y]*b[GMT_Y] + a[GMT_Z]*b[GMT_Z]);
}

double GMT_dot2v (struct GMT_CTRL *GMT_UNUSED(GMT), double *a, double *b)
{
	return (a[GMT_X]*b[GMT_X] + a[GMT_Y]*b[GMT_Y]);
}

double GMT_mag3v (struct GMT_CTRL *GMT_UNUSED(GMT), double *a)
{
	return (d_sqrt(a[GMT_X]*a[GMT_X] + a[GMT_Y]*a[GMT_Y] + a[GMT_Z]*a[GMT_Z]));
}

void GMT_add3v (struct GMT_CTRL *GMT_UNUSED(GMT), double *a, double *b, double *c)
{	/* C = A + B */
	int k;
	for (k = 0; k < 3; k++) c[k] = a[k] + b[k];
}

void GMT_sub3v (struct GMT_CTRL *GMT_UNUSED(GMT), double *a, double *b, double *c)
{	/* C = A - B */
	int k;
	for (k = 0; k < 3; k++) c[k] = a[k] - b[k];
}

void GMT_normalize3v (struct GMT_CTRL *GMT, double *a)
{
	double r_length;
	r_length = GMT_mag3v (GMT,a);
	if (r_length != 0.0) {
		r_length = 1.0 / r_length;
		a[GMT_X] *= r_length;
		a[GMT_Y] *= r_length;
		a[GMT_Z] *= r_length;
	}
}

void GMT_normalize2v (struct GMT_CTRL *GMT_UNUSED(GMT), double *a)
{
	double r_length;
	r_length = hypot (a[GMT_X], a[GMT_Y]);
	if (r_length != 0.0) {
		r_length = 1.0 / r_length;
		a[GMT_X] *= r_length;
		a[GMT_Y] *= r_length;
	}
}

void GMT_cross3v (struct GMT_CTRL *GMT_UNUSED(GMT), double *a, double *b, double *c)
{
	c[GMT_X] = a[GMT_Y] * b[GMT_Z] - a[GMT_Z] * b[GMT_Y];
	c[GMT_Y] = a[GMT_Z] * b[GMT_X] - a[GMT_X] * b[GMT_Z];
	c[GMT_Z] = a[GMT_X] * b[GMT_Y] - a[GMT_Y] * b[GMT_X];
}

void GMT_matrix_vect_mult (struct GMT_CTRL *GMT_UNUSED(GMT), unsigned int dim, double a[3][3], double b[3], double c[3])
{	/* c = A * b for 2 or 3 D */
	unsigned int i, j;

	for (i = 0; i < dim; i++) for (j = 0, c[i] = 0.0; j < dim; j++) c[i] += a[i][j] * b[j];
}

void GMT_make_rot_matrix2 (struct GMT_CTRL *GMT_UNUSED(GMT), double E[3], double w, double R[3][3])
{	/* Based on Cox and Hart, 1986 */
/*	E	Euler pole in in cartesian coordinates
 *	w	angular rotation in degrees
 *
 *	R	the 3x3 rotation matrix
 */

	double sin_w, cos_w, c, E_x, E_y, E_z, E_12c, E_13c, E_23c;

	sincosd (w, &sin_w, &cos_w);
	c = 1 - cos_w;

	E_x = E[0] * sin_w;
	E_y = E[1] * sin_w;
	E_z = E[2] * sin_w;
	E_12c = E[0] * E[1] * c;
	E_13c = E[0] * E[2] * c;
	E_23c = E[1] * E[2] * c;

	R[0][0] = E[0] * E[0] * c + cos_w;
	R[0][1] = E_12c - E_z;
	R[0][2] = E_13c + E_y;

	R[1][0] = E_12c + E_z;
	R[1][1] = E[1] * E[1] * c + cos_w;
	R[1][2] = E_23c - E_x;

	R[2][0] = E_13c - E_y;
	R[2][1] = E_23c + E_x;
	R[2][2] = E[2] * E[2] * c + cos_w;
}

void GMT_make_rot_matrix (struct GMT_CTRL *GMT, double lonp, double latp, double w, double R[3][3])
{
/*	lonp, latp	Euler pole in degrees
 *	w		angular rotation in degrees
 *
 *	R		the rotation matrix
 */

	double E[3];

        GMT_geo_to_cart (GMT, latp, lonp, E, true);
	GMT_make_rot_matrix2 (GMT, E, w, R);
}


void GMT_geo_to_cart (struct GMT_CTRL *GMT_UNUSED(GMT), double lat, double lon, double *a, bool degrees)
{
	/* Convert geographic latitude and longitude (lat, lon)
	   to a 3-vector of unit length (a). If degrees = true,
	   input coordinates are in degrees, otherwise in radian */

	double clat, clon, slon;

	if (degrees) {
		lat *= D2R;
		lon *= D2R;
	}
	sincos (lat, &a[GMT_Z], &clat);
	sincos (lon, &slon, &clon);
	a[GMT_X] = clat * clon;
	a[GMT_Y] = clat * slon;
}

void GMT_cart_to_geo (struct GMT_CTRL *GMT_UNUSED(GMT), double *lat, double *lon, double *a, bool degrees)
{
	/* Convert a 3-vector (a) of unit length into geographic
	   coordinates (lat, lon). If degrees = true, the output coordinates
	   are in degrees, otherwise in radian. */

	if (degrees) {
		*lat = d_asind (a[GMT_Z]);
		*lon = d_atan2d (a[GMT_Y], a[GMT_X]);
	}
	else {
		*lat = d_asin (a[GMT_Z]);
		*lon = d_atan2 (a[GMT_Y], a[GMT_X]);
	}
}

void GMT_polar_to_cart (struct GMT_CTRL *GMT_UNUSED(GMT), double r, double theta, double *a, bool degrees)
{
	/* Convert polar (cylindrical) coordinates r, theta
	   to a 2-vector of unit length (a). If degrees = true,
	   input theta is in degrees, otherwise in radian */

	if (degrees) theta *= D2R;
	sincos (theta, &a[GMT_Y], &a[GMT_X]);
	a[GMT_X] *= r;
	a[GMT_Y] *= r;
}

void GMT_cart_to_polar (struct GMT_CTRL *GMT_UNUSED(GMT), double *r, double *theta, double *a, bool degrees)
{
	/* Convert a 2-vector (a) of unit length into polar (cylindrical)
	   coordinates (r, theta). If degrees = true, the output coordinates
	   are in degrees, otherwise in radian. */

	*r = hypot (a[GMT_X], a[GMT_Y]);
	*theta = d_atan2 (a[GMT_Y], a[GMT_X]);
	if (degrees) *theta *= R2D;
}

uint64_t GMT_fix_up_path (struct GMT_CTRL *GMT, double **a_lon, double **a_lat, uint64_t n, double step, unsigned int mode)
{
	/* Takes pointers to a list of <n> lon/lat pairs (in degrees) and adds
	 * auxiliary points if the great circle distance between two given points exceeds
	 * <step> spherical degree.  If step <= 0 we use the default path_step.
	 * If mode=0: returns points along a great circle
	 * If mode=1: first follows meridian, then parallel
	 * If mode=2: first follows parallel, then meridian
	 * Returns the new number of points (original plus auxiliary).
	 */

	unsigned int k = 1;
	bool meridian;
#ifdef DEBUG
	bool dump = false;
#endif
	uint64_t i, j, n_new, n_step = 0;
	double a[3], b[3], x[3], *lon = NULL, *lat = NULL;
	double c, d, fraction, theta, minlon, maxlon;

	lon = *a_lon;	lat = *a_lat;	/* Input arrays */

	GMT_geo_to_cart (GMT, lat[0], lon[0], a, true);	/* Start point of current arc */
	GMT_prep_tmp_arrays (GMT, 1, 2);	/* Init or reallocate tmp vectors */
	GMT->hidden.mem_coord[GMT_X][0] = lon[0];
	GMT->hidden.mem_coord[GMT_Y][0] = lat[0];
	n_new = 1;
	if (step <= 0.0) step = GMT->current.map.path_step;	/* Based on GMT->current.setting.map_line_step converted to degrees */
	if (step <= 0.0) step = 0.1;				/* Safety valve when no -J and step not set. */

	for (i = 1; i < n; i++) {

		GMT_geo_to_cart (GMT, lat[i], lon[i], b, true);	/* End point of current arc */

		if (mode == 1) {	/* First follow meridian, then parallel */
			theta = fabs (lon[i]-lon[i-1]) * cosd (lat[i-1]);
			n_step = lrint (theta / step);
			for (j = 1; j < n_step; j++) {
				c = j / (double)n_step;
				GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp vectors */
				GMT->hidden.mem_coord[GMT_X][n_new] = lon[i-1] * (1 - c) + lon[i] * c;
				GMT->hidden.mem_coord[GMT_Y][n_new] = lat[i-1];
				n_new++;
			}
			theta = fabs (lat[i]-lat[i-1]);
			n_step = lrint (theta / step);
			for (j = k; j < n_step; j++) {	/* Start at 0 to make sure corner point is saved */
				c = j / (double)n_step;
				GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp vectors */
				GMT->hidden.mem_coord[GMT_X][n_new] = lon[i];
				GMT->hidden.mem_coord[GMT_Y][n_new] = lat[i-1] * (1 - c) + lat[i] * c;
				n_new++;
			}
			k = 0;
		}

		else if (mode == 2) {	/* First follow parallel, then meridian */
			theta = fabs (lat[i]-lat[i-1]);
			n_step = lrint (theta / step);
			for (j = 1; j < n_step; j++) {
				c = j / (double)n_step;
				GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp read vectors */
				GMT->hidden.mem_coord[GMT_X][n_new] = lon[i-1];
				GMT->hidden.mem_coord[GMT_Y][n_new] = lat[i-1] * (1 - c) + lat[i] * c;
				n_new++;
			}
			theta = fabs (lon[i]-lon[i-1]) * cosd(lat[i]);
			n_step = lrint (theta / step);
			for (j = k; j < n_step; j++) {	/* Start at 0 to make sure corner point is saved */
				c = j / (double)n_step;
				GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp read vectors */
				GMT->hidden.mem_coord[GMT_X][n_new] = lon[i-1] * (1 - c) + lon[i] * c;
				GMT->hidden.mem_coord[GMT_Y][n_new] = lat[i];
				n_new++;
			}
			k = 0;
		}

		/* Follow great circle */
		else if ((theta = d_acosd (GMT_dot3v (GMT, a, b))) == 180.0)	/* trouble, no unique great circle */
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: Two points in input list are antipodal - no resampling taken place!\n");

		else if ((n_step = lrint (theta / step)) > 1) {	/* Must insert (n_step - 1) points, i.e. create n_step intervals */
			fraction = 1.0 / (double)n_step;
			minlon = MIN (lon[i-1], lon[i]);
			maxlon = MAX (lon[i-1], lon[i]);
			meridian = doubleAlmostEqualZero (maxlon, minlon);	/* A meridian; make a gap so tests below will give right range */
			for (j = 1; j < n_step; j++) {
				c = j * fraction;
				d = 1 - c;
				for (k = 0; k < 3; k++) x[k] = a[k] * d + b[k] * c;
				GMT_normalize3v (GMT, x);		/* Make unit vector */
				GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp read vectors */
				GMT_cart_to_geo (GMT, &GMT->hidden.mem_coord[GMT_Y][n_new], &GMT->hidden.mem_coord[GMT_X][n_new], x, true);
				if (meridian)
					GMT->hidden.mem_coord[GMT_X][n_new] = minlon;
				else if (GMT->hidden.mem_coord[GMT_X][n_new] < minlon)
					GMT->hidden.mem_coord[GMT_X][n_new] += 360.0;
				else if (GMT->hidden.mem_coord[GMT_X][n_new] > maxlon)
					GMT->hidden.mem_coord[GMT_X][n_new] -= 360.0;
				n_new++;
			}
		}
		GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp read vectors */
		GMT->hidden.mem_coord[GMT_X][n_new] = lon[i];	GMT->hidden.mem_coord[GMT_Y][n_new] = lat[i];
		n_new++;
		GMT_cpy3v (a, b);
	}

	/* Destroy old allocated memory and put the new one in place */
	GMT_free (GMT, lon);
	GMT_free (GMT, lat);
	GMT_eliminate_lon_jumps (GMT, GMT->hidden.mem_coord[GMT_X], n_new);
	*a_lon = GMT_assign_vector (GMT, n_new, GMT_X);
	*a_lat = GMT_assign_vector (GMT, n_new, GMT_Y);

#ifdef DEBUG
	if (dump) {
		FILE *fp = fopen ("fixup.txt", "w");
		for (j = 0; j < n_new; j++) fprintf (fp, "%g\t%g\n", GMT->hidden.mem_coord[GMT_X][j], GMT->hidden.mem_coord[GMT_Y][j]);
		fclose (fp);
	}
#endif
	return (n_new);
}

uint64_t gmt_fix_up_path_cartesian (struct GMT_CTRL *GMT, double **a_x, double **a_y, uint64_t n, double step, unsigned int mode)
{
	/* Takes pointers to a list of <n> x/y pairs (in user units) and adds
	 * auxiliary points if the distance between two given points exceeds
	 * <step> units.
	 * If mode=0: returns points along a straight line
	 * If mode=1: staircase; first follows y, then x
	 * If mode=2: staircase; first follows x, then y
	 * Returns the new number of points (original plus auxiliary).
	 */

	unsigned int k = 1;
	uint64_t i, j, n_new, n_step = 0;
	double *x = NULL, *y = NULL, c;

	x = *a_x;	y = *a_y;

	GMT_prep_tmp_arrays (GMT, 1, 2);	/* Init or reallocate tmp vectors */
	GMT->hidden.mem_coord[GMT_X][0] = x[0];	GMT->hidden.mem_coord[GMT_Y][0] = y[0];	n_new = 1;
	if (step <= 0.0) step = 1.0;	/* Sanity valve; if step not given we set it to 1 */

	for (i = 1; i < n; i++) {
		if (mode == 1) {	/* First follow x, then y */
			n_step = lrint (fabs (x[i] - x[i-1]) / step);
			for (j = 1; j < n_step; j++) {
				c = j / (double)n_step;
				GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp read vectors */
				GMT->hidden.mem_coord[GMT_X][n_new] = x[i-1] * (1 - c) + x[i] * c;
				GMT->hidden.mem_coord[GMT_Y][n_new] = y[i-1];
				n_new++;
			}
			n_step = lrint (fabs (y[i]-y[i-1]) / step);
			for (j = k; j < n_step; j++) {	/* Start at 0 to make sure corner point is saved */
				c = j / (double)n_step;
				GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp read vectors */
				GMT->hidden.mem_coord[GMT_X][n_new] = x[i];
				GMT->hidden.mem_coord[GMT_Y][n_new] = y[i-1] * (1 - c) + y[i] * c;
				n_new++;
			}
			k = 0;
		}
		else if (mode == 2) {	/* First follow y, then x */
			n_step = lrint (fabs (y[i]-y[i-1]) / step);
			for (j = 1; j < n_step; j++) {
				c = j / (double)n_step;
				GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp read vectors */
				GMT->hidden.mem_coord[GMT_X][n_new] = x[i-1];
				GMT->hidden.mem_coord[GMT_Y][n_new] = y[i-1] * (1 - c) + y[i] * c;
				n_new++;
			}
			n_step = lrint (fabs (x[i]-x[i-1]) / step);
			for (j = k; j < n_step; j++) {	/* Start at 0 to make sure corner point is saved */
				c = j / (double)n_step;
				GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp read vectors */
				GMT->hidden.mem_coord[GMT_X][n_new] = x[i-1] * (1 - c) + x[i] * c;
				GMT->hidden.mem_coord[GMT_Y][n_new] = y[i];
				n_new++;
			}
			k = 0;
		}
		/* Follow straight line */
		else if ((n_step = lrint (hypot (x[i]-x[i-1], y[i]-y[i-1]) / step)) > 1) {	/* Must insert (n_step - 1) points, i.e. create n_step intervals */
			for (j = 1; j < n_step; j++) {
				c = j / (double)n_step;
				GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp read vectors */
				GMT->hidden.mem_coord[GMT_X][n_new] = x[i-1] * (1 - c) + x[i] * c;
				GMT->hidden.mem_coord[GMT_Y][n_new] = y[i-1] * (1 - c) + y[i] * c;
				n_new++;
			}
		}
		GMT_prep_tmp_arrays (GMT, n_new, 2);	/* Init or reallocate tmp read vectors */
		GMT->hidden.mem_coord[GMT_X][n_new] = x[i];	GMT->hidden.mem_coord[GMT_Y][n_new] = y[i];	n_new++;
	}

	/* Destroy old allocated memory and put the new one in place */
	GMT_free (GMT, x);	GMT_free (GMT, y);
	*a_x = GMT_assign_vector (GMT, n_new, GMT_X);
	*a_y = GMT_assign_vector (GMT, n_new, GMT_Y);
	
	return (n_new);
}

uint64_t gmt_resample_path_spherical (struct GMT_CTRL *GMT, double **lon, double **lat, uint64_t n_in, double step_out, enum GMT_enum_track mode)
{
	/* See GMT_resample_path below for details. */

	bool meridian, new_pair;
	uint64_t last_row_in = 0, row_in, row_out, n_out;
	unsigned int k;
	double dist_out, gap, d_lon = 0.0, L = 0.0, frac_to_a, frac_to_b, minlon, maxlon, a[3], b[3], c[3];
	double P[3], Rot0[3][3], Rot[3][3], total_angle_rad = 0.0, angle_rad, ya = 0.0, yb = 0.0;
	double *dist_in = NULL, *lon_out = NULL, *lat_out = NULL, *lon_in = *lon, *lat_in = *lat;

	if (step_out < 0.0) {	/* Safety valve */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal error: gmt_resample_path_spherical given negative step-size\n");
		return (EXIT_FAILURE);
	}
	if (mode > GMT_TRACK_SAMPLE_ADJ) {	/* Bad mode*/
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal error: gmt_resample_path_spherical given bad mode %d\n", mode);
		return (EXIT_FAILURE);
	}
	
	if (mode < GMT_TRACK_SAMPLE_FIX) {
		if (GMT->current.map.dist[GMT_MAP_DIST].arc)	/* Gave an increment in arc lenght (degree, min, sec) */
			step_out /= GMT->current.map.dist[GMT_MAP_DIST].scale;	/* Get degrees */
		else	/* Gave increment in spatial distance (km, meter, etc.) */
			step_out = (step_out / GMT->current.map.dist[GMT_MAP_DIST].scale) / GMT->current.proj.DIST_M_PR_DEG;	/* Get degrees */
		return (GMT_fix_up_path (GMT, lon, lat, n_in, step_out, mode));	/* Insert extra points only */
	}
	
	dist_in = GMT_dist_array (GMT, lon_in, lat_in, n_in, true);	/* Compute cumulative distances along line */
	
	if (step_out == 0.0) step_out = (dist_in[n_in-1] - dist_in[0])/100.0;	/* If nothing is selected we get 101 points */
	/* Determine n_out, the number of output points */
	if (mode == GMT_TRACK_SAMPLE_ADJ) {	/* Round to nearest multiple of step_out, then adjust step to match exactly */
		n_out = lrint (dist_in[n_in-1] / step_out);
		step_out = dist_in[n_in-1] / n_out;	/* Ensure exact fit */
	}
	else {	/* Stop when last multiple is reached */
		n_out = lrint (floor (dist_in[n_in-1] / step_out));
	}
	n_out++;	/* Since number of points = number of segments + 1 */
	
	lon_out = GMT_memory (GMT, NULL, n_out, double);
	lat_out = GMT_memory (GMT, NULL, n_out, double);
	
	lon_out[0] = lon_in[0];	lat_out[0] = lat_in[0];	/* Start at same origin */
	for (row_in = row_out = 1; row_out < n_out; row_out++) {	/* For remaining output points */
		dist_out = row_out * step_out;	/* Rhe desired output distance */
		while (row_in < n_in && dist_in[row_in] < dist_out) row_in++;	/* Wind so row points to the next input point with a distance >= dist_out */
		gap = dist_in[row_in] - dist_out;	/* Distance to next input datapoint */
		new_pair = (row_in > last_row_in);
		if (GMT_IS_ZERO (gap)) {	/* Use the input point as is */
			lon_out[row_out] = lon_in[row_in];	lat_out[row_out] = lat_in[row_in];
		}
		else {	/* Must interpolate along great-circle arc from a (point row_in-1) to b (point row_in) */
			if (new_pair) {	/* Must perform calculations on the two points */
				L = dist_in[row_in] - dist_in[row_in-1];	/* Distance between points a and b */
				ya = GMT_lat_swap (GMT, lat_in[row_in-1], GMT_LATSWAP_G2O);	/* Convert to geocentric */
				yb = GMT_lat_swap (GMT, lat_in[row_in],   GMT_LATSWAP_G2O);	/* Convert to geocentric */
			}
			frac_to_a = gap / L;
			frac_to_b = 1.0 - frac_to_a;
			if (GMT->current.map.loxodrome) {	/* Linear resampling along Mercator straight line */
				if (new_pair) GMT_set_delta_lon (lon_in[row_in-1], lon_in[row_in], d_lon);
				a[0] = D2R * lon_in[row_in-1];	a[1] = d_log (GMT, tand (45.0 + 0.5 * ya));
				b[0] = D2R * (lon_in[row_in-1] + d_lon);	b[1] = d_log (GMT, tand (45.0 + 0.5 * yb));
				for (k = 0; k < 2; k++) c[k] = a[k] * frac_to_a + b[k] * frac_to_b;	/* Linear interpolation to find output point c */
				lon_out[row_out] = c[0] * R2D;
				lat_out[row_out] = atand (sinh (c[1]));
				lat_out[row_out] = GMT_lat_swap (GMT, lat_out[row_out], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
			}
			else {	/* Spherical resampling along segment */
				if (new_pair) {	/* Must perform calculations on the two points */
					GMT_geo_to_cart (GMT, ya, lon_in[row_in-1], a, true);
					GMT_geo_to_cart (GMT, yb, lon_in[row_in],   b, true);
					total_angle_rad = d_acos (GMT_dot3v (GMT, a, b));	/* Get spherical angle from a to b in radians; this is same distance as L */
					GMT_cross3v (GMT, a, b, P);	/* Get pole P of plane trough a and b (and center of Earth) */
					GMT_normalize3v (GMT, P);		/* Make sure P has unit length */
					gmt_init_rot_matrix (Rot0, P);	/* Get partial rotation matrix since no actual angle is applied yet */
				}
				GMT_memcpy (Rot, Rot0, 9, double);			/* Get a copy of the "0-angle" rotation matrix */
				angle_rad = total_angle_rad * frac_to_b;		/* Angle we need to rotate from a to c */
				gmt_load_rot_matrix (angle_rad, Rot, P);		/* Build the actual rotation matrix for this angle */
				gmt_matrix_vect_mult (Rot, a, c);			/* Rotate from a to get c */
				GMT_cart_to_geo (GMT, &lat_out[row_out], &lon_out[row_out], c, true);
				lat_out[row_out] = GMT_lat_swap (GMT, lat_out[row_out], GMT_LATSWAP_O2G);	/* Convert back to geodetic */
			}
			minlon = MIN (lon_in[row_in-1], lon_in[row_in]);
			maxlon = MAX (lon_in[row_in-1], lon_in[row_in]);
			meridian = doubleAlmostEqualZero (maxlon, minlon);	/* A meridian; make sure we get right lon value */
			if (meridian)
				lon_out[row_out] = minlon;
			else if (lon_out[row_out] < minlon)
				lon_out[row_out] += 360.0;
			else if (lon_out[row_out] > maxlon)
				lon_out[row_out] -= 360.0;
		}
		last_row_in = row_in;
	}
	
	/* Destroy old allocated memory and put the new one in place */
	
	GMT_free (GMT, lon_in);
	GMT_free (GMT, lat_in);
	GMT_free (GMT, dist_in);
	*lon = lon_out;
	*lat = lat_out;
	return (n_out);
}

uint64_t gmt_resample_path_cartesian (struct GMT_CTRL *GMT, double **x, double **y, uint64_t n_in, double step_out, enum GMT_enum_track mode)
{
	/* See GMT_resample_path below for details. */

	uint64_t last_row_in = 0, row_in, row_out, n_out;
	bool new_pair;
	double dist_out, gap, L = 0.0, frac_to_a, frac_to_b;
	double *dist_in = NULL, *x_out = NULL, *y_out = NULL, *x_in = *x, *y_in = *y;

	if (step_out < 0.0) {	/* Safety valve */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal error: gmt_resample_path_cartesian given negative step-size\n");
		return (EXIT_FAILURE);
	}
	if (mode > GMT_TRACK_SAMPLE_ADJ) {	/* Bad mode*/
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Internal error: gmt_resample_path_cartesian given bad mode %d\n", mode);
		return (EXIT_FAILURE);
	}
	
	if (mode < GMT_TRACK_SAMPLE_FIX) return (gmt_fix_up_path_cartesian (GMT, x, y, n_in, step_out, mode));	/* Insert extra points only */
	
	dist_in = GMT_dist_array (GMT, x_in, y_in, n_in, true);	/* Compute cumulative distances along line */
	if (step_out == 0.0) step_out = (dist_in[n_in-1] - dist_in[0])/100.0;	/* If nothing is selected we get 101 points */
	
	/* Determine n_out, the number of output points */
	if (mode == GMT_TRACK_SAMPLE_ADJ) {	/* Round to nearest multiples, then adjust step to match exactly */
		n_out = lrint (dist_in[n_in-1] / step_out);
		step_out = dist_in[n_in-1] / n_out;
	}
	else {	/* Stop when last multiple is reached */
		n_out = lrint (floor (dist_in[n_in-1] / step_out));
	}
	n_out++;	/* Since number of points = number of segments + 1 */
	
	x_out = GMT_memory (GMT, NULL, n_out, double);
	y_out = GMT_memory (GMT, NULL, n_out, double);
	
	x_out[0] = x_in[0];	y_out[0] = y_in[0];	/* Start at same origin */
	for (row_in = row_out = 1; row_out < n_out; row_out++) {	/* For remaining output points */
		dist_out = row_out * step_out;	/* The desired output distance */
		while (row_in < n_in && dist_in[row_in] < dist_out) row_in++;	/* Wind so row points to the next input point with a distance >= dist_out */
		gap = dist_in[row_in] - dist_out;	/* Distance to next input datapoint */
		new_pair = (row_in > last_row_in);
		if (GMT_IS_ZERO (gap)) {	/* Use the input point as is */
			x_out[row_out] = x_in[row_in];	y_out[row_out] = y_in[row_in];
		}
		else {	/* Must interpolate along great-circle arc from a (point row_in-1) to b (point row_in) */
			if (new_pair) L = dist_in[row_in] - dist_in[row_in-1];	/* Distance between points a and b */
			frac_to_a = gap / L;
			frac_to_b = 1.0 - frac_to_a;
			x_out[row_out] = x_in[row_in-1] * frac_to_a + x_in[row_in] * frac_to_b;	/* Linear interpolation to find output point */
			y_out[row_out] = y_in[row_in-1] * frac_to_a + y_in[row_in] * frac_to_b;	/* Linear interpolation to find output point */
		}
		last_row_in = row_in;
	}
	
	/* Destroy old allocated memory and put the new one in place */
	
	GMT_free (GMT, x_in);
	GMT_free (GMT, y_in);
	GMT_free (GMT, dist_in);
	*x = x_out;
	*y = y_out;
	return (n_out);
}

uint64_t GMT_resample_path (struct GMT_CTRL *GMT, double **x, double **y, uint64_t n_in, double step_out, enum GMT_enum_track mode)
{
	/* Takes pointers to a list of <n_in> x/y pairs (in degrees or Cartesian units) and computes
	 * the distance along that path.  We then determine new coordinates at new distances that are
	 * multiples of the desired step <step_out> which are in the unit set via GMT_init_distaz (geo)
	 * or user Cartesian.  The new path will always contain the first input point, but anything
	 * beyond that start depends on the mode:
	 * mode = GMT_TRACK_FILL	: Keep input points; add intermediates if any gap exceeds step_out.
	 * mode = GMT_TRACK_FILL_M	: Same, but traverse along meridians, then parallels between points.
	 * mode = GMT_TRACK_FILL_P	: Same, but traverse along parallels, then meridians between points.
	 * mode = GMT_TRACK_SAMPLE_FIX	: Resample track equidistantly; old points may be lost. Use given spacing.
	 * mode = GMT_TRACK_SAMPLE_ADJ	: Resample track equidistantly; old points may be lost. Adjust spacing to fit tracklength exactly.
	 * Returns the new number of points.
	 */
	uint64_t n_out;
	if (GMT_is_geographic (GMT, GMT_IN))
		n_out = gmt_resample_path_spherical (GMT, x, y, n_in, step_out, mode);
	else
		n_out = gmt_resample_path_cartesian (GMT, x, y, n_in, step_out, mode);
	return (n_out);
}

int GMT_chol_dcmp (struct GMT_CTRL *GMT_UNUSED(GMT), double *a, double *d, double *cond, int nr, int n) {

	/* Given a, a symmetric positive definite matrix
	of size n, and row dimension nr, compute a lower
	triangular matrix b, the Cholesky decomposition 
	of a, so that a = bb'.
	The elements of b over-write the diagonal and sub-
	diagonal elements of a.  The diagonal elements of 
	a are saved in d, and a's super-diagonal elements
	are unchanged, permitting reconstruction of a in
	the event that the algorithm fails.
	Does not do any pivoting or balancing (I wrote it
	for application to Gram matrices, where all the
	diagonal elements of a are the same size.)
	Returns 0 on success, -k on failure, where k is
	the number (in 1 to n) of the point that failed.
	Elements a(i,j) for j < k, and a(k,k) will need
	restoration in this case.
	Success means that the procedure ran to completion
	without a negative square root or a divide by zero.
	It does not guarantee that the system is well-
	conditioned.  The condition number is returned
	in cond as max(b(i,i)) / min(b(i,i)).  Note that
	the condition number of a would be the square of
	this.  This condition number is only set if the
	procedure runs successfully.

	W H F Smith, 18 Feb 2000.
*/
	int i, j, k, ik, ij, kj, kk, nrp1;
	double eigmax, eigmin;

	nrp1 = nr + 1;

	eigmax = eigmin = sqrt (fabs (a[0]));

	for (k = 0, kk = 0; k < n; k++, kk+=nrp1 ) {
		d[k] = a[kk];
		for (j = 0, kj = k; j < k; j++, kj += nr) a[kk] -= (a[kj]*a[kj]);
		if (a[kk] <= 0.0) return (-(k+1));
		a[kk] = sqrt(a[kk]);
		if (a[kk] <= 0.0) return (-(k+1));	/* Shouldn't happen ?  */

		if (eigmax < a[kk]) eigmax = a[kk];
		if (eigmin > a[kk]) eigmin = a[kk];

		for (i = k+1; i < n; i++) {
			ik = i + k*nr;
			for (j = 0, ij = i, kj = k; j < k; j++, ij+=nr, kj+=nr) a[ik] -= (a[ij]*a[kj]);
			a[ik] /= a[kk];
		}
	}
	*cond = eigmax/eigmin;
	return (0);
}

void GMT_chol_recover (struct GMT_CTRL *GMT_UNUSED(GMT), double *a, double *d, int nr, int n, int nerr, bool donly) {

	/* Given a, a symmetric positive definite matrix of row dimension nr,
	and size n >= abs(nerr), one uses GMT_chol_dcmp() to attempt to find
	b, a lower triangular Cholesky decomposition of a, so that b*b' = a.
	If a is (numerically) not positive definite then GMT_chol_dcmp()
	returns a negative integer nerr, indicating that the diagonal 
	elements of a from a(1,1) to a(-nerr, -nerr) and the sub-diagonal
	elements in columns from 1 to abs(nerr)-1 have been overwritten,
	but the Cholesky decomposition did not run to completion.  A vector
	d has been assigned the original diagonal elements of a from 1 to
	abs(nerr), in this case.

	GMT_chol_recover() takes a and d and restores a so that some other
	solution of a may be attempted.  

	If (donly != 0) then only the diagonal elements of a will be restored.
	This might be enough if the next attempt will be to run GMT_jacobi()
	on a, for the jacobi routine uses only the upper right triangle of
	a.  If (donly == 0) then all elements of a will be restored, by
	transposing the upper half to the lower half.

	To use these routines, the call should be:

	if ( (ier = GMT_chol_dcmp (a, d, &cond, nr, n) ) != 0) {
		GMT_chol_recover (a, d, nr, ier, donly);
		[and then solve some other way, e.g. GMT_jacobi]
	}
	else {
		GMT_chol_solv (a, x, y, nr, n);
	}

	W H F Smith, 18 Feb 2000
*/

	int kbad, i, j, ii, ij, ji, nrp1;

	kbad = abs (nerr) - 1;
	nrp1 = nr + 1;

	for (i = 0, ii = 0; i <= kbad; i++, ii += nrp1) a[ii] = d[i];

	if (donly) return;

	for (j = 0; j < kbad; j++) {
		for (i = j+1, ij = j*nrp1 + 1, ji = j*nrp1 + nr; i < n; i++, ij++, ji+=nr) a[ij] = a[ji];
	}
	return;
}

void GMT_chol_solv (struct GMT_CTRL *GMT_UNUSED(GMT), double *a, double *x, double *y, int nr, int n) {
	/* Given an n by n linear system ax = y, with a a symmetric, 
	positive-definite matrix, y a known vector, and x an unknown
	vector, this routine finds x, if a holds the lower-triangular
	Cholesky factor of a obtained from GMT_chol_dcmp().  nr is the
	row dimension of a.
	The lower triangular Cholesky factor is b such that bb' = a,
	so x is found from y using bt = y, t = b'x, where t is a
	temporary vector.  t is found by forward elimination, and
	then x is found by backward elimination.  t is stored in x
	temporarily.
	This routine does not check the condition number of b.  It
	assumes that GMT_chol_dcmp() ran without error, which means
	that all diagonal elements b[ii] are positive; these are
	divisors in the loops below.

	W H F Smith, 18 Feb 2000
*/
	int i, j, ij, ji, ii, nrp1;

	nrp1 = nr + 1;

	/* Find t, store in i, working forward:  */
	for (i = 0, ii = 0; i < n; i++, ii += nrp1) {
		x[i] = y[i];
		for (j = 0, ij = i; j < i; j++, ij += nr) x[i] -= (a[ij] * x[j]);
		x[i] /= a[ii];
	}

	/* Find x, starting from t stored in x, going backward:  */
	for (i = n-1, ii = (n-1)*nrp1; i >= 0; i--, ii -= nrp1) {
		for (j = n-1, ji = (n-1)+i*nr; j > i; j--, ji--) x[i] -= (a[ji] * x[j]);
		x[i] /= a[ii];
	}
	return;
}
