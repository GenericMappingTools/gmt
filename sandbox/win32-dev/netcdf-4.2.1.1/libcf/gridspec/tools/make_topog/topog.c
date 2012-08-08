#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "topog.h"
#include "create_xgrid.h"
#include "mosaic_util.h"
#include "mpp_io.h"

#define D2R (M_PI/180.)

void filter_topo( int nx, int ny, int num_pass, int smooth_topo_allow_deepening, double *depth);
void set_depth(int nx, int ny, const double *xbnd, const double *ybnd, double alat1, double slon1,
	       double elon1, double alat2, double slon2, double elon2, double depth_in, double *depth);
void enforce_min_depth(int nx, int ny, int round_shallow, int deepen_shallow,
		       int fill_shallow, double min_depth, double *depth);

/*********************************************************************
   void create_rectangular_topog()
   Constructing a rectangular basin with a flat bottom, basin_depth can be 0 ( all land).
 ********************************************************************/
void create_rectangular_topog(int nx, int ny, double basin_depth, double *depth)
{
  int i;

  for(i=0; i<nx*ny; i++)  depth[i] = basin_depth;
}; /* create_rectangular_topog  */

/*********************************************************************
   void create_bowl_topog()
   From "Simulation of density-driven frictional downslope flow
   in z-coordinate mocean models"
   Winton et al. JPO, Vol 28, No 11, 2163-2174, November 1998
 ********************************************************************/
void create_bowl_topog(int nx, int ny, const double *x, const double *y, double bottom_depth,
		       double min_depth, double bowl_east, double bowl_south, double bowl_west,
		       double bowl_north, double *depth)
{
  double bottom, xx, yy;
  int i, j, nxp, nyp;

  nxp = nx+1;
  nyp = ny+1;
  
  for(j=0;j<ny;j++){
    for(i=0;i<nx;i++){
      xx = (x[j*nxp+i]+x[j*nxp+i+1]+x[(j+1)*nxp+i]+x[(j+1)*nxp+i+1])*0.25;
      yy = (y[j*nxp+i]+y[j*nxp+i+1]+y[(j+1)*nxp+i]+y[(j+1)*nxp+i+1])*0.25;	    
      if (xx <= bowl_west  || xx >= bowl_east || yy <= bowl_south || yy >= bowl_north)            
	bottom = min_depth;
      else
	bottom = min_depth + bottom_depth*(1.0-exp(-pow((yy-bowl_south)/2.0,2)))
	  *(1.0-exp(-pow((yy-bowl_north)/2.0,2)))
	  *(1.0-exp(-pow((xx-bowl_west)/4.0,2)))
	  *(1.0-exp(-pow((xx-bowl_east)/4.0,2)));
      depth[j*nx+i] = bottom;
    }
  }
}; /* create_bowl_topog */


/*********************************************************************
   void create_gaussian_topog()
   Constructing a gaussian bump
 ********************************************************************/
void create_gaussian_topog(int nx, int ny, const double *x, const double *y, double bottom_depth,
			   double min_depth, double gauss_amp, double gauss_scale, double slope_x,
			   double slope_y, double *depth)
{
  double bump_height, bump_scale, xcent, ycent, arg, bottom;
  double xe, xw, ys, yn;
  double *xt, *yt;
  int i, j, nxp, nyp;

  xt = (double *)malloc(nx*ny*sizeof(double));
  yt = (double *)malloc(nx*ny*sizeof(double));
  for(j=0; j<ny; j++) {
    for(i=0; i<nx; i++) {
      xt[j*nx+i] = (x[j*nxp+i] + x[j*nxp+i+1] + x[(j+1)*nxp+i] + x[(j+1)*nxp+i+1])*0.25;
      yt[j*nx+i] = (y[j*nxp+i] + y[j*nxp+i+1] + y[(j+1)*nxp+i] + y[(j+1)*nxp+i+1])*0.25;
    }
  }
  
  xw = xt[0];
  ys = yt[0];
  xe = xw;
  yn = ys;

  for(j=1;j<ny;j++){
    for(i=1;i<nx;i++){
      xw = min(xt[j*nx+i],xw); xe = max(xt[j*nx+i],xe);
      ys = min(yt[j*nx+i],ys); yn = max(yt[j*nx+i],yn);
    }
  }

  bump_height = gauss_amp*bottom_depth;
  bump_scale = gauss_scale*min(xe-xw, yn-ys);
  xcent = 0.5*(xe+xw);
  ycent = 0.5*(yn+ys);

  printf("Constructing a gaussian bump of height = %f meters with a scale width of %f degrees.\n",
	 bump_height, bump_scale);
  printf("The bump is centered at (lon,lat) = (%f,%f) deg.\n", xcent, ycent);
  printf("The ocean floor rises with a slope of %f meters/deg towards the east and %f "
	 " meters/deg to the north.", slope_x, slope_y) ;

  for(j=0;i<nx*ny;j++){
      arg = pow(xt[i]-xcent,2) + pow(yt[i]-ycent,2);
      bottom = bottom_depth - bump_height*exp(-arg/pow(bump_scale,2));
      bottom = bottom - slope_x*(xt[i]-xw)- slope_y*(yt[i]-ys);
      depth[i] = max(bottom,min_depth);
  }

  free(xt);
  free(yt);
  
}; /* create_gaussian_topog */


