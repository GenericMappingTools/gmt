#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mosaic_util.h"
#include "create_xgrid.h"
#include "constant.h"

#define AREA_RATIO_THRESH (1.e-6)  
#define MASK_THRESH       (0.5)
#define EPSLN             (1.0e-30)
double grid_box_radius(const double *x, const double *y, const double *z, int n);
double dist_between_boxes(const double *x1, const double *y1, const double *z1, int n1,
			  const double *x2, const double *y2, const double *z2, int n2);
int inside_edge(double x0, double y0, double x1, double y1, double x, double y);
/*******************************************************************************
  int get_maxxgrid
  return constants MAXXGRID.
*******************************************************************************/
int get_maxxgrid(void)  
{
  return MAXXGRID;
}

int get_maxxgrid_(void)
{
  return get_maxxgrid();
}

/*******************************************************************************
void get_grid_area(const int *nlon, const int *nlat, const double *lon, const double *lat, const double *area)
  return the grid area.
*******************************************************************************/
void get_grid_area_(const int *nlon, const int *nlat, const double *lon, const double *lat, double *area)
{
  get_grid_area(nlon, nlat, lon, lat, area);
}

void get_grid_area(const int *nlon, const int *nlat, const double *lon, const double *lat, double *area)
{
  int nx, ny, nxp, i, j, n_in;
  double x_in[20], y_in[20];
  
  nx = *nlon;
  ny = *nlat;
  nxp = nx + 1;

  for(j=0; j<ny; j++) for(i=0; i < nx; i++) {
    x_in[0] = lon[j*nxp+i];
    x_in[1] = lon[j*nxp+i+1];
    x_in[2] = lon[(j+1)*nxp+i+1];
    x_in[3] = lon[(j+1)*nxp+i];
    y_in[0] = lat[j*nxp+i];
    y_in[1] = lat[j*nxp+i+1];
    y_in[2] = lat[(j+1)*nxp+i+1];
    y_in[3] = lat[(j+1)*nxp+i];
    n_in = fix_lon(x_in, y_in, 4, M_PI);    
    area[j*nx+i] = poly_area(x_in, y_in, n_in);
  }

};  /* get_grid_area */


void get_grid_area_no_adjust(const int *nlon, const int *nlat, const double *lon, const double *lat, double *area)
{
  int nx, ny, nxp, i, j, n_in;
  double x_in[20], y_in[20];
  
  nx = *nlon;
  ny = *nlat;
  nxp = nx + 1;

  for(j=0; j<ny; j++) for(i=0; i < nx; i++) {
    x_in[0] = lon[j*nxp+i];
    x_in[1] = lon[j*nxp+i+1];
    x_in[2] = lon[(j+1)*nxp+i+1];
    x_in[3] = lon[(j+1)*nxp+i];
    y_in[0] = lat[j*nxp+i];
    y_in[1] = lat[j*nxp+i+1];
    y_in[2] = lat[(j+1)*nxp+i+1];
    y_in[3] = lat[(j+1)*nxp+i];
    n_in = 4;
    area[j*nx+i] = poly_area_no_adjust(x_in, y_in, n_in);
  }

};  /* get_grid_area_no_adjust */

/*******************************************************************************
  void create_xgrid_1dx2d_order1
  This routine generate exchange grids between two grids for the first order
  conservative interpolation. nlon_in,nlat_in,nlon_out,nlat_out are the size of the grid cell
  and lon_in,lat_in are 1-D grid bounds, lon_out,lat_out are geographic grid location of grid cell bounds. 
*******************************************************************************/
int create_xgrid_1dx2d_order1_(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out,
			       const double *lon_in, const double *lat_in, const double *lon_out, const double *lat_out,
			       const double *mask_in, int *i_in, int *j_in, int *i_out, int *j_out, double *xgrid_area)
{
  int nxgrid;
  
  nxgrid = create_xgrid_1dx2d_order1(nlon_in, nlat_in, nlon_out, nlat_out, lon_in, lat_in, lon_out, lat_out, mask_in,
			       i_in, j_in, i_out, j_out, xgrid_area);
  return nxgrid;
    
};  

int create_xgrid_1dx2d_order1(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out, const double *lon_in,
			      const double *lat_in, const double *lon_out, const double *lat_out,
			      const double *mask_in, int *i_in, int *j_in, int *i_out,
			      int *j_out, double *xgrid_area)
{

  int nx1, ny1, nx2, ny2, nx1p, nx2p;
  int i1, j1, i2, j2, nxgrid;
  double ll_lon, ll_lat, ur_lon, ur_lat, x_in[MV], y_in[MV], x_out[MV], y_out[MV];
  double *area_in, *area_out, min_area;
  double *tmpx, *tmpy;
  
  nx1 = *nlon_in;
  ny1 = *nlat_in;
  nx2 = *nlon_out;
  ny2 = *nlat_out;

  nxgrid = 0;
  nx1p = nx1 + 1;
  nx2p = nx2 + 1;

  area_in = (double *)malloc(nx1*ny1*sizeof(double));
  area_out = (double *)malloc(nx2*ny2*sizeof(double));
  tmpx = (double *)malloc((nx1+1)*(ny1+1)*sizeof(double));
  tmpy = (double *)malloc((nx1+1)*(ny1+1)*sizeof(double));
  for(j1=0; j1<=ny1; j1++) for(i1=0; i1<=nx1; i1++) {
    tmpx[j1*nx1p+i1] = lon_in[i1];
    tmpy[j1*nx1p+i1] = lat_in[j1];
  }
  /* This is just a temporary fix to solve the issue that there is one point in zonal direction */
  if(nx1 > 1)
     get_grid_area(nlon_in, nlat_in, tmpx, tmpy, area_in);
  else
    get_grid_area_no_adjust(nlon_in, nlat_in, tmpx, tmpy, area_in);
  
  get_grid_area(nlon_out, nlat_out, lon_out, lat_out, area_out);  
  free(tmpx);
  free(tmpy);  

  for(j1=0; j1<ny1; j1++) for(i1=0; i1<nx1; i1++) if( mask_in[j1*nx1+i1] > MASK_THRESH ) {

    ll_lon = lon_in[i1];   ll_lat = lat_in[j1];
    ur_lon = lon_in[i1+1]; ur_lat = lat_in[j1+1];
    for(j2=0; j2<ny2; j2++) for(i2=0; i2<nx2; i2++) {
      int n_in, n_out;
      double Xarea;
      
      y_in[0] = lat_out[j2*nx2p+i2];
      y_in[1] = lat_out[j2*nx2p+i2+1];
      y_in[2] = lat_out[(j2+1)*nx2p+i2+1];
      y_in[3] = lat_out[(j2+1)*nx2p+i2];
      if (  (y_in[0]<=ll_lat) && (y_in[1]<=ll_lat)
	    && (y_in[2]<=ll_lat) && (y_in[3]<=ll_lat) ) continue;
      if (  (y_in[0]>=ur_lat) && (y_in[1]>=ur_lat)
	    && (y_in[2]>=ur_lat) && (y_in[3]>=ur_lat) ) continue;

      x_in[0] = lon_out[j2*nx2p+i2];
      x_in[1] = lon_out[j2*nx2p+i2+1];
      x_in[2] = lon_out[(j2+1)*nx2p+i2+1];
      x_in[3] = lon_out[(j2+1)*nx2p+i2];
      n_in = fix_lon(x_in, y_in, 4, (ll_lon+ur_lon)/2);
      
      if ( (n_out = clip ( x_in, y_in, n_in, ll_lon, ll_lat, ur_lon, ur_lat, x_out, y_out )) > 0 ) {
	Xarea = poly_area (x_out, y_out, n_out ) * mask_in[j1*nx1+i1];
	min_area = min(area_in[j1*nx1+i1], area_out[j2*nx2+i2]);
	if( Xarea/min_area > AREA_RATIO_THRESH ) {
      	  xgrid_area[nxgrid] = Xarea;
	  i_in[nxgrid]    = i1;
	  j_in[nxgrid]    = j1;
	  i_out[nxgrid]   = i2;
	  j_out[nxgrid]   = j2;
	  ++nxgrid;
	  if(nxgrid > MAXXGRID) error_handler("nxgrid is greater than MAXXGRID, increase MAXXGRID");
	}
      }
    }
  }

  free(area_in);
  free(area_out);
  
  return nxgrid;
  
}; /* create_xgrid_1dx2d_order1 */


