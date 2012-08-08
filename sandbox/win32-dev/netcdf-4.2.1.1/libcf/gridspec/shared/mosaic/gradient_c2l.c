#include <math.h>
#include <stdlib.h>
#include "constant.h"
#include "mosaic_util.h"
#include "gradient_c2l.h"
#include <stdio.h>
void a2b_ord2(int nx, int ny, const double *qin, const double *edge_w, const double *edge_e,
	      const double *edge_s, const double *edge_n, double *qout,
	      int on_west_edge, int on_east_edge, int on_south_edge, int on_north_edge);


/*------------------------------------------------------------------------------
  Routine to compute gradient terms for SCRIP:
  SJL: Oct 5, 2007
  NOTe: pin has halo size = 1.
  the size of pin    will be (nx+2,ny+2), T-cell center, with halo = 1
  the size of dx     will be (nx, ny+1),  N-cell center
  the size of dy     will be (nx+1, ny),  E-cell center
  the size of area   will be (nx, ny),    T-cell center.
  The size of edge_w will be (ny+1),      C-cell center
  The size of edge_e will be (ny+1),      C-cell center
  The size of edge_s will be (nx+1),      C-cell center
  The size of edge_n will be (nx+1),      C-cell center
  The size of en_n   will be (nx, ny+1,3),N-cell center
  The size of en_e   will be (nx+1,ny,3), E-cell center
  The size of vlon   will be (nx, ny, 3)  T-cell center
  The size of vlat   will be (nx, ny, 3), T-cell center
  ----------------------------------------------------------------------------*/
void grad_c2l_(const int *nlon, const int *nlat, const double *pin, const double *dx, const double *dy, const double *area,
	       const double *edge_w, const double *edge_e, const double *edge_s, const double *edge_n,
	       const double *en_n, const double *en_e, const double *vlon, const double *vlat,
	       double *grad_x, double *grad_y, const int *on_west_edge, const int *on_east_edge,
	       const int *on_south_edge, const int *on_north_edge)
{
  grad_c2l(nlon, nlat, pin, dx, dy, area, edge_w, edge_e, edge_s, edge_n, en_n, en_e, vlon, vlat, grad_x, grad_y,
	   on_west_edge, on_east_edge, on_south_edge, on_north_edge);
}
  
void grad_c2l(const int *nlon, const int *nlat, const double *pin, const double *dx, const double *dy, const double *area,
	      const double *edge_w, const double *edge_e, const double *edge_s, const double *edge_n,
	      const double *en_n, const double *en_e, const double *vlon, const double *vlat,
	      double *grad_x, double *grad_y, const int *on_west_edge, const int *on_east_edge,
	      const int *on_south_edge, const int *on_north_edge)
{

  double *pb, *pdx, *pdy, *grad3;
  int nx, ny, nxp, nyp, i, j, m0, m1, m2, n;

  nx    = *nlon;
  ny    = *nlat;
  nxp   = nx+1;
  nyp   = ny+1;
  pb    = (double *)malloc(nxp*nyp*sizeof(double));
  pdx   = (double *)malloc(3*nx*(ny+1)*sizeof(double));
  pdy   = (double *)malloc(3*(nx+1)*ny*sizeof(double));
  grad3 = (double *)malloc(3*nx*ny*sizeof(double));
  a2b_ord2(nx, ny, pin, edge_w, edge_e, edge_s, edge_n, pb, *on_west_edge, *on_east_edge,*on_south_edge, *on_north_edge);
  
  for(j=0; j<nyp; j++) for(i=0; i<nx; i++) {
    m0 = j*nx+i;
    m1 = j*nxp+i;
    for(n=0; n<3; n++) {
      pdx[3*m0+n] = 0.5*(pb[m1]+pb[m1+1])*dx[m0]*en_n[3*m0+n];
    }
  }

  for(j=0; j<ny; j++) for(i=0; i<nxp; i++) {
    m0 = j*nxp+i;
    for(n=0; n<3; n++) {
      pdy[3*m0+n] = 0.5*(pb[m0]+pb[m0+nxp])*dy[m0]*en_e[3*m0+n];
    }
  }

  /* Compute 3D grad of the input scalar field  by Green's theorem */
  for(j=0; j<ny; j++) for(i=0; i<nx; i++) {
    m0 = 3*(j*nx+i);
    for(n=0; n<3; n++) {
      grad3[m0+n] = pdx[3*((j+1)*nx+i)+n]-pdx[m0+n]-pdy[3*(j*nxp+i)+n]+pdy[3*(j*nxp+i+1)+n];
    }
  }

  /* Compute inner product: V3 * grad (pe) */
  for(j=0; j<ny; j++) for(i=0; i<nx; i++) {
    m0 = j*nx+i;
    m1 = 3*m0;
    /* dq / d(Lamda)*/
    grad_x[m0] = (vlon[m1]*grad3[m1] + vlon[m1+1]*grad3[m1+1] + vlon[m1+2]*grad3[m1+2])/area[m0];
    grad_x[m0] *= RADIUS;
    /* dq / d(theta) */
    grad_y[m0] = (vlat[m1]*grad3[m1] + vlat[m1+1]*grad3[m1+1] + vlat[m1+2]*grad3[m1+2] )/area[m0];
    grad_y[m0] *= RADIUS;
  }

  free(pb);
  free(pdx);
  free(pdy);
  free(grad3);
  
}; /* grad_c2l */