/*********************************************************************

  void create_idealized_topog
   construct a highly "idealized" world ... piece by piece

   note: the purpose of this geometry/topography is to automatically
         map into arbitrary resolution as grid dimensions are
         changed, thereby facilitating the implementation
         and verification of the model on various computer platforms
         without referencing the topographic database.  Although it
         somewhat resembles the real world, it is NOT realistic.
   Note: this routine needs to be re-thought for generalized curvilinear coordinates

 ********************************************************************/
void create_idealized_topog( int nx, int ny, const double *x, const double *y,
			     double bottom_depth, double min_depth, double *depth)
{
  int i, j, nxp, nyp;
  double arg;
  double *xbnd, *ybnd;
  
  for(j=0;j<ny;j++){
    for(i=0;i<nx;i++) depth[j*nx+i]=bottom_depth;
  }
  
  //antarctica
  xbnd = (double *)malloc(nx*sizeof(double));
  ybnd = (double *)malloc(ny*sizeof(double));
  for(i=0; i<nx; i++) xbnd[i] = (x[i]+x[i+1])*0.5;
  for(j=0; j<ny; j++) ybnd[j] = (y[j*nxp]+y[(j+1)*nxp])*0.5;
  
  set_depth (nx, ny, xbnd, ybnd, -90.0, 0.0, 360.0, -80.0, 0.0, 360.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, -80.0, 360.0-25.0, 360.0, -70.0, 360.0, 360.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, -80.0, 0.0, 360.0, -70.0, 0.0, 170.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, -80.0, 360.0-135.0, 360.0-60.0, -68.0, 360.0-75.0, 360.0-60.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, -70.0, 0.0, 155.0, -67.0, 50.0, 145.0, 0.0, depth);

  // australia

  set_depth (nx, ny, xbnd, ybnd, -35.0, 116.0, 120.0, -31.0, 114.0, 130.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, -38.0, 140.0, 151.0, -31.0, 130.0, 151.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, -31.0, 115.0, 153.0, -20.0, 113.0, 149.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, -20.0, 113.0, 149.0, -11.0, 131.0, 143.0, 0.0, depth);

  // south america

  set_depth (nx, ny, xbnd, ybnd, -50.0, 360.0-74.0, 360.0-68.0, -40.0, 360.0-73.0, 360.0-62.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, -40.0, 360.0-73.0, 360.0-62.0, -20.0, 360.0-70.0, 360.0-40.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, -20.0, 360.0-70.0, 360.0-40.0, -16.0, 360.0-81.0, 360.0-35.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, -16.0, 360.0-81.0, 360.0-35.0, 0.0, 360.0-80.0, 360.0-50.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 0.0, 360.0-80.0, 360.0-50.0, 11.0, 360.0-75.0, 360.0-60.0, 0.0, depth);

  // central america

  set_depth (nx, ny, xbnd, ybnd, 6.0, 360.0-78.0, 360.0-75.0, 20.0, 360.0-105.0, 360.0-97.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 20.0, 360.0-105.0, 360.0-97.0, 30.0, 360.0-115.0, 360.0-94.0, 0.0, depth);

  // north america

  set_depth (nx, ny, xbnd, ybnd, 25.0, 360.0-82.0, 360.0-80.0, 30.0, 360.0-85.0, 360.0-81.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 30.0, 360.0-115.0, 360.0-80.0, 40.0, 360.0-124.0, 360.0-74.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 40.0, 360.0-124.0, 360.0-74.0, 50.0, 360.0-124.0, 360.0-57.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 50.0, 360.0-124.0, 360.0-57.0, 60.0, 360.0-140.0, 360.0-64.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 60.0, 360.0-165.0, 360.0-64.0, 65.0, 360.0-140.0, 360.0-64.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 65.0, 360.0-140.0, 360.0-64.0, 70.0, 360.0-162.0, 360.0-72.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 70.0, 360.0-162.0, 360.0-140.0, 72.0, 360.0-157.0, 360.0-157.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 70.0, 360.0-130.0, 360.0-70.0, 75.0, 360.0-120.0, 360.0-80.0, 0.0, depth);

  // greenland

  set_depth (nx, ny, xbnd, ybnd, 60.0, 360.0-45.0, 360.0-45.0, 75.0, 360.0-58.0, 360.0-19.0, 0.0, depth);

  // africa

  set_depth (nx, ny, xbnd, ybnd, -35.0, 19.0, 28.0, 6.0, 8.0, 50.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 6.0, 0.0, 50.0, 18.0, 0.0, 56.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 18.0, 0.0, 56.0, 26.0, 0.0, 59.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 6.0, 360.0-10.0, 360.0, 18.0, 360.0-18.0, 360.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 18.0, 360.0-18.0, 360.0, 26.0,  360.0-15.0, 360.0, 0.0, depth);

  // northern africa and europe and asia

  set_depth (nx, ny, xbnd, ybnd, 26.0, 360.0-15.0, 360.0, 40.0, 360.0-7.0, 360.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 40.0, 360.0-7.0, 360.0, 50.0, 360.0, 360.0, 0.0, depth);

  set_depth (nx, ny, xbnd, ybnd, 8.0, 77.0, 78.0, 26.0, 65.0, 90.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 4.0, 99.0, 100.0, 26.0, 90.0, 115.0, 0.0, depth);

  set_depth (nx, ny, xbnd, ybnd, 26.0, 0.0, 126.0, 40.0, 0.0, 122.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 40.0, 0.0, 130.0, 50.0, 0.0, 140.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 50.0, 0.0, 140.0, 60.0, 8.0, 140.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 60.0, 8.0, 163.0, 65.0, 13.0, 180.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 65.0, 13.0, 188.0, 70.0, 20.0, 180.0, 0.0, depth);
  set_depth (nx, ny, xbnd, ybnd, 70.0, 70.0, 180.0, 75.0, 90.0, 100.0, 0.0, depth);

  // add an "idealized" undulating topography

  nxp = nx+1;
  nyp = ny+1;
  for(j=0;j<ny;j++){
    for(i=0;i<nx;i++){
      if (depth[j*nx+i] > 0.0 ) {
	arg = bottom_depth*(1-0.4*fabs(cos(((j+1)*M_PI)/nyp)*sin(((i+1)*2*M_PI)/nxp)));
        arg = max(arg, min_depth);      
        depth[j*nx+i] = arg;
      }
    }
  }

  // add "idealized" ridges

  arg = 0.666*bottom_depth;
  
  set_depth (nx, ny, xbnd, ybnd, -20.0, 360.0-20.0, 360.0-10.0, 30.0, 360.0-45.0, 360.0-35.0, arg, depth);
  set_depth (nx, ny, xbnd, ybnd, 30.0, 360.0-45.0, 360.0-35.0, 60.0, 360.0-20.0,  360.0-30.0, arg, depth);
  set_depth (nx, ny, xbnd, ybnd, -60.0,360.0-100.0, 360.0-130.0, 40.0, 360.0-160.0, 180.0, arg, depth);

  arg = 0.5*bottom_depth;
  
  set_depth (nx, ny, xbnd, ybnd, -50.0, 360.0-120.0, 360.0-120.0, 30.0, 190.0, 190.0, arg, depth);

  free(xbnd);
  free(ybnd);
  
} /* create_idealized_topog */