/********************************************************************************
  void create_xgrid_1dx2d_order2
  This routine generate exchange grids between two grids for the second order
  conservative interpolation. nlon_in,nlat_in,nlon_out,nlat_out are the size of the grid cell
  and lon_in,lat_in are 1-D grid bounds, lon_out,lat_out are geographic grid location of grid cell bounds.
********************************************************************************/
int create_xgrid_1dx2d_order2_(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out,
			       const double *lon_in, const double *lat_in, const double *lon_out, const double *lat_out,
			       const double *mask_in, int *i_in, int *j_in, int *i_out, int *j_out,
			       double *xgrid_area, double *xgrid_clon, double *xgrid_clat)
{
  int nxgrid;
  nxgrid = create_xgrid_1dx2d_order2(nlon_in, nlat_in, nlon_out, nlat_out, lon_in, lat_in, lon_out, lat_out, mask_in, i_in,
                                     j_in, i_out, j_out, xgrid_area, xgrid_clon, xgrid_clat);
  return nxgrid;

};
int create_xgrid_1dx2d_order2(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out,
			      const double *lon_in, const double *lat_in, const double *lon_out, const double *lat_out,
			      const double *mask_in, int *i_in, int *j_in, int *i_out, int *j_out,
			      double *xgrid_area, double *xgrid_clon, double *xgrid_clat)
{

  int nx1, ny1, nx2, ny2, nx1p, nx2p;
  int i1, j1, i2, j2, nxgrid, n;
  double ll_lon, ll_lat, ur_lon, ur_lat, x_in[MV], y_in[MV], x_out[MV], y_out[MV];
  double *area_in, *area_out, min_area;
  double *tmpx, *tmpy;
  
  nx1 = *nlon_in;
  ny1 = *nlat_in;
  nx2 = *nlon_out;
  ny2 = *nlat_out;

  nxgrid = 0;
  nx1p = nx1 + 1;
  nx2p = nx2 + 1;

  area_in      = (double *)malloc(nx1*ny1*sizeof(double));
  area_out     = (double *)malloc(nx2*ny2*sizeof(double));
  tmpx = (double *)malloc((nx1+1)*(ny1+1)*sizeof(double));
  tmpy = (double *)malloc((nx1+1)*(ny1+1)*sizeof(double));
  for(j1=0; j1<=ny1; j1++) for(i1=0; i1<=nx1; i1++) {
    tmpx[j1*nx1p+i1] = lon_in[i1];
    tmpy[j1*nx1p+i1] = lat_in[j1];
  }
  get_grid_area(nlon_in, nlat_in, tmpx, tmpy, area_in);     
  get_grid_area(nlon_out, nlat_out, lon_out, lat_out, area_out);  
  free(tmpx);
  free(tmpy);    
  
  for(j1=0; j1<ny1; j1++) for(i1=0; i1<nx1; i1++) if( mask_in[j1*nx1+i1] > MASK_THRESH ) {

    ll_lon = lon_in[i1];   ll_lat = lat_in[j1];
    ur_lon = lon_in[i1+1]; ur_lat = lat_in[j1+1];
    for(j2=0; j2<ny2; j2++) for(i2=0; i2<nx2; i2++) {
      int n_in, n_out;
      double xarea, lon_in_avg;
      
      y_in[0] = lat_out[j2*nx2p+i2];
      y_in[1] = lat_out[j2*nx2p+i2+1];
      y_in[2] = lat_out[(j2+1)*nx2p+i2+1];
      y_in[3] = lat_out[(j2+1)*nx2p+i2];
      if (  (y_in[0]<=ll_lat) && (y_in[1]<=ll_lat)
	    && (y_in[2]<=ll_lat) && (y_in[3]<=ll_lat) ) continue;
      if (  (y_in[0]>=ur_lat) && (y_in[1]>=ur_lat)
	    && (y_in[2]>=ur_lat) && (y_in[3]>=ur_lat) ) continue;

      x_in[0] = lon_out[j2*nx2p+i2];
      x_in[1] = lon_out[j2*nx2p+i2+1];
      x_in[2] = lon_out[(j2+1)*nx2p+i2+1];
      x_in[3] = lon_out[(j2+1)*nx2p+i2];
      n_in = fix_lon(x_in, y_in, 4, (ll_lon+ur_lon)/2);
      lon_in_avg = avgval_double(n_in, x_in);
      
      if (  (n_out = clip ( x_in, y_in, n_in, ll_lon, ll_lat, ur_lon, ur_lat, x_out, y_out )) > 0 ) {
	xarea = poly_area (x_out, y_out, n_out ) * mask_in[j1*nx1+i1];	
        min_area = min(area_in[j1*nx1+i1], area_out[j2*nx2+i2]);
	if(xarea/min_area > AREA_RATIO_THRESH ) {	  
	  xgrid_area[nxgrid] = xarea;
	  xgrid_clon[nxgrid] = poly_ctrlon(x_out, y_out, n_out, lon_in_avg);
	  xgrid_clat[nxgrid] = poly_ctrlat (x_out, y_out, n_out );
	  i_in[nxgrid]    = i1;
	  j_in[nxgrid]    = j1;
	  i_out[nxgrid]   = i2;
	  j_out[nxgrid]   = j2;
	  ++nxgrid;
	  if(nxgrid > MAXXGRID) error_handler("nxgrid is greater than MAXXGRID, increase MAXXGRID");
	}
      }
    }
  }
  free(area_in);
  free(area_out);
  
  return nxgrid;
  
}; /* create_xgrid_1dx2d_order2 */