/*------------------------------------------------------------------------------
  qin:  A-grid field, size (nx+2, ny+2)
  qout: B-grid field, size (nx+1, ny+1)
  ----------------------------------------------------------------------------*/
void a2b_ord2(int nx, int ny, const double *qin, const double *edge_w, const double *edge_e,
	      const double *edge_s, const double *edge_n, double *qout,
	      int on_west_edge, int on_east_edge, int on_south_edge, int on_north_edge)
{
  int    nxp, nyp, i, j;
  int    istart, iend, jstart, jend;
  double *q1, *q2;
  const double r3 = 1./3.;

  nxp = nx+1;
  nyp = ny+1;
  q1 = (double *)malloc((nx+2)*sizeof(double));
  q2 = (double *)malloc((ny+2)*sizeof(double));

  
  if(on_west_edge)
    istart = 1;
  else
    istart = 0;
  if(on_east_edge)
    iend = nx;
  else
    iend = nxp;
  if(on_south_edge)
    jstart = 1;
  else
    jstart = 0;
  if(on_north_edge)
    jend = ny;
  else
    jend = nyp;
  
  /* internal region ( 1: nx-1, 1:ny-1) */
  for(j=jstart; j<jend; j++) for(i=istart; i<iend; i++) {
    qout[j*nxp+i] = 0.25*(qin[j*(nx+2)+i] + qin[j*(nx+2)+i+1] +
			  qin[(j+1)*(nx+2)+i] + qin[(j+1)*(nx+2)+i+1] );
  }

  /* Fix the 4 Corners */
  if(on_west_edge && on_south_edge)qout[        0] = r3*(qin[1* (nx+2)+1 ]+qin[1* (nx+2)    ]+qin[             1]); /* sw_corner */
  if(on_east_edge && on_south_edge)qout[       nx] = r3*(qin[1* (nx+2)+nx]+qin[          nx ]+qin[1*  (nx+2)+nxp]); /* se_corner */
  if(on_east_edge && on_north_edge)qout[ny*nxp+nx] = r3*(qin[ny*(nx+2)+nx]+qin[ny*(nx+2)+nxp]+qin[nyp*(nx+2)+nx ]); /* ne_corner */
  if(on_west_edge && on_north_edge)qout[ny*nxp   ] = r3*(qin[ny*(nx+2)+1 ]+qin[ny*(nx+2)    ]+qin[nyp*(nx+2)+1  ]); /* nw_corner */
  
  /* West Edges */
  if(on_west_edge) {
    for(j=jstart; j<=jend; j++) q2[j] = 0.5*(qin[j*(nx+2)] + qin[j*(nx+2)+1]);
    for(j=jstart; j<jend; j++) qout[j*nxp] = edge_w[j]*q2[j] + (1-edge_w[j])*q2[j+1];
  }
  
  /* East Edges */
  if(on_east_edge) {  
    for(j=jstart; j<=jend; j++)q2[j] = 0.5*(qin[j*(nx+2)+nx] + qin[j*(nx+2)+nxp]);
    for(j=jstart; j<jend; j++) qout[j*nxp+nx] = edge_e[j]*q2[j] + (1-edge_e[j])*q2[j+1];
  }
  
  /* south edge */
  if(on_south_edge) {  
    for(i=istart; i<=iend; i++) q1[i] = 0.5*(qin[i] + qin[(nx+2)+i]);
    for(i=istart; i<iend; i++) qout[i] = edge_s[i]*q1[i] + (1 - edge_s[i])*q1[i+1];
  }
  
  /* north edge */
  if(on_north_edge) {  
     for(i=istart; i<=iend; i++) q1[i] = 0.5*(qin[ny*(nx+2)+i] + qin[nyp*(nx+2)+i]);
     for(i=istart; i<iend; i++) qout[ny*nxp+i] = edge_n[i]*q1[i] + (1 - edge_n[i])*q1[i+1];  
  }
  
}; /* a2b_ord2 */