/*********************************************************************
   void set_depth(double alat1, double slon1, double elon1, double alat2,
                  double slon2, double elon2, double depth_in )
        set the topography depth "depth[i][j](i,j)" = "depth_in" within the area of
        the trapezoid bounded by vertices:
        (alat1,slon1), (alat1,elon1), (alat2,slon2), and (alat2,elon2)
   
        inputs:
   
        alat1 = southern latitude of trapezoid (degrees)
        slon1 = starting longitude of southern edge of trapezoid (deg)
        elon1 = ending longitude of southern edge of trapezoid (deg)
        alat2 = northern latitude of trapezoid (degrees)
        slon2 = starting longitude of northern edge of trapezoid (deg)
        elon2 = ending longitude of northern edge of trapezoid (deg)
        depth_in = constant depth value
   
 ********************************************************************/
void set_depth(int nx, int ny, const double *xbnd, const double *ybnd, double alat1, double slon1, double elon1, double alat2,
                      double slon2, double elon2, double depth_in, double *depth)
{
  double rdj, d;
  int i, j, i1, i2, j1, j2, is, ie, js, je, is1, ie1, is2, ie2;
  
  j1 = nearest_index(alat1, ybnd, ny );
  j2 = nearest_index(alat2, ybnd, ny );
  js = min(j1,j2);
  je = max(j1,j2);

  i1  = nearest_index(slon1, xbnd, nx);
  i2  = nearest_index(elon1, xbnd, nx);
  is1 = min(i1,i2);
  ie1 = max(i1,i2);

  i1  = nearest_index(slon2, xbnd, nx );
  i2  = nearest_index(elon2, xbnd, ny );
  is2 = min(i1,i2);
  ie2 = max(i1,i2);

  is = is1;
  ie = ie1;

  // fill in the area bounded by (js,is1), (js,ie1), (je,is2), (je,ie2)
  // the nudging of 1.e-5 is to insure that the test case resolution
  // generates the same topography and geometry on various computers.

  if (js == je) 
    rdj = 1.0;
  else
    rdj = 1.0/(je-js);

  for(j=js;j<=je;j++){
    for(i=is;i<=ie;i++) depth[j*nx+i] = depth_in;
    d = rdj*((j-js)*is2 + (je-j)*is1) + 1.0e-5;
    is = ceil(d);
    if( is - d > 0.5) is = is -1;
    d = rdj*((j-js)*ie2 + (je-j)*ie1) + 1.0e-5;
    ie = ceil(d);
    if( ie - d > 0.5) ie = ie -1;
  }

}; /* set_depth */