/*******************************************************************************
  void create_xgrid_2dx1d_order1
  This routine generate exchange grids between two grids for the first order
  conservative interpolation. nlon_in,nlat_in,nlon_out,nlat_out are the size of the grid cell
  and lon_out,lat_out are 1-D grid bounds, lon_in,lat_in are geographic grid location of grid cell bounds.
  mask is on grid lon_in/lat_in. 
*******************************************************************************/
int create_xgrid_2dx1d_order1_(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out,
			       const double *lon_in, const double *lat_in, const double *lon_out, const double *lat_out,
			       const double *mask_in, int *i_in, int *j_in, int *i_out,
			       int *j_out, double *xgrid_area)
{
  int nxgrid;
  
  nxgrid = create_xgrid_2dx1d_order1(nlon_in, nlat_in, nlon_out, nlat_out, lon_in, lat_in, lon_out, lat_out, mask_in,
			       i_in, j_in, i_out, j_out, xgrid_area);
  return nxgrid;
    
};  
int create_xgrid_2dx1d_order1(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out, const double *lon_in,
			      const double *lat_in, const double *lon_out, const double *lat_out,
			      const double *mask_in, int *i_in, int *j_in, int *i_out,
			      int *j_out, double *xgrid_area)
{

  int nx1, ny1, nx2, ny2, nx1p, nx2p;
  int i1, j1, i2, j2, nxgrid;
  double ll_lon, ll_lat, ur_lon, ur_lat, x_in[MV], y_in[MV], x_out[MV], y_out[MV];
  double *area_in, *area_out, min_area;
  double *tmpx, *tmpy;
  
  nx1 = *nlon_in;
  ny1 = *nlat_in;
  nx2 = *nlon_out;
  ny2 = *nlat_out;

  nxgrid = 0;
  nx1p = nx1 + 1;
  nx2p = nx2 + 1;
  area_in = (double *)malloc(nx1*ny1*sizeof(double));
  area_out = (double *)malloc(nx2*ny2*sizeof(double));
  tmpx = (double *)malloc((nx2+1)*(ny2+1)*sizeof(double));
  tmpy = (double *)malloc((nx2+1)*(ny2+1)*sizeof(double));
  for(j2=0; j2<=ny2; j2++) for(i2=0; i2<=nx2; i2++) {
    tmpx[j2*nx2p+i2] = lon_out[i2];
    tmpy[j2*nx2p+i2] = lat_out[j2];
  }
  get_grid_area(nlon_in, nlat_in, lon_in, lat_in, area_in);     
  get_grid_area(nlon_out, nlat_out, tmpx, tmpy, area_out);  

  free(tmpx);
  free(tmpy);
  
  for(j2=0; j2<ny2; j2++) for(i2=0; i2<nx2; i2++) {

    ll_lon = lon_out[i2];   ll_lat = lat_out[j2];
    ur_lon = lon_out[i2+1]; ur_lat = lat_out[j2+1];
    for(j1=0; j1<ny1; j1++) for(i1=0; i1<nx1; i1++) if( mask_in[j1*nx1+i1] > MASK_THRESH ) {
      int n_in, n_out;
      double Xarea;
      
      y_in[0] = lat_in[j1*nx1p+i1];
      y_in[1] = lat_in[j1*nx1p+i1+1];
      y_in[2] = lat_in[(j1+1)*nx1p+i1+1];
      y_in[3] = lat_in[(j1+1)*nx1p+i1];
      if (  (y_in[0]<=ll_lat) && (y_in[1]<=ll_lat)
	    && (y_in[2]<=ll_lat) && (y_in[3]<=ll_lat) ) continue;
      if (  (y_in[0]>=ur_lat) && (y_in[1]>=ur_lat)
	    && (y_in[2]>=ur_lat) && (y_in[3]>=ur_lat) ) continue;

      x_in[0] = lon_in[j1*nx1p+i1];
      x_in[1] = lon_in[j1*nx1p+i1+1];
      x_in[2] = lon_in[(j1+1)*nx1p+i1+1];
      x_in[3] = lon_in[(j1+1)*nx1p+i1];

      n_in = fix_lon(x_in, y_in, 4, (ll_lon+ur_lon)/2);
      
      if ( (n_out = clip ( x_in, y_in, n_in, ll_lon, ll_lat, ur_lon, ur_lat, x_out, y_out )) > 0 ) {
	Xarea = poly_area ( x_out, y_out, n_out ) * mask_in[j1*nx1+i1];
	min_area = min(area_in[j1*nx1+i1], area_out[j2*nx2+i2]);
	if( Xarea/min_area > AREA_RATIO_THRESH ) {
      	  xgrid_area[nxgrid] = Xarea;
	  i_in[nxgrid]    = i1;
	  j_in[nxgrid]    = j1;
	  i_out[nxgrid]   = i2;
	  j_out[nxgrid]   = j2;
	  ++nxgrid;
	  if(nxgrid > MAXXGRID) error_handler("nxgrid is greater than MAXXGRID, increase MAXXGRID");
	}
      }
    }
  }

  free(area_in);
  free(area_out);
  
  return nxgrid;
  
}; /* create_xgrid_2dx1d_order1 */


/********************************************************************************
  void create_xgrid_2dx1d_order2
  This routine generate exchange grids between two grids for the second order
  conservative interpolation. nlon_in,nlat_in,nlon_out,nlat_out are the size of the grid cell
  and lon_out,lat_out are 1-D grid bounds, lon_in,lat_in are geographic grid location of grid cell bounds.
  mask is on grid lon_in/lat_in. 
********************************************************************************/
int create_xgrid_2dx1d_order2_(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out,
			       const double *lon_in, const double *lat_in, const double *lon_out, const double *lat_out,
			       const double *mask_in, int *i_in, int *j_in, int *i_out, int *j_out,
			       double *xgrid_area, double *xgrid_clon, double *xgrid_clat)
{
  int nxgrid;
  nxgrid = create_xgrid_2dx1d_order2(nlon_in, nlat_in, nlon_out, nlat_out, lon_in, lat_in, lon_out, lat_out, mask_in, i_in,
                                     j_in, i_out, j_out, xgrid_area, xgrid_clon, xgrid_clat);
  return nxgrid;

};