void get_edge(int nx, int ny, const double *lont, const double *latt,
	      const double *lonc, const double *latc, double *edge_w, double *edge_e, double *edge_s, double *edge_n,
	      int on_west_edge, int on_east_edge, int on_south_edge, int on_north_edge)
{
  int i, j, nxp, nyp;
  int istart, iend, jstart, jend;
  double p1[2], p2[2];
  double *py, *px;
  double d1, d2;
  
  nxp = nx + 1;
  nyp = ny + 1;

  for(i=0; i<nxp; i++) {
    edge_s[i] = 0.5; /* dummy value */
    edge_n[i] = 0.5; /* dummy value */
  }
  for(j=0; j<nyp; j++) {
    edge_w[j] = 0.5; /* dummy value */
    edge_e[j] = 0.5; /* dummy value */
  }  
    
  px = (double *)malloc(2*(nx+2)*sizeof(double));
  py = (double *)malloc(2*(ny+2)*sizeof(double));

  if(on_west_edge)
    istart = 1;
  else
    istart = 0;
  if(on_east_edge)
    iend = nx;
  else
    iend = nxp;
  if(on_south_edge)
    jstart = 1;
  else
    jstart = 0;
  if(on_north_edge)
    jend = ny;
  else
    jend = nyp;  
  /* west edge */

  if(on_west_edge) {
    i=0;
    for(j=jstart; j<=jend; j++) {
      /* get mid point sphere */
      p1[0] = lont[j*(nx+2)+i  ]; p1[1] = latt[j*(nx+2)+i  ];
      p2[0] = lont[j*(nx+2)+i+1]; p2[1] = latt[j*(nx+2)+i+1];
      mid_pt_sphere(p1, p2, &(py[2*j]));
    }    
    
    for(j=jstart; j<jend; j++) {
      p1[0] = lonc[j*nxp+i];
      p1[1] = latc[j*nxp+i];
      d1 = great_circle_distance(py+2*j, p1);
      d2 = great_circle_distance(py+2*(j+1), p1);
      edge_w[j] = d2/(d1+d2);
    }
  }
  /* east edge */
  if(on_east_edge) {  
    i=nx;
    for(j=jstart; j<=jend; j++) {
      /* get mid point sphere */
      p1[0] = lont[j*(nx+2)+i  ]; p1[1] = latt[j*(nx+2)+i  ];
      p2[0] = lont[j*(nx+2)+i+1]; p2[1] = latt[j*(nx+2)+i+1];
      mid_pt_sphere(p1, p2, &(py[2*j]));
    }    
    
    for(j=jstart; j<jend; j++) {
      p1[0] = lonc[j*nxp+i];
      p1[1] = latc[j*nxp+i];
      d1 = great_circle_distance(&(py[2*j]), p1);
      d2 = great_circle_distance(&(py[2*(j+1)]), p1);
      edge_e[j] = d2/(d1+d2);
    }  
  }
  
  /* south edge */
  if(on_south_edge) {
    j=0;
    for(i=istart; i<=iend; i++) {
      p1[0] = lont[j    *(nx+2)+i]; p1[1] = latt[j    *(nx+2)+i];
      p2[0] = lont[(j+1)*(nx+2)+i]; p2[1] = latt[(j+1)*(nx+2)+i];
      mid_pt_sphere(p1, p2, &(px[2*i]));
    }
    for(i=istart; i<iend; i++) {
      p1[0] = lonc[j*nxp+i];
      p1[1] = latc[j*nxp+i];
      d1 = great_circle_distance(&(px[2*i]), p1);
      d2 = great_circle_distance(&(px[2*(i+1)]), p1);
      edge_s[i] = d2/(d1+d2);
    }
  }
  /* north edge */
  if(on_north_edge) {
    j=ny;
    for(i=istart; i<=iend; i++) {
      p1[0] = lont[j    *(nx+2)+i]; p1[1] = latt[j    *(nx+2)+i];
      p2[0] = lont[(j+1)*(nx+2)+i]; p2[1] = latt[(j+1)*(nx+2)+i];
      mid_pt_sphere(p1, p2, &(px[2*i]));
    }
    for(i=istart; i<iend; i++) {
      p1[0] = lonc[j*nxp+i];
      p1[1] = latc[j*nxp+i];
      d1 = great_circle_distance(&(px[2*i]), p1);
      d2 = great_circle_distance(&(px[2*(i+1)]), p1);
      edge_n[i] = d2/(d1+d2);
    }  
  }
  
  free(px);
  free(py);
  
}; /* get_edge */