/*********************************************************************
   void create_realistic_topog( )
   reading data from source data file topog_file and remap it onto current grid
 ********************************************************************/
void create_realistic_topog(int nx_dst, int ny_dst, const double *x_dst, const double *y_dst,
			    const char* topog_file, const char* topog_field, double scale_factor,
			    double fill_first_row, int filter_topog, int num_filter_pass,
			    int smooth_topo_allow_deepening, int round_shallow, int fill_shallow,
			    int deepen_shallow, double min_depth, double *depth )
{
  char xname[128], yname[128];
  int nx_src, ny_src, nxp_src, nyp_src, i, j, count, n;
  double *depth_src, *mask_src, *xt_src, *yt_src, *x_src, *y_src;
  double *x_out, *y_out;
  double missing;
  int fid, vid;
  
  /* first read source topography data to get source grid and source topography */
  fid = mpp_open(topog_file, MPP_READ);
  vid = mpp_get_varid(fid, topog_field);
  mpp_get_var_dimname(fid, vid, 1, xname);
  mpp_get_var_dimname(fid, vid, 0, yname);
  nx_src = mpp_get_dimlen( fid, xname );
  ny_src = mpp_get_dimlen( fid, yname );
  nxp_src = nx_src + 1;
  nyp_src = ny_src + 1;
  depth_src = (double *)malloc(nx_src*ny_src*sizeof(double));
  mask_src = (double *)malloc(nx_src*ny_src*sizeof(double));
  xt_src    = (double *)malloc(nx_src*sizeof(double));
  yt_src    = (double *)malloc(ny_src*sizeof(double));
  x_src    = (double *)malloc(nxp_src*nyp_src*sizeof(double)); 
  y_src    = (double *)malloc(nxp_src*nyp_src*sizeof(double));

  mpp_get_var_att(fid, vid, "missing_value", &missing);
  mpp_get_var_value(fid, vid, depth_src);
  vid = mpp_get_varid(fid, xname);
  mpp_get_var_value(fid, vid, xt_src);
  vid = mpp_get_varid(fid, yname);
  mpp_get_var_value(fid, vid, yt_src);
  mpp_close(fid);
  
  for(j=0; j<nyp_src; j++) {
     for(i=1; i<nx_src; i++) x_src[j*nxp_src+i] = (xt_src[i-1] + xt_src[i])*0.5;
     x_src[j*nxp_src] = 2*xt_src[0] - x_src[j*nxp_src+1];
     x_src[j*nxp_src+nx_src] = 2*xt_src[nx_src-1] - x_src[j*nxp_src+nx_src-1];
  }
  for(i=0; i<nxp_src; i++) {
    for(j=1; j<ny_src; j++) y_src[j*nxp_src+i] = (yt_src[j-1] + yt_src[j])*0.5;
    y_src[i] = 2*yt_src[0] - y_src[nxp_src+i];
    y_src[ny_src*nxp_src+i] = 2*yt_src[ny_src-1] - y_src[(ny_src-1)*nxp_src+i];
  }
    

  for(i=0; i<nx_src*ny_src; i++) {
    if(depth_src[i] == missing)
      mask_src[i] = 0.0;
    else {
      depth_src[i] = depth_src[i]*scale_factor;
      if( depth_src[i] <= 0.0)
	mask_src[i] = 0.0;
      else
	mask_src[i] = 1.0;
    }
  }

  /* scale grid to radius */
  for(i=0; i<nxp_src*nyp_src; i++) {
    x_src[i] = x_src[i]*D2R;
    y_src[i] = y_src[i]*D2R;
  }
  x_out = (double *)malloc((nx_dst+1)*(ny_dst+1)*sizeof(double));
  y_out = (double *)malloc((nx_dst+1)*(ny_dst+1)*sizeof(double));
  for(i=0; i<(nx_dst+1)*(ny_dst+1); i++) {
    x_out[i] = x_dst[i]*D2R;
    y_out[i] = y_dst[i]*D2R;
  }
  
  /* doing the conservative interpolation */
  conserve_interp(nx_src, ny_src, nx_dst, ny_dst, x_src, y_src, x_out, y_out,
		  mask_src, depth_src, depth );
  
  /* make first row of ocean model all land points for ice model */

  if(fill_first_row) {
    for(i=0;i<nx_dst;i++) {
      depth[i] = 0.0;
    }
  }

  if (filter_topog) filter_topo(nx_dst, ny_dst, num_filter_pass, smooth_topo_allow_deepening, depth);
  /* enforce minimum depth when one of round_shallow, deepen_shallow or fill_shallow
     is true ( At most one ).         */
  count = 0;
  if(round_shallow) count++;
  if(fill_shallow) count++;
  if(deepen_shallow) count++;
  if(count > 1) mpp_error("topog: at most one of round_shallow/deepen_shallow/fill_shallow can be set to true");
     
  if(count>0) enforce_min_depth(nx_dst, ny_dst, round_shallow, deepen_shallow, fill_shallow,
				min_depth, depth);

  free(depth_src);
  free(mask_src);
  free(x_src);
  free(y_src);
  free(xt_src);
  free(yt_src);
  free(x_out);
  free(y_out);
  
}; /* create_realistic_topog */