int create_xgrid_2dx1d_order2(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out,
			      const double *lon_in, const double *lat_in, const double *lon_out, const double *lat_out,
			      const double *mask_in, int *i_in, int *j_in, int *i_out, int *j_out,
			      double *xgrid_area, double *xgrid_clon, double *xgrid_clat)
{

  int nx1, ny1, nx2, ny2, nx1p, nx2p;
  int i1, j1, i2, j2, nxgrid, n;
  double ll_lon, ll_lat, ur_lon, ur_lat, x_in[MV], y_in[MV], x_out[MV], y_out[MV];
  double *tmpx, *tmpy;
  double *area_in, *area_out, min_area;
  double  lon_in_avg;
  
  nx1 = *nlon_in;
  ny1 = *nlat_in;
  nx2 = *nlon_out;
  ny2 = *nlat_out;

  nxgrid = 0;
  nx1p = nx1 + 1;
  nx2p = nx2 + 1;

  area_in      = (double *)malloc(nx1*ny1*sizeof(double));
  area_out     = (double *)malloc(nx2*ny2*sizeof(double));
  tmpx = (double *)malloc((nx2+1)*(ny2+1)*sizeof(double));
  tmpy = (double *)malloc((nx2+1)*(ny2+1)*sizeof(double));
  for(j2=0; j2<=ny2; j2++) for(i2=0; i2<=nx2; i2++) {
    tmpx[j2*nx2p+i2] = lon_out[i2];
    tmpy[j2*nx2p+i2] = lat_out[j2];
  }
  get_grid_area(nlon_in, nlat_in, lon_in, lat_in, area_in);     
  get_grid_area(nlon_out, nlat_out, tmpx, tmpy, area_out);  

  free(tmpx);
  free(tmpy);  
  
  for(j2=0; j2<ny2; j2++) for(i2=0; i2<nx2; i2++) {

    ll_lon = lon_out[i2];   ll_lat = lat_out[j2];
    ur_lon = lon_out[i2+1]; ur_lat = lat_out[j2+1];
    for(j1=0; j1<ny1; j1++) for(i1=0; i1<nx1; i1++) if( mask_in[j1*nx1+i1] > MASK_THRESH ) {
      int n_in, n_out;
      double xarea;
      
      y_in[0] = lat_in[j1*nx1p+i1];
      y_in[1] = lat_in[j1*nx1p+i1+1];
      y_in[2] = lat_in[(j1+1)*nx1p+i1+1];
      y_in[3] = lat_in[(j1+1)*nx1p+i1];
      if (  (y_in[0]<=ll_lat) && (y_in[1]<=ll_lat)
	    && (y_in[2]<=ll_lat) && (y_in[3]<=ll_lat) ) continue;
      if (  (y_in[0]>=ur_lat) && (y_in[1]>=ur_lat)
	    && (y_in[2]>=ur_lat) && (y_in[3]>=ur_lat) ) continue;

      x_in[0] = lon_in[j1*nx1p+i1];
      x_in[1] = lon_in[j1*nx1p+i1+1];
      x_in[2] = lon_in[(j1+1)*nx1p+i1+1];
      x_in[3] = lon_in[(j1+1)*nx1p+i1];

      n_in = fix_lon(x_in, y_in, 4, (ll_lon+ur_lon)/2);
      lon_in_avg = avgval_double(n_in, x_in);
      
      if (  (n_out = clip ( x_in, y_in, n_in, ll_lon, ll_lat, ur_lon, ur_lat, x_out, y_out )) > 0 ) {
	xarea = poly_area (x_out, y_out, n_out ) * mask_in[j1*nx1+i1];	
	min_area = min(area_in[j1*nx1+i1], area_out[j2*nx2+i2]);
	if(xarea/min_area > AREA_RATIO_THRESH ) {	  
	  xgrid_area[nxgrid] = xarea;
	  xgrid_clon[nxgrid] = poly_ctrlon(x_out, y_out, n_out, lon_in_avg);
	  xgrid_clat[nxgrid] = poly_ctrlat (x_out, y_out, n_out );
	  i_in[nxgrid]  = i1;
	  j_in[nxgrid]  = j1;
	  i_out[nxgrid] = i2;
	  j_out[nxgrid] = j2;
	  ++nxgrid;
	  if(nxgrid > MAXXGRID) error_handler("nxgrid is greater than MAXXGRID, increase MAXXGRID");
	}
      }
    }
  }

  free(area_in);
  free(area_out);  
  
  return nxgrid;
  
}; /* create_xgrid_2dx1d_order2 */

