#include <stdlib.h>
#include <stdio.h>
#include <complex.h>
#include <math.h>
#include "mosaic_util.h"
#include "tool_util.h"
#include "constant.h"
#include "create_hgrid.h"
#define  D2R (M_PI/180.)
#define  R2D (180./M_PI)
/*********************************************************************************
   some private routines used in this file
*********************************************************************************/
void calc_geocoords_centerpole(int nx, int ny, double *x, double *y);
void conformal_map_coords2xyz ( int ni, int nj, double *lx, double *ly, double *X, double *Y, double *Z );
void map_xyz2lonlat(int ni, int nj, double *X, double *Y, double *Z, double *lon, double *lat );
void rotate_about_xaxis(int ni, int nj, double *X, double *Y, double *Z, double angle);
void permutiles(int ni, int nj, double *b, int num);
void calc_fvgrid(int nx, int ny, int nratio, double *dx, double *dy, double *area);
double* angle_between_vectors(int ni, int nj, double *vec1, double *vec2);
double* excess_of_quad(int ni, int nj, double *vec1, double *vec2, double *vec3, double *vec4 );
double* plane_normal(int ni, int nj, double *P1, double *P2);
void calc_rotation_angle(int nxp, int nyp, double *x, double *y, double *angle_dx, double *angle_dy);


/*******************************************************************************
  void create_conformal_cubic_grid( int *npoints, int *nratio, char *method, char *orientation, double *x,
                          double *y, double *dx, double *dy, double *area, double *angle_dx,
                          double *angle_dy )
  create cubic grid. All six tiles grid will be generated.
*******************************************************************************/
void create_conformal_cubic_grid( int *npts, int *nratio, char *method, char *orientation, double *x,
			double *y, double *dx, double *dy, double *area, double *angle_dx,
			double *angle_dy )
{
  int nx, ny, nxp, nyp;

  nx  = *npts;
  ny  = nx;
  nxp = nx+1;
  nyp = nxp;
  
  /*calculate geographic coordinates. */
  if(strcmp(orientation, "center_pole") == 0)
    calc_geocoords_centerpole(nx, ny, x, y);
  else
    mpp_error("create_cubic_grid: only center pole orientation is implemented");  

  /* calculate cell length and area */
  calc_fvgrid(nx, ny, *nratio, dx, dy, area);  

  /*calculate rotation angle, just some workaround, will modify this in the future. */
  calc_rotation_angle(nxp, nyp, x, y, angle_dx, angle_dy );
  
}; /* create_conformal_cubic_grid */

