#include <stdlib.h>
#include <math.h>
#include "mosaic_util.h"
#include "interp.h"
#include "create_xgrid.h"

/*********************************************************************
   void cublic_spline(size1, size2, grid1, grid2, data1, data2, yp1, ypn  )

   This alborithm is to get an interpolation formula that is smooth in the first
   derivative, and continuous in the second derivative, both within an interval
   and its boundaries. It will be used to interpolation data in 1-D space.

   INPUT Arguments:
    grid1:    grid for input data grid.          
    grid2:    grid for output data grid.         
    size1:    size of input grid.                
    size2:    size of output grid.               
    data1:    input data associated with grid1.
    yp1:      first derivative of starting point.
              Set to 0 to be "natural"           (INPUT)
    ypn:      first derivative of ending point.
              Set to 0 to be "natural"           (INPUT)

   OUTPUT ARGUMENTS:
    data2:    output data associated with grid2. (OUTPUT)

*********************************************************************/

void cubic_spline(int size1, int size2, const double *grid1, const double *grid2, const double *data1,
		  double *data2, double yp1, double ypn  )
{
  double *y2=NULL, *u=NULL;
  double p, sig, qn, un, h, a, b;
  int i, k, n, klo, khi;
  
  for(i=1; i<size1; i++) {
    if( grid1[i] <= grid1[i-1] ) error_handler("cubic_spline: grid1 is not monotonic increasing");
  }

  for(i=0; i<size2; i++) {
    if( grid2[i] < grid1[0] || grid2[i] > grid1[size1-1]) error_handler("cubic_spline: grid2 lies outside grid1");
  }  

  if(size1 < 2) error_handler("cubic_spline: the size of input grid should be at least 2");
  if(size1 == 2) {  /* when size1 is 2, it just reduced to a linear interpolation */
    p = (data1[1]-data1[0])/(grid1[1]-grid1[0]);
    for(i=0; i< size2; i++) data2[i] = p*(grid2[i] - grid1[0]) + data1[0];
    return;
  }
  y2 = (double *)malloc(size1*sizeof(double));
  u = (double *)malloc(size1*sizeof(double));
  if (yp1 >.99e30) {
    y2[0]=0.;
    u[0]=0.;
  }
  else {
    y2[0]=-0.5;
    u[0]=(3./(grid1[1]-grid1[0]))*((data1[1]-data1[0])/(grid1[1]-grid1[0])-yp1);
  }

  for(i=1; i<size1-1; i++) {
    sig=(grid1[i]-grid1[i-1])/(grid1[i+1]-grid1[i-1]);
    p=sig*y2[i-1]+2.;
    y2[i]=(sig-1.)/p;
    u[i]=(6.*((data1[i+1]-data1[i])/(grid1[i+1]-grid1[i])-(data1[i]-data1[i-1])
	      /(grid1[i]-grid1[i-1]))/(grid1[i+1]-grid1[i-1])-sig*u[i-1])/p;

  }
  
  if (ypn > .99e30) {
    qn=0.;
    un=0.;
  }
  else {
    qn=0.5;
    un=(3./(grid1[size1-1]-grid1[size1-2]))*(ypn-(data1[size1-1]-data1[size1-2])/(grid1[size1-1]-grid1[size1-2]));
  }

  y2[size1-1]=(un-qn*u[size1-2])/(qn*y2[size1-2]+1.);

  for(k=size1-2; k>=0; k--) y2[k] = y2[k]*y2[k+1]+u[k];

  /* interpolate data onto grid2 */
  for(k=0; k<size2; k++) {
    n = nearest_index(grid2[k],grid1, size1);
    if (grid1[n] < grid2[k]) {
	 klo = n;
    }
    else {
      if(n==0) {
	klo = n;
      }
      else {
	klo = n -1;
      }
    }
    khi = klo+1;
    h   = grid1[khi]-grid1[klo];
    a   = (grid1[khi] - grid2[k])/h;
    b   = (grid2[k] - grid1[klo])/h;
    data2[k] = a*data1[klo] + b*data1[khi]+ ((pow(a,3.0)-a)*y2[klo] + (pow(b,3.0)-b)*y2[khi])*(pow(h,2.0))/6;
  }

  free(y2);
  free(u);
  
};/* cubic spline */


/*------------------------------------------------------------------------------
  void conserve_interp()
  conservative interpolation through exchange grid.
  Currently only first order interpolation are implemented here.
  ----------------------------------------------------------------------------*/
void conserve_interp(int nx_src, int ny_src, int nx_dst, int ny_dst, const double *x_src,
		     const double *y_src, const double *x_dst, const double *y_dst,
		     const double *mask_src, const double *data_src, double *data_dst )
{
  int n, nxgrid;
  int *xgrid_i1, *xgrid_j1, *xgrid_i2, *xgrid_j2;
  double *xgrid_area, *dst_area, *area_frac; 
  
  /* get the exchange grid between source and destination grid. */
  xgrid_i1   = (int    *)malloc(MAXXGRID*sizeof(int));
  xgrid_j1   = (int    *)malloc(MAXXGRID*sizeof(int));
  xgrid_i2   = (int    *)malloc(MAXXGRID*sizeof(int));
  xgrid_j2   = (int    *)malloc(MAXXGRID*sizeof(int));
  xgrid_area = (double *)malloc(MAXXGRID*sizeof(double));
  dst_area   = (double *)malloc(nx_dst*ny_dst*sizeof(double));
  nxgrid = create_xgrid_2dx2d_order1(&nx_src, &ny_src, &nx_dst, &ny_dst, x_src, y_src, x_dst, y_dst, mask_src,
	                       xgrid_i1, xgrid_j1, xgrid_i2, xgrid_j2, xgrid_area );
  get_grid_area(&nx_dst, &ny_dst, x_dst, y_dst, dst_area);
  area_frac = (double *)malloc(nxgrid*sizeof(double));
  for(n=0; n<nxgrid; n++) area_frac[n] = xgrid_area[n]/dst_area[xgrid_j2[n]*nx_dst+xgrid_i2[n]];
  
  for(n=0; n<nx_dst*ny_dst; n++) {
    data_dst[n] = 0;
  }
  for(n=0; n<nxgrid; n++) {
    data_dst[xgrid_j2[n]*nx_dst+xgrid_i2[n]] += data_src[xgrid_j1[n]*nx_src+xgrid_i1[n]]*area_frac[n];
  }

  free(xgrid_i1);
  free(xgrid_j1);
  free(xgrid_i2);
  free(xgrid_j2);
  free(xgrid_area);
  free(dst_area);
  free(area_frac);
  
}; /* conserve_interp */