void mid_pt_sphere(const double *p1, const double *p2, double *pm)
{
  double e1[3], e2[3], e3[3];

  latlon2xyz(1, p1, p1+1, e1, e1+1, e1+2);
  latlon2xyz(1, p2, p2+1, e2, e2+1, e2+2);
  mid_pt3_cart(e1, e2, e3);
  xyz2latlon(1, e3, e3+1, e3+2, pm, pm+1);

}

void mid_pt3_cart(const double *p1, const double *p2, double *e)
{
  double dd;

  e[0] = p1[0] + p2[0];
  e[1] = p1[1] + p2[1];
  e[2] = p1[2] + p2[2];
  dd = sqrt( e[0]*e[0] + e[1]*e[1] + e[2]*e[2] );
  e[0] /= dd;
  e[1] /= dd;
  e[2] /= dd;
}

/**********************************************************************************************
  This routine is used to calculate grid information for second order conservative interpolation
  from cubic grid to other grid
  the size of xt     will be (nx+2,ny+2), T-cell center, with halo = 1
  the size of yt     will be (nx+2,ny+2), T-cell center, with halo = 1
  the size of xc     will be (nx+1,ny+1), C-cell center
  the size of yc     will be (nx+1,ny+1), C-cell center
  the size of dx     will be (nx, ny+1),  N-cell center
  the size of dy     will be (nx+1, ny),  E-cell center
  the size of area   will be (nx, ny),    T-cell center.
  The size of edge_w will be (ny-1),      C-cell center, without two end point
  The size of edge_e will be (ny-1),      C-cell center, without two end point
  The size of edge_s will be (nx-1),      C-cell center, without two end point
  The size of edge_n will be (nx-1),      C-cell center, without two end point
  The size of en_n   will be (nx, ny+1,3),N-cell center
  The size of en_e   will be (nx+1,ny,3), E-cell center
  The size of vlon   will be (nx, ny)     T-cell center
  The size of vlat   will be (nx, ny),    T-cell center
**********************************************************************************************/
void calc_c2l_grid_info_(int *nx_pt, int *ny_pt, const double *xt, const double *yt, const double *xc, const double *yc,
		         double *dx, double *dy, double *area, double *edge_w, double *edge_e, double *edge_s,
		         double *edge_n, double *en_n, double *en_e, double *vlon, double *vlat,
			int *on_west_edge, int *on_east_edge, int *on_south_edge, int *on_north_edge)
{
    calc_c2l_grid_info(nx_pt, ny_pt, xt, yt, xc, yc, dx, dy, area, edge_w, edge_e, edge_s, edge_n,
		       en_n, en_e, vlon, vlat, on_west_edge, on_east_edge, on_south_edge, on_north_edge);

}