/***********************************************************************
   calc_geoocoords_centerpole(int nx, int ny, double *x, double *y);
calculate geographic coordinates for all six tiles.

***********************************************************************/
void calc_geocoords_centerpole(int nx, int ny, double *x, double *y)
{
  int i, j, n, m, nxp, nyp, nxh, nyh;
  double *lx, *ly, *X, *Y, *Z, *lonP, *latP, *lonE, *latE, *tmp;

  nxp = nx+1;
  nyp = ny+1;
  nxh = (nxp+1)/2;
  nyh = (nyp+1)/2;  
  
  lx = (double *)malloc(nxh*nyh*sizeof(double));
  ly = (double *)malloc(nxh*nyh*sizeof(double));

  n = 0;
  for(j=0; j<nyh; j++) {
    for(i=0; i<nxh; i++) {
      lx[n] = -1. + 2.0*i/(nxp-1);
      ly[n++] = -1. + 2.0*j/(nyp-1);
    }
  } 

  X = (double *)malloc(nxh*nyh*sizeof(double));
  Y = (double *)malloc(nxh*nyh*sizeof(double));
  Z = (double *)malloc(nxh*nyh*sizeof(double));

  /* calculating 3D coordinates on unit sphere */
  conformal_map_coords2xyz( nxh, nyh, lx, ly, X, Y, Z);

  lonP = (double *) malloc(nxp*nyp*sizeof(double));
  latP = (double *) malloc(nxp*nyp*sizeof(double));
  lonE = (double *) malloc(nxp*nyp*sizeof(double));
  latE = (double *) malloc(nxp*nyp*sizeof(double));
  
  /* map 3D coordinates to geographical coordinates. */
  map_xyz2lonlat( nxh, nyh, X, Y, Z, lx, ly );

  /* coyy data from lx, ly to lonP and latP */
  for(j=0;j<nyh;j++) {
    for(i=0; i<nxh; i++) {
      lonP[j*nxp+i] = lx[j*nxh+i];
      latP[j*nxp+i] = ly[j*nxh+i];
    }
  }

  /* enforce symmetry */
  for(j=0;j<nyh;j++) {
    for(i=0; i<nxh; i++) {
      if( i<j )
	latP[j*nxp+i] = 0.5*(latP[j*nxp+i] + latP[i*nxp+j]);
      else
	latP[j*nxp+i] = latP[i*nxp+j];       
      if(lonP[j*nxp+i] >= M_PI ) lonP[j*nxp+i] -= 2*M_PI;
    }
  }

  tmp = (double *) malloc(nxh*nyh*sizeof(double));
  n = 0;
  for(j=0; j<nyh; j++)
    for(i=0; i<nxh; i++) tmp[n++] = lonP[j*nxp+i];

  for(j=0;j<nyh;j++) {  
    for(i=0; i<nxh; i++) {
      lonP[j*nxp+i] = (tmp[j*nxh+i]-3./2.*M_PI-tmp[i*nxh+j])*0.5;
      if(i==j) lonP[j*nxp+i] = -3.0/4.0*M_PI;
    }
  }

  free(tmp);
  
  /*use symmetry to expand to full cubic */
  for(j=0;j<nyh;j++) {
    for(i=nxh;i<nxp; i++) {
      lonP[j*nxp+i] = -M_PI-lonP[j*nxp+nxp-i-1];
      latP[j*nxp+i] = latP[j*nxp+nxp-i-1];
    }
  }

  for(j=nyh; j<nyp; j++) {
    for(i=0; i<nxp; i++) {
      lonP[j*nxp+i] = -lonP[(nyp-j-1)*nxp+i];
      latP[j*nxp+i] = latP[(nyp-j-1)*nxp+i];
    }
  }       

  rotate_about_xaxis(nxh, nyh, X, Y, Z, M_PI/2);   

  map_xyz2lonlat( nxh, nyh, X, Y, Z, lx, ly );

  /* coyy data from lx, ly to lonE and latE */
  for(j=0;j<nyh;j++) {
    for(i=0; i<nxh; i++) {
      lonE[j*nxp+i] = lx[j*nxh+i];
      latE[j*nxp+i] = ly[j*nxh+i];
    }
  }
  
  free(lx);
  free(ly);
  
  /* enforce symmetry */
  for(j=0;j<nyh;j++) lonE[j*nxp] = -3./4.*M_PI;
  for(i=0;i<nxh;i++) {
    latE[(nyh-1)*nxp+i] = 0;
    latE[i]    = -latP[i];
  }
  /*use symmetry to expand to full cube. */
  for(j=0;j<nxh; j++) {
    for(i=nxh;i<nxp;i++) {
      lonE[j*nxp+i] = -M_PI-lonE[j*nxp+nxp-i-1];
      latE[j*nxp+i] = latE[j*nxp+nxp-i-1];
    }
  }

  for(j=nyh; j<nyp; j++) {
    for(i=0; i<nxp; i++) {
      lonE[j*nxp+i] = lonE[(nyp-j-1)*nxp+i];
      latE[j*nxp+i] = -latE[(nyp-j-1)*nxp+i];
    }
  }      

  /*convert to geographic grid */
  n = 0;
  /* tile 1 */
  for(m = 0; m < nxp*nyp; m++) { 
    x[n] = lonE[m]*R2D-90.;
    if(x[n]<=-180.) x[n] += 360.;
    y[n++] = latE[m]*R2D;
  }

  /* tile 2 */
  for(m = 0; m < nxp*nyp; m++) {
    x[n] = lonE[m]*R2D;
    y[n++] = latE[m]*R2D;
  }

  /* tile 3 */
  for(m = 0; m < nxp*nyp; m++) {
    x[n] = lonP[m]*R2D;
    y[n++] = latP[m]*R2D;
  }
    
  /* tile 4 */
  for(j=0; j<nyp; j++) {
    for(i=0; i<nxp; i++) {
      x[n] = lonE[i*nxp+j]*R2D+90.;
      y[n++] = latE[(nxp-i-1)*nxp+j]*R2D;
    }
  }

  /* tile 5 */
  for(j=0; j<nyp; j++) {
    for(i=0; i<nxp; i++) {
      x[n] = lonE[i*nxp+j]*R2D+180.;
      y[n++] = latE[(nxp-i-1)*nxp+j]*R2D;
    }
  }

  /* tile 6 */
  for(j=0; j<nyp; j++) {
    for(i=0; i<nxp; i++) {
      x[n] = lonP[(nxp-i-1)*nxp+nyp-j-1]*R2D;
      y[n++] = -latP[j*nxp+i]*R2D;
    }
  }

  permutiles(nxp, nyp, x,2);
  permutiles(nxp, nyp, y,2);

}; /*calc_geocords_centerpole */