/*********************************************************************

    void filter_topo( int nx, int ny, int num_pass, double *depth)

       smooth topographic depth "d" with "num_pass" applications of a 2D
       version of a shapiro filter (weights = 1/4, 1/2, 1/4) . 
       allow filtering to decrease the bottom depth but not increase it.
       do not allow original geometry to change.
       note: depth "d" should be on a grid of uniformly constant spacing

********************************************************************/
void filter_topo( int nx, int ny, int num_pass, int smooth_topo_allow_deepening, double *depth)
{
  double *rmask, *s;  
  double f[9], d_old;
  int n1, n2, i, j, n, m, ip, jp;
  size_t *dims;
  
  rmask = (double *)malloc(nx*ny*sizeof(double));
  s     = (double *)malloc(nx*ny*sizeof(double));

  /* 2D symmetric filter weights */

  f[0] = 1.0/16.0;
  f[1] = 1.0/8.0;
  f[2] = 1.0/16.0;
  f[3] = 1.0/8.0;
  f[4] = 1.0/4.0;
  f[5] = 1.0/8.0;
  f[6] = 1.0/16.0;
  f[7] = 1.0/8.0;
  f[8] = 1.0/16.0;
  
  /* geometry mask */
  for(i=0; i<nx*ny; i++) {
    if(depth[i] == 0.0)
      rmask[i] = 0.0;
    else
      rmask[i] = 1.0;
  }

  for(n=1;n<=num_pass;n++) {
    for(j=1;j<ny-1;j++) {
      for(i=1;i<nx-1;i++) {
	s[j*nx+i] = 0.0;
	d_old = depth[j*nx+i];
	for(ip=-1;ip<=1;ip++) {
	  for(jp=-1;jp<=1;jp++) {
	    m = (ip+1)*3 + jp+1;
	    if (rmask[(j+jp)*nx+i+ip] == 0.0) 
	      s[j*nx+i] = s[j*nx+i] + depth[j*nx+i]*f[m];
            else
	      s[j*nx+i] = s[j*nx+i] + depth[(j+jp)*nx+i+ip]*f[m];
	  }
	}
	if (! smooth_topo_allow_deepening) {
	  if (s[j*nx+i] > d_old)  s[j*nx+i] = d_old;
	}
	s[j*nx+i] = s[j*nx+i]*rmask[j*nx+i];
      }
    }
    for(i=0; i<nx*ny; i++) depth[i] = s[i];
  }
  
}; /* filter_topog */