/*******************************************************************************
  void create_xgrid_2DX2D_order1
  This routine generate exchange grids between two grids for the first order
  conservative interpolation. nlon_in,nlat_in,nlon_out,nlat_out are the size of the grid cell
  and lon_in,lat_in, lon_out,lat_out are geographic grid location of grid cell bounds.
  mask is on grid lon_in/lat_in.
*******************************************************************************/
#ifndef __AIX
int create_xgrid_2dx2d_order1_(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out,
			       const double *lon_in, const double *lat_in, const double *lon_out, const double *lat_out,
			       const double *mask_in, int *i_in, int *j_in, int *i_out,
			       int *j_out, double *xgrid_area)
{
  int nxgrid;
  
  nxgrid = create_xgrid_2dx2d_order1(nlon_in, nlat_in, nlon_out, nlat_out, lon_in, lat_in, lon_out, lat_out, mask_in,
			       i_in, j_in, i_out, j_out, xgrid_area);
  return nxgrid;
    
};  
#endif
int create_xgrid_2dx2d_order1(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out,
			      const double *lon_in, const double *lat_in, const double *lon_out, const double *lat_out,
			      const double *mask_in, int *i_in, int *j_in, int *i_out,
			      int *j_out, double *xgrid_area)
{

  int nx1, ny1, nx2, ny2, nx1p, nx2p, ny1p, ny2p, nxgrid, n1_in, n2_in;
  int n0, n1, n2, n3, i1, j1, i2, j2, l;
  double x1_in[MV], y1_in[MV], x2_in[MV], y2_in[MV], x_out[MV], y_out[MV];
  double lon_in_min, lon_out_min, lon_in_max, lon_out_max, lat_in_min, lat_in_max, lat_out_min, lat_out_max;
  double lon_in_avg;
  double *area_in, *area_out, min_area;  

  nx1 = *nlon_in;
  ny1 = *nlat_in;
  nx2 = *nlon_out;
  ny2 = *nlat_out;

  nxgrid = 0;
  nx1p = nx1 + 1;
  nx2p = nx2 + 1;

  area_in = (double *)malloc(nx1*ny1*sizeof(double));
  area_out = (double *)malloc(nx2*ny2*sizeof(double));
  get_grid_area(nlon_in, nlat_in, lon_in, lat_in, area_in);     
  get_grid_area(nlon_out, nlat_out, lon_out, lat_out, area_out);  

  for(j1=0; j1<ny1; j1++) for(i1=0; i1<nx1; i1++) if( mask_in[j1*nx1+i1] > MASK_THRESH ) {
    n0 = j1*nx1p+i1;       n1 = j1*nx1p+i1+1;
    n2 = (j1+1)*nx1p+i1+1; n3 = (j1+1)*nx1p+i1;      
    x1_in[0] = lon_in[n0]; y1_in[0] = lat_in[n0];
    x1_in[1] = lon_in[n1]; y1_in[1] = lat_in[n1];
    x1_in[2] = lon_in[n2]; y1_in[2] = lat_in[n2];
    x1_in[3] = lon_in[n3]; y1_in[3] = lat_in[n3];
    lat_in_min = minval_double(4, y1_in);
    lat_in_max = maxval_double(4, y1_in);
    n1_in = fix_lon(x1_in, y1_in, 4, M_PI);
    lon_in_min = minval_double(n1_in, x1_in);
    lon_in_max = maxval_double(n1_in, x1_in);
    lon_in_avg = avgval_double(n1_in, x1_in);
      
    for(j2=0; j2<ny2; j2++) for(i2=0; i2<nx2; i2++) {
      int n_in, n_out;
      double Xarea;
      n0 = j2*nx2p+i2; n1 = j2*nx2p+i2+1;
      n2 = (j2+1)*nx2p+i2+1; n3 = (j2+1)*nx2p+i2;
      x2_in[0] = lon_out[n0]; y2_in[0] = lat_out[n0];
      x2_in[1] = lon_out[n1]; y2_in[1] = lat_out[n1];
      x2_in[2] = lon_out[n2]; y2_in[2] = lat_out[n2];
      x2_in[3] = lon_out[n3]; y2_in[3] = lat_out[n3];

      lat_out_min = minval_double(4, y2_in);
      lat_out_max = maxval_double(4, y2_in);
      if(lat_out_min >= lat_in_max || lat_out_max <= lat_in_min ) continue;
      n2_in = fix_lon(x2_in, y2_in, 4, lon_in_avg);
      lon_out_min = minval_double(n2_in, x2_in);
      lon_out_max = maxval_double(n2_in, x2_in);    
      
      /* x2_in should in the same range as lon_in_in after lon_fix, so no need to
	 consider cyclic condition
      */
      if(lon_out_min >= lon_in_max || lon_out_max <= lon_in_min ) continue;

      if ( (n_out = clip_2dx2d ( x1_in, y1_in, n1_in, x2_in, y2_in, n2_in, x_out, y_out )) > 0) {
	Xarea = poly_area (x_out, y_out, n_out) * mask_in[j1*nx1+i1];
	min_area = min(area_in[j1*nx1+i1], area_out[j2*nx2+i2]);
	if( Xarea/min_area > AREA_RATIO_THRESH ) {
	  xgrid_area[nxgrid] = Xarea;
	  i_in[nxgrid]    = i1;
	  j_in[nxgrid]    = j1;
	  i_out[nxgrid]   = i2;
	  j_out[nxgrid]   = j2;
	  ++nxgrid;
	  if(nxgrid > MAXXGRID) error_handler("nxgrid is greater than MAXXGRID, increase MAXXGRID");
	}
      }
    }
  }

  free(area_in);
  free(area_out);
  return nxgrid;
  
};/* get_xgrid_2Dx2D_order1 */