/*************************************************************************
  void conformal_map_coords2xyz ( double *lx, double * ly, double *X, double *Y, double *Z )
  Conformal mapping of a cube onto a sphere maps (lx, ly) on the north-pole face of a cube
  to (X,Y,Z) coordinates in physical space. 
  Face is oriented normal to Z-axis with  X and Y increasing with lx and ly
  valid ranges:  -1 < lx < 1   -1 < ly < 1
  Based on matlab scripts from Alistair  ???? 
**************************************************************************/
  
void conformal_map_coords2xyz ( int ni, int nj, double *lx, double *ly,
     				   double *X, double *Y, double *Z )
{
  const double RA = sqrt(3.)-1;
  const double THRD = 1./3.;
  const complex CB = -1. + I;
  const complex CC = RA*CB/2.;
  const int order = 29;
  const double A[] = { 1.47713057321600,
		       -0.38183513110512,
		       -0.05573055466344,
		       -0.01895884801823,
		       -0.00791314396396,
		       -0.00486626515498,
		       -0.00329250387158,
		       -0.00235482619663,
		       -0.00175869000970,
		       -0.00135682443774,
		       -0.00107458043205,
		       -0.00086946107050,
		       -0.00071604933286,
		       -0.00059869243613,
		       -0.00050696402446,
		       -0.00043418115349,
		       -0.00037537743098,
		       -0.00032745130951,
		       -0.00028769063795,
		       -0.00025464473946,
		       -0.00022659577923,
		       -0.00020297175587,
		       -0.00018247947703,
		       -0.00016510295548,
		       -0.00014967258633,
		       -0.00013660647356,
		       -0.00012466390509,
		       -0.00011468147908,
		       -0.00010518717478,
		       -0.00009749136078,
  };
  complex w, zc,a,b;
  int i, j, n, m;
  double xc, yc, h, t;
  size_t *dims;

  for(n=0; n< ni*nj; n++) {
    xc = fabs(lx[n]);
    yc = fabs(ly[n]);
    X[n] = xc;
    Y[n] = yc;
    xc   = 1 - xc;
    yc   = 1 - yc;
    if(fabs(ly[n]) > fabs(lx[n]) ) {
      xc = 1-Y[n];
      yc = 1-X[n];
    }
    zc  = cpow((xc+I*yc)/2.,4);
    /*Evaluate the Taylor series.  */
    w = 0;

    for(m=order; m>=0; m--) w = ( w + A[m] ) * zc;
    if( w != 0. ) w =  cpow(I,THRD) * cpow( w*I, THRD);
    w = (w-RA)/(CB+CC*w);
    X[n] = creal(w);  
    Y[n] = cimag(w);
    h    = 2./(1+cpow(X[n],2)+cpow(Y[n],2));
    X[n] = X[n]*h;
    Y[n] = Y[n]*h;
    Z[n] = h-1;
  }

  for(n=0; n< ni*nj; n++) {
    if(fabs(ly[n]) > fabs(lx[n]) ) {
      t = X[n];
      X[n] = Y[n];
      Y[n] = t;
    }

    if(lx[n]<0)  X[n] = -X[n];
    if(lx[n]==0) X[n] = 0;
    if(ly[n]<0)  Y[n] = -Y[n];
    if(ly[n]==0) Y[n] = 0;
  }

}; /* conformal_map_coords2xyz */