void calc_c2l_grid_info(int *nx_pt, int *ny_pt, const double *xt, const double *yt, const double *xc, const double *yc,
		        double *dx, double *dy, double *area, double *edge_w, double *edge_e, double *edge_s,
		        double *edge_n, double *en_n, double *en_e, double *vlon, double *vlat,
			int *on_west_edge, int *on_east_edge, int *on_south_edge, int *on_north_edge)
{
  double *x, *y, *z, *xt_tmp, *yt_tmp;
  int    nx, ny, nxp, nyp, i, j;
  double p1[3], p2[3], p3[3], p4[3];


  nx  = *nx_pt;
  ny  = *ny_pt;
  nxp = nx+1;
  nyp = ny+1;

  for(j=0; j<nyp; j++) for(i=0; i<nx; i++) {
    p1[0] = xc[j*nxp+i];
    p1[1] = yc[j*nxp+i];
    p2[0] = xc[j*nxp+i+1];
    p2[1] = yc[j*nxp+i+1];
    dx[j*nx+i] = great_circle_distance(p1, p2);
  }

  for(j=0; j<ny; j++) for(i=0; i<nxp; i++) {
    p1[0] = xc[j*nxp+i];
    p1[1] = yc[j*nxp+i];
    p2[0] = xc[(j+1)*nxp+i];
    p2[1] = yc[(j+1)*nxp+i];
    dy[j*nxp+i] = great_circle_distance(p1, p2);
  }
  
  for(j=0; j<ny; j++) for(i=0; i<nx; i++) {
    p1[0] = xc[j*nxp+i];       /* ll lon */
    p1[1] = yc[j*nxp+i];       /* ll lat */
    p2[0] = xc[(j+1)*nxp+i];   /* ul lon */
    p2[1] = yc[(j+1)*nxp+i];   /* ul lat */
    p3[0] = xc[j*nxp+i+1];     /* lr lon */
    p3[1] = yc[j*nxp+i+1];     /* lr lat */
    p4[0] = xc[(j+1)*nxp+i+1]; /* ur lon */
    p4[1] = yc[(j+1)*nxp+i+1]; /* ur lat */	
    area[j*nx+i] = spherical_excess_area(p1, p2, p3, p4, RADIUS);
  }

  x = (double *)malloc(nxp*nyp*sizeof(double));
  y = (double *)malloc(nxp*nyp*sizeof(double));
  z = (double *)malloc(nxp*nyp*sizeof(double));
  
  latlon2xyz(nxp*nyp, xc, yc, x, y, z);
  for(j=0; j<nyp; j++) for(i=0; i<nx; i++) {
    p1[0] = x[j*nxp+i];
    p1[1] = y[j*nxp+i];
    p1[2] = z[j*nxp+i];
    p2[0] = x[j*nxp+i+1];
    p2[1] = y[j*nxp+i+1];
    p2[2] = z[j*nxp+i+1];
    vect_cross(p1, p2, en_n+3*(j*nx+i) );
    normalize_vect(en_n+3*(j*nx+i));
  }

  for(j=0; j<ny; j++) for(i=0; i<nxp; i++) {
    p2[0] = x[j*nxp+i];
    p2[1] = y[j*nxp+i];
    p2[2] = z[j*nxp+i];
    p1[0] = x[(j+1)*nxp+i];
    p1[1] = y[(j+1)*nxp+i];
    p1[2] = z[(j+1)*nxp+i];
    vect_cross(p1, p2, en_e+3*(j*nxp+i) );
    normalize_vect(en_e+3*(j*nxp+i));
  }  
  
  xt_tmp = (double *)malloc(nx*ny*sizeof(double));
  yt_tmp = (double *)malloc(nx*ny*sizeof(double));
  for(j=0; j<ny; j++)for(i=0; i<nx; i++) {
    xt_tmp[j*nx+i] = xt[(j+1)*(nx+2)+i+1];
    yt_tmp[j*nx+i] = yt[(j+1)*(nx+2)+i+1];
  }  
  unit_vect_latlon(nx*ny, xt_tmp, yt_tmp, vlon, vlat);
  get_edge(nx, ny, xt, yt, xc, yc, edge_w, edge_e, edge_s, edge_n, *on_west_edge, *on_east_edge,
	   *on_south_edge, *on_north_edge);

  free(x);
  free(y);
  free(z);
  free(xt_tmp);
  free(yt_tmp);
  
}