/********************************************************************************
  void create_xgrid_2dx1d_order2
  This routine generate exchange grids between two grids for the second order
  conservative interpolation. nlon_in,nlat_in,nlon_out,nlat_out are the size of the grid cell
  and lon_in,lat_in, lon_out,lat_out are geographic grid location of grid cell bounds.
  mask is on grid lon_in/lat_in. 
********************************************************************************/
#ifndef __AIX
int create_xgrid_2dx2d_order2_(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out,
			       const double *lon_in, const double *lat_in, const double *lon_out, const double *lat_out,
			       const double *mask_in, int *i_in, int *j_in, int *i_out, int *j_out,
			       double *xgrid_area, double *xgrid_clon, double *xgrid_clat)
{
  int nxgrid;
  nxgrid = create_xgrid_2dx2d_order2(nlon_in, nlat_in, nlon_out, nlat_out, lon_in, lat_in, lon_out, lat_out, mask_in, i_in,
                                     j_in, i_out, j_out, xgrid_area, xgrid_clon, xgrid_clat);
  return nxgrid;

};
#endif
int create_xgrid_2dx2d_order2(const int *nlon_in, const int *nlat_in, const int *nlon_out, const int *nlat_out,
			      const double *lon_in, const double *lat_in, const double *lon_out, const double *lat_out,
			      const double *mask_in, int *i_in, int *j_in, int *i_out, int *j_out,
			      double *xgrid_area, double *xgrid_clon, double *xgrid_clat)
{

  int nx1, nx2, ny1, ny2, nx1p, nx2p, ny1p, ny2p, nxgrid, n1_in, n2_in;
  int n0, n1, n2, n3, i1, j1, i2, j2, l, n;
  double x1_in[MV], y1_in[MV], x2_in[MV], y2_in[MV], x_out[MV], y_out[MV];
  double lon_in_min, lon_out_min, lon_in_max, lon_out_max, lat_in_min, lat_in_max, lat_out_min, lat_out_max;
  double lon_in_avg, xctrlon, xctrlat;
  double *area_in, *area_out, min_area;
  
  nx1 = *nlon_in;
  ny1 = *nlat_in;
  nx2 = *nlon_out;
  ny2 = *nlat_out;  
  nxgrid = 0;
  nx1p = nx1 + 1;
  nx2p = nx2 + 1;
  ny1p = ny1 + 1;
  ny2p = ny2 + 1;

  area_in  = (double *)malloc(nx1*ny1*sizeof(double));
  area_out = (double *)malloc(nx2*ny2*sizeof(double));
  get_grid_area(nlon_in, nlat_in, lon_in, lat_in, area_in);     
  get_grid_area(nlon_out, nlat_out, lon_out, lat_out, area_out);
  
  for(j1=0; j1<ny1; j1++) for(i1=0; i1<nx1; i1++) if( mask_in[j1*nx1+i1] > MASK_THRESH ) {
    n0 = j1*nx1p+i1;       n1 = j1*nx1p+i1+1;
    n2 = (j1+1)*nx1p+i1+1; n3 = (j1+1)*nx1p+i1;      
    x1_in[0] = lon_in[n0]; y1_in[0] = lat_in[n0];
    x1_in[1] = lon_in[n1]; y1_in[1] = lat_in[n1];
    x1_in[2] = lon_in[n2]; y1_in[2] = lat_in[n2];
    x1_in[3] = lon_in[n3]; y1_in[3] = lat_in[n3];
    lat_in_min = minval_double(4, y1_in);
    lat_in_max = maxval_double(4, y1_in);
    n1_in = fix_lon(x1_in, y1_in, 4, M_PI);
    lon_in_min = minval_double(n1_in, x1_in);
    lon_in_max = maxval_double(n1_in, x1_in);
    lon_in_avg = avgval_double(n1_in, x1_in);
      
    for(j2=0; j2<ny2; j2++) for(i2=0; i2<nx2; i2++) {
      int n_in, n_out;
      double xarea;
      n0 = j2*nx2p+i2; n1 = j2*nx2p+i2+1;
      n2 = (j2+1)*nx2p+i2+1; n3 = (j2+1)*nx2p+i2;
      x2_in[0] = lon_out[n0]; y2_in[0] = lat_out[n0];
      x2_in[1] = lon_out[n1]; y2_in[1] = lat_out[n1];
      x2_in[2] = lon_out[n2]; y2_in[2] = lat_out[n2];
      x2_in[3] = lon_out[n3]; y2_in[3] = lat_out[n3];

      lat_out_min = minval_double(4, y2_in);
      lat_out_max = maxval_double(4, y2_in);
      if(lat_out_min >= lat_in_max || lat_out_max <= lat_in_min ) continue;
      n2_in = fix_lon(x2_in, y2_in, 4, lon_in_avg);
      lon_out_min = minval_double(n2_in, x2_in);
      lon_out_max = maxval_double(n2_in, x2_in);    

      /* x2_in should in the same range as x1_in after lon_fix, so no need to
	 consider cyclic condition
      */
      if(lon_out_min >= lon_in_max || lon_out_max <= lon_in_min ) continue;

      if (  (n_out = clip_2dx2d( x1_in, y1_in, n1_in, x2_in, y2_in, n2_in, x_out, y_out )) > 0) {
	xarea = poly_area (x_out, y_out, n_out ) * mask_in[j1*nx1+i1];	
	min_area = min(area_in[j1*nx1+i1], area_out[j2*nx2+i2]);
	if( xarea/min_area > AREA_RATIO_THRESH ) {
	  xgrid_area[nxgrid] = xarea;
	  xgrid_clon[nxgrid] = poly_ctrlon(x_out, y_out, n_out, lon_in_avg);
	  xgrid_clat[nxgrid] = poly_ctrlat (x_out, y_out, n_out );
	  i_in[nxgrid]       = i1;
	  j_in[nxgrid]       = j1;
	  i_out[nxgrid]      = i2;
	  j_out[nxgrid]      = j2;
	  ++nxgrid;
	  if(nxgrid > MAXXGRID) error_handler("nxgrid is greater than MAXXGRID, increase MAXXGRID");
	}
      }
    }
  }

  free(area_in);
  free(area_out);  
  
  return nxgrid;
  
};/* get_xgrid_2Dx2D_order2 */

/*******************************************************************************
   Sutherland-Hodgeman algorithm sequentially clips parts outside 4 boundaries
*******************************************************************************/

int clip(const double lon_in[], const double lat_in[], int n_in, double ll_lon, double ll_lat,
	 double ur_lon, double ur_lat, double lon_out[], double lat_out[])
{
  double x_tmp[MV], y_tmp[MV], x_last, y_last;
  int i_in, i_out, n_out, inside_last, inside;

  /* clip polygon with LEFT boundary - clip V_IN to V_TMP */
  x_last = lon_in[n_in-1];
  y_last = lat_in[n_in-1];
  inside_last = (x_last >= ll_lon);
  for (i_in=0,i_out=0;i_in<n_in;i_in++) {
 
    /* if crossing LEFT boundary - output intersection */
    if ((inside=(lon_in[i_in] >= ll_lon))!=inside_last) {
      x_tmp[i_out] = ll_lon;
      y_tmp[i_out++] = y_last + (ll_lon - x_last) * (lat_in[i_in] - y_last) / (lon_in[i_in] - x_last);
    }

    /* if "to" point is right of LEFT boundary, output it */
    if (inside) {
      x_tmp[i_out]   = lon_in[i_in];
      y_tmp[i_out++] = lat_in[i_in];
    }
    x_last = lon_in[i_in];
    y_last = lat_in[i_in];
    inside_last = inside;
  }
  if (!(n_out=i_out)) return(0);

  /* clip polygon with RIGHT boundary - clip V_TMP to V_OUT */
  x_last = x_tmp[n_out-1];
  y_last = y_tmp[n_out-1];
  inside_last = (x_last <= ur_lon);
  for (i_in=0,i_out=0;i_in<n_out;i_in++) {
 
    /* if crossing RIGHT boundary - output intersection */
    if ((inside=(x_tmp[i_in] <= ur_lon))!=inside_last) {
      lon_out[i_out]   = ur_lon;
      lat_out[i_out++] = y_last + (ur_lon - x_last) * (y_tmp[i_in] - y_last)
                                                 / (x_tmp[i_in] - x_last);
    }

    /* if "to" point is left of RIGHT boundary, output it */
    if (inside) {
      lon_out[i_out]   = x_tmp[i_in];
      lat_out[i_out++] = y_tmp[i_in];
    }
    
    x_last = x_tmp[i_in];
    y_last = y_tmp[i_in];
    inside_last = inside;
  }
  if (!(n_out=i_out)) return(0);

  /* clip polygon with BOTTOM boundary - clip V_OUT to V_TMP */
  x_last = lon_out[n_out-1];
  y_last = lat_out[n_out-1];
  inside_last = (y_last >= ll_lat);
  for (i_in=0,i_out=0;i_in<n_out;i_in++) {
 
    /* if crossing BOTTOM boundary - output intersection */
    if ((inside=(lat_out[i_in] >= ll_lat))!=inside_last) {
      y_tmp[i_out]   = ll_lat;
      x_tmp[i_out++] = x_last + (ll_lat - y_last) * (lon_out[i_in] - x_last) / (lat_out[i_in] - y_last);
    }

    /* if "to" point is above BOTTOM boundary, output it */
    if (inside) {
      x_tmp[i_out]   = lon_out[i_in];
      y_tmp[i_out++] = lat_out[i_in];
    }
    x_last = lon_out[i_in];
    y_last = lat_out[i_in];
    inside_last = inside;
  }
  if (!(n_out=i_out)) return(0);

  /* clip polygon with TOP boundary - clip V_TMP to V_OUT */
  x_last = x_tmp[n_out-1];
  y_last = y_tmp[n_out-1];
  inside_last = (y_last <= ur_lat);
  for (i_in=0,i_out=0;i_in<n_out;i_in++) {
 
    /* if crossing TOP boundary - output intersection */
    if ((inside=(y_tmp[i_in] <= ur_lat))!=inside_last) {
      lat_out[i_out]   = ur_lat;
      lon_out[i_out++] = x_last + (ur_lat - y_last) * (x_tmp[i_in] - x_last) / (y_tmp[i_in] - y_last);
    }

    /* if "to" point is below TOP boundary, output it */
    if (inside) {
      lon_out[i_out]   = x_tmp[i_in];
      lat_out[i_out++] = y_tmp[i_in];
    }
    x_last = x_tmp[i_in];
    y_last = y_tmp[i_in];
    inside_last = inside;
  }
  return(i_out);
}; /* clip */  