/**********************************************************
  Convert 3-D coordinates (x,y,z) to (lon,lat)
  Assumes "lat" is positive with "z", equatorial plane
  falls at z=0  and "lon" is measured anti-clockwise (eastward)
  from x-axis (y=0) about z-axis.

************************************************************/

void map_xyz2lonlat(int ni, int nj, double *X, double *Y, double *Z, 
		    double *lon, double *lat )
{
  int i, j, n;
  double req;

  for(n=0; n<ni*nj; n++) {
    /*latitude */
    req = sqrt(X[n]*X[n]+Y[n]*Y[n]);
    if( req == 0)
      if(Z[n] == 0 )
	lat[n] = 0.;
      else
        lat[n] = M_PI*0.5;
    else
      lat[n] = atan( Z[n]/req );

    /*longitude */
    if(X[n] == 0)
      lon[n] = M_PI*0.5;
    else
      lon[n] = atan(Y[n]/X[n]);

    if(X[n]<0 && Y[n] >=0) lon[n] += M_PI;
    if(X[n]<=0 && Y[n] < 0) lon[n] -= M_PI;
  }
  
};  /* map_xyz2lonlat */


/**************************************************************
   void rotate_about_xaxis(int ni, int nj, double *X, double *Y, 
	              	   double *Z, double angle)

   Rotate about X axis by "angle"

***************************************************************/

void rotate_about_xaxis(int ni, int nj, double *X, double *Y, 
	          	double *Z, double angle) {
  int i, j, n;
  double s,c,old;
  const double tolerance = 1.e-9;

  s=sin(angle);
  c=cos(angle);

  if (c<tolerance) {
    c=0;
    if(s>0)
      s = 1;
    else
      s = -1;
  }

  for(n=0; n<ni*nj; n++) {
    old = Y[n];
    Y[n] = c*Y[n]-s*Z[n];
    Z[n] = s*old+c*Z[n];
  }

}; /* rotate_about_xaxis */


/*************************************************************************
   void permutetiles(int ni, int nj, double *b, int n)

   shifts the tile data left by n places around the equator
   n=1, tile 2->1, 4->2, 5->4, 1->5, the tiles 3 and 6 get rotated 90 degs.

*************************************************************************/

void permutiles(int ni, int nj, double *b, int num) {

  int i, j, k, n;
  int ntiles = 6;
  double *c=NULL;
  
  c = (double *)malloc(ni*nj*ntiles*sizeof(double));

  for(k=0; k<num; k++) {
    for(j=0;j<nj;j++) {
      for(i=0;i<ni;i++) {
	n = j*ni+i;
	c[n]         = b[ni*nj+n];
	c[ni*nj+n]   = b[3*ni*nj+i*ni+nj-j-1];
	c[2*ni*nj+n] = b[2*ni*nj+i*ni+nj-j-1];
	c[3*ni*nj+n] = b[4*ni*nj+n];
	c[4*ni*nj+n] = b[(ni-i-1)*ni+j];
	c[5*ni*nj+n] = b[5*ni*nj+(ni-i-1)*ni+j];
      }
    }
    for(n=0; n<ni*nj*ntiles; n++) b[n] = c[n];
  }

  free(c);
};  /* permutiles */

