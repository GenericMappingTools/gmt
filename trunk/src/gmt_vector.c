/*--------------------------------------------------------------------
 *	$Id: gmt_vector.c,v 1.37 2011-03-03 21:02:51 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

/*
 * Author:	Walter H.F. Smith
 * Date:	12-JUL-2000
 * Version:	4.1.x
 */
 
#define GMT_WITH_NO_PS
#include "gmt.h"

#define MAX_SWEEPS 50

GMT_LONG	GMT_jacobi (double *a, GMT_LONG *n, GMT_LONG *m, double *d, double *v, double *b, double *z, GMT_LONG *nrots) {
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
 * expression to be evaluated as TRUE or FALSE and execution to 
 * continue.  If the run environment doesn't allow this, the routine
 * won't work properly.
 *
 * programmer:	W. H. F. Smith, 7 June, 1991.
 * Revised:	PW: 12-MAR-1998 for GMT 3.1
 * Revision by WHF Smith, March 03, 2000, to speed up loop indexes.
 */
	GMT_LONG	p, q, pp, pq, mp1, pm, qm, nsweeps, j, jm, i, k;
	double	sum, threshold, g, h, t, theta, c, s, tau;


	/* Begin by initializing v, b, d, and z.  v = identity matrix,
		b = d = diag(a), and z = 0:  */
	
	memset ((void *)v, 0, (size_t)((*m)*(*n)*sizeof(double)) );
	memset ((void *)z, 0, (size_t)((*n)*sizeof(double)) );
	
	mp1 = (*m) + 1;
	
	for (p = 0, pp = 0; p < (*n); p++, pp+=mp1) {
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
		for (q = 1, qm = (*m); q < (*n); q++, qm += (*m) ) {
			for (p = 0, pq = qm; p < q; p++, pq++) {
				sum += fabs(a[pq]);
			}
		}
		
		/* Exit this loop (converged) when sum == 0.0  */
		if (sum == 0.0) break;


		/* If (nsweeps < 3) do only bigger elements;  else all  */
		threshold =  (nsweeps < 3) ? 0.2 * sum / ( (*n) * (*n) ) : 0.0;

		/* Now sweep whole upper triangle doing Givens rotations:  */
		
		for (q = 1, qm = (*m); q < (*n); q++, qm += (*m) ) {
			for (p = 0, pm = 0, pq = qm; p < q; p++, pm += (*m), pq++) {
				/* In 3/2000 I swapped order of these loops,
					to allow simple incrementing of pq  */
			
				if (a[pq] == 0.0) continue;	/* New 3/2000  */
			
				g = 100.0 * fabs(a[pq]);
				
				/* After four sweeps, if g is small relative
					to a(p,p) and a(q,q), skip the 
					rotation and set a(p,q) to zero.  */

				if ( (nsweeps > 3) && ( (fabs(d[p])+g) == fabs(d[p]) ) && ( (fabs(d[q])+g) == fabs(d[q]) ) ) {
					a[pq] = 0.0;
				}
				else if (fabs(a[pq]) > threshold) {

					h = d[q] - d[p];
					
					if (h == 0.0) {
						t = 1.0;	/* This if block is new 3/2000  */
					}
					else if ( (fabs(h)+g) ==  fabs(h) ) {
						t = a[pq] / h;
					}
					else {
						theta = 0.5 * h / a[pq];
						t = 1.0 / (fabs(theta) + sqrt(1.0 + theta*theta) );
						if (theta < 0.0) t = -t;
					}

					c = 1.0 / sqrt(1.0 + t*t);
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
					for (j = p+1, jm = (*m)*(p+1); j < q; j++, jm += (*m) ) {
						g = a[p + jm];
						h = a[j + qm];
						a[p + jm] = g - s * (h + g * tau);
						a[j + qm] = h + s * (g - h * tau);
					}
					for (j = q+1, jm = (*m)*(q+1); j < (*n); j++, jm += (*m) ) {
						g = a[p + jm];
						h = a[q + jm];
						a[p + jm] = g - s * (h + g * tau);
						a[q + jm] = h + s * (g - h * tau);
					}

					for (j = 0; j < (*n); j++) {
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

		for (p = 0; p < (*n); p++) {
			b[p] += z[p];	/* Update the b copy of diagonal  */
			d[p] = b[p];	/* Replace d with b to reduce round-off error  */
			z[p] = 0.0;	/* Clear z.  */
		}
	}

	/* Get here via break when converged, or when nsweeps == MAX_SWEEPS.
		Sort eigenvalues by insertion:  */

	for (i = 0; i < (*n)-1; i++) {
		k = i;
		g = d[i];
		for (j = i+1; j < (*n); j++) {  /* Find max location  */
			if (d[j] >= g) {
				k = j;
				g = d[j];
			}
		}
		if (k != i) {  /*  Need to swap value and vector  */
			d[k] = d[i];
			d[i] = g;
			p = i * (*m);
			q = k * (*m);
			for (j = 0; j < (*n); j++) {
				g = v[j + p];
				v[j + p] = v[j + q];
				v[j + q] = g;
			}
		}
	}

	/* Return 0 if converged; else print warning and return -1:  */

	if (nsweeps == MAX_SWEEPS) {
		fprintf (stderr, "GMT_jacobi:  Failed to converge in %ld sweeps\n", nsweeps);
		return(-1);
	}
	return(0);
}

#ifdef OBSOLETE
GMT_LONG	GMT_jacobi_old (double *a, GMT_LONG *n, GMT_LONG *m, double *d, double *v, double *b, double *z, GMT_LONG *nrots)
/*
 *
 * Find eigenvalues & eigenvectors of a square symmetric matrix by Jacobi's
 * method, which is a convergent series of Givens rotations.
 * Modified from Numerical Recipes FORTRAN edition.
 * Returns integer 0 if OK, -1 if failure to converge in MAX_SWEEPS.
 *
 * programmer:	W. H. F. Smith, 7 June, 1991.
 * Revised:	PW: 12-MAR-1998 for GMT 3.1
 *
 * Caveat Emptor!  Assumes underflows return zero without killing execution.
 * I am not sure what happens if the eigenvalues are degenerate or not distinct.
 */
 

      	    	/* Sent.  n by n matrix in full storage mode.
		On return, superdiagonal elements are destroyed.  */
   	   	/* Sent.  row and column dimension of a as used.  */
   	   	/* Sent.  row and column dimension of a and v as
		allocated, so that a(i,j) is at a[i + (*m)*j].  */
      	    	/* Returned.  vector of n eigenvalues of a.  */
      	    	/* Returned.  n x n matrix of eigenvectors of a,
		with row dimension m  */
      	    	/* Work vector of n elements must be supplied.  */
      	    	/* Another work vector of n elements must be supplied.  */
   	       	/* Returned.  number of Givens rotations performed.  */

{
	GMT_LONG	ip, iq, nsweeps, i, j, k;
	double	sum, threshold, g, h, t, theta, c, s, tau, p;


	/* Begin by initializing v, b, d, and z.  v = identity matrix,
		b = d = diag(a), and z = 0:  */

	for (ip = 0; ip < (*n); ip++) {
		for (iq = 0; iq < (*n); iq++) {
			v[ip + (*m)*iq] = 0.0;
		}
		v[ip + (*m)*ip] = 1.0;
		b[ip] = a[ip + (*m)*ip];
		d[ip] = b[ip];
		z[ip] = 0.0;
	}

	/* End of initializations.  Set counters and begin:  */

	(*nrots) = 0;
	nsweeps = 0;

	while (nsweeps < MAX_SWEEPS) {

		/* Convergence test:
			Sum off-diagonal elements of upper triangle.
			When sum == 0.0 (underflow !) we have converged.
			In this case, break out of while loop.  */

		sum = 0.0;
		for (ip = 0; ip < (*n)-1; ip++) {
			for (iq = ip+1; iq < (*n); iq++) {
				sum += fabs(a[ip + (*m)*iq]);
			}
		}
		if (sum == 0.0) break;

		/* Now we are not converged.
			If (nsweeps < 3) do only the big elements;
				else do them all.  */
		if (nsweeps < 3) {
			threshold = 0.2 * sum / ( (*n) * (*n) );
		}
		else {
			threshold = 0.0;
		}

		/* Now sweep whole upper triangle doing Givens rotations:  */

		for (ip = 0; ip < (*n) - 1; ip++) {
			for (iq = ip+1; iq < (*n); iq++) {

				/* After four sweeps, if the off-diagonal
					element is "small", skip the rotation
					and just set it to zero.  "Small" is
					a relative test by addition:  */

				g = 100.0 * fabs(a[ip + (*m)*iq]);

				if ( (nsweeps > 3) && ( (fabs(d[ip])+g) ==  fabs(d[ip]) ) && ( (fabs(d[iq])+g) ==  fabs(d[iq]) ) ) {
					a[ip + (*m)*iq] = 0.0;
				}
				else if (fabs(a[ip + (*m)*iq]) > threshold) {

					h = d[iq] - d[ip];

					if ( (fabs(h)+g) ==  fabs(h) ) {
						/* I think this could divide by zero if a(i,j) = a(i,i) = a(j,j) = 0.0.
							Would this occur only in a degenerate matrix?  */
						t = a[ip + (*m)*iq] / h;
					}
					else {
						theta = 0.5 * h / a[ip + (*m)*iq];
						t = 1.0 / (fabs(theta) + sqrt(1.0 + theta*theta) );
						if (theta < 0.0) t = -t;
					}

					c = 1.0 / sqrt(1.0 + t*t);
					s = t * c;
					tau = s / (1.0 + c);
					
					h = t * a[ip + (*m)*iq];
					z[ip] -= h;
					z[iq] += h;
					d[ip] -= h;
					d[iq] += h;
					a[ip + (*m)*iq] = 0.0;

					for (j = 0; j < ip; j++) {
						g = a[j + (*m)*ip];
						h = a[j + (*m)*iq];
						a[j + (*m)*ip] = g - s * (h + g * tau);
						a[j + (*m)*iq] = h + s * (g - h * tau);
					}
					for (j = ip+1; j < iq; j++) {
						g = a[ip + (*m)*j];
						h = a[j + (*m)*iq];
						a[ip + (*m)*j] = g - s * (h + g * tau);
						a[j + (*m)*iq] = h + s * (g - h * tau);
					}
					for (j = iq+1; j < (*n); j++) {
						g = a[ip + (*m)*j];
						h = a[iq + (*m)*j];
						a[ip + (*m)*j] = g - s * (h + g * tau);
						a[iq + (*m)*j] = h + s * (g - h * tau);
					}

					for (j = 0; j < (*n); j++) {
						g = v[j + (*m)*ip];
						h = v[j + (*m)*iq];
						v[j + (*m)*ip] = g - s * (h + g * tau);
						v[j + (*m)*iq] = h + s * (g - h * tau);
					}

					(*nrots)++;
				}
			}
		}

		for (ip = 0; ip < (*n); ip++) {
			b[ip] += z[ip];
			d[ip] = b[ip];
			z[ip] = 0.0;
		}

		nsweeps++;
	}

	/* Get here via break when converged, or when nsweeps == MAX_SWEEPS.
		Sort eigenvalues by insertion:  */

	for (i = 0; i < (*n)-1; i++) {
		k = i;
		p = d[i];
		for (j = i+1; j < (*n); j++) {  /* Find max location  */
			if (d[j] >= p) {
				k = j;
				p = d[j];
			}
		}
		if (k != i) {  /*  Need to swap value and vector  */
			d[k] = d[i];
			d[i] = p;
			for (j = 0; j < (*n); j++) {
				p = v[j + (*m)*i];
				v[j + (*m)*i] = v[j + (*m)*k];
				v[j + (*m)*k] = p;
			}
		}
	}

	/* Return 0 if converged; else print warning and return -1:  */

	if (nsweeps == MAX_SWEEPS) {
		fprintf(stderr,"GMT_jacobi:  Failed to converge in %d sweeps\n", nsweeps);
		return(-1);
	}
	return(0);
}
#endif

void GMT_gauss (double *a, double *vec, GMT_LONG n_in, GMT_LONG nstore_in, double test, GMT_LONG *ierror, GMT_LONG itriag)
{
 
/* subroutine gauss, by william menke */
/* july 1978 (modified feb 1983, nov 85) */
 
/* a subroutine to solve a system of n linear equations in n unknowns*/
/* gaussian reduction with partial pivoting is used */
/*      a               (sent, destroyed)       n by n matrix           */
/*      vec             (sent, overwritten)     n vector, replaced w/ solution*/
/*      nstore          (sent)                  dimension of a  */
/*      test            (sent)                  div by zero check number*/
/*      ierror          (returned)              zero on no error*/
/*      itriag          (sent)                  matrix triangularized only*/
/*                                               on TRUE useful when solving*/
/*                                               multiple systems with same a */
        static GMT_LONG l1;
        GMT_LONG *line, *isub, i = 0, j, k, l, j2, n, nstore;
	GMT_LONG iet, ieb;
        double big, testa, b, sum;

        iet=0;  /* initial error flags, one for triagularization*/
        ieb=0;  /* one for backsolving */
	n = n_in;
	nstore = nstore_in;
	(void)GMT_alloc_memory2 ((void **)&line, (void **)&isub, n, 0, sizeof (GMT_LONG), "GMT_gauss");

/* triangularize the matrix a*/
/* replacing the zero elements of the triangularized matrix */
/* with the coefficients needed to transform the vector vec */

        if (itriag) {   /* triangularize matrix */
 
                for( j=0; j<n; j++ ) {      /*line is an array of flags*/
                        line[j]=0; 
                        /* elements of a are not moved during pivoting*/
                        /* line=0 flags unused lines */
                        }    /*end for j*/
                        
                for( j=0; j<n-1; j++ ) {
                        /*  triangularize matrix by partial pivoting */
                       big = 0.0; /* find biggest element in j-th column*/
                                  /* of unused portion of matrix*/
                       for( l1=0; l1<n; l1++ ) {
                               if( line[l1]==0 ) {
                                       testa=(double) fabs((double) (*(a+l1*nstore+j)));
                                       if (testa>big) {
                                                i=l1;
                                                big=testa;
                                                } /*end if*/
                                        } /*end if*/
                                } /*end for l1*/
                       if( big<=test) {   /* test for div by 0 */
                               iet=1;
                               } /*end if*/
 
                       line[i]=1;  /* selected unused line becomes used line */
                       isub[j]=i;  /* isub points to j-th row of tri. matrix */
 
                       sum=1.0/(*(a+i*nstore+j)); 
                                /*reduce matrix towards triangle */
                       for( k=0; k<n; k++ ) {
                                if( line[k]==0 ) {
                                        b=(*(a+k*nstore+j))*sum;
                                        for( l=j+1; l<n; l++ ) {
                                               *(a+k*nstore+l)= (*(a+k*nstore+l)) -b*(*(a+i*nstore+l));
                                               } /*end for l*/
                                       *(a+k*nstore+j)=b;
                                        } /*end if*/
                                } /*end for k*/
                        } /*end for j*/
 
               for( j=0; j<n; j++ ) {
                        /*find last unused row and set its pointer*/
                        /*  this row contains the apex of the triangle*/
                        if( line[j]==0) {
                                l1=j;   /*apex of triangle*/
                                isub[n-1]=j;
                                break;
                                } /*end if*/
                        } /*end for j*/
 
                } /*end if itriag true*/
                
        /*start backsolving*/
        
        for( i=0; i<n; i++ ) {  /* invert pointers. line(i) now gives*/
                                /* row no in triang matrix of i-th row*/
                                /* of actual matrix */
                line[isub[i]] = i;
                } /*end for i*/
 
        for( j=0; j<n-1; j++) { /*transform the vector to match triang. matrix*/
               b=vec[isub[j]];
               for( k=0; k<n; k++ ) {
                      if (line[k]>j) {  /* skip elements outside of triangle*/
                                vec[k]=vec[k]-(*(a+k*nstore+j))*b;
                                } /*end if*/
                        } /*end for k*/
                } /*end for j*/
 
      b = *(a+l1*nstore+(n-1));   /*apex of triangle*/
      if( ((double)fabs( (double) b))<=test) {
                /*check for div by zero in backsolving*/
                ieb=2;
                } /*end if*/
      vec[isub[n-1]]=vec[isub[n-1]]/b;
 
      for( j=n-2; j>=0; j-- ) { /* backsolve rest of triangle*/
                sum=vec[isub[j]];
                for( j2=j+1; j2<n; j2++ ) {
                        sum = sum - vec[isub[j2]] * (*(a+isub[j]*nstore+j2));
                        } /*end for j2*/
                        b = *(a+isub[j]*nstore+j);
               if( ((double)fabs((double)b))<=test) {
                        /* test for div by 0 in backsolving */
                        ieb=2;
                        } /*end if*/
                vec[isub[j]]=sum/b;   /*solution returned in vec*/
                } /*end for j*/

/*put the solution vector into the proper order*/

      for( i=0; i<n; i++ ) {    /* reorder solution */
                for( k=i; k<n; k++ ) {  /* search for i-th solution element */
                        if( line[k]==i ) {
                                j=k;
                                break;
                                } /*end if*/
                        } /*end for k*/
               b = vec[j];       /* swap solution and pointer elements*/
               vec[j] = vec[i];
               vec[i] = b;
               line[j] = line[i];
                } /*end for i*/
 
		GMT_free ((void *)isub);
		GMT_free ((void *)line);
      *ierror = iet + ieb;   /* set final error flag*/
}

/*  cartesian_stuff.c  --  bits and pieces for doing spherical trig
 *  in terms of dot and cross products.
 *
 * w. h. f. smith, 16 June. 1989
 *
 */

double GMT_dot3v (double *a, double *b)
{
	return (a[0]*b[0] + a[1]*b[1] + a[2]*b[2]);
}

double GMT_mag3v (double *a)
{
	return (d_sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]));
}

void GMT_normalize3v (double *a)
{
	double r_length;
	r_length = GMT_mag3v(a);
	if (r_length != 0.0) {
		r_length = 1.0 / r_length;
		a[0] *= r_length;
		a[1] *= r_length;
		a[2] *= r_length;
	}
}

void GMT_cross3v (double *a, double *b, double *c)
{
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
}

void GMT_geo_to_cart (double lat, double lon, double *a, GMT_LONG degrees)
{
	/* Convert geographic latitude and longitude (lat, lon)
	   to a 3-vector of unit length (a). If degrees = TRUE,
	   input coordinates are in degrees, otherwise in radian */

	double clat, clon, slon;

	if (degrees) {
		lat *= D2R;
		lon *= D2R;
	}
	sincos (lat, &a[2], &clat);
	sincos (lon, &slon, &clon);
	a[0] = clat * clon;
	a[1] = clat * slon;
}

void GMT_cart_to_geo (double *lat, double *lon, double *a, GMT_LONG degrees)
{
	/* Convert a 3-vector (a) of unit length into geographic
	   coordinates (lat, lon). If degrees = TRUE, the output coordinates
	   are in degrees, otherwise in radian. */

	if (degrees) {
		*lat = d_asind (a[2]);
		*lon = d_atan2d (a[1], a[0]);
	}
	else {
		*lat = d_asin (a[2]);
		*lon = d_atan2 (a[1], a[0]);
	}
}

GMT_LONG GMT_fix_up_path (double **a_lon, double **a_lat, GMT_LONG n, double step, GMT_LONG mode)
{
	/* Takes pointers to a list of <n> lon/lat pairs (in degrees) and adds
	 * auxiliary points if the great circle distance between two given points exceeds
	 * <step> spherical degree.
	 * If mode=0: returns points along a great circle
	 * If mode=1: first follows meridian, then parallel
	 * If mode=2: first follows parallel, then meridian
	 * Returns the new number of points (original plus auxiliary).
	 *
	 * The original argument "greenwich" became superfluous when the algorithm for
	 * wrapping the inserted points was altered on 23-May-2007. This was also when
	 * it was discovered that the calling programs sometimes defined "step" in
	 * inches of map projection.
	 */
      
	GMT_LONG i, j, n_tmp, n_step = 0, n_alloc = 0;
	GMT_LONG meridian;
	double *lon_tmp = NULL, *lat_tmp = NULL, *old;
	double a[3], b[3], x[3], *lon, *lat;
	double c, d, fraction, theta, minlon, maxlon;
	
	lon = *a_lon;
	lat = *a_lat;
	
	GMT_geo_to_cart (lat[0], lon[0], a, TRUE);
	n_alloc = GMT_alloc_memory2 ((void **)&lon_tmp, (void **)&lat_tmp, 1, 0, sizeof (double), "GMT_fix_up_path");
	lon_tmp[0] = lon[0];	lat_tmp[0] = lat[0];
	n_tmp = 1;
	if (step <= 0.0) step = 1.0;
	
	for (i = 1; i < n; i++) {
		
		GMT_geo_to_cart (lat[i], lon[i], b, TRUE);

		if (mode == 1) {	/* First follow meridian, then parallel */
			theta = fabs(lon[i]-lon[i-1]) * cosd(lat[i-1]);
			n_step = irint (theta / step);
			for (j = 1; j < n_step; j++) {
				c = j / (double)n_step;
				if (n_tmp == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&lon_tmp, (void **)&lat_tmp, n_tmp, n_alloc, sizeof (double), "GMT_fix_up_path");
				lon_tmp[n_tmp] = lon[i-1] * (1 - c) + lon[i] * c;
				lat_tmp[n_tmp] = lat[i-1];
				n_tmp++;
			}
			theta = fabs(lat[i]-lat[i-1]);
			n_step = irint (theta / step);
			for (j = 0; j < n_step; j++) {	/* Start at 0 to make sure corner point is saved */
				c = j / (double)n_step;
				if (n_tmp == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&lon_tmp, (void **)&lat_tmp, n_tmp, n_alloc, sizeof (double), "GMT_fix_up_path");
				lon_tmp[n_tmp] = lon[i];
				lat_tmp[n_tmp] = lat[i-1] * (1 - c) + lat[i] * c;
				n_tmp++;
			}
		}

		else if (mode == 2) {	/* First follow parallel, then meridian */
			theta = fabs(lat[i]-lat[i-1]);
			n_step = irint (theta / step);
			for (j = 1; j < n_step; j++) {
				c = j / (double)n_step;
				if (n_tmp == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&lon_tmp, (void **)&lat_tmp, n_tmp, n_alloc, sizeof (double), "GMT_fix_up_path");
				lon_tmp[n_tmp] = lon[i-1];
				lat_tmp[n_tmp] = lat[i-1] * (1 - c) + lat[i] * c;
				n_tmp++;
			}
			theta = fabs(lon[i]-lon[i-1]) * cosd(lat[i]);
			n_step = irint (theta / step);
			for (j = 0; j < n_step; j++) {	/* Start at 0 to make sure corner point is saved */
				c = j / (double)n_step;
				if (n_tmp == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&lon_tmp, (void **)&lat_tmp, n_tmp, n_alloc, sizeof (double), "GMT_fix_up_path");
				lon_tmp[n_tmp] = lon[i-1] * (1 - c) + lon[i] * c;
				lat_tmp[n_tmp] = lat[i];
				n_tmp++;
			}
		}

		/* Follow great circle */
		else if ((theta = d_acosd (GMT_dot3v (a, b))) == 180.0) {	/* trouble, no unique great circle */
			if (gmtdefs.verbose) fprintf (stderr, "%s: GMT Warning: Two points in input list are antipodal - no resampling taken place!\n", GMT_program);
		}

		else if ((n_step = irint (theta / step)) > 1) {	/* Must insert (n_step - 1) points, i.e. create n_step intervals */
			fraction = 1.0 / (float) n_step;
			minlon = MIN(lon[i-1],lon[i]);
			maxlon = MAX(lon[i-1],lon[i]);
			meridian = GMT_IS_ZERO (maxlon - minlon);	/* A meridian; make a gap so tests below will give right range */
			for (j = 1; j < n_step; j++) {
				c = j * fraction;
				d = 1 - c;
				x[0] = a[0] * d + b[0] * c;
				x[1] = a[1] * d + b[1] * c;
				x[2] = a[2] * d + b[2] * c;
				GMT_normalize3v (x);
				if (n_tmp == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&lon_tmp, (void **)&lat_tmp, n_tmp, n_alloc, sizeof (double), "GMT_fix_up_path");
				GMT_cart_to_geo (&lat_tmp[n_tmp], &lon_tmp[n_tmp], x, TRUE);
				if (meridian)
					lon_tmp[n_tmp] = minlon;
				else if (lon_tmp[n_tmp] < minlon)
					lon_tmp[n_tmp] += 360.0;
				else if (lon_tmp[n_tmp] > maxlon)
					lon_tmp[n_tmp] -= 360.0;
				n_tmp++;
			}
		}
		if (n_tmp == n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&lon_tmp, (void **)&lat_tmp, n_tmp, n_alloc, sizeof (double), "GMT_fix_up_path");
		lon_tmp[n_tmp] = lon[i];	lat_tmp[n_tmp] = lat[i];
		n_tmp++;
		a[0] = b[0];	a[1] = b[1];	a[2] = b[2];
	}
	n_alloc = GMT_alloc_memory2 ((void **)&lon_tmp, (void **)&lat_tmp, 0, n_tmp, sizeof (double), "GMT_fix_up_path");
	
	old = lon;
	lon = lon_tmp;
	GMT_free ((void *) old);
	old = lat;
	lat = lat_tmp;
	GMT_free ((void *) old);
	*a_lon = lon;
	*a_lat = lat;
	return (n_tmp);
}		

GMT_LONG GMT_chol_dcmp (double *a, double *d, double *cond, GMT_LONG nr, GMT_LONG n) {

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
	GMT_LONG	i, j, k, ik, ij, kj, kk, nrp1;
	double	eigmax, eigmin;

	nrp1 = nr + 1;
	
	eigmax = eigmin = sqrt ( fabs (a[0]) );

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

void GMT_chol_recover (double *a, double *d, GMT_LONG nr, GMT_LONG n, GMT_LONG nerr, GMT_LONG donly) {

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
	
	GMT_LONG	kbad, i, j, ii, ij, ji, nrp1;
	
	kbad = GMT_abs(nerr) - 1;
	nrp1 = nr + 1;
	
	for (i = 0, ii = 0; i <= kbad; i++, ii += nrp1) {
		a[ii] = d[i];
	}
	
	if (donly) return;
	
	for (j = 0; j < kbad; j++) {
		for (i = j+1, ij = j*nrp1 + 1, ji = j*nrp1 + nr; i < n; i++, ij++, ji+=nr) {
			a[ij] = a[ji];
		}
	}
	return;
}

void GMT_chol_solv (double *a, double *x, double *y, GMT_LONG nr, GMT_LONG n) {
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
	GMT_LONG	i, j, ij, ji, ii, nrp1;
	
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