/*********************************************************************
  void enforce_min_depth()
  ! limit the minimum value of depth, depth should be >= min_depth. 
 ********************************************************************/
void enforce_min_depth(int nx, int ny, int round_shallow, int deepen_shallow,
			   int fill_shallow, double min_depth, double *depth)
{
  int i, j, n, l;
  
  if(mpp_pe() == mpp_root_pe() ) printf(" Enforcing the minimum topography depth to be %f\n",min_depth);

  n = 0;
  if(round_shallow) {    
    for(j=0;j<ny;j++){
      for(i=0;i<nx;i++){
	l = j*nx+i;
	if (depth[l] != 0 && depth[l] < min_depth) {
	  n = n + 1;
	  if ( depth[l] < 0.5*min_depth) {
	    printf("Making location i,j,depth= %d,%d,%f to land.\n",i, j, depth[l]);
	    depth[l] = 0.0;
	  }
	  else {
	    printf("Setting location i,j,depth= %d,%d,%f to minimum ocean depth.\n",i,j, depth[l]);
	    depth[l] = min_depth;
	  }
	}
      }
    }
  }
  else if (fill_shallow) {
    for(j=0;j<ny;j++){
      for(i=0;i<nx;i++){
	l = j*nx+i;
	if (depth[l] != 0 && depth[l] < min_depth) {
	  n = n + 1;
	  printf("Making location i,j,depth= %d,%d,%f to land.\n",i, j, depth[l]);
	  depth[l] = 0.0;
	}
      }
    }
  }
  else if (deepen_shallow) {
    for(j=0;j<ny;j++){
      for(i=0;i<nx;i++){
	l = j*nx+i;
	if (depth[l] != 0 && depth[l] < min_depth) {
	  n = n + 1;
	  printf("Setting location i,j,depth= %d,%d,%f to minimum ocean depth.\n",i,j, depth[l]);
	  depth[l] = min_depth;
	}
      }
    }
  }

}; /* enforce_min_depth */