/*************************************************************************

     calc_fvgrid( lx, vector ly, vector dxl, vector dyl, vector areal)

     Calculates finite volume grid info (dxl,dyl,areal) for conformal cubic grid 
     with 3-D coordinates (X, Y, Z)
     Meant to be used for single quadrant of tile but does work for full tile

**************************************************************************/

void calc_fvgrid(int nx, int ny, int nratio, double *dx, double *dy, double *area)
{
  int nxf, nyf, nif, njf, nxp, nyp, nxh, nyh, i, j, n, m;
  double ar;
  double *lx, *ly, *X, *Y, *Z, *vec1, *vec2, *vec3, *vec4, *dxl, *dyl, *areal;
  size_t *dims;
  
  nxp = nx+1;
  nyp = ny+1;
  nxh = (nxp+1)/2;
  nyh = (nyp+1)/2;

  nxf = nx*nratio+1;
  nyf = nxf; 
  nif = (nxf+1)/2;
  njf = nif;

  lx = (double *)malloc(nif*njf*sizeof(double));
  ly = (double *)malloc(nif*njf*sizeof(double));

  n = 0;
  for(j=0; j<njf; j++) {
    for(i=0; i<nif; i++) {
      lx[n] = -1. + 2.0*i/(nxf-1);
      ly[n++] = -1. + 2.0*j/(nyf-1);
    }
  } 

  X = (double *)malloc(nif*njf*sizeof(double));
  Y = (double *)malloc(nif*njf*sizeof(double));
  Z = (double *)malloc(nif*njf*sizeof(double));

 /* calculating 3D coordinates on unit sphere */
  conformal_map_coords2xyz( nif, njf, lx, ly, X, Y, Z);

  vec1  = (double *)malloc((nif-1)*njf*3*sizeof(double));
  vec2  = (double *)malloc((nif-1)*njf*3*sizeof(double));

  for(j=0;j<njf;j++) {
    for(i=0;i<nif-1;i++) {
      n = j*(nif-1)+i;
      vec1[n] = X[j*nif+i];
      vec2[n] = X[j*nif+i+1];
      vec1[(nif-1)*njf+n] = Y[j*nif+i];
      vec2[(nif-1)*njf+n] = Y[j*nif+i+1];
      vec1[2*(nif-1)*njf+n] = Z[j*nif+i];
      vec2[2*(nif-1)*njf+n] = Z[j*nif+i+1];
    }
  }
  
  dxl = angle_between_vectors( nif-1, njf, vec1, vec2);

  free(vec1);
  free(vec2);

  vec1  = (double *)malloc(nif*(njf-1)*3*sizeof(double));
  vec2  = (double *)malloc(nif*(njf-1)*3*sizeof(double));

  for(j=0;j<njf-1;j++) {
    for(i=0;i<nif;i++) {
      n = j*nif+i;
      vec1[n] = X[j*nif+i];
      vec2[n] = X[(j+1)*nif+i];
      vec1[nif*(njf-1)+n] = Y[j*nif+i];
      vec2[nif*(njf-1)+n] = Y[(j+1)*nif+i];
      vec1[2*nif*(njf-1)+n] = Z[j*nif+i];
      vec2[2*nif*(njf-1)+n] = Z[(j+1)*nif+i];
    }
  }  

  dyl = angle_between_vectors( nif, njf-1, vec1, vec2);
  
  free(vec1);
  free(vec2);

  vec1  = (double *)malloc((nif-1)*(njf-1)*3*sizeof(double));
  vec2  = (double *)malloc((nif-1)*(njf-1)*3*sizeof(double)); 
  vec3  = (double *)malloc((nif-1)*(njf-1)*3*sizeof(double));
  vec4  = (double *)malloc((nif-1)*(njf-1)*3*sizeof(double)); 

  for(j=0;j<njf-1;j++) {
    for(i=0;i<nif-1;i++) {
      n = j*(nif-1)+i;
      vec1[n] = X[j*nif+i];
      vec2[n] = X[j*nif+i+1];
      vec3[n] = X[(j+1)*nif+i+1];
      vec4[n] = X[(j+1)*nif+i];
      vec1[(nif-1)*(njf-1)+n] = Y[j*nif+i];
      vec2[(nif-1)*(njf-1)+n] = Y[j*nif+i+1];
      vec3[(nif-1)*(njf-1)+n] = Y[(j+1)*nif+i+1];
      vec4[(nif-1)*(njf-1)+n] = Y[(j+1)*nif+i];
      vec1[2*(nif-1)*(njf-1)+n] = Z[j*nif+i];
      vec2[2*(nif-1)*(njf-1)+n] = Z[j*nif+i+1];
      vec3[2*(nif-1)*(njf-1)+n] = Z[(j+1)*nif+i+1];
      vec4[2*(nif-1)*(njf-1)+n] = Z[(j+1)*nif+i];
    }
  }  

  areal = excess_of_quad( nif-1, njf-1, vec1, vec2, vec3, vec4);
  free(vec1);
  free(vec2);
  free(vec3);
  free(vec4);  

  /*Force some symmetry (probably does nothing) */
  for(j=0; j<njf; j++) {
    for(i=0; i<nif-1; i++) dxl[j*(nif-1)+i] = (dxl[j*(nif-1)+i]+dyl[i*nif+j])*0.5;
  }

  for(j=0; j<njf-1; j++) {
    for(i=0; i<nif; i++) dyl[j*nif+i] = dxl[i*(nif-1)+j];
  }

  for(j=0; j<njf-1; j++) {
    for(i=0; i<nif-1; i++) {
      if(j<i) 
	areal[j*(nif-1)+i] = (areal[j*(nif-1)+i]+areal[i*(nif-1)+j])*0.5;
      else if(j>i)
        areal[j*(nif-1)+i] = areal[i*(nif-1)+j];
    }
  }

  /* Use symmetry to fill second octant */
  for(j=1; j<njf; j++) {
    for(i=0; i<j; i++) {
      dxl[j*(nif-1)+i] = dyl[i*nif+j];
    }
  }

  for(j=1; j<njf-1; j++) {
    for(i=0; i<j; i++) {
      areal[j*(nif-1)+i] = areal[i*(nif-1)+j];
    }
  }

  /* copy data from fine grid to super grid. */
  
  for(j=0;j<nyh;j++) {
    for(i=0;i<nxh-1;i++) {
      ar = 0;
      for(n=0;n<nratio;n++) ar = ar + dxl[j*nratio*(nif-1)+i*nratio+n];
      dx[j*nx+i] = ar*RADIUS;
    }
  }
    
  for(j=0;j<nyh-1;j++) {
    for(i=0;i<nxh-1;i++) {
      ar = 0;
      for(n=0; n<nratio; n++) 
	for(m=0; m<nratio; m++) ar += areal[(j*nratio+n)*(nif-1)+i*nratio+m];
      area[j*nx+i] = ar*RADIUS*RADIUS;
    }
  }
  
  /*use reflection symmetry of quadrants to fill face. */
  for(j=0;j<nyh; j++) {
    for(i=nxh-1; i<nx; i++) {
      dx[j*nx+i] = dx[j*nx+nxp-i-2];
    }
  }

  for(j=nyh;j<nyp; j++) {
    for(i=0; i<nx; i++) {
      dx[j*nx+i] = dx[(ny-j)*nx+i];
    }
  }

  for(j=0;j<nyh-1; j++) {
    for(i=nxh-1; i<nx; i++) {
      area[j*nx+i]   = area[j*nx+nxp-i-2];
    }
  }

  for(j=nyh-1;j<ny; j++) {
    for(i=0; i<nx; i++) {
      area[j*ny+i]   = area[(nyp-j-2)*nx+i];
    }
  }

  /* copy dx to dy */
  for(j=0;j<ny;j++) 
    for(i=0;i<nxp;i++) dy[j*nxp+i] =  dx[i*nx+j] ;

}; /* calc_fvgrid */