/*******************************************************************************
   Revise Sutherland-Hodgeman algorithm to find the vertices of the overlapping
   between any two grid boxes. It return the number of vertices for the exchange grid.
*******************************************************************************/

int clip_2dx2d(const double lon1_in[], const double lat1_in[], int n1_in, 
	 const double lon2_in[], const double lat2_in[], int n2_in, 
	 double lon_out[], double lat_out[])
{
  double lon_tmp[MV], lat_tmp[MV];
  double x1_0, y1_0, x1_1, y1_1, x2_0, y2_0, x2_1, y2_1;
  double dx1, dy1, dx2, dy2, determ, ds1, ds2;
  int i_out, n_out, inside_last, inside, i1, i2;

  /* clip polygon with each boundary of the polygon */
  /* We treat lon1_in/lat1_in as clip polygon and lon2_in/lat2_in as subject polygon */
  n_out = n1_in;
  for(i1=0; i1<n1_in; i1++) {
    lon_tmp[i1] = lon1_in[i1];
    lat_tmp[i1] = lat1_in[i1];
  }
  x2_0 = lon2_in[n2_in-1];
  y2_0 = lat2_in[n2_in-1];
  for(i2=0; i2<n2_in; i2++) {
    x2_1 = lon2_in[i2];
    y2_1 = lat2_in[i2];
    x1_0 = lon_tmp[n_out-1];
    y1_0 = lat_tmp[n_out-1];
    inside_last = inside_edge( x2_0, y2_0, x2_1, y2_1, x1_0, y1_0);
    for(i1=0, i_out=0; i1<n_out; i1++) {
      x1_1 = lon_tmp[i1];
      y1_1 = lat_tmp[i1];
      if((inside = inside_edge(x2_0, y2_0, x2_1, y2_1, x1_1, y1_1)) != inside_last ) {
        /* there is intersection, the line between <x1_0,y1_0> and  <x1_1,y1_1>
           should not parallel to the line between <x2_0,y2_0> and  <x2_1,y2_1>
           may need to consider truncation error */
	dy1 = y1_1-y1_0;
	dy2 = y2_1-y2_0;
	dx1 = x1_1-x1_0;
	dx2 = x2_1-x2_0;
	ds1 = y1_0*x1_1 - y1_1*x1_0;
	ds2 = y2_0*x2_1 - y2_1*x2_0;
	determ = dy2*dx1 - dy1*dx2;
        if(fabs(determ) < EPSLN) {
	  error_handler("the line between <x1_0,y1_0> and  <x1_1,y1_1> should not parallel to "
				     "the line between <x2_0,y2_0> and  <x2_1,y2_1>");
	}
	lon_out[i_out]   = (dx2*ds1 - dx1*ds2)/determ;
	lat_out[i_out++] = (dy2*ds1 - dy1*ds2)/determ;
	

      }
      if(inside) {
	lon_out[i_out]   = x1_1;
	lat_out[i_out++] = y1_1;	
      }
      x1_0 = x1_1;
      y1_0 = y1_1;
      inside_last = inside;
    }
    if(!(n_out=i_out)) return 0;
    for(i1=0; i1<n_out; i1++) {
      lon_tmp[i1] = lon_out[i1];
      lat_tmp[i1] = lat_out[i1];
    }
    /* shift the starting point */
    x2_0 = x2_1;
    y2_0 = y2_1;
  }
  return(n_out);
}; /* clip */
    

/*------------------------------------------------------------------------------
  double poly_ctrlat(const double x[], const double y[], int n)
  This routine is used to calculate the latitude of the centroid 
   ---------------------------------------------------------------------------*/

double poly_ctrlat(const double x[], const double y[], int n)
{
  double ctrlat = 0.0;
  int    i;

  for (i=0;i<n;i++) {
    int ip = (i+1) % n;
    double dx = (x[ip]-x[i]);
    double dy, avg_y, hdy;
    double lat1, lat2;
    lat1 = y[ip];
    lat2 = y[i];
    dy = lat2 - lat1;
    hdy = dy*0.5;
    avg_y = (lat1+lat2)*0.5;
    if      (dx==0.0) continue;
    if(dx > M_PI)  dx = dx - 2.0*M_PI;
    if(dx < -M_PI) dx = dx + 2.0*M_PI;

    if ( fabs(hdy)< SMALL_VALUE ) /* cheap area calculation along latitude */
      ctrlat -= dx*(2*cos(avg_y) + lat2*sin(avg_y) - cos(lat1) );
    else 
      ctrlat -= dx*( (sin(hdy)/hdy)*(2*cos(avg_y) + lat2*sin(avg_y)) - cos(lat1) );
  }
  return (ctrlat*RADIUS*RADIUS);
}; /* poly_ctrlat */        

/*------------------------------------------------------------------------------
  double poly_ctrlon(const double x[], const double y[], int n, double clon)
  This routine is used to calculate the lontitude of the centroid.
   ---------------------------------------------------------------------------*/