/******************************************************************************* 
   array<double>* angle_between_vectors(array<double> vec1, array<double> vec2)
*******************************************************************************/

double* angle_between_vectors(int ni, int nj, double *vec1, double *vec2) {
  int n;
  double vector_prod, nrm1, nrm2;
  double *angle;
  
  angle = (double *)malloc(ni*nj*sizeof(double));

  for(n=0; n<ni*nj; n++) {
    vector_prod=vec1[n]*vec2[n] + vec1[ni*nj+n]*vec2[ni*nj+n] + vec1[2*ni*nj+n]*vec2[2*ni*nj+n];
      nrm1=pow(vec1[n],2)+pow(vec1[ni*nj+n],2)+pow(vec1[2*ni*nj+n],2);
      nrm2=pow(vec2[n],2)+pow(vec2[ni*nj+n],2)+pow(vec2[2*ni*nj+n],2);
      angle[n] = acos( vector_prod/sqrt(nrm1*nrm2) );
  }
  return angle;
}; /* angle_between_vectors */

/*****************************************************************
   double* excess_of_quad(int ni, int nj, double *vec1, double *vec2, 
                          double *vec3, double *vec4 )
*******************************************************************/
double* excess_of_quad(int ni, int nj, double *vec1, double *vec2, double *vec3, double *vec4 )
{
  int n;
  double ang12, ang23, ang34, ang41;
  double *excess, *plane1, *plane2, *plane3, *plane4;
  double *angle12, *angle23, *angle34, *angle41;
  
  excess = (double *)malloc(ni*nj*sizeof(double));

  plane1=plane_normal(ni, nj, vec1, vec2);
  plane2=plane_normal(ni, nj, vec2, vec3);
  plane3=plane_normal(ni, nj, vec3, vec4);
  plane4=plane_normal(ni, nj, vec4, vec1);
  angle12=angle_between_vectors(ni, nj, plane2,plane1);
  angle23=angle_between_vectors(ni, nj, plane3,plane2);
  angle34=angle_between_vectors(ni, nj, plane4,plane3);
  angle41=angle_between_vectors(ni, nj, plane1,plane4);

  for(n=0; n<ni*nj; n++) {
    ang12 = M_PI-angle12[n];
    ang23 = M_PI-angle23[n];
    ang34 = M_PI-angle34[n];
    ang41 = M_PI-angle41[n];
    excess[n] = ang12+ang23+ang34+ang41-2*M_PI;
  }

  free(plane1);
  free(plane2);
  free(plane3);
  free(plane4);
  free(angle12);
  free(angle23);
  free(angle34);
  free(angle41);
  
  return excess;

}; /* excess_of_quad */

/***********************************************************************
   double* plane_normal(int ni, int nj, double *P1, double *P2)
***********************************************************************/

double* plane_normal(int ni, int nj, double *P1, double *P2)
{
  int i, j, n;
  double p1, p2, p3, mag;
  double *plane;
  
  plane = (double *)malloc(ni*nj*3*sizeof(double));

  for(j=0;j<nj;j++) {
    for(i=0;i<ni;i++) {
      n = j*ni + i;
      p1 = P1[ni*nj+n] * P2[2*ni*nj+n] - P1[2*ni*nj+n] * P2[ni*nj+n];
      p2 = P1[2*ni*nj+n] * P2[n] - P1[n] * P2[2*ni*nj+n];
      p3 = P1[n] * P2[ni*nj+n] - P1[ni*nj+n] * P2[n];
      mag=sqrt(pow(p1,2) + pow(p2,2) + pow(p3,2));
      plane[n]=p1/mag;
      plane[ni*nj+n]=p2/mag;
      plane[2*ni*nj+n]=p3/mag;
    }
  }

  return plane;

};

/******************************************************************

  void calc_rotation_angle()

******************************************************************/

void calc_rotation_angle(int nxp, int nyp, double *x, double *y, double *angle_dx, double *angle_dy)
{
  int ip1, im1, jp1, jm1, tp1, tm1, i, j, n, ntiles, nx, ny;
  double lon_scale;

  nx = nxp - 1;
  ny = nyp - 1;
  ntiles = 6;
  for(n=0; n<ntiles; n++) {
    for(j=0; j<nyp; j++) {
      for(i=0; i<nxp; i++) {
	lon_scale = cos(y[n*nxp*nyp+j*nxp+i]*D2R);
	tp1 = n;
	tm1 = n;
	ip1 = i+1;
	im1 = i-1;
	jp1 = j;
	jm1 = j;

        if(ip1 >= nxp) {  /* find the neighbor tile. */
	  if(n % 2 == 0) { /* tile 1, 3, 5 */
	    tp1 = n+1;
	    ip1 = 0;
	  }
	  else { /* tile 2, 4, 6 */
	    tp1 = n+2;
	    if(tp1 >= ntiles) tp1 -= ntiles;
	    ip1 = ny-j-1;
	    jp1 = 0;
	  }
	}        
        if(im1 < 0) {  /* find the neighbor tile. */
	  if(n % 2 == 0) { /* tile 1, 3, 5 */
	    tm1 = n-2;
	    if(tm1 < 0) tm1 += ntiles;
	    jm1 = ny;
	    im1 = nx-j;
	  }
	  else { /* tile 2, 4, 6 */
	    tm1 = n-1;
	    im1 = nx;
	  }
	}

	angle_dx[n*nxp*nyp+j*nxp+i] = atan2(y[tp1*nxp*nyp+jp1*nxp+ip1]-y[tm1*nxp*nyp+jm1*nxp+im1],
					    (x[tp1*nxp*nyp+jp1*nxp+ip1]-x[tm1*nxp*nyp+jm1*nxp+im1])*lon_scale )*R2D;
	tp1 = n;
	tm1 = n;
	ip1 = i;
	im1 = i;
	jp1 = j+1;
	jm1 = j-1;
        if(jp1 >=nyp) {  /* find the neighbor tile. */
	  if(n % 2 == 0) { /* tile 1, 3, 5 */
	    tp1 = n+2;
	    if(tp1 >= ntiles) tp1 -= ntiles;
	    jp1 = nx-i;
	    ip1 = 0;
	  }
	  else { /* tile 2, 4, 6 */
	    tp1 = n+1;
	    if(tp1 >= ntiles) tp1 -= ntiles;
	    jp1 = 0;
	  }
	}        
        if(jm1 < 0) {  /* find the neighbor tile. */
	  if(n % 2 == 0) { /* tile 1, 3, 5 */
	    tm1 = n-1;
	    if(tm1 < 0) tm1 += ntiles;
	    jm1 = ny;
	  }
	  else { /* tile 2, 4, 6 */
	    tm1 = n-2;
	    if(tm1 < 0) tm1 += ntiles;
	    im1 = nx;
	    jm1 = nx-i;
	  }
	}	

	angle_dy[n*nxp*nyp+j*nxp+i] = atan2(y[tp1*nxp*nyp+jp1*nxp+ip1]-y[tm1*nxp*nyp+jm1*nxp+im1],
					    (x[tp1*nxp*nyp+jp1*nxp+ip1]-x[tm1*nxp*nyp+jm1*nxp+im1])*lon_scale )*R2D;
      }
    }
  }

}; /* calc_rotation_angle */
 