double poly_ctrlon(const double x[], const double y[], int n, double clon)
{
  double ctrlon = 0.0;
  int    i;

  clon = clon;
  for (i=0;i<n;i++) {
    int ip = (i+1) % n;
    double phi1, phi2, dphi, lat1, lat2, dphi1, dphi2;
    double f1, f2, fac, fint;
    phi1   = x[ip];
    phi2   = x[i];
    lat1 = y[ip];
    lat2 = y[i];    
    dphi   = phi1 - phi2;
    
    if      (dphi==0.0) continue;

    f1 = 0.5*(cos(lat1)*sin(lat1)+lat1);
    f2 = 0.5*(cos(lat2)*sin(lat2)+lat2);

     /* this will make sure longitude of centroid is at 
        the same interval as the center of any grid */  
    if(dphi > M_PI)  dphi = dphi - 2.0*M_PI;
    if(dphi < -M_PI) dphi = dphi + 2.0*M_PI;
    dphi1 = phi1 - clon;
    if( dphi1 > M_PI) dphi1 -= 2.0*M_PI;
    if( dphi1 <-M_PI) dphi1 += 2.0*M_PI;
    dphi2 = phi2 -clon;
    if( dphi2 > M_PI) dphi2 -= 2.0*M_PI;
    if( dphi2 <-M_PI) dphi2 += 2.0*M_PI;    

    if(abs(dphi2 -dphi1) < M_PI) {
      ctrlon -= dphi * (dphi1*f1+dphi2*f2)/2.0;
    }
    else {
      if(dphi1 > 0.0)
	fac = M_PI;
      else
	fac = -M_PI;
      fint = f1 + (f2-f1)*(fac-dphi1)/abs(dphi);
      ctrlon -= 0.5*dphi1*(dphi1-fac)*f1 - 0.5*dphi2*(dphi2+fac)*f2
	+ 0.5*fac*(dphi1+dphi2)*fint;
	}
    
  }
  return (ctrlon*RADIUS*RADIUS);
};   /* poly_ctrlon */

/* -----------------------------------------------------------------------------
   double box_ctrlat(double ll_lon, double ll_lat, double ur_lon, double ur_lat)
   This routine is used to calculate the latitude of the centroid.
   ---------------------------------------------------------------------------*/
double box_ctrlat(double ll_lon, double ll_lat, double ur_lon, double ur_lat)
{
  double dphi = ur_lon-ll_lon;
  double ctrlat;
  
  if(dphi > M_PI)  dphi = dphi - 2.0*M_PI;
  if(dphi < -M_PI) dphi = dphi + 2.0*M_PI;
  ctrlat = dphi*(cos(ur_lat) + ur_lat*sin(ur_lat)-(cos(ll_lat) + ll_lat*sin(ll_lat)));
  return (ctrlat*RADIUS*RADIUS); 
}; /* box_ctrlat */

/*------------------------------------------------------------------------------
  double box_ctrlon(double ll_lon, double ll_lat, double ur_lon, double ur_lat, double clon)
  This routine is used to calculate the lontitude of the centroid 
   ----------------------------------------------------------------------------*/
double box_ctrlon(double ll_lon, double ll_lat, double ur_lon, double ur_lat, double clon)
{
  double phi1, phi2, dphi, lat1, lat2, dphi1, dphi2;
  double f1, f2, fac, fint;  
  double ctrlon  = 0.0;
  int i;
  clon = clon;  
  for( i =0; i<2; i++) {
    if(i == 0) {
      phi1 = ur_lon;
      phi2 = ll_lon;
      lat1 = lat2 = ll_lat;
    }
    else {
      phi1 = ll_lon;
      phi2 = ur_lon;
      lat1 = lat2 = ur_lat;
    }
    dphi   = phi1 - phi2;
    f1 = 0.5*(cos(lat1)*sin(lat1)+lat1);
    f2 = 0.5*(cos(lat2)*sin(lat2)+lat2);

    if(dphi > M_PI)  dphi = dphi - 2.0*M_PI;
    if(dphi < -M_PI) dphi = dphi + 2.0*M_PI;
    /* make sure the center is in the same grid box. */
    dphi1 = phi1 - clon;
    if( dphi1 > M_PI) dphi1 -= 2.0*M_PI;
    if( dphi1 <-M_PI) dphi1 += 2.0*M_PI;
    dphi2 = phi2 -clon;
    if( dphi2 > M_PI) dphi2 -= 2.0*M_PI;
    if( dphi2 <-M_PI) dphi2 += 2.0*M_PI;    

    if(abs(dphi2 -dphi1) < M_PI) {
      ctrlon -= dphi * (dphi1*f1+dphi2*f2)/2.0;
    }
    else {
      if(dphi1 > 0.0)
	fac = M_PI;
      else
	fac = -M_PI;
      fint = f1 + (f2-f1)*(fac-dphi1)/abs(dphi);
      ctrlon -= 0.5*dphi1*(dphi1-fac)*f1 - 0.5*dphi2*(dphi2+fac)*f2
	+ 0.5*fac*(dphi1+dphi2)*fint;
    }
  }
  return (ctrlon*RADIUS*RADIUS);    
} /* box_ctrlon */

/*******************************************************************************
  double grid_box_radius(double *x, double *y, double *z, int n);
  Find the radius of the grid box, the radius is defined the
  maximum distance between any two vertices
*******************************************************************************/ 
double grid_box_radius(const double *x, const double *y, const double *z, int n)
{
  double radius;
  int i, j;
  
  radius = 0;
  for(i=0; i<n-1; i++) {
    for(j=i+1; j<n; j++) {
      radius = max(radius, pow(x[i]-x[j],2.)+pow(y[i]-y[j],2.)+pow(z[i]-z[j],2.));
    }
  }
  
  radius = sqrt(radius);

  return (radius);
  
}; /* grid_box_radius */

/*******************************************************************************
  double dist_between_boxes(const double *x1, const double *y1, const double *z1, int n1,
			    const double *x2, const double *y2, const double *z2, int n2);
  Find the distance between any two grid boxes. The distance is defined by the maximum
  distance between any vertices of these two box
*******************************************************************************/
double dist_between_boxes(const double *x1, const double *y1, const double *z1, int n1,
			  const double *x2, const double *y2, const double *z2, int n2)
{
  double dist;
  int i, j;

  for(i=0; i<n1; i++) {
    for(j=0; j<n2; j++) {   
      dist = max(dist, pow(x1[i]-x2[j],2.)+pow(y1[i]-y2[j],2.)+pow(z1[i]-z2[j],2.));
    }
  }

  dist = sqrt(dist);
  return (dist);

}; /* dist_between_boxes */

/*******************************************************************************
 int inside_edge(double x0, double y0, double x1, double y1, double x, double y)
 determine a point(x,y) is inside or outside a given edge with vertex,
 (x0,y0) and (x1,y1). return 1 if inside and 0 if outside. <y1-y0, -(x1-x0)> is
 the outward edge normal from vertex <x0,y0> to <x1,y1>. <x-x0,y-y0> is the vector
 from <x0,y0> to <x,y>. 
 if Inner produce <x-x0,y-y0>*<y1-y0, -(x1-x0)> > 0, outside, otherwise inside.
 inner product value = 0 also treate as inside.
*******************************************************************************/
 int inside_edge(double x0, double y0, double x1, double y1, double x, double y)
 {
   const double SMALL = 1.e-12;
   double product;
   
   product = ( x-x0 )*(y1-y0) + (x0-x1)*(y-y0);
   return (product<=SMALL) ? 1:0;
   
 }; /* inside_edge */

