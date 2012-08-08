

#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <math.h> 
#include "constant.h" 
#include "mosaic_util.h" 
#include "tool_util.h"
#include "interp.h"
#include "mpp.h"
#include "mpp_domain.h"
#include "mpp_io.h"
#include "conserve_interp.h"
#include "bilinear_interp.h"
#include "fregrid_util.h"

#define  D2R (M_PI/180.)
#define  R2D (180./M_PI)

const double SMALL = 1.0e-4;
double distant(double a, double b, double met1, double met2);
double bp_lam(double x, double y, double bpeq, double rp);
double bp_phi(double x, double y, double bpsp, double bpnp);
double lon_in_range(double lon, double lon_strt);
void vtx_insert(double *x, double *y, int *n, int n_ins);
void vtx_delete(double *x, double *y, int *n, int n_del);
int lon_fix(double *x, double *y, int n_in, double tlon);

/***************************************************************************
  void get_file_path(const char *file, char *dir)
  get the directory where file is located. The dir will be the complate path
  before the last "/". If no "/" exist in file, the path will be current ".".
***************************************************************************/
void get_file_path(const char *file, char *dir)
{
  int len;
  char *strptr = NULL;

  /* get the diretory */
 
  strptr = strrchr(file, '/');
  if(strptr) {
    len = strptr - file;
    strncpy(dir, file, len);
  }
  else {
    len = 1;
    strcpy(dir, ".");
  }
  dir[len] = 0;

}; /* get_file_path */

int get_int_entry(char *line, int* value)
{
  char* pch;
  int num;
  
  pch = strtok(line, ", ");
  num = 0;
  while( pch != NULL) {
    value[num++] = atoi(pch);
    pch = strtok(NULL, ", ");
  }
  return num;
    
};

int get_double_entry(char *line, double *value)
{
  char* pch;
  int num;
  
  pch = strtok(line, ", ");
  num = 0;
  while( pch != NULL) {
    value[num++] = atof(pch);
    pch = strtok(NULL, ", ");
  }
  return num;
};

/*********************************************************************
  double spherical_dist(double x1, double y1, double x2, double y2)
  return distance between spherical grid on the earth
*********************************************************************/

double spherical_dist(double x1, double y1, double x2, double y2)
{
  double dist = 0.0;
  double h1, h2;
  
  if(x1 == x2) {
    h1 = RADIUS;
    h2 = RADIUS;
    dist = distant(y1,y2,h1,h2);
  }
  else if(y1 == y2) {
    h1 = RADIUS * cos(y1*D2R);
    h2 = RADIUS * cos(y2*D2R);
    dist = distant(x1,x2,h1,h2);
  }
  else 
    mpp_error("tool_till: This is not rectangular grid");

  return dist;
}; /* spherical_dist */
  

/*********************************************************************
  void double bipolar_dist(double x1, double y1, double x2, double y2)
  return distance of bipolar grids
*********************************************************************/
double bipolar_dist(double x1, double y1, double x2, double y2,
		    double bpeq, double bpsp, double bpnp, double rp )
{
  double dist, x[2],y[2], bp_lon[2], bp_lat[2], metric[2];
  double h1[2], h2[2], chic;
  int n;
  
  x[0] = x1;  x[1] = x2;
  y[0] = y1;  y[1] = y2;
  
  /*--- get the bipolar grid and metric term ----------------------------*/
  for(n=0; n<2; n++){
    bp_lon[n] = bp_lam(x[n],y[n],bpeq, rp);     /* longitude (degrees) in bipolar grid system */
    bp_lat[n] = bp_phi(x[n],y[n],bpsp, bpnp);  /* latitude (degrees) in bipolar grid system */
    h1[n]     = RADIUS*cos(bp_lat[n]*D2R);
    h2[n]     = RADIUS;
    metric[n] = 1.0;
    if (fabs(y[n]-90.0) < SMALL || fabs(bp_lon[n]*D2R) >= SMALL
	|| fabs(bp_lat[n]*D2R) >= SMALL) {
      chic = acos(cos(bp_lon[n]*D2R)*cos(bp_lat[n]*D2R));            /* eqn. 6 */
      metric[n] = rp*(1/pow(cos(chic/2),2))/(1+(pow(rp,2))*(pow(tan(chic/2),2)));/* eq 3 */
    }
  }

  /*--- then calculate the distance -------------------------------------*/
  if(x1 == x2) 
    dist = distant(bp_lon[0],bp_lon[1],metric[0]*h1[0],metric[1]*h1[1]);
  else if(y1 == y2) 
    dist = distant(bp_lat[0],bp_lat[1],metric[0]*h2[0],metric[1]*h2[1]);
  else
    mpp_error("tool_util: This tripolar grid not transformed from rectangular grid");    

  return dist;
  
}; /* bipolar_dist */

/*********************************************************************
  double distant(double a, double b, double met1, double met2)
  return distant on the earth
*********************************************************************/
double distant(double a, double b, double met1, double met2)
{
   return fabs(a-b)*D2R*(met1+met2)/2. ;
}; /* distant */

/*********************************************************************
   double spherical_area(double x1, double y1, double x2, double y2,
                   double x3, double y3, double x4, double y4 )            
   rectangular grid box area
 ********************************************************************/
double spherical_area(double x1, double y1, double x2, double y2,
		      double x3, double y3, double x4, double y4 )
{
  double area, dx, lat1, lat2, x[4],y[4];
  int i, ip;
  
  x[0] = x1; y[0] = y1;
  x[1] = x2; y[1] = y2;
  x[2] = x3; y[2] = y3;
  x[3] = x4; y[3] = y4;

  area = 0.0;

  for(i=0; i<4; i++) {
    ip = i+1;
    if(ip ==4) ip = 0;
    dx = (x[ip] - x[i])*D2R;
    lat1 = y[ip]*D2R;
    lat2 = y[i]*D2R;
    if(dx==0.0) continue;
    if(dx > M_PI)  dx = dx - 2.0*M_PI;
    if(dx < -M_PI) dx = dx + 2.0*M_PI;

    if (lat1 == lat2) /* cheap area calculation along latitude  */
      area = area - dx*sin(lat1);
    else 
      area = area - dx*(sin(lat1)+sin(lat2))/2;   /*  TRAPEZOID_RULE */
  }

  area = area * RADIUS * RADIUS;

  return area;
}; /* spherical_area */

/*********************************************************************
   double bipolar_area(double x1, double y1, double x2, double y2,
                       double x3, double y3, double x4, double y4 )            
   bipolar grid  area
 ********************************************************************/
double bipolar_area(double x1, double y1, double x2, double y2,
			  double x3, double y3, double x4, double y4 )
{
  double area, dx, lat1, lat2, x[8],y[8];
  int i, ip, n;
  
  x[0] = x1; y[0] = y1;
  x[1] = x2; y[1] = y2;
  x[2] = x3; y[2] = y3;
  x[3] = x4; y[3] = y4;


  /*--- first fix the longitude at the pole -----------------------------*/
  n = lon_fix(x, y, 4, 180.);

  /*--- calculate the area ----------------------------------------------  */
  area = 0.0;  
  for(i=0; i<n; i++){
    ip = i+1;
    if(ip == n) ip = 0;
    dx   = (x[ip] - x[i])*D2R;
    lat1 = y[ip]*D2R;
    lat2 = y[i]*D2R;
    if(dx==0.0) continue;
    if(dx > M_PI)  dx = dx - 2.0*M_PI;
    if(dx < -M_PI) dx = dx + 2.0*M_PI;

    if (lat1 == lat2)  /* cheap area calculation along latitude */
      area = area - dx*sin(lat1);
    else
      area = area - dx*(sin(lat1)+sin(lat2))/2;   /*  TRAPEZOID_RULE */
  }
  
  area = area * RADIUS * RADIUS;

  return area;
}; /* bipolar_area */

/*********************************************************************
  double lat_dist(double x1, double x2)
  distance (in degrees) between points on lat. circle
 ********************************************************************/
  double lat_dist(double x1, double x2)
{
  return min(fmod(x1-x2+720,360.),fmod(x2-x1+720,360.));
};


/*********************************************************************
  double bp_lam(double x, double y, double bpeq)
  find bipolar grid longitude given geo. coordinates
 ********************************************************************/
  double bp_lam(double x, double y, double bpeq, double rp)
{
  double bp_lam;

  /*  bp_lam = ((90-y)/(90-lat_join))*90 */
  /* invert eqn. 5 with phic=0 to place point at specified geo. lat */
  bp_lam = 2.*atan(tan((0.5*M_PI-y*D2R)/2)/rp)*R2D;
  if (lat_dist(x,bpeq)<90.) bp_lam = -bp_lam;
  return bp_lam;
}; /* bp_lam */

/*********************************************************************
   double bp_phi(double x, double y, double bpsp, double bpnp)
   find bipolar grid latitude given geo. coordinates
 ********************************************************************/
   double bp_phi(double x, double y, double bpsp, double bpnp)
{
  if (lat_dist(x,bpsp)<90.)
    return (-90+lat_dist(x,bpsp));
  else
    return ( 90-lat_dist(x,bpnp));
}; /* bp_phi */


/*********************************************************************
  void tp_trans(double& lon, double& lat, double lon_ref)
  calculate tripolar grid
 ********************************************************************/
void tp_trans(double *lon, double *lat, double lon_ref, double lon_start, 
		    double lam0, double bpeq, double bpsp, double bpnp, double rp )
{
  double lamc, phic, lams, chic, phis;
  
  lamc = bp_lam(*lon, *lat, bpeq, rp )*D2R;
  phic = bp_phi(*lon, *lat, bpsp, bpnp)*D2R;

  if (fabs(*lat-90.) < SMALL) {
       if (phic > 0) 
	 *lon=lon_in_range(lon_start,lon_ref);
       else
	 *lon=lon_start+180.;
       chic = acos(cos(lamc)*cos(phic));                     /* eqn. 6 */
       phis = M_PI*0.5-2*atan(rp*tan(chic/2));                   /* eqn. 5 */
       *lat = phis*R2D;
       return;
  }

  if (fabs(lamc) < SMALL && fabs(phic) < SMALL) {
    *lat=90.;
    *lon=lon_ref;
  }
  else {
    lams = fmod(lam0+M_PI+M_PI/2-atan2(sin(lamc),tan(phic)),2*M_PI);  /* eqn. 5 */
    chic = acos(cos(lamc)*cos(phic));                          /* eqn. 6 */
    phis = M_PI*0.5-2*atan(rp*tan(chic/2));                        /* eqn. 5 */
    *lon = lams*R2D;
    *lon = lon_in_range(*lon,lon_ref); 
    *lat = phis*R2D;
  }
}; /* tp_trans */

/*********************************************************************
  double Lon_in_range(double lon, double lon_strt)
  Returns lon_strt <= longitude <= lon_strt+360
 ********************************************************************/
double lon_in_range(double lon, double lon_strt)
{
  double lon_in_range, lon_end;

  lon_in_range = lon;
  lon_end = lon_strt+360.;

  if (fabs(lon_in_range - lon_strt) < SMALL) 
    lon_in_range = lon_strt;
  else if (fabs(lon_in_range - lon_end) < SMALL)
    lon_in_range = lon_strt;
  else {
    while(1) {
      if (lon_in_range < lon_strt)          
	lon_in_range = lon_in_range +  360.;
      else if (lon_in_range  >  lon_end)
	lon_in_range  = lon_in_range - 360.;
      else
	break;
    }
  }
  return lon_in_range;
}; /* lon_in_range */


/*********************************************************************
   int lon_fix(double *x, double *y, int n_in, double tlon) 
   fix longitude at pole.
 ********************************************************************/
int lon_fix(double *x, double *y, int n_in, double tlon)
{
  int i, ip, im, n_out;
  double x_sum, dx;
  
  n_out = n_in;
  i     = 0;
  while( i < n_out) {
    if(fabs(y[i]) >= 90.-SMALL) {
      im = i - 1;
      if(im < 0) im = im + n_out;
      ip = i + 1;
      if(ip >= n_out) ip = ip - n_out;
      /*--- all pole points must be paired ---------------------------- */
      if(y[im] == y[i] && y[ip] == y[i] ) {
	vtx_delete(x,y, &n_out, i);
	i = i - 1;
      }
      else if(y[im] != y[i] && y[ip] != y[i] ) {
        vtx_insert(x,y,&n_out,i);
	i = i + 1;
      }
    }
    i = i + 1;
  }

  /*--- first of pole pair has longitude of previous vertex -------------
    --- second of pole pair has longitude of subsequent vertex ---------- */
  for(i=0;i<n_out;i++){
    if(fabs(y[i]) >= 90.-SMALL) {
      im= i - 1;
      if(im < 0) im = im + n_out;
      ip = i + 1;
      if(ip >= n_out) ip = ip - n_out;

      if(y[im] != y[i]) x[i] = x[im];
      if(y[ip] != y[i]) x[i] = x[ip];
    }
  }

  if(n_out == 0) return 0;

  x_sum = x[1];
  for(i=1;i< n_out;i++){
    dx = x[i] - x[i-1];
    if(dx < -180) 
      dx = dx + 360;
    else if (dx >  180)
      dx = dx - 360;

    x[i] = x[i-1] + dx;
    x_sum = x_sum + x[i];
  }

  dx = x_sum/(n_out) - tlon;
  if (dx < -180.) 
    for(i=0;i<n_out;i++) x[i] = x[i] + 360.;
  else if (dx > 180.)
    for(i=0;i<n_out;i++) x[i] = x[i] - 360.;

  return n_out;
  
}; /* lon_fix */


/*********************************************************************
   void vtx_delete(double *x, double *y, int *n, int n_del)
   delete vertex
 ********************************************************************/
void vtx_delete(double *x, double *y, int *n, int n_del)
{
  int i;

  for(i=n_del; i<=*n-2; i++)
    {
      x[i] = x[i+1];
      y[i] = y[i+1];
    }
  (*n)--;
}; /* vtx_delete */

/*********************************************************************
   void Vtx_insert(double *x, double *y, int *n, int n_del)
   insert vertex
 ********************************************************************/
void vtx_insert(double *x, double *y, int *n, int n_ins)
{
  int i;

  for(i=*n-1; i>=n_ins; i--){
    x[i+1] = x[i];
    y[i+1] = y[i];
  }
  (*n)++;

}; /* vtx_insert */


/*----------------------------------------------------------------------
    void vect_cross(e, p1, p2)
    Perform cross products of 3D vectors: e = P1 X P2
    -------------------------------------------------------------------*/
    
/********************************************************************************
  void compute_grid_bound(int nb, const couble *bnds, const int *npts, int *grid_size, const char *center_cell)
  compute the 1-D grid location.
********************************************************************************/
double* compute_grid_bound(int nb, const double *bnds, const int *npts, int *grid_size, const char *center)
{
  int    refine, i, n, np;
  double *grid=NULL, *tmp=NULL;
  double *grid1=NULL, *grid2=NULL;

  if(!strcmp(center, "none") )
    refine = 1;
  else if(!strcmp(center, "t_cell") || !strcmp(center, "c_cell") )
    refine = 2;
  else
    mpp_error("tool_util: center should be 'none', 'c_cell' or 't_cell' ");
	  
  grid1 = (double *)malloc(nb*sizeof(double));
  grid1[0] = 1;
  n = 0;
  for(i=1; i<nb; i++) {
    if(npts[i-1]%refine) mpp_error("tool_util: when center_cell is not 'none', npts should be divided by 2");
    n += npts[i-1]/refine;
    grid1[i] = n+1;
  }
  np = n + 1;
  *grid_size = n*refine;
  tmp   = (double *)malloc(np*sizeof(double));
  grid  = (double *)malloc((*grid_size+1)*sizeof(double));
  grid2 = (double *)malloc(np*sizeof(double));
  for(i=0;i<np;i++) grid2[i] = i + 1.0;

  cubic_spline( nb, np, grid1, grid2, bnds, tmp, 1e30, 1e30);
  if(!strcmp(center, "none")) {
    for(i=0; i<np; i++) grid[i] = tmp[i];
  }
  else if(!strcmp(center, "t_cell")) {
    for(i=0; i<np; i++) grid[2*i] = tmp[i];
    for(i=0; i<n;  i++) grid[2*i+1] = 0.5*(tmp[i]+tmp[i+1]);
  }
  else if( !strcmp(center, "c_cell")) {
    for(i=0; i<np; i++) grid[2*i] = tmp[i];
    grid[1] = 0.5*(tmp[0]+tmp[1]);
    for(i=1; i<n;  i++) grid[2*i+1] = 2*grid[2*i] - grid[2*i-1];
  }
    
  free(grid1);
  free(grid2);
  free(tmp);  

  return grid;
  
};/* compute_grid_bound */

double* get_subregion(int ni, double *data, int is, int ie, int js, int je)
{
  int i, j, pos;
  double *ldata;
  
  ldata = (double *)malloc((ie-is+1)*(je-js+1)*sizeof(double));
  pos = 0;
  for(j=js; j<=je; j++)
    for(i=is; i<=ie; i++) ldata[pos++] = data[j*ni+i];

  return ldata;
  
}; /* get_subregion */

#define MAXBOUNDS 100
#define STRINGLEN 255
#define GRID_VERSION "0.2"
#define TAGNAME "$Name:  $"

int
gs_make_hgrid(char *grid_type, int *nlat, int *nlon, 
	      int nxbnds0, int nybnds0, int nxbnds1, int nybnds1, 
	      int nxbnds2, int nybnds2, double lat_join, int nratio, 
	      double simple_dx, double simple_dy, int ntilex, int ntiley,
              char *gridname, char *center, char *history, double *xbnds, 
	      double *ybnds)
{
  
   int  ndivx[] = {1,1,1,1,1,1};
   int  ndivy[] = {1,1,1,1,1,1};
   char method[32] = "conformal";
   char orientation[32] = "center_pole";
   int  nxbnds=2, nybnds=2;
   char my_grid_file[MAXBOUNDS][STRINGLEN];
   int nx, ny, nxp, nyp, ntiles=1, ntiles_file;
   double *x=NULL, *y=NULL, *dx=NULL, *dy=NULL, *angle_dx=NULL, *angle_dy=NULL, *area=NULL;
  
   char geometry[32] = "spherical";
   char projection[32] = "none";
   char arcx[32] = "small_circle";
   char north_pole_tile[32] = "0.0 90.0";
   char north_pole_arcx[32] = "0.0 90.0";
   char discretization[32]  = "logically_rectangular";
   char conformal[32]       = "true";
   char mesg[256], str[128];
   int isc, iec, jsc, jec, nxc, nyc, layout[2];
   domain2D domain;
   int n, errflg, c, i;  

   /* check the command-line arguments to make sure the value are suitable */
   if( strcmp(grid_type,"regular_lonlat_grid") ==0 ) {
      nxbnds = nxbnds0; nybnds = nybnds0;
      if( nxbnds <2 || nybnds < 2) mpp_error("make_hgrid: grid type is 'regular_lonlat_grid', "
      "both nxbnds and nybnds should be no less than 2");
      if( nxbnds != nxbnds1 || nxbnds != nxbnds2+1 )
	 mpp_error("make_hgrid: grid type is 'regular_lonlat_grid', nxbnds does"
	 "not match number of entry in xbnds or nlon");
      if( nybnds != nybnds1 || nybnds != nybnds2+1 )
	 mpp_error("make_hgrid: grid type is 'regular_lonlat_grid', nybnds does "
	 "not match number of entry in ybnds or nlat");
   }
   else if( strcmp(grid_type,"tripolar_grid") ==0 ) {
      strcpy(projection, "tripolar");
      nxbnds = nxbnds0; nybnds = nybnds0;
      if( nxbnds != 2) mpp_error("make_hgrid: grid type is 'tripolar_grid', nxbnds should be 2");
      if( nybnds < 2) mpp_error("make_hgrid: grid type is 'tripolar_grid', nybnds should be no less than 2");
      if( nxbnds != nxbnds1 || nxbnds != nxbnds2+1 )
	 mpp_error("make_hgrid: grid type is 'tripolar_grid', nxbnds does not match number of entry in xbnds or nlon");
      if( nybnds != nybnds1 || nybnds != nybnds2+1 )
	 mpp_error("make_hgrid: grid type is 'tripolar_grid', nybnds does not match number of entry in ybnds or nlat");
   }
   else if( strcmp(grid_type,"from_file") ==0 ) {
      /* For ascii file, nlon and nlat should be specified through --nlon, --nlat
	 For netcdf file, grid resolution will be read from grid file
      */
    
      if(ntiles_file == 0) mpp_error("make_hgrid: grid_type is 'from_file', but my_grid_file is not specified");
      ntiles = ntiles_file;
      for(n=0; n<ntiles; n++) {
	 if(strstr(my_grid_file[n],".nc") ) {
	    /* get the grid size for each tile, the grid is on model grid, should need to multiply by 2 */
	    int fid;
	    fid = mpp_open(my_grid_file[n], MPP_READ);
	    nlon[n] = mpp_get_dimlen(fid, "grid_xt")*2;
	    nlat[n] = mpp_get_dimlen(fid, "grid_yt")*2;
	    mpp_close(fid);
	 }
	 else {
	    if(nxbnds2 != ntiles || nybnds2 != ntiles ) mpp_error("make_hgrid: grid type is 'from_file', number entry entered "
	    "through --nlon and --nlat should be equal to number of files "
	    "specified through --my_grid_file");
	 }
      }
      /* for simplify purpose, currently we assume all the tile have the same grid size */
      for(n=1; n<ntiles; n++) {
	 if( nlon[n] != nlon[0] || nlat[n] != nlat[0])  mpp_error("make_hgrid: grid_type is from_file, all the tiles should "
	 "have same grid size, contact developer");
      }
   }
   else if( strcmp(grid_type,"simple_cartesian_grid") ==0) {
      strcpy(geometry, "planar");
      strcpy(north_pole_tile, "none");
      if(nxbnds1 != 2 || nybnds1 != 2 ) mpp_error("make_hgrid: grid type is 'simple_cartesian_grid', number entry entered "
      "through --xbnds and --ybnds should be 2");
      if(nxbnds2 != 1 || nybnds2 != 1 ) mpp_error("make_hgrid: grid type is 'simple_cartesian_grid', number entry entered "
      "through --nlon and --nlat should be 1");
      if(simple_dx == 0 || simple_dy == 0) mpp_error("make_hgrid: grid_type is 'simple_cartesian_grid', "
      "both simple_dx and simple_dy both should be specified");
   }
   else if ( strcmp(grid_type,"spectral_grid") ==0 ) {
      if(nxbnds2 != 1 || nybnds2 != 1 ) mpp_error("make_hgrid: grid type is 'spectral_grid', number entry entered "
      "through --nlon and --nlat should be 1");    
   }
   else if( strcmp(grid_type,"conformal_cubic_grid") ==0 ) {
      strcpy(projection, "cube_gnomonic");
      strcpy(conformal, "FALSE");
      if(nxbnds2 != 1 ) mpp_error("make_hgrid: grid type is 'conformal_cubic_grid', number entry entered "
      "through --nlon should be 1");
      if(nratio < 1) mpp_error("make_hgrid: grid type is 'conformal_cubic_grid', nratio should be a positive integer");
   }
   else if(  !strcmp(grid_type,"gnomonic_ed") ) {
      strcpy(projection, "cube_gnomonic");
      strcpy(conformal, "FALSE");
      if(nxbnds2 != 1 ) mpp_error("make_hgrid: grid type is 'gnomonic_cubic_grid', number entry entered "
      "through --nlon should be 1");
   }
   else {
      mpp_error("make_hgrid: only grid_type = 'regular_lonlat_grid', 'tripolar_grid', 'from_file', "
      "'gnomonic_ed', 'conformal_cubic_grid', 'simple_cartesian_grid' and "
      "'spectral_grid' is implemented");  
   }
  
   /* get super grid size */

   if( !strcmp(grid_type,"gnomonic_ed") || !strcmp(grid_type,"conformal_cubic_grid") ) {
      nx = nlon[0];
      ny = nx;
   }
   else {
      nx = 0;
      ny = 0;
      for(n=0; n<nxbnds-1; n++) nx += nlon[n];
      for(n=0; n<nybnds-1; n++) ny += nlat[n];  
   }
   nxp = nx + 1;
   nyp = ny + 1;

   if( !strcmp(grid_type,"gnomonic_ed") || !strcmp(grid_type,"conformal_cubic_grid") ) {
      ntiles = 6;
      /* Cubic grid is required to run on single processor.*/
      if(mpp_npes() > 1) mpp_error( "make_hgrid: cubic grid generation must be run one processor, contact developer");
   }
   /* Currently we restrict nx can be divided by ndivx and ny can be divided by ndivy */
   if(ntilex >0 && ntilex != ntiles) mpp_error("make_hgrid: number of entry specified through --ndivx does not equal ntiles");
   if(ntiley >0 && ntiley != ntiles) mpp_error("make_hgrid: number of entry specified through --ndivy does not equal ntiles");   
   for(n=0; n<ntiles; n++) {
      if( nx%ndivx[n] ) mpp_error("make_hgrid: nx can not be divided by ndivx");
      if( ny%ndivy[n] ) mpp_error("make_hgrid: ny can not be divided by ndivy");
   }

   if(strcmp(center,"none") && strcmp(center,"c_cell") && strcmp(center,"t_cell") )
      mpp_error("make_hgrid: center should be 'none', 'c_cell' or 't_cell' ");
  
   /* set up domain decomposition, x and y will be on global domain and
      other fields will be on compute domain. 
   */

   mpp_define_layout( nx, ny, mpp_npes(), layout);
   mpp_define_domain2d( nx, ny, layout, 0, 0, &domain);
   mpp_get_compute_domain2d(domain, &isc, &iec, &jsc, &jec);
   nxc = iec - isc + 1;
   nyc = jec - jsc + 1;

   /* create grid information */
   x        = (double *) malloc(nxp*nyp*ntiles*sizeof(double));
   y        = (double *) malloc(nxp*nyp*ntiles*sizeof(double));
   dx       = (double *) malloc(nxc*(nyc+1)*ntiles*sizeof(double));
   dy       = (double *) malloc((nxc+1)*nyc*ntiles*sizeof(double));
   area     = (double *) malloc(nxc    *nyc*ntiles*sizeof(double));
   angle_dx = (double *) malloc((nxc+1)*(nyc+1)*ntiles*sizeof(double));
   if( strcmp(conformal,"true") !=0 )angle_dy = (double *) malloc(nxp*nyp*ntiles*sizeof(double));
  
   if(strcmp(grid_type,"regular_lonlat_grid") ==0) 
      create_regular_lonlat_grid(&nxbnds, &nybnds, xbnds, ybnds, nlon, nlat, &isc, &iec, &jsc, &jec,
      x, y, dx, dy, area, angle_dx, center);
   else if(strcmp(grid_type,"tripolar_grid") ==0) 
      create_tripolar_grid(&nxbnds, &nybnds, xbnds, ybnds, nlon, nlat, &lat_join, &isc, &iec, &jsc, &jec,
      x, y, dx, dy, area, angle_dx, center);
   else if( strcmp(grid_type,"from_file") ==0 ) {
      for(n=0; n<ntiles; n++) {
	 int n1, n2, n3, n4;
	 n1 = n * nxp * nyp;
	 n2 = n * nx  * nyp;
	 n3 = n * nxp * ny;
	 n4 = n * nx  * ny;
	 create_grid_from_file(my_grid_file[n], &nx, &ny, x+n1, y+n1, dx+n2, dy+n3, area+n4, angle_dx+n1);
      }
   }
   else if(strcmp(grid_type,"simple_cartesian_grid") ==0) 
      create_simple_cartesian_grid(xbnds, ybnds, &nx, &ny, &simple_dx, &simple_dy, &isc, &iec, &jsc, &jec,
      x, y, dx, dy, area, angle_dx );
   else if(strcmp(grid_type,"spectral_grid") ==0 )
      create_spectral_grid(&nx, &ny, &isc, &iec, &jsc, &jec, x, y, dx, dy, area, angle_dx );
   else if(strcmp(grid_type,"conformal_cubic_grid") ==0 ) 
      create_conformal_cubic_grid(&nx, &nratio, method, orientation, x, y, dx, dy, area, angle_dx, angle_dy );
   else if(strcmp(grid_type,"gnomonic_ed") ==0 ) 
      create_gnomonic_cubic_grid(grid_type, &nx, x, y, dx, dy, area, angle_dx, angle_dy );
  
   /* write out data */
   {
      int fid, id_tile, id_x, id_y, id_dx, id_dy, id_area, id_angle_dx, id_angle_dy, id_arcx;
      int dimlist[5], dims[2], i, j, l, ni, nj, nip, njp, m;
      size_t start[4], nwrite[4];
      double *tmp, *gdata;
      char tilename[128] = "";
      char outfile[128] = "";
    
      l = 0;
      for(n=0 ; n< ntiles; n++) {
	 for(j=0; j<ndivy[n]; j++) {
	    for(i=0; i<ndivx[n]; i++) {
	       ++l;
	       sprintf(tilename, "tile%d", l);
	       if(ntiles>1)
		  sprintf(outfile, "%s.tile%d.nc", gridname, l);
	       else
		  sprintf(outfile, "%s.nc", gridname);
	       fid = mpp_open(outfile, MPP_WRITE);
	       /* define dimenison */
	       ni = nx/ndivx[n];
	       nj = ny/ndivy[n];
	       nip = ni + 1;
	       njp = nj + 1;
	       dimlist[0] = mpp_def_dim(fid, "string", STRINGLEN);
	       dimlist[1] = mpp_def_dim(fid, "nx", ni);
	       dimlist[2] = mpp_def_dim(fid, "ny", nj);
	       dimlist[3] = mpp_def_dim(fid, "nxp", nip);
	       dimlist[4] = mpp_def_dim(fid, "nyp", njp);
	       /* define variable */
	       if( strcmp(north_pole_tile, "none") == 0) /* no north pole, then no projection */
		  id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 4, "standard_name", "grid_tile_spec",
		  "geometry", geometry, "discretization", discretization, "conformal", conformal );
	       else if( strcmp(projection, "none") == 0) 
		  id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 5, "standard_name", "grid_tile_spec",
		  "geometry", geometry, "north_pole", north_pole_tile, "discretization",
		  discretization, "conformal", conformal );
	       else
		  id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 6, "standard_name", "grid_tile_spec",
		  "geometry", geometry, "north_pole", north_pole_tile, "projection", projection,
		  "discretization", discretization, "conformal", conformal );
	       dims[0] = dimlist[4]; dims[1] = dimlist[3];
	       id_x = mpp_def_var(fid, "x", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_longitude",
	       "units", "degree_east");
	       id_y = mpp_def_var(fid, "y", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_latitude",
	       "units", "degree_north");
	       dims[0] = dimlist[4]; dims[1] = dimlist[1];
	       id_dx = mpp_def_var(fid, "dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_x_distance",
	       "units", "meters");
	       dims[0] = dimlist[2]; dims[1] = dimlist[3];
	       id_dy = mpp_def_var(fid, "dy", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_y_distance",
	       "units", "meters");
	       dims[0] = dimlist[2]; dims[1] = dimlist[1];
	       id_area = mpp_def_var(fid, "area", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_cell_area",
	       "units", "m2" );
	       dims[0] = dimlist[4]; dims[1] = dimlist[3];
	       id_angle_dx = mpp_def_var(fid, "angle_dx", MPP_DOUBLE, 2, dims, 2, "standard_name",
	       "grid_vertex_x_angle_WRT_geographic_east", "units", "degrees_east");
	       if(strcmp(conformal, "true") != 0)
		  id_angle_dy = mpp_def_var(fid, "angle_dy", MPP_DOUBLE, 2, dims, 2, "standard_name",
		  "grid_vertex_y_angle_WRT_geographic_north", "units", "degrees_north");
	       if( strcmp(north_pole_arcx, "none") == 0)
		  id_arcx = mpp_def_var(fid, "arcx", MPP_CHAR, 1, dimlist, 1, "standard_name", "grid_edge_x_arc_type" );
	       else
		  id_arcx = mpp_def_var(fid, "arcx", MPP_CHAR, 1, dimlist, 2, "standard_name", "grid_edge_x_arc_type",
		  "north_pole", north_pole_arcx );
	       mpp_def_global_att(fid, "grid_version", GRID_VERSION);
	       mpp_def_global_att(fid, "code_version", TAGNAME);
	       mpp_def_global_att(fid, "history", history);
      
	       mpp_end_def(fid);
	       for(m=0; m<4; m++) { start[m] = 0; nwrite[m] = 0; }
	       nwrite[0] = strlen(tilename);
	       mpp_put_var_value_block(fid, id_tile, start, nwrite, tilename );

	       tmp = get_subregion(nxp, x+n*nxp*nyp, i*ni, (i+1)*ni, j*nj, (j+1)*nj);
	       mpp_put_var_value(fid, id_x, tmp);
	       free(tmp);
	       tmp = get_subregion(nxp, y+n*nxp*nyp, i*ni, (i+1)*ni, j*nj, (j+1)*nj);
	       mpp_put_var_value(fid, id_y, tmp);
	       free(tmp);
	       gdata = (double *)malloc(nx*nyp*sizeof(double));
	       mpp_global_field_double(domain, nxc, nyc+1, dx+n*nx*nyp, gdata);
	       tmp = get_subregion( nx, gdata, i*ni, (i+1)*ni-1, j*nj, (j+1)*nj);
	       mpp_put_var_value(fid, id_dx, tmp);
	       free(tmp);
	       free(gdata);
	       gdata = (double *)malloc(nxp*ny*sizeof(double));
	       mpp_global_field_double(domain, nxc+1, nyc, dy+n*nxp*ny, gdata);
	       tmp = get_subregion( nxp, gdata, i*ni, (i+1)*ni, j*nj, (j+1)*nj-1);
	       mpp_put_var_value(fid, id_dy, tmp);
	       free(tmp);
	       free(gdata);	  
	       gdata = (double *)malloc(nx*ny*sizeof(double));
	       mpp_global_field_double(domain, nxc, nyc, area+n*nx*ny, gdata);
	       tmp = get_subregion( nx, gdata, i*ni, (i+1)*ni-1, j*nj, (j+1)*nj-1);
	       mpp_put_var_value(fid, id_area, tmp);
	       free(tmp);
	       free(gdata);
	       gdata = (double *)malloc(nxp*nyp*sizeof(double));
	       mpp_global_field_double(domain, nxc+1, nyc+1, angle_dx+n*nxp*nyp, gdata);
	       tmp = get_subregion( nxp, gdata, i*ni, (i+1)*ni, j*nj, (j+1)*nj);
	       mpp_put_var_value(fid, id_angle_dx, tmp);
	       free(tmp);
	       free(gdata);
	  
	       if(strcmp(conformal, "true") != 0) {
		  gdata = (double *)malloc(nxp*nyp*sizeof(double));
		  mpp_global_field_double(domain, nxc+1, nyc+1, angle_dy+n*nxp*nyp, gdata);
		  tmp = get_subregion( nxp, gdata, i*ni, (i+1)*ni, j*nj, (j+1)*nj);
		  mpp_put_var_value(fid, id_angle_dy, tmp);
		  free(tmp);
		  free(gdata);
	       }
	       nwrite[0] = strlen(arcx);
	       mpp_put_var_value_block(fid, id_arcx, start, nwrite, arcx );
	       mpp_close(fid);
	    }
	 }
      }
   }

   free(x);
   free(y);
   free(dx);
   free(dy);
   free(area);
   free(angle_dx);
   if(strcmp(conformal, "true") != 0) free(angle_dy);
}

int
gs_fregrid(char *history, char *mosaic_in, char *mosaic_out, char *dir_in, 
           char *dir_out, char **input_file, int nfiles, char **output_file, 
           int nfiles_out, char *remap_file, char **scalar_name, int nscalar,
           char **u_name, int nvector, char **v_name, int nvector2, 
           char *interp_method, char *test_case, double test_param, 
           unsigned int opcode, int grid_type, unsigned int finer_step,
           int fill_missing, int nlon, int nlat, int check_conserve, 
           int y_at_center, double lonbegin, double lonend, double latbegin,
           double latend, int kbegin, int kend, int lbegin, int lend)
{
   int     ntiles_in = 0;              /* number of tiles in input mosaic */
   int     ntiles_out = 0;             /* number of tiles in output mosaic */
   int     option_index, c, i, n, m, l;
   char    txt[STRING];

   Grid_config   *grid_in    = NULL;   /* store input grid  */
   Grid_config   *grid_out   = NULL;   /* store output grid */
   Field_config  *scalar_in  = NULL;   /* store input scalar data */
   Field_config  *scalar_out = NULL;   /* store output scalar data */
   Field_config  *u_in       = NULL;   /* store input vector u-component */
   Field_config  *v_in       = NULL;   /* store input vector v-component */
   Field_config  *u_out      = NULL;   /* store input vector u-component */
   Field_config  *v_out      = NULL;   /* store input vector v-component */
   File_config   *file_in    = NULL;   /* store input file information */
   File_config   *file_out   = NULL;   /* store output file information */
   File_config   *file2_in   = NULL;   /* store input file information */
   File_config   *file2_out  = NULL;   /* store output file information */
   Bound_config  *bound_T    = NULL;   /* store halo update information for T-cell*/
   Interp_config *interp     = NULL;   /* store remapping information */
   int save_weight_only      = 0;
  
   int fid;

   /* check the arguments */
   if( !mosaic_in  ) mpp_error("fregrid: input_mosaic is not specified");
   if( !mosaic_out ) {
      if(nlon == 0 || nlat ==0 ) mpp_error("fregrid: when output_mosaic is not specified, nlon and nlat should be specified");
      if(lonend <= lonbegin) mpp_error("fregrid: when output_mosaic is not specified, lonEnd should be larger than lonBegin");
      if(latend <= latbegin) mpp_error("fregrid: when output_mosaic is not specified, latEnd should be larger than latBegin");
   }
   else {
      if(nlon !=0 || nlat != 0) mpp_error("fregrid: when output_mosaic is specified, nlon and nlat should not be specified");
   }
  
   if( nfiles == 0) {
      if(nvector > 0 || nscalar > 0 || nvector2 > 0)
	 mpp_error("fregrid: when --input_file is not specified, --scalar_field, --u_field and --v_field should also not be specified");
      if(!remap_file) mpp_error("fregrid: when --input_file is not specified, remap_file must be specified to save weight information");
      save_weight_only = 1;
      if(mpp_pe()==mpp_root_pe())printf("NOTE: No input file specified in this run, no data file will be regridded "
      "and only weight information is calculated.\n");
   }
   else if( nfiles == 1 || nfiles ==2) {
      if( nvector != nvector2 ) mpp_error("fregrid: number of fields specified in u_field must be the same as specified in v_field");
      if( nscalar+nvector==0 ) mpp_error("fregrid: both scalar_field and vector_field are not specified");
      /* when nvector =2 and nscalar=0, nfiles can be 2 otherwise nfiles must be 1 */
      if( nscalar && nfiles != 1 )
	 mpp_error("fregrid: when scalar_field is specified, number of files must be 1");
      if( nfiles_out == 0 ) {
	 for(i=0; i<nfiles; i++) strcpy(output_file[i], input_file[i]);
      }
      else if (nfiles_out != nfiles )
	 mpp_error("fregrid:number of input file is not equal to number of output file");
   }
   else
      mpp_error("fregrid: number of input file should be 1 or 2");

   if(kbegin != 0 || kend != -1) { /* at least one of kbegin and kend is set */
      if(kbegin < 1 || kend < kbegin) mpp_error("fregrid:KlevelBegin should be a positive integer and no larger "
      "than KlevelEnd when you want pick certain klevel");
   }
   if(lbegin != 0 || lend != -1) { /* at least one of lbegin and lend is set */
      if(lbegin < 1 || lend < lbegin) mpp_error("fregrid:LstepBegin should be a positive integer and no larger "
      "than LstepEnd when you want pick certain Lstep");
   }
  
   if(nvector > 0) {
      opcode |= VECTOR;
      if(grid_type == AGRID)
	 opcode |= AGRID;
      else if(grid_type == BGRID)
	 opcode |= BGRID;
   }
  
   /* get the mosaic information of input and output mosaic*/
   fid = mpp_open(mosaic_in, MPP_READ);
   ntiles_in = mpp_get_dimlen(fid, "ntiles");
   mpp_close(fid);
   if(mosaic_out) {
      fid = mpp_open(mosaic_out, MPP_READ);
      ntiles_out = mpp_get_dimlen(fid, "ntiles");
      mpp_close(fid);
   }
   else
      ntiles_out = 1;

   if(!strcmp(interp_method, "conserve_order1") ) {
      if(mpp_pe() == mpp_root_pe())printf("****fregrid: first order conservative scheme will be used for regridding.\n");
      opcode |= CONSERVE_ORDER1;
   }
   else if(!strcmp(interp_method, "conserve_order2") ) {
      if(mpp_pe() == mpp_root_pe())printf("****fregrid: second order conservative scheme will be used for regridding.\n");
      opcode |= CONSERVE_ORDER2;
   }
   else if(!strcmp(interp_method, "bilinear") ) {
      if(mpp_pe() == mpp_root_pe())printf("****fregrid: bilinear remapping scheme will be used for regridding.\n");  
      opcode |= BILINEAR;
   }
   else
      mpp_error("fregrid: interp_method must be 'conserve_order1', 'conserve_order2' or 'bilinear'");

   if(test_case) {
      if(nfiles != 1) mpp_error("fregrid: when test_case is specified, nfiles should be 1");
      sprintf(output_file[0], "%s.%s.output", test_case, interp_method);
   }

   if(check_conserve) opcode |= CHECK_CONSERVE;
  
   if( opcode & BILINEAR ) {
      int ncontact;
      ncontact = read_mosaic_ncontacts(mosaic_in);
      if( nlon == 0 || nlat == 0) mpp_error("fregrid: when interp_method is bilinear, nlon and nlat should be specified");
      if(ntiles_in != 6) mpp_error("fregrid: when interp_method is bilinear, the input mosaic should be 6 tile cubic grid");
      if(ncontact !=12)  mpp_error("fregrid: when interp_method is bilinear, the input mosaic should be 12 contact cubic grid");
      if(mpp_npes() > 1) mpp_error("fregrid: parallel is not implemented for bilinear remapping");
   }
   else
      y_at_center = 1;


   /* memory allocation for data structure */
   grid_in   = (Grid_config *)malloc(ntiles_in *sizeof(Grid_config));
   grid_out  = (Grid_config *)malloc(ntiles_out*sizeof(Grid_config));
   bound_T   = (Bound_config *)malloc(ntiles_in *sizeof(Bound_config));
   interp    = (Interp_config *)malloc(ntiles_out*sizeof(Interp_config));
   get_input_grid( ntiles_in, grid_in, bound_T, mosaic_in, opcode );
   if(mosaic_out)
      get_output_grid_from_mosaic( ntiles_out, grid_out, mosaic_out, opcode );
   else
      get_output_grid_by_size(ntiles_out, grid_out, lonbegin, lonend, latbegin, latend,
      nlon, nlat, finer_step, y_at_center, opcode);

   if(remap_file) set_remap_file(ntiles_out, mosaic_out, remap_file, interp, &opcode, save_weight_only);

   /* preparing for the interpolation, if remapping information exist, read it from remap_file,
      otherwise create the remapping information and write it to remap_file
   */
   if( opcode & BILINEAR ) /* bilinear interpolation from cubic to lalon */
      setup_bilinear_interp(ntiles_in, grid_in, ntiles_out, grid_out, interp, opcode );
   else
      setup_conserve_interp(ntiles_in, grid_in, ntiles_out, grid_out, interp, opcode);
  
   if(save_weight_only) {
      if(mpp_pe() == mpp_root_pe() ) {
	 printf("NOTE: Successfully running fregrid and the following files which store weight information are generated.\n");
	 for(n=0; n<ntiles_out; n++) {
	    printf("****%s\n", interp[n].remap_file);
	 }
      }
      mpp_end();
      return 0;     
   }
  
   file_in   = (File_config *)malloc(ntiles_in *sizeof(File_config));
   file_out  = (File_config *)malloc(ntiles_out*sizeof(File_config));
 
   if(nfiles == 2) {
      file2_in   = (File_config *)malloc(ntiles_in *sizeof(File_config));
      file2_out  = (File_config *)malloc(ntiles_out*sizeof(File_config));
   }
   if(nscalar > 0) {
      scalar_in  = (Field_config *)malloc(ntiles_in *sizeof(Field_config));
      scalar_out = (Field_config *)malloc(ntiles_out *sizeof(Field_config));
   }
   if(nvector > 0) {
      u_in  = (Field_config *)malloc(ntiles_in *sizeof(Field_config));
      u_out = (Field_config *)malloc(ntiles_out *sizeof(Field_config));    
      v_in  = (Field_config *)malloc(ntiles_in *sizeof(Field_config));
      v_out = (Field_config *)malloc(ntiles_out *sizeof(Field_config));
   }
  
      
   set_mosaic_data_file(ntiles_in, mosaic_in, dir_in, file_in,  input_file[0]);
   set_mosaic_data_file(ntiles_out, mosaic_out, dir_out, file_out, output_file[0]);
   if(nfiles == 2) {
      set_mosaic_data_file(ntiles_in, mosaic_in, dir_in, file2_in,  input_file[1]);
      set_mosaic_data_file(ntiles_out, mosaic_out, dir_out, file2_out, output_file[1]);    
   }

   for(n=0; n<ntiles_in; n++) file_in[n].fid = mpp_open(file_in[n].name, MPP_READ);

   set_field_struct ( ntiles_in,   scalar_in,   nscalar, scalar_name[0], file_in);
   set_field_struct ( ntiles_out,  scalar_out,  nscalar, scalar_name[0], file_out);
   set_field_struct ( ntiles_in,   u_in,        nvector, u_name[0], file_in);
   set_field_struct ( ntiles_out,  u_out,       nvector, u_name[0], file_out);
   if(nfiles == 1) {
      set_field_struct ( ntiles_in,   v_in,        nvector, v_name[0], file_in);
      set_field_struct ( ntiles_out,  v_out,       nvector, v_name[0], file_out);
   }
   else {
      set_field_struct ( ntiles_in,   v_in,        nvector, v_name[0], file2_in);
      set_field_struct ( ntiles_out,  v_out,       nvector, v_name[0], file2_out);
   }

   get_input_metadata(ntiles_in, nfiles, file_in, file2_in, scalar_in, u_in, v_in, grid_in, kbegin, kend, lbegin, lend );

   set_output_metadata(ntiles_in, nfiles, file_in, file2_in, scalar_in, u_in, v_in,
   ntiles_out, file_out, file2_out, scalar_out, u_out, v_out, grid_out, history, TAGNAME );

   if(nscalar > 0) get_field_missing(ntiles_in, scalar_in);
   if(nvector > 0) {
      get_field_missing(ntiles_in, u_in);
      get_field_missing(ntiles_in, v_in);
   }
  
   /* set time step to 1, only test scalar field now, nz need to be 1 */
   if(test_case) {
      if(nscalar != 1 || nvector != 0) mpp_error("fregrid: when test_case is specified, nscalar must be 1 and nvector must be 0");
      if(scalar_in->var->nz != 1) mpp_error("fregrid: when test_case is specified, number of vertical level must be 1");
      file_in->nt = 1;
      file_out->nt = 1;
   }
   
   /* Then doing the regridding */
   for(m=0; m<file_in->nt; m++) {
      int memsize, level;

      write_output_time(ntiles_out, file_out, m);
      if(nfiles > 1) write_output_time(ntiles_out, file2_out, m);
    
      /* first interp scalar variable */
      for(l=0; l<nscalar; l++) {
	 if( !scalar_in->var[l].has_taxis && m>0) continue;
	 level = m + scalar_in->var[l].lstart;
	 if(test_case)
	    get_test_input_data(test_case, test_param, ntiles_in, scalar_in, grid_in, bound_T, opcode);
	 else
	    get_input_data(ntiles_in, scalar_in, grid_in, bound_T, l, level, opcode);
	 allocate_field_data(ntiles_out, scalar_out, grid_out, l);
	 if( opcode & BILINEAR )
	    do_scalar_bilinear_interp(interp, l, ntiles_in, grid_in, grid_out, scalar_in, scalar_out, finer_step, fill_missing);
	 else
	    do_scalar_conserve_interp(interp, l, ntiles_in, grid_in, ntiles_out, grid_out, scalar_in, scalar_out, opcode);
	 write_field_data(ntiles_out, scalar_out, grid_out, l, m);
	 if(opcode & CONSERVE_ORDER2) {
	    for(n=0; n<ntiles_in; n++) {
	       free(scalar_in[n].grad_x);
	       free(scalar_in[n].grad_y);
	       if(scalar_in[n].var[l].has_missing) free(scalar_in[n].grad_mask);
	    }
	 }
	 for(n=0; n<ntiles_in; n++) free(scalar_in[n].data);
	 for(n=0; n<ntiles_out; n++) free(scalar_out[n].data);
      }

      /* then interp vector field */
      for(l=0; l<nvector; l++) {
	 if( !u_in[n].var[l].has_taxis && m>0) continue;
	 level = m + u_in->var[l].lstart;
	 get_input_data(ntiles_in, u_in, grid_in, bound_T, l, level, opcode);
	 get_input_data(ntiles_in, v_in, grid_in, bound_T, l, level, opcode);
	 allocate_field_data(ntiles_out, u_out, grid_out, l);
	 allocate_field_data(ntiles_out, v_out, grid_out, l);
	 if( opcode & BILINEAR )
	    do_vector_bilinear_interp(interp, l, ntiles_in, grid_in, ntiles_out, grid_out, u_in, v_in, u_out, v_out, finer_step, fill_missing);
	 else
	    do_vector_conserve_interp(interp, l, ntiles_in, grid_in, ntiles_out, grid_out, u_in, v_in, u_out, v_out, opcode);
      
	 write_field_data(ntiles_out, u_out, grid_out, l, m);
	 write_field_data(ntiles_out, v_out, grid_out, l, m);
	 for(n=0; n<ntiles_in; n++) {
	    free(u_in[n].data);
	    free(v_in[n].data);
	 }
	 for(n=0; n<ntiles_out; n++) {
	    free(u_out[n].data);
	    free(v_out[n].data);
	 }      
      }
   }

   if(mpp_pe() == mpp_root_pe() ) {
      printf("Successfully running fregrid and the following output file are generated.\n");
      for(n=0; n<ntiles_out; n++) {
	 mpp_close(file_out[n].fid);
	 printf("****%s\n", file_out[n].name);
	 if( nfiles > 1 ) {
	    mpp_close(file2_out[n].fid);
	    printf("****%s\n", file2_out[n].name);
	 }
      }
   }
   return 0;
}
int 
gs_make_topog(char *history, char *mosaic_file, char *topog_type, 
              int x_refine, int y_refine, 
              double basin_depth, char *topog_file, double bottom_depth, 
              double min_depth, double scale_factor, int num_filter_pass, 
              double gauss_amp, double gauss_scale, double slope_x,
              double slope_y, double bowl_south, double bowl_north, 
              double bowl_west, double bowl_east, int fill_first_row, 
              int filter_topog, int round_shallow, int fill_shallow, 
              int deepen_shallow, int smooth_topo_allow_deepening, 
              char *output_file)
{
  char   *topog_field = NULL;
  int    option_index, i, c;

  if(x_refine != 2 || y_refine != 2 ) mpp_error("Error from make_topog: x_refine and y_refine should be 2, contact developer");
  if(mpp_pe() == mpp_root_pe()) printf("NOTE from make_topog ==> x_refine is %d, y_refine is %d\n",
				       x_refine, y_refine);

  if (strcmp(topog_type,"rectangular_basin") == 0) {
    if(mpp_pe() == mpp_root_pe()) printf("NOTE from make_topog ==> the basin depth is %f\n",basin_depth);
  }
  else if (strcmp(topog_type,"gaussian") == 0) {
    if(mpp_pe() == mpp_root_pe()){
      printf("NOTE from make_topog ==> bottom_depth is: %f\n", bottom_depth );
      printf("NOTE from make_topog ==> min_depth is: %f\n", min_depth );
      printf("NOTE from make_topog ==> gauss_amp is: %f\n", gauss_amp );
      printf("NOTE from make_topog ==> gauss_scale is: %f\n", gauss_scale );
      printf("NOTE from make_topog ==> slope_x is: %f\n", slope_x );
      printf("NOTE from make_topog ==> slope_y is: %f\n", slope_y );      
    }
  }
  else if(strcmp(topog_type,"bowl") == 0) {
    if(mpp_pe() == mpp_root_pe()){
      printf("NOTE from make_topog ==> bottom_depth is: %f\n",bottom_depth);
      printf("NOTE from make_topog ==> min_depth is: %f\n",min_depth);
      printf("NOTE from make_topog ==> bowl_south is: %f\n",bowl_south);
      printf("NOTE from make_topog ==> bowl_north is: %f\n",bowl_north);
      printf("NOTE from make_topog ==> bowl_west is: %f\n",bowl_west);
      printf("NOTE from make_topog ==> bowl_east is: %f\n",bowl_east);
    }
  }
  else if(strcmp(topog_type,"idealized") == 0) {
    if(mpp_pe() == mpp_root_pe()){
      printf("NOTE from make_topog ==> bottom_depth is: %f\n",bottom_depth);
      printf("NOTE from make_topog ==> min_depth is: %f\n",min_depth);
    }
  }
  else if(strcmp(topog_type,"realistic") == 0) {
    if(!topog_file || !topog_field)
      mpp_error("Error from make_topog: when topog_type is realistic, topog_file and topog_field must be specified.");
    if(mpp_pe() == mpp_root_pe()){
      printf("NOTE from make_topog ==> bottom_depth is: %f\n",bottom_depth);
      printf("NOTE from make_topog ==> min_depth is: %f\n",min_depth);
      printf("NOTE from make_topog ==> topog_file is: %s\n", topog_file);
      printf("NOTE from make_topog ==> topog_field is: %s\n", topog_field);
      printf("NOTE from make_topog ==> scale_factor is: %f\n", scale_factor);
      printf("NOTE from make_topog ==> num_filter_pass is: %d\n", num_filter_pass);
      if(fill_first_row) printf("NOTE from make_topog ==>make first row of ocean model all land points.\n");
      if(filter_topog) printf("NOTE from make_topog ==>will apply filter to topography.\n");
      if(round_shallow) printf("NOTE from make_topog ==>Make cells land if depth is less than 1/2 "
			       "mimumim depth, otherwise make ocean.\n");
      if(fill_shallow) printf("NOTE from make_topog ==>Make cells less than minimum depth land.\n");
      if(deepen_shallow) printf("NOTE from make_topog ==>Make cells less than minimum depth equal to minimum depth.\n");
      if(smooth_topo_allow_deepening) printf("NOTE from make_topog ==>allow filter to deepen cells.\n");
    }
  }
  else {
    mpp_error("make_topog: topog_type should be rectangular_basin, gaussian, bowl, idealized or realistic");
  }
  
  if(mpp_pe() == mpp_root_pe()) {
    printf("**************************************************\n");
    printf("Begin to generate topography \n");
  }

  {
#define STRING 255
    int m_fid, g_fid, vid;
    int ntiles, fid, dim_ntiles, n, dims[2];
    size_t start[4], nread[4], nwrite[4];
    int *nx, *ny, *nxp, *nyp;
    int *id_depth;
    double *depth, *x, *y;
    char **tile_files;
    char history[512], dimx_name[128], dimy_name[128], depth_name[128];
    char gridfile[256], griddir[256];
    
    /* history will be write out as global attribute
       in output file to specify the command line arguments
    */

    /* grid should be located in the same directory of mosaic file */
    get_file_path(mosaic_file, griddir);
    
    /* get mosaic dimension */
    m_fid = mpp_open(mosaic_file, MPP_READ);
    ntiles = mpp_get_dimlen( m_fid, "ntiles");
    tile_files = (char **)malloc(ntiles*sizeof(double *));
    id_depth = (int *)malloc(ntiles*sizeof(int));
    /* loop through each tile to get tile information and set up meta data for output file */
    fid = mpp_open(output_file, MPP_WRITE);
    mpp_def_global_att(fid, "grid_version", GRID_VERSION);
    mpp_def_global_att(fid, "code_version", TAGNAME);
    mpp_def_global_att(fid, "history", history);
    dim_ntiles = mpp_def_dim(fid, "ntiles", ntiles);
    nx = (int *)malloc(ntiles*sizeof(int));
    ny = (int *)malloc(ntiles*sizeof(int));
    nxp = (int *)malloc(ntiles*sizeof(int));
    nyp = (int *)malloc(ntiles*sizeof(int));   
    for( n = 0; n < ntiles; n++ ) {
      tile_files[n] = (char *)malloc(STRING*sizeof(double));
      start[0] = n;
      start[1] = 0;
      nread[0] = 1;
      nread[1] = STRING;
      vid = mpp_get_varid(m_fid, "gridfiles");
      mpp_get_var_value_block(m_fid, vid, start, nread, gridfile);
      sprintf(tile_files[n], "%s/%s", griddir, gridfile);
      g_fid = mpp_open(tile_files[n], MPP_READ);
      nx[n] = mpp_get_dimlen(g_fid, "nx");
      ny[n] = mpp_get_dimlen(g_fid, "ny");
      if( nx[n]%x_refine != 0 ) mpp_error("make_topog: supergrid x-size can not be divided by x_refine");
      if( ny[n]%y_refine != 0 ) mpp_error("make_topog: supergrid y-size can not be divided by y_refine");
      nx[n] /= x_refine;
      ny[n] /= y_refine;
      nxp[n] = nx[n] + 1;
      nyp[n] = ny[n] + 1;
      if(ntiles == 1) {
	strcpy(dimx_name, "nx");
	strcpy(dimy_name, "ny");
	strcpy(depth_name, "depth");
      }
      else {
	sprintf(dimx_name, "nx_tile%d", n+1);
	sprintf(dimy_name, "ny_tile%d", n+1);
	sprintf(depth_name, "depth_tile%d", n+1);
      }

      dims[1] = mpp_def_dim(fid, dimx_name, nx[n]); 
      dims[0] = mpp_def_dim(fid, dimy_name, ny[n]);
      id_depth[n] = mpp_def_var(fid, depth_name, NC_DOUBLE, 2, dims,  2, "standard_name",
				"topographic depth at T-cell centers", "units", "meters");
      mpp_close(g_fid);
    }
    mpp_close(m_fid);
    mpp_end_def(fid);

    /* Generate topography and write out to the output_file */
    for(n=0; n<ntiles; n++) {
      int layout[2], isc, iec, jsc, jec, nxc, nyc, ni, i, j;
      double *gdata, *tmp;
      domain2D domain;
      
      /* define the domain, each tile will be run on all the processors. */
      mpp_define_layout( nx[n], ny[n], mpp_npes(), layout);
      mpp_define_domain2d( nx[n], ny[n], layout, 0, 0, &domain);
      mpp_get_compute_domain2d( domain, &isc, &iec, &jsc, &jec);
      nxc = iec - isc + 1;
      nyc = jec - jsc + 1;
      
      depth = (double *)malloc(nxc*nyc*sizeof(double));
      x     = (double *)malloc((nxc+1)*(nyc+1)*sizeof(double));
      y     = (double *)malloc((nxc+1)*(nyc+1)*sizeof(double));
      tmp   = (double *)malloc((nxc*x_refine+1)*(nyc*y_refine+1)*sizeof(double));
      start[0] = jsc*y_refine; start[1] = isc*x_refine;
      nread[0] = nyc*y_refine+1; nread[1] = nxc*x_refine+1;
      ni       = nxc*x_refine+1;
      g_fid = mpp_open(tile_files[n], MPP_READ);
      vid = mpp_get_varid(g_fid, "x");
      mpp_get_var_value_block(g_fid, vid, start, nread, tmp);
      for(j = 0; j < nyc+1; j++) for(i = 0; i < nxc+1; i++)
	x[j*(nxc+1)+i] = tmp[(j*y_refine)*ni+i*x_refine];
      vid = mpp_get_varid(g_fid, "y");
      mpp_get_var_value_block( g_fid, vid, start, nread, tmp);
      mpp_close(g_fid);
      for(j = 0; j < nyc+1; j++) for(i = 0; i < nxc+1; i++)
	y[j*(nxc+1)+i] = tmp[(j*y_refine)*ni+i*x_refine];
      if (strcmp(topog_type,"rectangular_basin") == 0)
	create_rectangular_topog(nx[n], ny[n], basin_depth, depth);
      else if (strcmp(topog_type,"gaussian") == 0)
	create_gaussian_topog(nx[n], ny[n], x, y, bottom_depth, min_depth,
			      gauss_amp, gauss_scale, slope_x, slope_y, depth);
      else if (strcmp(topog_type,"bowl") == 0)
	create_bowl_topog(nx[n], ny[n], x, y, bottom_depth, min_depth, bowl_east,
			  bowl_south, bowl_west, bowl_north, depth);
      else if (strcmp(topog_type,"idealized") == 0)
	create_idealized_topog( nx[n], ny[n], x, y, bottom_depth, min_depth, depth);
      else if (strcmp(topog_type,"realistic") == 0)
	create_realistic_topog(nxc, nyc, x, y, topog_file, topog_field, scale_factor,
			       fill_first_row, filter_topog, num_filter_pass,
			       smooth_topo_allow_deepening, round_shallow, fill_shallow,
			       deepen_shallow, min_depth, depth );
      gdata = (double *)malloc(nx[n]*ny[n]*sizeof(double));
      mpp_global_field_double(domain, nxc, nyc, depth, gdata);
      mpp_put_var_value(fid, id_depth[n], gdata);
      free(x);
      free(y);
      free(tmp);
      free(depth);
      free(gdata);
      mpp_delete_domain2d(&domain);
    }
    mpp_close(fid);
  
    /*release memory */
    free(id_depth);
    for(n=0; n<ntiles; n++) free(tile_files[n]);
    free(tile_files);
    free(nx);
    free(ny);
    free(nxp);
    free(nyp);
  }
  return 0;
}

#define MAXTILE 100
#define MAXCONTACT 100
#define SHORTSTRING 32

int
gs_make_solo_mosaic(char *history, int ntiles, char *mosaic_name, char *grid_descriptor,
                    char **tilefile, double periodx, double periody, char *dir)
{
   char *pch=NULL;
   char tiletype[MAXTILE][SHORTSTRING];
   char tile_name[MAXTILE][STRING];
   int nfiles=0, ncontact=0;
   int *nxp, *nyp;
   double **x, **y;
   int contact_tile1[MAXCONTACT], contact_tile2[MAXCONTACT];
   int contact_tile1_istart[MAXCONTACT], contact_tile1_iend[MAXCONTACT];
   int contact_tile1_jstart[MAXCONTACT], contact_tile1_jend[MAXCONTACT];
   int contact_tile2_istart[MAXCONTACT], contact_tile2_iend[MAXCONTACT];
   int contact_tile2_jstart[MAXCONTACT], contact_tile2_jend[MAXCONTACT];
   int c, i, n, m, l, errflg;
    
   /*--- if file name is not specified through -f, file name will be horizontal_grid.tile#.nc */
   if(nfiles == 0) {
      if(ntiles == 1) {
	 sprintf(tilefile[0],"horizontal_grid.nc");
      }
      else {
	 for(n=0; n<ntiles; n++) {
	    sprintf(tilefile[n],"horizontal_grid.tile%d.nc",n+1);
	 }
      }
   }
   else { /* Check if ntile are matching number of grid file passed through -f */
      if( nfiles != ntiles) mpp_error("make_solo_mosaic: number of grid files specified through -n "
      " does not equal to number of files specified through -f = ");
   }

   n = strlen(dir);
   if( dir[n-1] != '/') strcat(dir, "/");

   /*First read all the grid files.*/
   nxp = (int *)malloc(ntiles*sizeof(int));
   nyp = (int *)malloc(ntiles*sizeof(int));
   x = (double **)malloc(ntiles*sizeof(double *));
   y = (double **)malloc(ntiles*sizeof(double *));
   for(n=0; n<ntiles; n++) {
      char filepath[512];
      int fid, vid;
      sprintf(filepath, "%s%s",dir, tilefile[n]);
      fid = mpp_open(filepath, MPP_READ);
      nxp[n] = mpp_get_dimlen(fid, "nxp");
      nyp[n] = mpp_get_dimlen(fid, "nyp");
      x[n] = (double *)malloc(nxp[n]*nyp[n]*sizeof(double));
      y[n] = (double *)malloc(nxp[n]*nyp[n]*sizeof(double));
      vid = mpp_get_varid(fid, "tile");
      mpp_get_var_value(fid, vid, tile_name[n]);
      vid = mpp_get_varid(fid, "x");
      mpp_get_var_value(fid, vid, x[n]);
      vid = mpp_get_varid(fid, "y");
      mpp_get_var_value(fid, vid, y[n]);
      mpp_close(fid);
   }


   /*find the contact region between tiles, currently assume the contact region are align-contact
     There should be no contact between same directions of two tiles ( w-w, e-e, s-s, n-n)
     We assume no contact between w-s, e-n ( will added in if needed ) . */
   ncontact = 0;
   for(n=0; n<ntiles; n++) {
      for(m=n; m<ntiles; m++) {
	 int count;
	 int istart1[MAXCONTACT], iend1[MAXCONTACT], jstart1[MAXCONTACT], jend1[MAXCONTACT];
	 int istart2[MAXCONTACT], iend2[MAXCONTACT], jstart2[MAXCONTACT], jend2[MAXCONTACT];
	 count = get_align_contact(n+1, m+1, nxp[n], nyp[n], nxp[m], nyp[m], x[n], y[n], x[m], y[m],
	 periodx, periody, istart1, iend1, jstart1, jend1, istart2, iend2,
	 jstart2, jend2);
	 if(ncontact+count>MAXCONTACT) mpp_error("make_solo_mosaic: number of contacts is more than MAXCONTACT");
	 for(l=0; l<count; l++) {
	    contact_tile1_istart[ncontact] = istart1[l];
	    contact_tile1_iend  [ncontact] = iend1[l];
	    contact_tile1_jstart[ncontact] = jstart1[l];
	    contact_tile1_jend  [ncontact] = jend1[l];
	    contact_tile2_istart[ncontact] = istart2[l];
	    contact_tile2_iend  [ncontact] = iend2[l];
	    contact_tile2_jstart[ncontact] = jstart2[l];
	    contact_tile2_jend  [ncontact] = jend2[l];
	    contact_tile1       [ncontact] = n;
	    contact_tile2       [ncontact] = m;
	    ncontact++;
	 }
      }
   }

   /* write out data */
   {
      char str[STRING], outfile[STRING];
      int fid, dim_ntiles, dim_ncontact, dim_string, id_mosaic, id_gridtiles, id_contacts;
      int id_contact_index, id_griddir, id_gridfiles, dim[2];
      size_t start[4], nwrite[4];

      sprintf(outfile, "%s.nc", mosaic_name);
      fid = mpp_open(outfile, MPP_WRITE);
      /* define dimenison */
      dim_ntiles = mpp_def_dim(fid, "ntiles", ntiles);
      if(ncontact>0) dim_ncontact = mpp_def_dim(fid, "ncontact", ncontact);
      dim_string = mpp_def_dim(fid, "string", STRING);    
      /* define variable */
      id_mosaic = mpp_def_var(fid, "mosaic", MPP_CHAR, 1, &dim_string, 4, "standard_name",
      "grid_mosaic_spec", "children", "gridtiles", "contact_regions", "contacts",
      "grid_descriptor", grid_descriptor);
      dim[0] = dim_ntiles; dim[1] = dim_string;
      id_griddir   = mpp_def_var(fid, "gridlocation", MPP_CHAR, 1, &dim[1], 1,
      "standard_name", "grid_file_location");
      id_gridfiles = mpp_def_var(fid, "gridfiles", MPP_CHAR, 2, dim, 0);
      id_gridtiles = mpp_def_var(fid, "gridtiles", MPP_CHAR, 2, dim, 0);

      if(ncontact>0) {
	 dim[0] = dim_ncontact; dim[1] = dim_string;
	 id_contacts = mpp_def_var(fid, "contacts", MPP_CHAR, 2, dim, 5, "standard_name", "grid_contact_spec",
	 "contact_type", "boundary", "alignment", "true",
	 "contact_index", "contact_index", "orientation", "orient");
	 id_contact_index = mpp_def_var(fid, "contact_index", MPP_CHAR, 2, dim, 1, "standard_name",
	 "starting_ending_point_index_of_contact");

      }
      mpp_def_global_att(fid, "grid_version", GRID_VERSION);
      mpp_def_global_att(fid, "code_version", TAGNAME);
      mpp_def_global_att(fid, "history", history);
      mpp_end_def(fid);

      /* write out data */
      for(i=0; i<4; i++) {
	 start[i] = 0; nwrite[i] = 1;
      }
      nwrite[0] = strlen(mosaic_name);
      mpp_put_var_value_block(fid, id_mosaic, start, nwrite, mosaic_name);
      nwrite[0] = strlen(dir);
      mpp_put_var_value_block(fid, id_griddir, start, nwrite, dir);
      nwrite[0] = 1;
      for(n=0; n<ntiles; n++) {
	 start[0] = n; nwrite[1] = strlen(tile_name[n]);
	 mpp_put_var_value_block(fid, id_gridtiles, start, nwrite, tile_name[n]);
	 nwrite[1] = strlen(tilefile[n]);
	 mpp_put_var_value_block(fid, id_gridfiles, start, nwrite, tilefile[n]);
      }
    
      for(n=0; n<ncontact; n++) {
	 sprintf(str,"%s:%s::%s:%s", mosaic_name, tile_name[contact_tile1[n]], mosaic_name,
	 tile_name[contact_tile2[n]]);
	 start[0] = n; nwrite[1] = strlen(str);
	 mpp_put_var_value_block(fid, id_contacts, start, nwrite, str);
	 sprintf(str,"%d:%d,%d:%d::%d:%d,%d:%d", contact_tile1_istart[n], contact_tile1_iend[n],
	 contact_tile1_jstart[n], contact_tile1_jend[n], contact_tile2_istart[n],
	 contact_tile2_iend[n], contact_tile2_jstart[n], contact_tile2_jend[n] );
	 nwrite[1] = strlen(str);
	 mpp_put_var_value_block(fid, id_contact_index, start, nwrite, str);
      }
      mpp_close(fid);    
   }
   return 0;
}

#include "create_xgrid.h"
#include "constant.h"
#include "mosaic_util.h"
#include "tool_util.h"

const double LARGE_VALUE       = 1e20;
const int    X_CYCLIC          = 1;
const int    Y_CYCLIC          = 2;
const int    CUBIC_GRID        = 4;
const int    x_refine          = 2;
const int    y_refine          = 2;
const double EPSLN             = 1.e-4;
const double MIN_AREA_RATIO    = 1.e-6;
const char   subA_name[]       = "subA";
const char   tocell_name[]     = "tocell";
const char   travel_name[]     = "travel";
const char   basin_name[]      = "basin";
const char   cellarea_name[]   = "cellarea";
const char   celllength_name[] = "celllength";
const char   landfrac_name[]   = "land_frac";
const char   version[]         = "0.1";
const int    ncells = 3;
char   xaxis_name[128];
char   yaxis_name[128];
char   gridx_name[] = "grid_x";
char   gridy_name[] = "grid_y";
char   x_name[]     = "x";
char   y_name[]     = "y";
int    sizeof_int  = 0;
int    sizeof_double = 0;
double suba_cutoff = 1.e12;  
#define D2R (M_PI/180.)


int
gs_river_regrid(char *history, char *mosaic_file, char *river_src_file, 
   char *output_file)
{
   unsigned int opcode = 0;  
   int          option_index, c;
   int          ntiles, n;
   char         land_mosaic_dir[256];
   char         land_mosaic[256];
   char         land_mosaic_file[256];  
  
   river_type river_in;
   river_type *river_out; /* may be more than one tile */

   /* check the arguments */
   if( !mosaic_file    ) mpp_error("fregrid: mosaic_grid is not specified");
   if( !river_src_file ) mpp_error("fregrid: river_source_file is not specified");

#ifdef test_qsort  
   {
      /* test qsort_index here */
      int    i, size = 7;
      double array[]={12,5,18,25,14, 4,11};
      int    rank[]={1,2,3,4,5,6,7};

      qsort_index(array, 0, size-1, rank);
      for(i=0; i<size; i++) printf("value = %f, rank = %d \n", array[i], rank[i]);
   }
#endif
  
   /* First read the source data and source grid */
   get_source_data(river_src_file, &river_in);

   /* get the mosaic grid information */
   {
      int fid, vid;
      get_file_path(mosaic_file, land_mosaic_dir);
      fid = mpp_open(mosaic_file, MPP_READ);
      vid = mpp_get_varid(fid, "lnd_mosaic_file");
      mpp_get_var_value(fid, vid, land_mosaic_file);
      sprintf(land_mosaic, "%s/%s", land_mosaic_dir, land_mosaic_file);
      mpp_close(fid);
      fid = mpp_open(land_mosaic, MPP_READ);
      ntiles = mpp_get_dimlen( fid, "ntiles");
      mpp_close(fid);
   }
   river_out = (river_type *)malloc(ntiles*sizeof(river_type));
  
   get_mosaic_grid(mosaic_file, land_mosaic, ntiles, river_out, &opcode);

   init_river_data(ntiles, river_out, &river_in);
  
   calc_max_subA(&river_in, river_out, ntiles, opcode );

   calc_tocell(ntiles, river_out, opcode );

   calc_river_data(ntiles, river_out, opcode );

   sort_basin(ntiles, river_out);
  
   check_river_data(ntiles, river_out);
  
   write_river_data(river_src_file, output_file, river_out, history, ntiles);
  
   printf("Successfully running river_regrid and the following output file are generated.\n");
   for(n=0; n<ntiles; n++) printf("****%s\n", river_out[n].filename);
   return 0;
}

/*------------------------------------------------------------------------------
  void get_source_data(char *src_file)
  read the source file to get the source grid and river network data
  -----------------------------------------------------------------------------*/
void get_source_data(const char *src_file, river_type *river_data)
{
   int nx, ny, nxp, nyp;
   int i, j;
   int fid, vid;
   double dbl_missing;

   fid = mpp_open(src_file, MPP_READ);
   vid = mpp_get_varid(fid, subA_name);
  
   mpp_get_var_dimname(fid, vid, 1, xaxis_name);
   mpp_get_var_dimname(fid, vid, 0, yaxis_name);
   nx  = mpp_get_dimlen(fid, xaxis_name );
   ny  = mpp_get_dimlen(fid, yaxis_name );
   nxp = nx + 1;
   nyp = ny + 1;
   river_data->nx  = nx;
   river_data->ny  = ny;
  
   river_data->xt   = (double *)malloc(nx*sizeof(double));
   river_data->yt   = (double *)malloc(ny*sizeof(double));
   river_data->xb   = (double *)malloc(nxp*sizeof(double)); 
   river_data->yb   = (double *)malloc(nyp*sizeof(double));
   river_data->xb_r = (double *)malloc(nxp*sizeof(double)); 
   river_data->yb_r = (double *)malloc(nyp*sizeof(double));
   river_data->subA = (double *)malloc(nx*ny*sizeof(double));
   mpp_get_var_att(fid, vid, "missing_value", &(river_data->subA_missing) );
   vid = mpp_get_varid(fid, tocell_name);
   mpp_get_var_att(fid, vid, "missing_value", &dbl_missing );
   river_data->tocell_missing = dbl_missing;
   vid = mpp_get_varid(fid, travel_name);
   mpp_get_var_att(fid, vid, "missing_value", &dbl_missing );
   river_data->travel_missing = dbl_missing;
   vid = mpp_get_varid(fid, basin_name);
   mpp_get_var_att(fid, vid, "missing_value", &dbl_missing );
   river_data->basin_missing = dbl_missing;
   vid = mpp_get_varid(fid, cellarea_name);
   mpp_get_var_att(fid, vid, "missing_value", &(river_data->cellarea_missing) );
   vid = mpp_get_varid(fid, celllength_name);
   mpp_get_var_att(fid, vid, "missing_value", &(river_data->celllength_missing) );

   vid = mpp_get_varid(fid, subA_name);
   mpp_get_var_value(fid, vid, river_data->subA);
   vid = mpp_get_varid(fid, xaxis_name);
   mpp_get_var_value(fid, vid, river_data->xt);
   vid = mpp_get_varid(fid, yaxis_name);
   mpp_get_var_value(fid, vid, river_data->yt);
   mpp_close(fid);
  
   for(i=1; i<nx; i++ ) river_data->xb[i] = 0.5*(river_data->xt[i-1]+river_data->xt[i]);
   for(j=1; j<ny; j++ ) river_data->yb[j] = 0.5*(river_data->yt[j-1]+river_data->yt[j]);
   river_data->xb[0] = 2*river_data->xt[0] - river_data->xb[1];
   river_data->yb[0] = 2*river_data->yt[0] - river_data->yb[1];
   river_data->xb[nx] = 2*river_data->xt[nx-1] - river_data->xb[nx-1];
   river_data->yb[ny] = 2*river_data->yt[ny-1] - river_data->yb[ny-1];

   /* make sure the xb is in the range [0,360] and yb is in the range [-90,90] */
   if(fabs(river_data->xb[0]) > EPSLN) mpp_error("river_regrid: The starting longitude of grid bound is not 0");
   if(fabs(river_data->xb[nx]-360) > EPSLN) mpp_error("river_regrid: The ending longitude of grid bound is not 360");  
   if(fabs(river_data->yb[0]+90) > EPSLN) mpp_error("river_regrid: The starting latitude of grid bound is not -90");
   if(fabs(river_data->yb[ny]-90) > EPSLN) mpp_error("river_regrid: The ending latitude of grid bound is not 90");  
   river_data->xb[0]  = 0.;
   river_data->xb[nx] = 360.;
   river_data->yb[0]  = -90.;
   river_data->yb[ny] = 90.;
   for(j=0; j<nyp; j++) river_data->yb_r[j] = river_data->yb[j]*D2R;
   for(i=0; i<nxp; i++) river_data->xb_r[i] = river_data->xb[i]*D2R;

}

/*----------------------------------------------------------------------
  read the grid of detination mosaic. 
  void get_mosaic_grid(char *file)
  where file is the coupler mosaic file.
  --------------------------------------------------------------------*/
void get_mosaic_grid(const char *coupler_mosaic, const char *land_mosaic, int ntiles, river_type *river_data, unsigned int *opcode)
{
   int    n_xgrid_files, nx, ny, nxp, nyp, ni, nj, nip, njp;
   int    n, m, i, j, ii, jj, nxp2, nyp2;
   size_t start[4], nread[4];
   char   dir[STRING], gridfile[STRING], tilefile[STRING];
   char   tilename[STRING], land_mosaic_name[STRING];
   char   **xgrid_file;
   double *x, *y;
   double **pxt, **pyt;
   char *pfile;
   int  m_fid, m_vid, g_fid, g_vid;

   /* coupler_mosaic, land_mosaic, and exchange grid file should be located in the same directory */
   get_file_path(coupler_mosaic, dir);
  
   m_fid = mpp_open(coupler_mosaic, MPP_READ);
   m_vid = mpp_get_varid(m_fid, "lnd_mosaic");
   mpp_get_var_value(m_fid, m_vid, land_mosaic_name);
  
   /* get the exchange grid file name */
   n_xgrid_files = mpp_get_dimlen(m_fid, "nfile_aXl");
   xgrid_file = (char **)malloc(n_xgrid_files*sizeof(char *));

   m_vid = mpp_get_varid(m_fid, "aXl_file");
   for(n=0; n<n_xgrid_files; n++) {
      xgrid_file[n] = (char *)malloc(STRING*sizeof(char));
      start[0] = n;
      start[1] = 0;
      nread[0] = 1;
      nread[1] = STRING;    
      mpp_get_var_value_block(m_fid, m_vid, start, nread, xgrid_file[n]);
   }
   mpp_close(m_fid);
   m_fid = mpp_open(land_mosaic, MPP_READ);
   m_vid = mpp_get_varid(m_fid, "gridfiles");
  
   for( n = 0; n < ntiles; n++ ) {
      double *area;
    
      start[0] = n;
      start[1] = 0;
      nread[0] = 1;
      nread[1] = STRING;
      mpp_get_var_value_block(m_fid, m_vid, start, nread, tilefile);
      sprintf(gridfile, "%s/%s", dir, tilefile);
      g_fid = mpp_open(gridfile, MPP_READ);
      ni = mpp_get_dimlen(g_fid, "nx");
      nj = mpp_get_dimlen(g_fid, "ny");
      nip = ni + 1;
      njp = nj + 1;
      x = (double *)malloc(nip*njp*sizeof(double));
      y = (double *)malloc(nip*njp*sizeof(double));
      g_vid = mpp_get_varid(g_fid, "x");
      mpp_get_var_value(g_fid, g_vid, x);
      g_vid = mpp_get_varid(g_fid, "y");
      mpp_get_var_value(g_fid, g_vid, y);
      mpp_close(g_fid);
      if( ni%x_refine != 0 ) mpp_error("river_regrid: supergrid x-size can not be divided by x_refine");
      if( nj%y_refine != 0 ) mpp_error("river_regrid: supergrid y-size can not be divided by y_refine");
      nx   = ni/x_refine;
      ny   = nj/y_refine;
      nxp  = nx + 1;
      nyp  = ny + 1;
      nxp2 = nx + 2;
      nyp2 = ny + 2;
      river_data[n].nx       = nx;
      river_data[n].ny       = ny;
      river_data[n].xt       = (double *)malloc(nxp2*nyp2*sizeof(double));        
      river_data[n].yt       = (double *)malloc(nxp2*nyp2*sizeof(double));
      river_data[n].xb       = (double *)malloc(nxp*nyp*sizeof(double));
      river_data[n].yb       = (double *)malloc(nxp*nyp*sizeof(double));
      river_data[n].xb_r     = (double *)malloc(nxp*nyp*sizeof(double));
      river_data[n].yb_r     = (double *)malloc(nxp*nyp*sizeof(double));    
      river_data[n].area     = (double *)malloc(nx*ny*sizeof(double));
      river_data[n].landfrac = (double *)malloc(nx*ny*sizeof(double));    
      area                   = (double *)malloc(nx*ny*sizeof(double));
    
      /* copy the data from super grid to fine grid */
      for(i=0; i<nx*ny; i++) area[i] = 0.0;
      for(j = 0; j < ny; j++) for(i = 0; i < nx; i++) {
	    ii = i+1;
	    jj = j+1;
	    river_data[n].xt[jj*nxp2+ii] = x[(j*y_refine+1)*nip+i*x_refine+1];
	    river_data[n].yt[jj*nxp2+ii] = y[(j*y_refine+1)*nip+i*x_refine+1];
	 }
      for(j = 0; j < nyp; j++) for(i = 0; i < nxp; i++) {
	    river_data[n].xb[j*nxp+i] = x[(j*y_refine)*nip+i*x_refine];
	    river_data[n].yb[j*nxp+i] = y[(j*y_refine)*nip+i*x_refine];
	 }     
      free(x);
      free(y);
      /* calculate cell area */

      for(m=0; m<nxp*nyp; m++) {
	 river_data[n].xb_r[m] = river_data[n].xb[m] * D2R;
	 river_data[n].yb_r[m] = river_data[n].yb[m] * D2R;
      }
      get_grid_area(&nx, &ny, river_data[n].xb_r, river_data[n].yb_r, river_data[n].area);

      /* calculate the land fraction */
      sprintf(tilename, "X%s_tile%d", land_mosaic_name, n+1);
      for(m=0; m<n_xgrid_files; m++) {
	 if(strstr(xgrid_file[m],tilename)) {
	    int    nxgrid, l;
	    int    *i1, *j1, *i2, *j2;
	    double *xgrid_area;
	    char filewithpath[512];
	
	    sprintf(filewithpath, "%s/%s", dir, xgrid_file[m]);
	    g_fid = mpp_open(filewithpath, MPP_READ);
	    nxgrid = mpp_get_dimlen(g_fid, "ncells");
	    mpp_close(g_fid);
	    i1         = (int    *)malloc(nxgrid*sizeof(int));
	    j1         = (int    *)malloc(nxgrid*sizeof(int));	  
	    i2         = (int    *)malloc(nxgrid*sizeof(int));
	    j2         = (int    *)malloc(nxgrid*sizeof(int));
	    xgrid_area = (double *)malloc(nxgrid*sizeof(double));
	    read_mosaic_xgrid_order1(filewithpath, i1, j1, i2, j2, xgrid_area);
	    for(l=0; l<nxgrid; l++) area[j2[l]*nx+i2[l]] += xgrid_area[l];
	    free(i1);
	    free(j1);
	    free(i2);
	    free(j2);
	 }
      }

      for(m=0; m<nx*ny; m++) area[m] *= 4*M_PI*RADIUS*RADIUS;
      for(m=0; m<nx*ny; m++) {
	 river_data[n].landfrac[m] = area[m]/river_data[n].area[m];
	 /* consider truncation error */
	 if(fabs(river_data[n].landfrac[m]-1) < EPSLN) river_data[n].landfrac[m] = 1;
	 if(fabs(river_data[n].landfrac[m])   < EPSLN) river_data[n].landfrac[m] = 0;
	 if(river_data[n].landfrac[m] > 1 || river_data[n].landfrac[m] < 0)
	    mpp_error("river_regrid: land_frac should be between 0 or 1");
      }
      free(area);
   } /* n = 0, ntiles */

   mpp_close(m_fid);
  
   /* currently we are assuming all the times have the same grid size */
   for(n=1; n<ntiles; n++) {
      if(river_data[n].nx != river_data[0].nx || river_data[n].ny != river_data[0].ny )
	 mpp_error("river_regrid: all the tiles should have the same grid size");
   }
  
   /*----------------------------------------------------------------------------
     get boundary condition, currently we are assuming the following case,
     solid walls: number of contact will be 0.
     x-cyclic:    number of contact will be 1.
     y-cyclic:    number of contact will be 1.
     torus:       number of contact will be 2.
     cubic:       number of contact will be 12.
     --------------------------------------------------------------------------*/
   {
      int ncontact;
      int *tile1, *tile2;
      int *istart1, *iend1, *jstart1, *jend1;
      int *istart2, *iend2, *jstart2, *jend2;
    
      ncontact = read_mosaic_ncontacts(land_mosaic);
      tile1   = (int *)malloc(ncontact*sizeof(int));
      tile2   = (int *)malloc(ncontact*sizeof(int));
      istart1 = (int *)malloc(ncontact*sizeof(int));
      iend1   = (int *)malloc(ncontact*sizeof(int));
      jstart1 = (int *)malloc(ncontact*sizeof(int));
      jend1   = (int *)malloc(ncontact*sizeof(int));
      istart2 = (int *)malloc(ncontact*sizeof(int));
      iend2   = (int *)malloc(ncontact*sizeof(int));
      jstart2 = (int *)malloc(ncontact*sizeof(int));
      jend2   = (int *)malloc(ncontact*sizeof(int));
      if(ncontact >0) {
	 read_mosaic_contact(land_mosaic, tile1, tile2, istart1, iend1,
	 jstart1, jend1, istart2, iend2, jstart2, jend2);
	 if(ncontact <= 2) { /* x-cyclic of y-cyclic */
	    if(ntiles !=1) mpp_error("river_regrid: number of tiles must be 1 for single contact");
	    for(n=0; n<ncontact; n++) {
	       if(istart1[n] == iend1[n] && istart2[n] == iend2[n] ) /* x_cyclic */
		  *opcode |= X_CYCLIC;
	       else if(jstart1[n] == jend1[n] && istart2[n] == iend2[n] ) /* x_cyclic */
		  *opcode |= Y_CYCLIC;
	       else
		  mpp_error("river_regrid: for one-tile mosaic, the boundary condition should be either x-cyclic or y-cyclic");
	    }
	 }
	 else if(ncontact == 12) {
	    if(ntiles != 6) mpp_error("river_regrid: the mosaic must be a 6-tile cubic grid for 12 contacts.");
	    *opcode |= CUBIC_GRID;
	 }
	 else
	    mpp_error("river_regrid: the number of contact should be either 0, 1, 2 or 6");
      }

      free(tile1);
      free(tile2);
      free(istart1);
      free(iend1);
      free(jstart1);
      free(jend1);
      free(istart2);
      free(iend2);
      free(jstart2);
      free(jend2);
   }

   /* update halo of xt and yt */
   pxt = (double **)malloc(ntiles*sizeof(double *));
   pyt = (double **)malloc(ntiles*sizeof(double *));
   for(n=0; n<ntiles; n++) {
      pxt[n] = river_data[n].xt;
      pyt[n] = river_data[n].yt;
   }
   update_halo_double(ntiles, pxt, nx, ny, *opcode);
   update_halo_double(ntiles, pyt, nx, ny, *opcode);
   free(pxt);
   free(pyt);
  
}; /* get_mosaic_grid */

/*------------------------------------------------------------------------------
  void init_river_data
  allocate memory to river data and initialize these river data with missing value
  ----------------------------------------------------------------------------*/
void init_river_data(int ntiles, river_type *river_out, const river_type * const river_in)
{
   int nx, ny, nxp2, nyp2, n, i;
   int tocell_missing, subA_missing, travel_missing, basin_missing;
   double cellarea_missing, celllength_missing;
  
   nx = river_out->nx;
   ny = river_out->ny;
   nxp2 = nx + 2;
   nyp2 = ny + 2;
   subA_missing   = river_in->subA_missing;
   tocell_missing = river_in->tocell_missing;
   travel_missing = river_in->travel_missing;
   basin_missing  = river_in->basin_missing;
   cellarea_missing = river_in->cellarea_missing;
   celllength_missing = river_in->celllength_missing;
   for(n=0; n<ntiles; n++) {
      river_out[n].subA_missing   = subA_missing;
      river_out[n].tocell_missing = tocell_missing;
      river_out[n].travel_missing = travel_missing;
      river_out[n].basin_missing  = basin_missing;
      river_out[n].cellarea_missing = cellarea_missing;
      river_out[n].celllength_missing  = celllength_missing;
      river_out[n].subA       = (double *)malloc(nxp2*nyp2*sizeof(double));
      river_out[n].tocell     = (int    *)malloc(nxp2*nyp2*sizeof(int   ));
      river_out[n].travel     = (int    *)malloc(nxp2*nyp2*sizeof(int   ));
      river_out[n].basin      = (int    *)malloc(nxp2*nyp2*sizeof(int   ));
      river_out[n].dir        = (int    *)malloc(nxp2*nyp2*sizeof(int   ));
      river_out[n].cellarea   = (double *)malloc(nx  *ny  *sizeof(double));
      river_out[n].celllength = (double *)malloc(nx  *ny  *sizeof(double));
      river_out[n].last_point = (int    *)malloc(nx  *ny  *sizeof(int   ));
      for(i=0; i<nxp2*nyp2; i++) {
	 river_out[n].subA  [i] = subA_missing;
	 river_out[n].travel[i] = travel_missing;
	 river_out[n].basin [i] = basin_missing;
	 river_out[n].tocell[i] = tocell_missing;
	 river_out[n].dir[i]    = -1;
      }
      for(i=0; i<nx*ny; i++) {
	 river_out[n].cellarea[i]   = cellarea_missing;
	 river_out[n].celllength[i] = celllength_missing;
	 river_out[n].last_point[i] =  0;
      }
   }

}


/*------------------------------------------------------------------------------
  void calc_max_subA(const river_type *river_in, river_type *river_out, int ntiles)
  find max value of suba in each model grid cell.
  ----------------------------------------------------------------------------*/
void calc_max_subA(const river_type *river_in, river_type *river_out, int ntiles, unsigned int opcode)
{
   int nx_in, ny_in;
   int nx_out, ny_out, nxp_out, nxp1, nyp1, nxp2, nyp2;
   int n, i, j, i1, j1, ii, jj, n_out;
   double missing, ll_y, ur_y, ll_x, ur_x;
   double xv1[4], yv1[4], xv2[20], yv2[20];
   double *xb_in, *yb_in;
   double *xb_out, *yb_out;
   double **psubA;
  
   nx_in = river_in->nx;
   ny_in = river_in->ny;
   xb_in = river_in->xb_r;
   yb_in = river_in->yb_r;
   missing = river_out->subA_missing;
  
   for(n=0; n<ntiles; n++) {
      nx_out   = river_out[n].nx;
      ny_out   = river_out[n].ny;
      nxp2     = nx_out + 2;
      nyp2     = ny_out + 2;    
      nxp_out  = nx_out+1;
      xb_out   = river_out[n].xb_r;
      yb_out   = river_out[n].yb_r;
      for(j=0; j<ny_out; j++) for(i=0; i<nx_out; i++) {
	    j1 = j+1;
	    i1 = i+1;
	    /* set partial land cell and ocean cell to have large subA value */
	    if(river_out[n].landfrac[j*nx_out+i] < 1) {
	       river_out[n].subA[j1*nxp2+i1] = LARGE_VALUE;
	       continue;
	    }
      
	    xv1[0] = xb_out[j*nxp_out+i];
	    xv1[1] = xb_out[j*nxp_out+i1];
	    xv1[2] = xb_out[j1*nxp_out+i1];
	    xv1[3] = xb_out[j1*nxp_out+i]; 
	    yv1[0] = yb_out[j*nxp_out+i];
	    yv1[1] = yb_out[j*nxp_out+i1];
	    yv1[2] = yb_out[j1*nxp_out+i1];
	    yv1[3] = yb_out[j1*nxp_out+i]; 
      
	    for(jj=0; jj<ny_in; jj++) {
	       ll_y = yb_in[jj];
	       ur_y = yb_in[jj+1];
	       if (  (yv1[0]<=ll_y) && (yv1[1]<=ll_y)
	       && (yv1[2]<=ll_y) && (yv1[3]<=ll_y) ) continue;
	       if (  (yv1[0]>=ur_y) && (yv1[1]>=ur_y)
	       && (yv1[2]>=ur_y) && (yv1[3]>=ur_y) ) continue;
	
	       for(ii=0; ii<nx_in; ii++) {
		  if(river_in->subA[jj*nx_in+ii] == missing) continue;
		  ll_x = xb_in[ii];
		  ur_x = xb_in[ii+1];
		  /* adjust xv1 to make sure it is in the range as ll_x and ur_x */
		  adjust_lon(xv1, 0.5*(ll_x + ur_x) );
		  if ( (xv1[0]<=ll_x) && (xv1[1]<=ll_x) && (xv1[2]<=ll_x) && (xv1[3]<=ll_x) ) continue;
		  if ( (xv1[0]>=ur_x) && (xv1[1]>=ur_x) && (xv1[2]>=ur_x) && (xv1[3]>=ur_x) ) continue;	  
		  if ( (n_out = clip ( xv1, yv1, 4, ll_x, ll_y, ur_x, ur_y, xv2, yv2 )) > 0 ) {
		     double xarea; 
		     xarea = poly_area (xv2, yv2, n_out );
		     if( xarea/river_out[n].area[j*nx_out+i] < MIN_AREA_RATIO ) continue;
		     if(river_in->subA[jj*nx_in+ii] > river_out[n].subA[j1*nxp2+i1])
			river_out[n].subA[j1*nxp2+i1] = river_in->subA[jj*nx_in+ii];
		  }
	       }
	    }
	 }
   }
	
   /* fill the halo of subA */
   psubA = (double **)malloc(ntiles*sizeof(double *));
   for(n=0; n<ntiles; n++) psubA[n] = river_out[n].subA; 
   update_halo_double(ntiles, psubA, river_out->nx, river_out->ny, opcode); 
   free(psubA);
};/* calc_max_subA */

/*------------------------------------------------------------------------------
  void update_halo_double(int ntiles, double *data, int nx, int ny, unsigned int opcode)
  We assume all the tiles have the same size.
  -----------------------------------------------------------------------------*/
void update_halo_double(int ntiles, double **data, int nx, int ny, unsigned int opcode)
{
   int nxp1, nyp1, nxp2, i, j, n;
   int te, tw, ts, tn;
  
   nxp1   = nx + 1;
   nyp1   = ny + 1;
   nxp2   = nx + 2;
   if(opcode & X_CYCLIC) {
      for(j=1; j<nyp1; j++) {
	 data[0][j*nxp2]      = data[0][j*nxp2+nx];     /* West */
	 data[0][j*nxp2+nxp1] = data[0][j*nxp2+1];      /* east */
      }
   }
   if(opcode & Y_CYCLIC) {
      for(i=1; i<nyp1; i++) {
	 data[0][i]           = data[0][ny*nxp2+i];     /* south */
	 data[0][nyp1*nxp2+i] = data[0][nxp2+i];        /* north */
      }
   }
   if(opcode & X_CYCLIC && opcode & Y_CYCLIC) {
      data[0][0]              = data[0][ny*nxp2];      /* southwest */
      data[0][nxp1]           = data[0][ny*nxp2+1];    /* southeast */    
      data[0][nyp1*nxp2+nxp1] = data[0][nxp2+1];       /* northeast */
      data[0][nyp1*nxp2]      = data[0][nxp2+nx];      /* northwest */     
   }

   if(opcode & CUBIC_GRID) {
      for(n=0; n<ntiles; n++) {
      
	 if(n%2) { /* tile 2 4 6 */
	    tw = (n+5)%ntiles; te = (n+2)%ntiles; ts = (n+4)%ntiles; tn = (n+1)%ntiles;
	    for(j=1; j<nyp1; j++) {
	       data[n][j*nxp2]      = data[tw][j*nxp2+nx];     /* west */
	       data[n][j*nxp2+nxp1] = data[te][nxp2+(nxp1-j)]; /* east */
	    }
	    for(i=1; i<nxp1; i++) {
	       data[n][i]           = data[ts][(nyp1-i)*nxp2+nx]; /* south */
	       data[n][nyp1*nxp2+i] = data[tn][nxp2+i];           /* north */
	    }
	 } else {  /* tile 1, 3, 5 */
	    tw = (n+4)%ntiles; te = (n+1)%ntiles; ts = (n+5)%ntiles; tn = (n+2)%ntiles;
	    for(j=1; j<nyp1; j++) {
	       data[n][j*nxp2]      = data[tw][ny*nxp2+nxp1-j]; /* west */
	       data[n][j*nxp2+nxp1] = data[te][j*nxp2+1];       /* east */
	    }
	    for(i=1; i<nxp1; i++) {
	       data[n][i]           = data[ts][ny*nxp2+i];       /* south */
	       data[n][nyp1*nxp2+i] = data[tn][(nyp1-i)*nxp2+1]; /* north */
	    }
	 }
      }
   }
  
};/* update_halo */

void update_halo_int(int ntiles, int **data, int nx, int ny, unsigned int opcode)
{
   double **ldata;
   int n, i, j, nxp2, nyp2;
  
   nxp2 = nx + 2;
   nyp2 = ny + 2;


  
   ldata = (double **)malloc(ntiles*sizeof(double *));
   for(n=0; n<ntiles; n++) {
      ldata[n] =  (double *)malloc(nxp2*nyp2*sizeof(double));
      for(i=0; i<nxp2*nyp2; i++) ldata[n][i] = data[n][i];
   }
   update_halo_double(ntiles, ldata, nx, ny, opcode);
   for(n=0; n<ntiles; n++) {
      for(i=0; i<nxp2*nyp2; i++) data[n][i] = ldata[n][i];
      free(ldata[n]);
   }
   free(ldata);
  
}

int adjust_lon(double x[], double tlon)
{
   double x_sum, dx;
   int i, npts = 4;

   x_sum = x[0];
   for (i=1;i<npts;i++) {
      double dx = x[i]-x[i-1];

      if      (dx < -M_PI) dx = dx + 2*M_PI;
      else if (dx >  M_PI) dx = dx - 2*M_PI;
      x_sum += (x[i] = x[i-1] + dx);
   }

   dx = (x_sum/npts)-tlon;
   if      (dx < -M_PI) for (i=0;i<npts;i++) x[i] += 2*M_PI;
   else if (dx >  M_PI) for (i=0;i<npts;i++) x[i] -= 2*M_PI;

}; /* adjust_lon */

/*------------------------------------------------------------------------------
  Find the tocell value for the new grid.
  void calc_tocell( )
  ----------------------------------------------------------------------------*/
void calc_tocell(int ntiles, river_type *river_data, unsigned int opcode )  
{
   const int out_flow[] = { 8, 4, 2, 16, -1, 1, 32, 64, 128};
   const int out_dir[]  = { 3, 2, 1, 4,  -1, 0, 5,   6,   7};
   int n, nx, ny, nxp2, ioff, joff;
   int i, j, ii, jj, iget, jget, im1, jm1;
   double tval, subA_missing, subA, subA_me;
   int **ptocell, **pdir;
  
   nx = river_data->nx;
   ny = river_data->ny;
   nxp2 = nx + 2;
   subA_missing = river_data->subA_missing;
   ptocell = (int **)malloc(ntiles*sizeof(int *));
   pdir    = (int **)malloc(ntiles*sizeof(int *));
   for(n=0; n<ntiles; n++) {
      ptocell[n] = river_data[n].tocell;
      pdir   [n] = river_data[n].dir;
   }
  
   for(n=0; n<ntiles; n++) {
      for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {
	    jm1 = j - 1;
	    im1 = i - 1;
	    if(river_data[n].landfrac[jm1*nx+im1] == 0) {
	       /* do nothing */
	    }
	    else if(river_data[n].landfrac[jm1*nx+im1] < 1) {
	       ptocell[n][j*nxp2+i] = 0;
	    }
	    else {
	       subA_me = river_data[n].subA[j*nxp2+i];
	       tval= -999 ;  iget= -1 ;  jget= -1;
	       if(subA_me > suba_cutoff ) { /* tocell will be land cell with larger subA value, instead of neighboring ocean cell. */
		  for(joff=0; joff<ncells; joff++) {
		     jj = jm1+joff;
		     for(ioff=0; ioff<ncells; ioff++) {
			ii = im1+ioff;
			if(ioff == 1 && joff == 1) continue;
			subA = river_data[n].subA[jj*nxp2+ii];
			if(subA > subA_me) {
			   if(tval == -999 ) {
			      if(subA > tval) {
				 iget = ioff;
				 jget = joff;
				 tval = subA;
			      }
			   }
			   else if(tval < LARGE_VALUE) {
			      if(subA > tval && subA < LARGE_VALUE) {
				 iget = ioff;
				 jget = joff;
				 tval = subA;
			      }
			   }
			   else {
			      iget = ioff;
			      jget = joff;
			      tval = subA;
			   }
			}
		     }
		  }
	       }
	       else {
		  for(joff=0; joff<ncells; joff++) {
		     jj = jm1+joff;
		     for(ioff=0; ioff<ncells; ioff++) {
			ii = im1+ioff;
			if(ioff == 1 && joff == 1) continue;

			subA = river_data[n].subA[jj*nxp2+ii];
			if( subA != subA_missing ) {
			   if( subA >= tval) {
			      iget = ioff;
			      jget = joff;
			      tval = subA;
			   }
			}
		     }
		  }
	       }
	       if(iget >= 0 && jget >= 0) {
		  if(tval > river_data[n].subA[j*nxp2+i]) {
		     ptocell[n][j*nxp2+i] = out_flow[jget*ncells+iget];
		     pdir   [n][j*nxp2+i] = out_dir [jget*ncells+iget];
		     if(pow(2,pdir[n][j*nxp2+i]) != ptocell[n][j*nxp2+i] )
			mpp_error("river_regrid: pow(2,dir) should equal to tocell");
		  }
		  else
		     ptocell[n][j*nxp2+i] = 0;
	       }
	       else 
		  ptocell[n][j*nxp2+i] = 0;
	    }
	 }
   }
   update_halo_int(ntiles, ptocell, nx, ny, opcode);   
   update_halo_int(ntiles, pdir, nx, ny, opcode);
   free(ptocell);
   free(pdir);
  
};/* calc_tocell */


/*------------------------------------------------------------------------------
  void calc_river_data()
  calculate travel and basin according to tocell, as well as celllength, cellarea.
  For each basin, one and only one river point will have tocell = 0, this point will
  have travel = 1.
  ----------------------------------------------------------------------------*/
void calc_river_data(int ntiles, river_type* river_data, unsigned int opcode )
{
   int    nx, ny, nxp2, nyp2, n, i, j, ii, jj, nxp, nyp, im1, jm1, ioff, joff;
   int    cur_basin, cur_travel;
   int    not_done, dir, i_dest, j_dest;
   int    basin_missing, travel_missing, tocell_missing;
   int    maxtravel, maxbasin, travelnow;
   double subA_missing;
   int    **pbasin, **ptravel, **pdir;
   double **psubA;
   double xv[4], yv[4];
  
   const int di[] = {1,1,0,-1,-1,-1,0,1};
   const int dj[] = {0,-1,-1,-1,0,1,1,1};
   nx = river_data->nx;
   ny = river_data->ny;
   nxp  = nx+1;
   nyp  = ny+1;
   nxp2 = nx+2;
   nyp2 = ny+2;
   basin_missing  = river_data->basin_missing;
   subA_missing   = river_data->subA_missing;
   tocell_missing = river_data->tocell_missing;
   cur_basin = 0;

   /* set up pointer */
   pbasin  = (int **)malloc(ntiles*sizeof(int *));
   ptravel = (int **)malloc(ntiles*sizeof(int *));
   pdir    = (int **)malloc(ntiles*sizeof(int *));
   psubA   = (double **)malloc(ntiles*sizeof(double *));
   for(n=0; n<ntiles; n++) {
      pbasin [n] = river_data[n].basin;
      ptravel[n] = river_data[n].travel;
      psubA  [n] = river_data[n].subA;
      pdir   [n] = river_data[n].dir;
   }


   /* reinitialize subA */
   for(n=0; n<ntiles; n++) {
      for(i=0; i<nxp2*nyp2; i++) psubA[n][i] = subA_missing;    
   }
  
   /* calculate celllength and cellarea */
   for(n=0; n<ntiles; n++) {
      for(j=0; j<ny; j++) for(i=0; i<nx; i++) {
	    if(river_data[n].landfrac[j*nx+i] > 0) {
	       ii = i+1;
	       jj = j+1;

	       xv[0] = river_data[n].xb_r[j*nxp+i];
	       xv[1] = river_data[n].xb_r[j*nxp+ii];
	       xv[2] = river_data[n].xb_r[jj*nxp+ii];
	       xv[3] = river_data[n].xb_r[jj*nxp+i];
	       yv[0] = river_data[n].yb_r[j*nxp+i];
	       yv[1] = river_data[n].yb_r[j*nxp+ii];
	       yv[2] = river_data[n].yb_r[jj*nxp+ii];
	       yv[3] = river_data[n].yb_r[jj*nxp+i];
	       river_data[n].cellarea[j*nx+i] = river_data[n].area[j*nx+i]*river_data[n].landfrac[j*nx+i];
	       dir = pdir[n][jj*nxp2+ii];
	       if( dir >= 0) {
		  i_dest = ii + di[dir];
		  j_dest = jj + dj[dir];	
		  river_data[n].celllength[j*nx+i] = distance(river_data[n].xt[jj*nxp2+ii],
		  river_data[n].yt[jj*nxp2+ii],
		  river_data[n].xt[j_dest*nxp2+i_dest],
		  river_data[n].yt[j_dest*nxp2+i_dest] );
	       }
	       else
		  river_data[n].celllength[j*nx+i] = 0;
	    }
	 }
   }

  
   /* define the basinid and travel for the coast point */
  
   for(n=0; n<ntiles; n++) {
      for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {
	    if(river_data[n].tocell[j*nxp2+i] == 0) {
	       pbasin[n] [j*nxp2+i] = ++cur_basin;
	       ptravel[n][j*nxp2+i] = 0;
	       river_data[n].last_point[(j-1)*nx+i-1] = 1;
	    }
	    else if(river_data[n].tocell[j*nxp2+i] > 0){
	       dir = pdir[n][j*nxp2+i];
	       i_dest = i + di[dir];
	       j_dest = j + dj[dir];
	       if(river_data[n].tocell[j_dest*nxp2+i_dest] == tocell_missing) {
		  pbasin[n] [j*nxp2+i] = ++cur_basin;
		  ptravel[n][j*nxp2+i] = 1;
		  river_data[n].last_point[(j-1)*nx+i-1] = 1;
	       }
	    }
	 }
   }

   update_halo_int(ntiles, pbasin, nx, ny, opcode);
   update_halo_int(ntiles, ptravel, nx, ny, opcode);
  
   /* then define the travel and basin for all other points */
   cur_travel = 0;

   not_done = 1;
   while(not_done) {
      not_done = 0;
      for(n=0; n<ntiles; n++) {
	 for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {
	       dir = pdir[n][j*nxp2+i];
	       if( dir >= 0 && pbasin[n][j*nxp2+i] == basin_missing ) {
		  i_dest = i + di[dir];
		  j_dest = j + dj[dir];
		  if( ptravel[n][j_dest*nxp2+i_dest] == cur_travel ) {
		     not_done = 1; /* still have points need to be updateed */
		     if(pbasin[n][j_dest*nxp2+i_dest] == basin_missing) {
			mpp_error("river_grid: the tocell should have valid basin value");
		     }
		     pbasin[n][j*nxp2+i] = pbasin[n][j_dest*nxp2+i_dest];
		     ptravel[n][j*nxp2+i] = cur_travel+1;
		  }
	       }
	    }
      }
      cur_travel++;
      update_halo_int(ntiles, pbasin, nx, ny, opcode);
      update_halo_int(ntiles, ptravel, nx, ny, opcode);    
   }

   /* figure out maximum travel and maximum basin*/
   maxtravel = -1;
   maxbasin  = -1;
   basin_missing = river_data->basin_missing;
   for(n=0; n<ntiles; n++) {
      for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {    
	    maxtravel = max(maxtravel, ptravel[n][j*nxp2+i]);
	    maxbasin  = max(maxbasin,  pbasin[n][j*nxp2+i]);
	 }
   }

   for(travelnow=maxtravel; travelnow>=0; travelnow--) {
      for(n=0; n<ntiles; n++) for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {
	       jm1 = j - 1;
	       im1 = i - 1;
	       if(ptravel[n][j*nxp2+i] == travelnow) {
		  psubA[n][j*nxp2+i] = river_data[n].cellarea[jm1*nx+im1];
		  /* add the subA of from cell */
		  for(joff=0; joff<ncells; joff++) for(ioff=0; ioff<ncells; ioff++) {
			if(ioff == 1 && joff == 1) continue;
			jj = jm1 + joff;
			ii = im1 + ioff;
			dir = pdir[n][jj*nxp2+ii];
			/* consider about the rotation when it is on the boundary */
			if(dir >=0 ) {
			   if( opcode & CUBIC_GRID ) {
			      if(ii == 0) { /* west */ 
				 if(n%2==0) dir = (dir+2)%8;  /* tile 1 3 5 */
			      }
			      else if(ii == nxp) { /*east */
				 if(n%2==1) dir = (dir+2)%8;  /* tile 2 4 6 */
			      }
			      else if(jj == 0) { /* south */
				 if(n%2==1) dir = (dir+6)%8;  /* tile 2 4 6 */
			      }
			      else if(jj == nyp) { /*north */
				 if(n%2==0) dir = (dir+6)%8;  /* tile 1 3 5 */
			      }
			   }
			   i_dest = ii + di[dir];
			   j_dest = jj + dj[dir];
			   if(i_dest == i && j_dest == j) psubA[n][j*nxp2+i] += psubA[n][jj*nxp2+ii];
			}
		     }
	       }
	    }
      update_halo_double(ntiles, psubA, nx, ny, opcode);
   }
  
   free(pbasin);
   free(ptravel);
   free(psubA);
   free(pdir);
}; /*  calc_travel */

/*------------------------------------------------------------------------------
  void check_river_data( )
  check to make sure all the river points have been assigned travel and basin value
  and all the ocean points will have missing value.
  ----------------------------------------------------------------------------*/
  
void check_river_data(int ntiles, river_type *river_data )
{
   int maxtravel = -1;
   int maxbasin  = -1;
   int nx, ny, nxp2, nyp2;
   int n, i, j, im1, jm1, ioff, joff, ii, jj;
   int tocell, travel, basin;
   int tocell_missing, travel_missing, basin_missing;
   int ncoast_full_land, nsink;
   double subA, subA_missing;
   int *ncoast;
  
   /* print out maximum travel and number of rivers */
   maxtravel = -1;
   maxbasin  = -1;
   nx = river_data->nx;
   ny = river_data->ny;
   nxp2 = nx + 2;
   nyp2 = ny + 2;
   subA_missing = river_data->subA_missing;
   tocell_missing = river_data->tocell_missing;
   basin_missing = river_data->basin_missing;
   travel_missing = river_data->travel_missing;
  
   for(n=0; n<ntiles; n++) {
      for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {    
	    maxtravel = max(maxtravel, river_data[n].travel[j*nxp2+i]);
	    maxbasin  = max(maxbasin, river_data[n].basin[j*nxp2+i]);
	 }
   }
   printf("==> NOTE from river_regrid: maximum travel is %d and maximum basin is %d.\n", maxtravel, maxbasin);

   ncoast_full_land = 0;
   nsink = 0;
   for(n=0; n<ntiles; n++) {
      for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {
	    jm1 = j - 1;
	    im1 = i -1;
	    subA   = river_data[n].subA  [j*nxp2+i];
	    tocell = river_data[n].tocell[j*nxp2+i];
	    travel = river_data[n].travel[j*nxp2+i];
	    basin  = river_data[n].basin [j*nxp2+i];
	    if( river_data[n].landfrac[jm1*nx+im1] == 0) {
	       if(tocell!= tocell_missing || travel != travel_missing ||
	       basin != basin_missing || subA != subA_missing) {
		  printf("At ocean points (i=%d,j=%d), subA = %f, tocell = %d, travel = %d, basin = %d.\n ",
		  i, j, subA, tocell, travel, basin);
		  mpp_error("river_regrid, subA, tocell, travel, or basin is not missing value for some ocean points");
	       }
	    }
	    else {
	       if(tocell == tocell_missing || travel == travel_missing ||
	       basin == basin_missing || subA == subA_missing) {
		  printf("At river points (i=%d,j=%d), subA = %f, tocell = %d, travel = %d, basin = %d.\n ",
		  i, j, subA, tocell, travel, basin);
		  mpp_error("river_regrid, subA, tocell, travel, or basin is missing value for some river points");
	       }
	    }
	    /* check if the points with land_frac == 1 and tocell=0, those points should be sink.*/
	    if(river_data[n].landfrac[jm1*nx+im1] == 1 && tocell == 0) {
	       for(joff=0; joff<ncells; joff++) {
		  jj = jm1+joff;
		  for(ioff=0; ioff<ncells; ioff++) {
		     ii = im1+ioff;
		     if(ioff == 1 && joff == 1) continue;
		     if(river_data[n].tocell[jj*nxp2+ii] == tocell_missing) {
			ncoast_full_land++;
			printf("At point (%d,%d), tocell = 0 and landfrac = 1 is a coast point\n", i, j);
			goto done_check;
		     }
		  }
	       }
	       printf("At point (%d,%d), tocell = 0 and landfrac = 1 is a sink point\n", i, j);
	       nsink++;
	      done_check: continue;	
	    }
	 }
   }

   if(ncoast_full_land > 0)
      printf("Warning from river_regrid: there are %d coast points is a full land cell\n", ncoast_full_land);
   else
      printf("NOTE from river_regrid: there are no coast points is a full land cell\n");

   if(nsink > 0)
      printf("Warning from river_regrid: there are %d sink points is a full land cell\n", nsink);
   else
      printf("NOTE from river_regrid: there are no sink points is a full land cell\n");
  
   /* check river travel to make sure there is one and only one point with travel = 0. */
   ncoast = (int *)malloc(maxbasin*sizeof(int));
   for(n=0; n<maxbasin; n++) ncoast[n] = 0;
   for(n=0; n<ntiles; n++) {
      for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {
	    if(river_data[n].last_point[(j-1)*nx+i-1] ) ++ncoast[river_data[n].basin[j*nxp2+i]-1];
	 }
   }
   for(n=0; n<maxbasin; n++) {
      if(ncoast[n] <= 0) {
	 printf("river with basin = %d has no point with travel = 0.\n", n+1);
	 mpp_error("river_regrid: some river has no point with travel = 0");
      }
      else if(ncoast[n] >1) {
	 printf("river with basin = %d has more than one point with travel = 0.\n", n+1);
	 mpp_error("river_regrid: some river has more than one point with travel = 0");
      }
   }
   free(ncoast);
  

}; /* check_river_data */

/*------------------------------------------------------------------------------
  void sort_basin(int ntiles, river_type* river_data);
  sorting the basin according to subA
  The river with larger subA at coast point will get smaller basinid.
  The basinid will be reorganized.
  -----------------------------------------------------------------------------*/
void sort_basin(int ntiles, river_type* river_data)
{

   int    nx, ny, nxp2, nyp2, i, j, n;
   int    maxbasin, basin_missing, basin;
   int    *rank, *indx;
   double subA_missing, *maxsuba;
  
   river_type *river_tmp;

   nx   = river_data->nx;
   ny   = river_data->ny;
   nxp2 = nx + 2;
   nyp2 = ny + 2;
   river_tmp = (river_type *)malloc(ntiles*sizeof(river_type));

   /* calculate maximum basin */
   maxbasin  = -1;
   basin_missing = river_data->basin_missing;
   for(n=0; n<ntiles; n++) {
      for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {    
	    maxbasin  = max(maxbasin,  river_data[n].basin[j*nxp2+i]);
	 }
   }  

   /* copy basinid data to the tmp data */
   for(n=0; n<ntiles; n++) {
      river_tmp[n].basin = (int *)malloc(nxp2*nyp2*sizeof(int));
      for(i=0; i<nxp2*nyp2; i++) {
	 river_tmp[n].basin[i] = river_data[n].basin[i];
      }
   }

   /* calculate maximum subA for each basin */
   subA_missing = river_data->subA_missing;
   maxsuba = (double *)malloc(maxbasin*sizeof(double));
   indx    = (int    *)malloc(maxbasin*sizeof(int   ));
   rank    = (int    *)malloc(maxbasin*sizeof(int   ));
   for(n=0; n<maxbasin; n++) {
      maxsuba[n] = subA_missing;
      indx[n]    = n;
      rank[n]    = -1;
   }
   for(n=0; n<ntiles; n++) for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {
	    if(river_data[n].last_point[(j-1)*nx+i-1]) { /* coast point */
	       basin = river_tmp[n].basin[j*nxp2+i];
	       if( basin > maxbasin || basin < 1) mpp_error("river_regrid: basin should be between 1 and maxbasin");
	       maxsuba[basin-1] = river_data[n].subA[j*nxp2+i];
	    }
	 }

   /* make sure maxsuba is assigned properly */
   for(n=0; n<maxbasin; n++) {
      if(maxsuba[n] == subA_missing) mpp_error("river_regrid: maxsuba is not assigned for some basin");
   }
  
   /* sort maxsuba to get the index rank */
   qsort_index(maxsuba, 0, maxbasin-1, indx);
   for(n=0; n<maxbasin; n++) rank[indx[n]] = n;
   for(n=0; n<maxbasin; n++) {
      if(rank[n] < 0) mpp_error("river_regrid: rank should be no less than 0");
   }
  
   /* now assign basin according to the index rank */
   for(n=0; n<ntiles; n++) for(j=1; j<=ny; j++) for(i=1; i<=nx; i++) {
	    basin = river_tmp[n].basin[j*nxp2+i];
	    if(basin != basin_missing) {
	       if( basin > maxbasin || basin < 1) mpp_error("river_regrid: basin should be a positive integer no larger than maxbasin");
	       river_data[n].basin[j*nxp2+i] = maxbasin - rank[basin-1];
	    }
	 }
  
   /* release the memory */
   for(n=0; n<ntiles; n++) free(river_tmp[n].basin);
   free(river_tmp);
   free(rank);
   free(maxsuba);

};/* sort_basin */



/*------------------------------------------------------------------------------
  void write_river_data()
  write out river network output data which is on land grid.
  ----------------------------------------------------------------------------*/
void write_river_data(const char *river_src_file, const char *output_file, river_type* river_data, const char *history, int ntiles)
{
   double *subA, *yt, *xt;
   int    *tocell, *travel, *basin;
   int id_subA, id_subA_in, id_tocell, id_tocell_in;
   int id_travel, id_travel_in, id_basin, id_basin_in;
   int id_cellarea, id_celllength, id_landfrac;
   int id_cellarea_in, id_celllength_in, id_x, id_y;
   int id_xaxis, id_yaxis, id_xaxis_in, id_yaxis_in;
   int fid, fid_in, dimid[2];
   int n, i, j, ii, jj, nx, ny, nxp2;
  
   fid_in = mpp_open(river_src_file, MPP_READ);
   id_subA_in = mpp_get_varid(fid_in, subA_name);
   id_tocell_in = mpp_get_varid(fid_in, tocell_name);
   id_travel_in = mpp_get_varid(fid_in, travel_name);
   id_basin_in = mpp_get_varid(fid_in, basin_name);
   id_cellarea_in = mpp_get_varid(fid_in, cellarea_name);
   id_celllength_in = mpp_get_varid(fid_in, celllength_name);
   id_xaxis_in = mpp_get_varid(fid_in, xaxis_name);
   id_yaxis_in = mpp_get_varid(fid_in, yaxis_name);
   for(n=0; n<ntiles; n++) {
      if(ntiles>1)
	 sprintf(river_data[n].filename, "%s.tile%d.nc", output_file, n+1);
      else
	 sprintf(river_data[n].filename, "%s.nc", output_file);

      nx = river_data[n].nx;
      ny = river_data[n].ny;
      nxp2 = nx + 2;
      fid = mpp_open(river_data[n].filename, MPP_WRITE);
      mpp_def_global_att(fid, "version", version);  
      mpp_def_global_att(fid, "code_version", TAGNAME);
      mpp_def_global_att(fid, "history", history);
      dimid[1] = mpp_def_dim(fid, gridx_name, nx);
      dimid[0] = mpp_def_dim(fid, gridy_name, ny);
      id_xaxis = mpp_def_var(fid, gridx_name, MPP_DOUBLE, 1, &(dimid[1]), 0);
      id_yaxis = mpp_def_var(fid, gridy_name, MPP_DOUBLE, 1, dimid, 0);
      mpp_copy_var_att(fid_in, id_xaxis_in, fid, id_xaxis);
      mpp_copy_var_att(fid_in, id_yaxis_in, fid, id_yaxis);
      id_subA = mpp_def_var(fid, subA_name, MPP_DOUBLE, 2, dimid , 0);
      mpp_copy_var_att(fid_in, id_subA_in, fid, id_subA);
      id_tocell = mpp_def_var(fid, tocell_name, MPP_INT, 2, dimid, 0);
      mpp_copy_var_att(fid_in, id_tocell_in, fid, id_tocell);
      id_travel = mpp_def_var(fid, travel_name, MPP_INT, 2, dimid , 0);
      mpp_copy_var_att(fid_in, id_travel_in, fid, id_travel);
      id_basin = mpp_def_var(fid, basin_name, MPP_INT, 2, dimid , 0);
      mpp_copy_var_att(fid_in, id_basin_in, fid, id_basin);
      id_cellarea = mpp_def_var(fid, cellarea_name, MPP_DOUBLE, 2, dimid , 0);
      mpp_copy_var_att(fid_in, id_cellarea_in, fid, id_cellarea);
      id_celllength = mpp_def_var(fid, celllength_name, MPP_DOUBLE, 2, dimid , 0);
      mpp_copy_var_att(fid_in, id_celllength_in, fid, id_celllength);
      id_landfrac = mpp_def_var(fid, landfrac_name, MPP_DOUBLE, 2, dimid, 2,
      "long_name", "land fraction", "units", "none");
      id_x        = mpp_def_var(fid, x_name, MPP_DOUBLE, 2, dimid, 2,
      "long_name", "Geographic longitude", "units", "degree_east");
      id_y        = mpp_def_var(fid, y_name, MPP_DOUBLE, 2, dimid, 2,
      "long_name", "Geographic latitude", "units", "degree_north");    
      mpp_end_def(fid);
      xt = (double *)malloc(nx*sizeof(double));
      yt = (double *)malloc(ny*sizeof(double));
      /*
	for lat-lon grid, actual lon, lat will be written out,
	for cubic grid, index will be written out
      */
      if(ntiles == 1) {
	 for(i=0; i<nx; i++) xt[i] = river_data[n].xt[nxp2+i+1];
	 for(j=0; j<ny; j++) yt[j] = river_data[n].yt[(j+1)*nxp2+1];

      }
      else {
	 for(i=0; i<nx; i++) xt[i] = i+1;
	 for(j=0; j<ny; j++) yt[j] = j+1;
      }
      mpp_put_var_value(fid, id_xaxis, xt);
      mpp_put_var_value(fid, id_yaxis, yt);
      free(xt);
      free(yt);

      /* all the fields is on data domain, so need to copy to compute domain */
      subA = (double *)malloc(nx*ny*sizeof(double));
      travel = (int *)malloc(nx*ny*sizeof(int));
      basin  = (int *)malloc(nx*ny*sizeof(int));
      tocell = (int *)malloc(nx*ny*sizeof(int));
      xt = (double *)malloc(nx*ny*sizeof(double));
      yt = (double *)malloc(nx*ny*sizeof(double));
      for(j=0; j<ny; j++) for(i=0; i<nx; i++) {
	    ii = i + 1;
	    jj = j + 1;
	    subA[j*nx+i] = river_data[n].subA[jj*nxp2+ii];
	    tocell[j*nx+i] = river_data[n].tocell[jj*nxp2+ii];
	    travel[j*nx+i] = river_data[n].travel[jj*nxp2+ii];
	    basin [j*nx+i] = river_data[n].basin [jj*nxp2+ii];
	    xt    [j*nx+i] = river_data[n].xt    [jj*nxp2+ii];
	    yt    [j*nx+i] = river_data[n].yt    [jj*nxp2+ii];
	 }
    
      mpp_put_var_value(fid, id_subA, subA);
      mpp_put_var_value(fid, id_tocell, tocell);
      mpp_put_var_value(fid, id_travel, travel);
      mpp_put_var_value(fid, id_basin, basin);
      mpp_put_var_value(fid, id_x, xt);
      mpp_put_var_value(fid, id_y, yt);
      mpp_put_var_value(fid, id_cellarea, river_data[n].cellarea);
      mpp_put_var_value(fid, id_celllength, river_data[n].celllength);
      mpp_put_var_value(fid, id_landfrac, river_data[n].landfrac);
      mpp_close(fid);
      free(subA);
      free(tocell);
      free(basin);
      free(travel);
      free(xt);
      free(yt);
   }
   mpp_close(fid_in);

};/* write_river_data */


/*------------------------------------------------------------------------------
  double distance(double lon1, double lat1, double lon2, double lat2)
  find the distance of any two grid.
  ----------------------------------------------------------------------------*/
double distance(double lon1, double lat1, double lon2, double lat2)
{
   double s1, s2, dx;
   double dist;
    
   dx = lon2 - lon1;
   if(dx > 180.) dx = dx - 360.;
   if(dx < -180.) dx = dx + 360.;

   if(lon1 == lon2)
      dist = fabs(lat2 - lat1) * D2R;
   else if(lat1 == lat2) 
      dist = fabs(dx) * D2R * cos(lat1*D2R);
   else {   /* diagonal distance */
      s1 =  fabs(dx)  * D2R * cos(lat1*D2R);
      s2 =  fabs(lat2 - lat1) * D2R;
      dist = sqrt(s1*s1+s2*s2);
   }
   dist *= RADIUS;

   return dist;
}    

/*------------------------------------------------------------------------------
  void qsort_index(double array[], int start, int end, int rank[])
  sort array in increasing order and get the rank of each member in the original array
  the array size of array and rank will be size. 
  
  ----------------------------------------------------------------------------*/
#define swap(a,b,t) ((t)=(a),(a)=(b),(b)=(t))
void qsort_index(double array[], int start, int end, int rank[])
{
   double pivot;
   int tmp_int;
   double tmp_double;

   if (end > start) {
      int l = start + 1;
      int r = end;
      int pivot_index = (start+(end-start)/2);
      swap(array[start], array[pivot_index], tmp_double); /*** choose arbitrary pivot ***/
      swap(rank[start],  rank[pivot_index],  tmp_int);
      pivot = array[start];
      while(l < r) {
         if (array[l] <= pivot) {
            l++;
         } else {
	    while(l < r && array[r] >= pivot) /*** skip superfluous swaps ***/
	    {
	       --r;
	    }
            swap(array[l], array[r], tmp_double);
	    swap(rank[l],  rank[r],  tmp_int);
         }
      }
      if(l != end || array[end] > pivot ) {
	 l--;
	 swap(array[start], array[l], tmp_double);
	 swap(rank[start],  rank[l],  tmp_int);
	 qsort_index(array, start, l, rank);
	 qsort_index(array, r, end, rank);
      }
      else { /* the first element is the largest one */
	 swap(array[start], array[l], tmp_double);
	 swap(rank[start],  rank[l],  tmp_int);
	 qsort_index(array, start, l-1, rank);
      }
   }
}
#define PI M_PI
  
int
gs_transfer_to_mosaic(char *old_file, char *mosaic_dir)
{  
   char *pch=NULL, history[512], entry[1280];
   char tilefile[MAXTILE][STRING], tiletype[MAXTILE][SHORTSTRING];
   char tile_name[MAXTILE][STRING];
   int ntiles=0, nfiles=0, ncontact=0;
   double periodx=0, periody=0;
   int contact_tile1[MAXCONTACT], contact_tile2[MAXCONTACT];
   int contact_tile1_istart[MAXCONTACT], contact_tile1_iend[MAXCONTACT];
   int contact_tile1_jstart[MAXCONTACT], contact_tile1_jend[MAXCONTACT];
   int contact_tile2_istart[MAXCONTACT], contact_tile2_iend[MAXCONTACT];
   int contact_tile2_jstart[MAXCONTACT], contact_tile2_jend[MAXCONTACT];
   char mosaic_name[128] = "atmos_mosaic";
   char grid_descriptor[128] = "";
   int c, i, n, m, l, errflg, check=0;

   char atmos_name[128]="atmos", land_name[128]="land", ocean_name[128]="ocean";
   char agrid_file[128], lgrid_file[128], ogrid_file[128];

   int is_coupled_grid = 0, is_ocean_only =1; 
   int interp_order=1;
   double *x, *y, *dx, *dy, *area, *angle_dx, *angle_dy; 
   int nx, ny, nxp, nyp;
   const char mosaic_version[] = "0.2";

   if(!old_file) mpp_error("Usage:\n \t \t transfer_to_mosaic --input_file input_file.nc");
   if(mpp_field_exist(old_file, "AREA_ATMxOCN") ) is_coupled_grid = 1;
   if(mpp_field_exist(old_file, "AREA_ATM") ) is_ocean_only = 0;
   if(mpp_field_exist(old_file, "DI_ATMxOCN") ) interp_order= 2;
// -----------------ocean_hgrid.nc---------------------------------[
// Ocean
   {
      int nx_C, ny_C, nx_T, ny_T, nvertex;
      int fid_old, vid;
  
      double *x_C, *y_C, *x_T, *y_T;
      double *x_vert_T, *y_vert_T;


      double *ds_00_02_C, *ds_00_20_C, *ds_01_11_C, *ds_01_21_C, *ds_02_22_C, 
         *ds_10_11_C, *ds_10_12_C, *ds_11_21_C, *ds_11_12_C, *ds_20_22_C, 
         *ds_01_11_T, *ds_01_21_T, *ds_02_22_T, *ds_10_11_T, *ds_10_12_T, 
         *ds_11_12_T, *ds_11_21_T, *ds_20_22_T, 
         *ds_01_21_E, *ds_10_12_N, *angle_C; 

 
      int i,j,k, i1, i2, j1, j2, k0, k1, k2, k3;
      int ji, ji0, ji1, ji2, ji3;
      int j1i1, j1i2, j2i1, j2i2; 
      double unknown = 0.0; /* double unknown = -1.0E+10; */

      fid_old = mpp_open(old_file, MPP_READ);
      nx_C = mpp_get_dimlen(fid_old, "grid_x_C");
      ny_C = mpp_get_dimlen(fid_old, "grid_y_C");
      nx_T = mpp_get_dimlen(fid_old, "grid_x_T");
      ny_T = mpp_get_dimlen(fid_old, "grid_y_T");
      nvertex = mpp_get_dimlen(fid_old, "vertex");

      printf("\nReading file: %s \n", old_file); 


/* Reading Ocean Grid Variables */
      x_C = (double *) malloc(ny_C * nx_C * sizeof(double)); 
      y_C = (double *) malloc(ny_C * nx_C * sizeof(double));
      x_T = (double *) malloc(ny_T * nx_T * sizeof(double));
      y_T = (double *) malloc(ny_T * nx_T * sizeof(double));
      x_vert_T = (double *) malloc(nvertex*ny_T*nx_T*sizeof(double));
      y_vert_T = (double *) malloc(nvertex*ny_T*nx_T*sizeof(double));
      ds_01_11_C = (double *) malloc(ny_C * nx_C * sizeof(double)); 
      ds_10_11_C = (double *) malloc(ny_C * nx_C * sizeof(double)); 
      ds_11_21_C = (double *) malloc(ny_C * nx_C * sizeof(double)); 
      ds_11_12_C = (double *) malloc(ny_C * nx_C * sizeof(double)); 
      ds_01_11_T = (double *) malloc(ny_T * nx_T * sizeof(double));  
      ds_10_11_T = (double *) malloc(ny_T * nx_T * sizeof(double));  
      ds_11_12_T = (double *) malloc(ny_T * nx_T * sizeof(double));  
      ds_11_21_T = (double *) malloc(ny_T * nx_T * sizeof(double));
      ds_02_22_T = (double *) malloc(ny_T * nx_T * sizeof(double));  
      ds_20_22_T = (double *) malloc(ny_T * nx_T * sizeof(double));
      angle_C    = (double *) malloc(ny_C * nx_C * sizeof(double));
  
      vid = mpp_get_varid(fid_old, "x_C"); mpp_get_var_value(fid_old, vid, x_C);
      vid = mpp_get_varid(fid_old, "y_C"); mpp_get_var_value(fid_old, vid, y_C);
      vid = mpp_get_varid(fid_old, "x_T"); mpp_get_var_value(fid_old, vid, x_T);
      vid = mpp_get_varid(fid_old, "y_T"); mpp_get_var_value(fid_old, vid, y_T);
      vid = mpp_get_varid(fid_old, "x_vert_T"); mpp_get_var_value(fid_old, vid, x_vert_T);
      vid = mpp_get_varid(fid_old, "y_vert_T"); mpp_get_var_value(fid_old, vid, y_vert_T);
      vid = mpp_get_varid(fid_old, "ds_01_11_C"); mpp_get_var_value(fid_old, vid, ds_01_11_C);  
      vid = mpp_get_varid(fid_old, "ds_10_11_C"); mpp_get_var_value(fid_old, vid, ds_10_11_C);
      vid = mpp_get_varid(fid_old, "ds_11_21_C"); mpp_get_var_value(fid_old, vid, ds_11_21_C);    
      vid = mpp_get_varid(fid_old, "ds_11_12_C"); mpp_get_var_value(fid_old, vid, ds_11_12_C);
      vid = mpp_get_varid(fid_old, "ds_01_11_T"); mpp_get_var_value(fid_old, vid, ds_01_11_T);  
      vid = mpp_get_varid(fid_old, "ds_10_11_T"); mpp_get_var_value(fid_old, vid, ds_10_11_T);
      vid = mpp_get_varid(fid_old, "ds_11_21_T"); mpp_get_var_value(fid_old, vid, ds_11_21_T);    
      vid = mpp_get_varid(fid_old, "ds_11_12_T"); mpp_get_var_value(fid_old, vid, ds_11_12_T);  
      vid = mpp_get_varid(fid_old, "ds_02_22_T"); mpp_get_var_value(fid_old, vid, ds_02_22_T);    
      vid = mpp_get_varid(fid_old, "ds_20_22_T"); mpp_get_var_value(fid_old, vid, ds_20_22_T);  
      vid = mpp_get_varid(fid_old, "angle_C"); mpp_get_var_value(fid_old, vid, angle_C);  
      mpp_close(fid_old);
  
      nx = 2*nx_T;  nxp = nx +1;
      ny = 2*ny_T;  nyp = ny +1;
      x = (double *) malloc(nxp*nyp * sizeof(double));
      y = (double *) malloc(nxp*nyp* sizeof(double));
      dx = (double *) malloc(nx*nyp * sizeof(double));
      dy = (double *) malloc(nxp*ny * sizeof(double));
      area = (double *) malloc(nx*ny * sizeof(double));
      angle_dx = (double *) malloc(nxp*nyp * sizeof(double));

      for(j = 0; j<ny_T; j++) {
	 for(i = 0; i<nx_T; i++) {

	    i2=2*(i+1); i1=i2-1;   j2=2*(j+1); j1=j2-1;

	    j1i1=j1*nxp+i1; j1i2=j1*nxp+i2; j2i2=j2*nxp+i2; j2i1=j2*nxp+i1; 
	    ji=j*nx_T+i;
	    ji0=ji; ji1=ji0+nx_T*ny_T; ji2=ji1+nx_T*ny_T; ji3=ji2+nx_T*ny_T;

	    x[j1i1] = x_T[ji];
	    x[j1i2] = 0.5*(x_vert_T[ji1]+x_vert_T[ji2]);
	    x[j2i2] = x_vert_T[ji2];
	    x[j2i1] = 0.5*(x_vert_T[ji3]+x_vert_T[ji2]);

	    y[j1i1] = y_T[ji];
	    y[j1i2] = 0.5*(y_vert_T[ji1]+y_vert_T[ji2]);
	    y[j2i2] = y_vert_T[ji2];
	    y[j2i1] = 0.5*(y_vert_T[ji3]+y_vert_T[ji2]);

/*      Mapping distance to new mosaic grid format from old grid

	dx(1,1)=                  dx(2,1)=           
	Ci-1,j------ds_11_21_C(i-1,j)---+-----ds_01_11_C(i,j)--------Ci,j
	|                          |                            |
	|                          |                            |
	|                          |                            |
	|                     ds_11_12_T                   ds_10_11_C(i,j)
	|                          |                            |
	|         dx(1,1)=         |          dx(2,1)=          |
	+---------ds_01_11_T------Tij---------ds_11_21_T--------+
	|                          |                            |
	|                          |                            |
	|                          |                            |
	|                     ds_11_12_T                   ds_11_12_C(i,j-1)
	|                          |                            |
	|                          |                            |
	Ci-1,j-1--------------------------+----------------------------Ci,j-1

*/
	    dx[(2*j+1)*nx  + (2*i)]   = ds_01_11_T[j*nx_T + i];
	    dx[(2*j+1)*nx  + (2*i+1)] = ds_11_21_T[j*nx_T + i];
	    if(i>0) dx[(2*j+2)*nx  + (2*i)]   = ds_11_21_C[j*nx_T + (i-1)];
	    dx[(2*j+2)*nx  + (2*i+1)] = ds_01_11_C[j*nx_T + i];

	    dy[(2*j  )*nxp + (2*i+1)]   = ds_10_11_T[j*nx_T + i];
	    if(j>0) dy[(2*j  )*nxp + (2*i+2)]   = ds_11_12_C[(j-1)*nx_T + i];
	    dy[(2*j+1)*nxp + (2*i+1)]   = ds_11_12_T[j*nx_T + i];
	    dy[(2*j+1)*nxp + (2*i+2)]   = ds_10_11_C[j*nx_T + i];

	    angle_dx[(2*j+1)*nxp + 2*i+1] = unknown;
	    angle_dx[(2*j+1)*nxp + 2*i+2] = unknown;
	    angle_dx[(2*j+2)*nxp + 2*i+1] = unknown;
	    angle_dx[(2*j+2)*nxp + 2*i+2] = angle_C[j*nx_C + i ]; 

	    if (j==0) {
	       x[i1] = 0.5*(x_vert_T[ji0]+x_vert_T[ji1]);
	       x[i2] = x_vert_T[ji1];
	       y[i1] = 0.5*(y_vert_T[ji0]+y_vert_T[ji1]);
	       y[i2] = y_vert_T[ji1];

	       dx[ (2*i+0)] = unknown;
	       dx[ (2*i+1)] = unknown;
	       /* dy[(2*j  )*nxp + (2*i+2)]   = unknown; */
	       dy[(2*j  )*nxp + (2*i+2)]   = ds_20_22_T[j*nx_T + i] - ds_10_11_C[j*nx_T + i];

	       angle_dx[2*i+1] = unknown;
	       angle_dx[2*i+2] = unknown;
	    }

	    if (i==0) {
	       x[j1*nxp] = 0.5*(x_vert_T[ji0]+x_vert_T[ji3]);
	       x[j2*nxp] = x_vert_T[ji3];
	       y[j1*nxp] = 0.5*(y_vert_T[ji0]+y_vert_T[ji3]);
	       y[j2*nxp] = y_vert_T[ji3];

	       /* dx[(2*j+2)*nx  + (2*i)]   = unknown;*/
	       dx[(2*j+2)*nx  + (2*i)]   = ds_02_22_T[j*nx_T + i] - ds_01_11_C[j*nx_T + i];
	       dy[(2*j  )*nxp ]    = unknown;
	       dy[(2*j+1)*nxp ]    = unknown;

	       angle_dx[(2*j+1)*nxp] = unknown;
	       angle_dx[(2*j+2)*nxp] = unknown;
	    }
	    if (i==0 && j==0) {
	       x[0] = x_vert_T[ji0]; y[0] = y_vert_T[ji0];
	       angle_dx[0] = unknown;
	    }

	 }
      }

      //for(j = 0; j<nyp; j++) { for(i = 0; i<nxp; i++) { if(angle_dx[j*nxp+i]>0.0) printf("%d %d %f\n ",i,j,angle_dx[j*nxp+i]); } }
      //for(j = 0; j<ny_T; j++) { printf("%f ",y_C[j*nx_T+(nx_T+0)/4]); }
      //for(i = 0; i<nx_T; i++) { if(y_T[(ny_T-1)*nx_T+i]>85.) printf("%f ",y_T[(ny_T-1)*nx_T+i]); }
      //printf("\n\n");
      //for(j = 0; j<nyp; j++) { printf("%f ",y[j*nxp+(nxp-1)/4]); }
      //for(i = 0; i<nxp; i++) { if(y[(nyp-1)*nxp+i]>85.) printf("%f ",y[(nyp-1)*nxp+i]); }


      /* write ocean-grid data */
      /*-------ocean_hgrid.nc-------*/
      {
	 int fid, id_tile, id_x, id_y, id_dx, id_dy, id_area, id_angle_dx, id_angle_dy, id_arcx;
	 int dimlist[5], dims[2], i, j, l, ni, nj, nip, njp;
	 char tile_spec_version[] = "0.2", geometry[] = "spherical", discretization[] = "logically_rectangular", conformal[]="true";
	 char outfile[128] = "", tilename[128]="";
	 size_t start[4], nwrite[4];
    
	 sprintf(outfile, "%s_hgrid.nc",ocean_name);
	 printf("Writing %s\n", outfile);
	 printf("\t nx_C=%d, ny_C=%d, x_T=%d, ny_T=%d \n", nx_C, ny_C,nx_T, ny_T); 
	 sprintf(tilename, "tile%d", 1);
	 fid = mpp_open(outfile, MPP_WRITE);

	 dimlist[0] = mpp_def_dim(fid, "string", STRINGLEN);
	 dimlist[1] = mpp_def_dim(fid, "nxp", nxp);
	 dimlist[2] = mpp_def_dim(fid, "nyp", nyp);
	 dimlist[3] = mpp_def_dim(fid, "nx", nx);
	 dimlist[4] = mpp_def_dim(fid, "ny", ny);

	 id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 5, "standard_name", "grid_tile_spec",
	 "tile_spec_version", tile_spec_version, "geometry", geometry,
	 "discretization", discretization, "conformal", conformal );


	 dims[0] = dimlist[2]; dims[1] = dimlist[1];
	 id_x = mpp_def_var(fid, "x", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_longitude",
	 "units", "degree_east");
	 id_y = mpp_def_var(fid, "y", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_latitude",
	 "units", "degree_north");

	 dims[0] = dimlist[2]; dims[1] = dimlist[3];
	 id_dx = mpp_def_var(fid, "dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_x_distance",
	 "units", "meters");
	 dims[0] = dimlist[4]; dims[1] = dimlist[1];
	 id_dy = mpp_def_var(fid, "dy", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_y_distance",
	 "units", "meters");

	 dims[0] = dimlist[2]; dims[1] = dimlist[1];
	 id_angle_dx= mpp_def_var(fid, "angle_dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_vertex_x_angle_WRT_geographic_east",
	 "units", "degrees_east");

	 dims[0] = dimlist[4]; dims[1] = dimlist[3];
	 id_area= mpp_def_var(fid, "area", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_cell_area",
	 "units", "m2");

	 mpp_end_def(fid);

	 for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 0; }
	 nwrite[0] = strlen(tilename);
	 mpp_put_var_value_block(fid, id_tile, start, nwrite, tilename );
	 mpp_put_var_value(fid, id_x, x);
	 mpp_put_var_value(fid, id_y, y);
	 mpp_put_var_value(fid, id_dx, dx);
	 mpp_put_var_value(fid, id_dy, dy);
	 mpp_put_var_value(fid, id_angle_dx, angle_dx);
	 mpp_close(fid);
      }
   }
// ----------------------------------------------------------------]
// -----------------atmos_hgrid.nc---------------------------------[
// Atmosphere
   int nxba, nyba, nxta, nyta, nxap, nyap;
   if(is_coupled_grid) {
      double *x_atmos, *y_atmos;
      double *xba, *yba, *xta, *yta;
      int i,j, fid_old, vid;

      fid_old = mpp_open(old_file, MPP_READ);
      nxba = mpp_get_dimlen(fid_old, "xba");
      nyba = mpp_get_dimlen(fid_old, "yba");
      nxta = mpp_get_dimlen(fid_old, "xta");
      nyta = mpp_get_dimlen(fid_old, "yta");
      xba = (double *) malloc(nxba * sizeof(double)); 
      yba = (double *) malloc(nyba * sizeof(double)); 
      xta = (double *) malloc(nxta * sizeof(double)); 
      yta = (double *) malloc(nyta * sizeof(double)); 
      vid = mpp_get_varid(fid_old, "xba"); mpp_get_var_value(fid_old, vid, xba);
      vid = mpp_get_varid(fid_old, "yba"); mpp_get_var_value(fid_old, vid, yba);
      vid = mpp_get_varid(fid_old, "xta"); mpp_get_var_value(fid_old, vid, xta);
      vid = mpp_get_varid(fid_old, "yta"); mpp_get_var_value(fid_old, vid, yta);
      mpp_close(fid_old);
  
      nxap = nxba + nxta; nyap = nyba + nyta; 
      x_atmos = (double *) malloc(nxap*nyap * sizeof(double));
      y_atmos = (double *) malloc(nxap*nyap * sizeof(double));

      for(i = 0; i<nxba; i++) x_atmos[2*i]   = xba[i]; for(i = 0; i<nxta; i++) x_atmos[2*i+1]   = xta[i]; 

      for(j = 0; j<nyba; j++) y_atmos[2*j*nxap]   = yba[j]; 
      for(j = 0; j<nyta; j++) y_atmos[(2*j+1)*nxap]   = yta[j]; 

      for(j = 1; j<nyap; j++) for(i = 0; i<nxap; i++) x_atmos[j*nxap + i]   = x_atmos[i];
      for(i = 1; i<nxap; i++) for(j = 0; j<nyap; j++) y_atmos[j*nxap + i]   = y_atmos[j*nxap];

      /* write Atmosphere-grid data */
      {
	 int fid, id_tile, id_x, id_y, id_dx, id_dy, id_area, id_angle_dx, id_angle_dy, id_arcx;
	 int dimlist[5], dims[2], i, j, l, ni, nj, nip, njp;
	 char tile_spec_version[] = "0.2", geometry[] = "spherical", discretization[] = "logically_rectangular", conformal[]="true";
	 char outfile[128] = "", tilename[128]="";
	 size_t start[4], nwrite[4];
    
	 sprintf(outfile, "%s_hgrid.nc", atmos_name);
	 printf("Writing %s\n", outfile);
	 printf("\t nxba=%d, nyba=%d, nxta=%d, nyta=%d \n", nxba, nyba,nxta, nyta); 

	 sprintf(tilename, "tile%d", 1);
	 fid = mpp_open(outfile, MPP_WRITE);
	 dimlist[0] = mpp_def_dim(fid, "string", STRINGLEN);
	 dimlist[1] = mpp_def_dim(fid, "nxp", nxap);
	 dimlist[2] = mpp_def_dim(fid, "nyp", nyap);
	 dimlist[3] = mpp_def_dim(fid, "nx", nxap-1);
	 dimlist[4] = mpp_def_dim(fid, "ny", nyap-1);
	 id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 5, "standard_name", "grid_tile_spec", "tile_spec_version", 
	 tile_spec_version, "geometry", geometry, "discretization", discretization, "conformal", conformal );
	 dims[0] = dimlist[2]; dims[1] = dimlist[1];
	 id_x = mpp_def_var(fid, "x", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_longitude", "units", "degree_east");
	 id_y = mpp_def_var(fid, "y", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_latitude", "units", "degree_north");

	 /* Following variables are not defined */
	 dims[0] = dimlist[2]; dims[1] = dimlist[3];
	 id_dx = mpp_def_var(fid, "dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_x_distance", "units", "meters");
	 dims[0] = dimlist[4]; dims[1] = dimlist[1];
	 id_dy = mpp_def_var(fid, "dy", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_y_distance", "units", "meters");
	 dims[0] = dimlist[2]; dims[1] = dimlist[1];
	 id_angle_dx= mpp_def_var(fid, "angle_dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_vertex_x_angle_WRT_geographic_east",
	 "units", "degrees_east");
	 dims[0] = dimlist[4]; dims[1] = dimlist[3];
	 id_angle_dx= mpp_def_var(fid, "area", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_cell_area", "units", "m2");
	 /* ------------------------------------------- */
	 mpp_end_def(fid);
	 for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 0; }
	 nwrite[0] = strlen(tilename);
	 mpp_put_var_value_block(fid, id_tile, start, nwrite, tilename );
	 mpp_put_var_value(fid, id_x, x_atmos);
	 mpp_put_var_value(fid, id_y, y_atmos);
	 mpp_close(fid);
      }
      free(xba); free(yba); free(xta); free(yta);  free(x_atmos); free(y_atmos);
   }
// ----------------------------------------------------------------]

//.................. Land .......................
//..................land_hgrid.nc................
   int nxbl, nybl, nxtl, nytl,  nxlp, nylp;
   if(is_coupled_grid){
      double *x_land, *y_land;
      double *xbl, *ybl, *xtl, *ytl;
      int i,j, fid_old, vid;

      fid_old = mpp_open(old_file, MPP_READ);
  
      nxbl = mpp_get_dimlen(fid_old, "xbl");
      nybl = mpp_get_dimlen(fid_old, "ybl");
      nxtl = mpp_get_dimlen(fid_old, "xtl");
      nytl = mpp_get_dimlen(fid_old, "ytl");
      xbl = (double *) malloc(nxbl * sizeof(double)); 
      ybl = (double *) malloc(nybl * sizeof(double)); 
      xtl = (double *) malloc(nxtl * sizeof(double)); 
      ytl = (double *) malloc(nytl * sizeof(double)); 
      vid = mpp_get_varid(fid_old, "xbl"); mpp_get_var_value(fid_old, vid, xbl);
      vid = mpp_get_varid(fid_old, "ybl"); mpp_get_var_value(fid_old, vid, ybl);
      vid = mpp_get_varid(fid_old, "xtl"); mpp_get_var_value(fid_old, vid, xtl);
      vid = mpp_get_varid(fid_old, "ytl"); mpp_get_var_value(fid_old, vid, ytl);
      mpp_close(fid_old);
  
      nxlp = nxbl + nxtl; nylp = nybl + nytl; 
      x_land  = (double *) malloc(nxlp*nylp * sizeof(double));
      y_land  = (double *) malloc(nxlp*nylp * sizeof(double));
  
      for(i = 0; i<nxbl; i++) x_land[2*i]   = xbl[i]; 
      for(i = 0; i<nxtl; i++) x_land[2*i+1]   = xtl[i]; 

      for(j = 0; j<nybl; j++) y_land[2*j*nxlp]   = ybl[j]; 
      for(j = 0; j<nytl; j++) y_land[(2*j+1)*nxlp]   = ytl[j]; 

      for(j = 1; j<nylp; j++) for(i = 0; i<nxlp; i++) x_land[j*nxlp + i]   = x_land[i];

      for(i = 1; i<nxlp; i++) 
	 for(j = 0; j<nylp; j++) 
	    y_land[j*nxlp + i]   = y_land[j*nxlp];

      /* write Land-grid data */
      {
	 int fid, id_tile, id_x, id_y, id_dx, id_dy, id_area, id_angle_dx, id_angle_dy, id_arcx;
	 int dimlist[5], dims[2], i, j, l, ni, nj, nip, njp;
	 char tile_spec_version[] = "0.2", geometry[] = "spherical", discretization[] = "logically_rectangular", conformal[]="true";
	 char outfile[128] = "", tilename[128]="";
	 size_t start[4], nwrite[4];
    
	 sprintf(outfile, "%s_hgrid.nc",land_name);
	 printf("Writing %s\n", outfile);
	 printf("\t nxbl=%d, nybl=%d, nxtl=%d, nytl=%d \n", nxbl, nybl,nxtl, nytl); 

	 sprintf(tilename, "tile%d", 1);
	 fid = mpp_open(outfile, MPP_WRITE);
	 dimlist[0] = mpp_def_dim(fid, "string", STRINGLEN);
	 dimlist[1] = mpp_def_dim(fid, "nxp", nxlp);
	 dimlist[2] = mpp_def_dim(fid, "nyp", nylp);
	 dimlist[3] = mpp_def_dim(fid, "nx", nxlp-1);
	 dimlist[4] = mpp_def_dim(fid, "ny", nylp-1);
	 id_tile = mpp_def_var(fid, "tile", MPP_CHAR, 1, dimlist, 5, "standard_name", "grid_tile_spec", "tile_spec_version", 
	 tile_spec_version, "geometry", geometry, "discretization", discretization, "conformal", conformal );
	 dims[0] = dimlist[2]; dims[1] = dimlist[1];
	 id_x = mpp_def_var(fid, "x", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_longitude", "units", "degree_east");
	 id_y = mpp_def_var(fid, "y", MPP_DOUBLE, 2, dims, 2, "standard_name", "geographic_latitude", "units", "degree_north");

	 /* Following variables are not defined */
	 dims[0] = dimlist[2]; dims[1] = dimlist[3];
	 id_dx = mpp_def_var(fid, "dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_x_distance", "units", "meters");
	 dims[0] = dimlist[4]; dims[1] = dimlist[1];
	 id_dy = mpp_def_var(fid, "dy", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_edge_y_distance", "units", "meters");
	 dims[0] = dimlist[2]; dims[1] = dimlist[1];
	 id_angle_dx= mpp_def_var(fid, "angle_dx", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_vertex_x_angle_WRT_geographic_east",
	 "units", "degrees_east");
	 dims[0] = dimlist[4]; dims[1] = dimlist[3];
	 id_angle_dx= mpp_def_var(fid, "area", MPP_DOUBLE, 2, dims, 2, "standard_name", "grid_cell_area", "units", "m2");
	 /* ------------------------------------------- */

	 mpp_end_def(fid);
	 for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 0; }
	 nwrite[0] = strlen(tilename);
	 mpp_put_var_value_block(fid, id_tile, start, nwrite, tilename );
	 mpp_put_var_value(fid, id_x, x_land);
	 mpp_put_var_value(fid, id_y, y_land);
	 mpp_close(fid);
      }
      free(xbl); free(ybl); free(xtl); free(ytl);  free(x_land); free(y_land);
   }
//-------------------------------ocean_vgrid.nc----------------------------------
   { 
      double *z, *zb, *zt;
      int nzb, nzt, nz, i,j, fid_old, vid;

      fid_old = mpp_open(old_file, MPP_READ);
      nzb = mpp_get_dimlen(fid_old, "zb");
      nzt = mpp_get_dimlen(fid_old, "zt");
      zb = (double *) malloc(nzb * sizeof(double)); 
      zt = (double *) malloc(nzt * sizeof(double)); 
      vid = mpp_get_varid(fid_old, "zb"); mpp_get_var_value(fid_old, vid, zb);
      vid = mpp_get_varid(fid_old, "zt"); mpp_get_var_value(fid_old, vid, zt);
      mpp_close(fid_old);
  
      nz = nzb + nzt + 1; 
      z = (double *) malloc(nz * sizeof(double));
      z[0]=0.0; 
      for(i = 0; i<nzt; i++) { z[2*i + 1]   = zt[i]; z[2*i+2]   = zb[i]; } 

      {
	 int fid, dim, varid;
	 char outfile[128] = "";

	 sprintf(outfile, "%s_vgrid.nc",ocean_name);
	 printf("Writing %s\n", outfile);
	 printf("\t nzb=%d, nzt=%d\n", nzb, nzt); 

	 fid = mpp_open(outfile, MPP_WRITE);
	 dim  = mpp_def_dim(fid, "nzv", nz);
	 varid = mpp_def_var(fid, "zeta", MPP_DOUBLE, 1, &dim, 2, "standard_name", "vertical_grid_vertex", "units", "meters");
	 mpp_end_def(fid);
	 mpp_put_var_value(fid, varid, z);
	 mpp_close(fid);
      }
      free(zb); free(zt); free(z);
   }


//-------------------------------topog.nc----------------------------------
   { 
      double *depth_t;
      int nx, ny, ntiles, i,j, fid_old, vid;

      fid_old = mpp_open(old_file, MPP_READ);  
      nx= mpp_get_dimlen(fid_old, "grid_x_T");
      ny= mpp_get_dimlen(fid_old, "grid_y_T");
      depth_t= (double *) malloc(nx*ny* sizeof(double));
      vid = mpp_get_varid(fid_old, "depth_t");  mpp_get_var_value(fid_old, vid, depth_t);
      mpp_close(fid_old);
  
      {
	 int fid, dim[2], varid, ntiles = 1, dim_ntile;
	 char outfile[128] = "";
	 sprintf(outfile, "topog.nc");
	 printf("Writing %s\n", outfile);
	 printf("\t nx=%d, ny=%d\n", nx, ny);
	 fid = mpp_open(outfile, MPP_WRITE);
	 dim_ntile = mpp_def_dim(fid, "ntiles", 1);
	 dim[1]  = mpp_def_dim(fid, "nx", nx);
	 dim[0]  = mpp_def_dim(fid, "ny", ny);
	 varid = mpp_def_var(fid, "depth", MPP_DOUBLE, 2, dim, 2, "standard_name", "topographic depth at T-cell centers", "units", "meters");
	 mpp_end_def(fid);
	 mpp_put_var_value(fid, varid, depth_t);
	 mpp_close(fid);
      }
      free(depth_t); 
   }

//................atmos_mosaic.nc............................
   if(is_coupled_grid)  {
      char str[STRING], outfile[STRING];
      int fid, dim_ntiles, dim_ncontact, dim_string, id_mosaic, id_gridtiles, id_contacts;
      int id_contact_index, id_griddir, id_gridfiles, dim[2];

      size_t start[4], nwrite[4];

//--------Initilize values --------------
      ntiles = 1, ncontact = 0; 

      sprintf(mosaic_name, "%s_mosaic",atmos_name);
      sprintf(outfile, "%s.nc", mosaic_name);
      printf("Writing %s\n", outfile);

      sprintf(tile_name[0], "tile%d", 1);
      sprintf(tilefile[0], "%s_hgrid.nc",atmos_name);
      fid = mpp_open(outfile, MPP_WRITE);

      contact_tile1_istart[0]=nxba-1; contact_tile1_iend[0]=nxba-1; 
      contact_tile1_jstart[0]=1; contact_tile1_jend[0]=nyba-1; 

      contact_tile2_istart[0]=1; contact_tile2_iend[0]=1; 
      contact_tile2_jstart[0]=1; contact_tile2_jend[0]=nyba-1;
//--------Initilize values --------------

      dim_ntiles = mpp_def_dim(fid, "ntiles", ntiles);
      if(ncontact>0) dim_ncontact = mpp_def_dim(fid, "ncontact", ncontact);
      dim_string = mpp_def_dim(fid, "string", STRING);

      id_mosaic = mpp_def_var(fid, "mosaic", MPP_CHAR, 1, &dim_string, 5, "standard_name",
      "grid_mosaic_spec", "mosaic_spec_version", mosaic_version,
      "children", "gridtiles", "contact_regions", "contacts",
      "grid_descriptor", grid_descriptor);
      dim[0] = dim_ntiles; dim[1] = dim_string;
      id_griddir   = mpp_def_var(fid, "gridlocation", MPP_CHAR, 1, &dim[1], 1,
      "standard_name", "grid_file_location");
      id_gridfiles = mpp_def_var(fid, "gridfiles", MPP_CHAR, 2, dim, 0);
      id_gridtiles = mpp_def_var(fid, "gridtiles", MPP_CHAR, 2, dim, 0);

      if(ncontact>0) {
	 dim[0] = dim_ncontact; dim[1] = dim_string;
	 id_contacts = mpp_def_var(fid, "contacts", MPP_CHAR, 2, dim, 5, "standard_name", "grid_contact_spec",
	 "contact_type", "boundary", "alignment", "true",
	 "contact_index", "contact_index", "orientation", "orient");
	 id_contact_index = mpp_def_var(fid, "contact_index", MPP_CHAR, 2, dim, 1, "standard_name",
	 "starting_ending_point_index_of_contact");

      }

      //mpp_def_global(fid, "history", history);
      mpp_end_def(fid);

      /* write out data */
      for(i=0; i<4; i++) {
	 start[i] = 0; nwrite[i] = 1;
      }
      nwrite[0] = strlen(mosaic_name);
      mpp_put_var_value_block(fid, id_mosaic, start, nwrite, mosaic_name);
      nwrite[0] = strlen(mosaic_dir);
      mpp_put_var_value_block(fid, id_griddir, start, nwrite, mosaic_dir);
      nwrite[0] = 1;
      for(n=0; n<ntiles; n++) {
	 start[0] = n; nwrite[1] = strlen(tile_name[n]);
	 mpp_put_var_value_block(fid, id_gridtiles, start, nwrite, tile_name[n]);
	 nwrite[1] = strlen(tilefile[n]);
	 mpp_put_var_value_block(fid, id_gridfiles, start, nwrite, tilefile[n]);
      }

      for(n=0; n<ncontact; n++) {
	 sprintf(str,"%s:%s::%s:%s", mosaic_name, tile_name[contact_tile1[n]], mosaic_name,
	 tile_name[contact_tile2[n]]);
	 start[0] = n; nwrite[1] = strlen(str);
	 mpp_put_var_value_block(fid, id_contacts, start, nwrite, str);
	 sprintf(str,"%d:%d,%d:%d::%d:%d,%d:%d", contact_tile1_istart[n], contact_tile1_iend[n],
	 contact_tile1_jstart[n], contact_tile1_jend[n], contact_tile2_istart[n],
	 contact_tile2_iend[n], contact_tile2_jstart[n], contact_tile2_jend[n] );
	 nwrite[1] = strlen(str);
	 mpp_put_var_value_block(fid, id_contact_index, start, nwrite, str);
      }
      mpp_close(fid);
   }

//................land_mosaic.nc............................
   if(is_coupled_grid)  {
      char str[STRING], outfile[STRING];
      int fid, dim_ntiles, dim_ncontact, dim_string, id_mosaic, id_gridtiles, id_contacts;
      int id_contact_index, id_griddir, id_gridfiles, dim[2];

      size_t start[4], nwrite[4];

//--------Initilize values --------------
      ntiles = 1, ncontact = 1; 
      contact_tile1_istart[1]=1; contact_tile1_iend[1]=1;
      contact_tile1_jstart[1]=1; contact_tile1_jend[1]=nytl;

      contact_tile2_istart[1]=nxtl; contact_tile2_iend[1]=nxtl;
      contact_tile2_jstart[1]=1;    contact_tile2_jend[1]=nytl;
    
      sprintf(mosaic_name, "%s_mosaic",land_name);
      sprintf(outfile, "%s.nc", mosaic_name);
      printf("Writing %s\n", outfile);

      sprintf(tile_name[0], "tile%d", 1);
      sprintf(tilefile[0], "%s_hgrid.nc",land_name);
      fid = mpp_open(outfile, MPP_WRITE);

      contact_tile1_istart[0]=nxbl-1; contact_tile1_iend[0]=nxbl-1; 
      contact_tile1_jstart[0]=1; contact_tile1_jend[0]=nybl-1; 

      contact_tile2_istart[0]=1; contact_tile2_iend[0]=1; 
      contact_tile2_jstart[0]=1; contact_tile2_jend[0]=nybl-1;
//--------Initilize values --------------

      dim_ntiles = mpp_def_dim(fid, "ntiles", ntiles);
      if(ncontact>0) dim_ncontact = mpp_def_dim(fid, "ncontact", ncontact);
      dim_string = mpp_def_dim(fid, "string", STRING);

      id_mosaic = mpp_def_var(fid, "mosaic", MPP_CHAR, 1, &dim_string, 5, "standard_name",
      "grid_mosaic_spec", "mosaic_spec_version", mosaic_version,
      "children", "gridtiles", "contact_regions", "contacts",
      "grid_descriptor", grid_descriptor);
      dim[0] = dim_ntiles; dim[1] = dim_string;
      id_griddir   = mpp_def_var(fid, "gridlocation", MPP_CHAR, 1, &dim[1], 1,
      "standard_name", "grid_file_location");
      id_gridfiles = mpp_def_var(fid, "gridfiles", MPP_CHAR, 2, dim, 0);
      id_gridtiles = mpp_def_var(fid, "gridtiles", MPP_CHAR, 2, dim, 0);

      if(ncontact>0) {
	 dim[0] = dim_ncontact; dim[1] = dim_string;
	 id_contacts = mpp_def_var(fid, "contacts", MPP_CHAR, 2, dim, 5, "standard_name", "grid_contact_spec",
	 "contact_type", "boundary", "alignment", "true",
	 "contact_index", "contact_index", "orientation", "orient");
	 id_contact_index = mpp_def_var(fid, "contact_index", MPP_CHAR, 2, dim, 1, "standard_name",
	 "starting_ending_point_index_of_contact");

      }

      //mpp_def_global(fid, "history", history);
      mpp_end_def(fid);

      /* write out data */
      for(i=0; i<4; i++) {
	 start[i] = 0; nwrite[i] = 1;
      }
      nwrite[0] = strlen(mosaic_name);
      mpp_put_var_value_block(fid, id_mosaic, start, nwrite, mosaic_name);
      nwrite[0] = strlen(mosaic_dir);
      mpp_put_var_value_block(fid, id_griddir, start, nwrite, mosaic_dir);
      nwrite[0] = 1;
      for(n=0; n<ntiles; n++) {
	 start[0] = n; nwrite[1] = strlen(tile_name[n]);
	 mpp_put_var_value_block(fid, id_gridtiles, start, nwrite, tile_name[n]);
	 nwrite[1] = strlen(tilefile[n]);
	 mpp_put_var_value_block(fid, id_gridfiles, start, nwrite, tilefile[n]);
      }

      for(n=0; n<ncontact; n++) {
	 sprintf(str,"%s:%s::%s:%s", mosaic_name, tile_name[contact_tile1[n]], mosaic_name,
	 tile_name[contact_tile2[n]]);
	 start[0] = n; nwrite[1] = strlen(str);
	 mpp_put_var_value_block(fid, id_contacts, start, nwrite, str);
	 sprintf(str,"%d:%d,%d:%d::%d:%d,%d:%d", contact_tile1_istart[n], contact_tile1_iend[n],
	 contact_tile1_jstart[n], contact_tile1_jend[n], contact_tile2_istart[n],
	 contact_tile2_iend[n], contact_tile2_jstart[n], contact_tile2_jend[n] );
	 nwrite[1] = strlen(str);
	 mpp_put_var_value_block(fid, id_contact_index, start, nwrite, str);
      }
      mpp_close(fid);
   }

/* ...................... ocean_mosaic.nc .................*/
   {
      char str[STRING], outfile[STRING], x_boundary_type1[255]="", y_boundary_type1[255]="";
      int fid, dim_ntiles, dim_ncontact, dim_string, id_mosaic, id_gridtiles, id_contacts;
      int id_contact_index, id_griddir, id_gridfiles, dim[2], itmp, fid_old;

      size_t start[4], nwrite[4];

//--------Initilize values --------------
      ntiles = 1, ncontact = 0; 

      sprintf(mosaic_name, "%s_mosaic",ocean_name);
      sprintf(outfile, "%s.nc", mosaic_name);
      printf("Writing %s\n", outfile);

      sprintf(tile_name[0], "tile%d", 1);
      sprintf(tilefile[0], "%s_hgrid.nc", ocean_name);
    
      /* ........... Read Boundary Type ................ */
      fid_old = mpp_open(old_file, MPP_READ);
      mpp_get_global_att(fid_old, "x_boundary_type",x_boundary_type1);
      mpp_get_global_att(fid_old, "y_boundary_type",y_boundary_type1);
      mpp_close(fid_old);
      if (strcmp(x_boundary_type1, "cyclic")==0) {
	 n = ncontact; ncontact = ncontact + 1; 
	 contact_tile1_istart[n]=360; contact_tile1_iend[n]=360; 
	 contact_tile1_jstart[n]=1; contact_tile1_jend[n]=180; 

	 contact_tile2_istart[n]=1; contact_tile2_iend[n]=1; 
	 contact_tile2_jstart[n]=1; contact_tile2_jend[n]=180;
	 printf("\t x_boundary_type = %s, ncontact=%d \n",x_boundary_type1,ncontact);
      }
    
      if (strcmp(y_boundary_type1, "fold_north_edge")==0) {
	 n = ncontact; ncontact = ncontact + 1; 
	 contact_tile1_istart[n]=1; contact_tile1_iend[n]=180;
	 contact_tile1_jstart[n]=180; contact_tile1_jend[n]=180;

	 contact_tile2_istart[n]=360; contact_tile2_iend[n]=181;
	 contact_tile2_jstart[n]=180; contact_tile2_jend[n]=180;
	 printf("\t y_boundary_type = %s, ncontact=%d \n",y_boundary_type1,ncontact);
      }

      fid = mpp_open(outfile, MPP_WRITE);

//--------Initilize values --------------

      dim_ntiles = mpp_def_dim(fid, "ntiles", ntiles);
      if(ncontact>0) dim_ncontact = mpp_def_dim(fid, "ncontact", ncontact);
      dim_string = mpp_def_dim(fid, "string", STRING);

      id_mosaic = mpp_def_var(fid, "mosaic", MPP_CHAR, 1, &dim_string, 5, "standard_name",
      "grid_mosaic_spec", "mosaic_spec_version", mosaic_version,
      "children", "gridtiles", "contact_regions", "contacts",
      "grid_descriptor", grid_descriptor);
      dim[0] = dim_ntiles; dim[1] = dim_string;
      id_griddir   = mpp_def_var(fid, "gridlocation", MPP_CHAR, 1, &dim[1], 1,
      "standard_name", "grid_file_location");
      id_gridfiles = mpp_def_var(fid, "gridfiles", MPP_CHAR, 2, dim, 0);
      id_gridtiles = mpp_def_var(fid, "gridtiles", MPP_CHAR, 2, dim, 0);

      if(ncontact>0) {
	 dim[0] = dim_ncontact; dim[1] = dim_string;
	 id_contacts = mpp_def_var(fid, "contacts", MPP_CHAR, 2, dim, 5, "standard_name", "grid_contact_spec",
	 "contact_type", "boundary", "alignment", "true",
	 "contact_index", "contact_index", "orientation", "orient");
	 id_contact_index = mpp_def_var(fid, "contact_index", MPP_CHAR, 2, dim, 1, "standard_name",
	 "starting_ending_point_index_of_contact");

      }

      //mpp_def_global(fid, "history", history);
      mpp_end_def(fid);

      /* write out data */
      for(i=0; i<4; i++) {
	 start[i] = 0; nwrite[i] = 1;
      }
      nwrite[0] = strlen(mosaic_name);
      mpp_put_var_value_block(fid, id_mosaic, start, nwrite, mosaic_name);
      nwrite[0] = strlen(mosaic_dir);
      mpp_put_var_value_block(fid, id_griddir, start, nwrite, mosaic_dir);
      nwrite[0] = 1;
      for(n=0; n<ntiles; n++) {
	 start[0] = n; nwrite[1] = strlen(tile_name[n]);
	 mpp_put_var_value_block(fid, id_gridtiles, start, nwrite, tile_name[n]);
	 nwrite[1] = strlen(tilefile[n]);
	 mpp_put_var_value_block(fid, id_gridfiles, start, nwrite, tilefile[n]);
      }

      for(n=0; n<ncontact; n++) {
	 sprintf(str,"%s:%s::%s:%s", mosaic_name, tile_name[contact_tile1[n]], mosaic_name,
	 tile_name[contact_tile2[n]]);
	 start[0] = n; nwrite[1] = strlen(str);
	 mpp_put_var_value_block(fid, id_contacts, start, nwrite, str);
	 sprintf(str,"%d:%d,%d:%d::%d:%d,%d:%d", contact_tile1_istart[n], contact_tile1_iend[n],
	 contact_tile1_jstart[n], contact_tile1_jend[n], contact_tile2_istart[n],
	 contact_tile2_iend[n], contact_tile2_jstart[n], contact_tile2_jend[n] );
	 nwrite[1] = strlen(str);
	 mpp_put_var_value_block(fid, id_contact_index, start, nwrite, str);
      }
      mpp_close(fid);
   }
/* ...................... atmosXland.nc .................*/
   if(is_coupled_grid)  {
      const char version[] = "0.2";
      char lxo_file[STRING], axo_file[STRING], axl_file[STRING];
      char amosaic_file[STRING], lmosaic_file[STRING], omosaic_file[STRING];
      char otopog_file[STRING];

      char atile_name[255], ltile_name[255], otile_name[255];
      char amosaic_name[255], lmosaic_name[255], omosaic_name[255];
      int  fid_old, vid;
  
      strcpy(atile_name,"tile1"); strcpy(ltile_name,"tile1"); strcpy(otile_name,"tile1");
      strcpy(amosaic_name,"atmos"); strcpy(lmosaic_name,"land"); strcpy(omosaic_name,"ocean");

//---------------
      int no, nl, na, n;
      size_t  naxl, naxo, naxl_step;
      int     *atmxlnd_ia, *atmxlnd_ja, *atmxlnd_il, *atmxlnd_jl;
      int     *atmxocn_ia, *atmxocn_ja, *atmxocn_io, *atmxocn_jo;
      int     *lndxocn_il, *lndxocn_jl, *lndxocn_io, *lndxocn_jo;
      double  *atmxlnd_area, *atmxlnd_di, *atmxlnd_dj;
      double  *atmxocn_area, *atmxocn_di, *atmxocn_dj;
      double  *lndxocn_area, *lndxocn_di, *lndxocn_dj;

      fid_old = mpp_open(old_file, MPP_READ);
      naxl = mpp_get_dimlen(fid_old, "i_atmXlnd");
//----------------
 
      /* write out atmXlnd data*/
      if(naxl>0) {
	 size_t start[4], nwrite[4];
                                                                                                                                                          
	 int fid, dim_string, dim_ncells, dim_two, dims[4];
	 int id_xgrid_area, id_contact, n;
	 int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;
	 char contact[STRING];
                                                                                                                                                          
	 for(i=0; i<4; i++) {
            start[i] = 0; nwrite[i] = 1;
	 }
	 sprintf(axl_file, "%s_mosaicX%s_mosaic.nc", amosaic_name, lmosaic_name);
	 printf("Writing %s_mosaicX%s_mosaic.nc\n", amosaic_name, lmosaic_name);
	 sprintf(contact, "%s_mosaic:%s::%s_mosaic:%s", amosaic_name, atile_name, lmosaic_name, ltile_name);
	 fid = mpp_open(axl_file, MPP_WRITE);
	 //mpp_def_global(fid, "history", history);
	 dim_string = mpp_def_dim(fid, "string", STRING);
	 dim_ncells = mpp_def_dim(fid, "ncells", naxl);
	 dim_two    = mpp_def_dim(fid, "two", 2);
	 if(interp_order == 2) {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 8, "standard_name", "grid_contact_spec",
	    "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
	    "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area",
	    "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
	 }
	 else {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 6, "standard_name", "grid_contact_spec",
	    "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
	    "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area");
	 }
                                                                                                                                                          
	 dims[0] = dim_ncells; dims[1] = dim_two;
	 id_tile1_cell = mpp_def_var(fid, "tile1_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic1");
	 id_tile2_cell = mpp_def_var(fid, "tile2_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic2");
	 id_xgrid_area = mpp_def_var(fid, "xgrid_area", MPP_DOUBLE, 1, &dim_ncells, 2, "standard_name",
	 "exchange_grid_area", "units", "m2");
	 if(interp_order == 2) {
            id_tile1_dist = mpp_def_var(fid, "tile1_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent1_cell_centroid");
            id_tile2_dist = mpp_def_var(fid, "tile2_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent2_cell_centroid");
	 }
	 mpp_end_def(fid);

	 for(i=0; i<4; i++) {start[i] = 0; nwrite[i] = 1;}
	 nwrite[0] = strlen(contact);
	 mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
	 nwrite[0] = naxl;

	 atmxlnd_area = (double *)malloc(naxl*sizeof(double)); 
	 atmxlnd_ia   = (int    *)malloc(naxl*sizeof(int   )); 
	 atmxlnd_ja   = (int    *)malloc(naxl*sizeof(int   )); 
	 atmxlnd_il   = (int    *)malloc(naxl*sizeof(int   )); 
	 atmxlnd_jl   = (int    *)malloc(naxl*sizeof(int   )); 
	 vid = mpp_get_varid(fid_old, "AREA_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_area);
	 vid = mpp_get_varid(fid_old, "I_ATM_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_ia);
	 vid = mpp_get_varid(fid_old, "J_ATM_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_ja);
	 vid = mpp_get_varid(fid_old, "I_LND_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_il);
	 vid = mpp_get_varid(fid_old, "J_LND_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_jl);
	  
	 for(i=0;i<naxl;i++)atmxlnd_area[i] = atmxlnd_area[i] * 4.0 * PI * RADIUS * RADIUS; 

	 mpp_put_var_value(fid, id_xgrid_area, atmxlnd_area);
	 mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, atmxlnd_ia);
	 mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, atmxlnd_il);
	 start[1] = 1;
	 mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, atmxlnd_ja);
	 mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, atmxlnd_jl);
	 if(interp_order == 2) {
            atmxlnd_di = (double *)malloc(naxl*sizeof(double)); 
            atmxlnd_dj = (double *)malloc(naxl*sizeof(double)); 
	    vid = mpp_get_varid(fid_old, "DI_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_di);
	    vid = mpp_get_varid(fid_old, "DJ_ATMxLND"); mpp_get_var_value(fid_old, vid, atmxlnd_dj);
            start[1] = 0;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, atmxlnd_di);
            start[1] = 1;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, atmxlnd_dj);
	 }
	  
	 mpp_close(fid);
	 free(atmxlnd_area);
      }
      mpp_close(fid_old);
   }
/* ...................... atmosXocean.nc .................*/
   if(is_coupled_grid)  {
      const char version[] = "0.2";
      char lxo_file[STRING];
      char axo_file[STRING];
      char axl_file[STRING];
      char amosaic_file[STRING];
      char lmosaic_file[STRING];
      char omosaic_file[STRING];
      char otopog_file[STRING];

      char atile_name[255], ltile_name[255], otile_name[255];
      char amosaic_name[255], lmosaic_name[255], omosaic_name[255];

      strcpy(atile_name,"tile1"); strcpy(ltile_name,"tile1"); strcpy(otile_name,"tile1");
      strcpy(amosaic_name,"atmos"); strcpy(lmosaic_name,"land"); strcpy(omosaic_name,"ocean");

//---------------
      int no, nl, na, n;
      size_t  naxl, naxo, naxl_step;
      int     *atmxlnd_ia, *atmxlnd_ja, *atmxlnd_il, *atmxlnd_jl;
      int     *atmxocn_ia, *atmxocn_ja, *atmxocn_io, *atmxocn_jo;
      int     *lndxocn_il, *lndxocn_jl, *lndxocn_io, *lndxocn_jo;
      double  *atmxlnd_area, *atmxlnd_di, *atmxlnd_dj;
      double  *atmxocn_area, *atmxocn_di, *atmxocn_dj;
      double  *lndxocn_area, *lndxocn_di, *lndxocn_dj;
      int fid_old, vid;

      fid_old = mpp_open(old_file, MPP_READ);
      naxo = mpp_get_dimlen(fid_old, "i_atmXocn");
//----------------
 
      /* write out atmXlnd data*/
      if(naxo>0) {
	 size_t start[4], nwrite[4];
                                                                                                                                                          
	 int fid, dim_string, dim_ncells, dim_two, dims[4];
	 int id_xgrid_area, id_contact, n;
	 int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;
	 char contact[STRING];
                                                                                                                                                          
	 for(i=0; i<4; i++) {
            start[i] = 0; nwrite[i] = 1;
	 }
	 sprintf(axo_file, "%s_mosaicX%s_mosaic.nc", amosaic_name, omosaic_name);
	 printf("Writing %s_mosaicX%s_mosaic.nc\n", amosaic_name, omosaic_name);
	 sprintf(contact, "%s_mosaic:%s::%s_mosaic:%s", amosaic_name, atile_name, omosaic_name, otile_name);
	 fid = mpp_open(axo_file, MPP_WRITE);
	 //mpp_def_global(fid, "history", history);
	 dim_string = mpp_def_dim(fid, "string", STRING);
	 dim_ncells = mpp_def_dim(fid, "ncells", naxo);
	 dim_two    = mpp_def_dim(fid, "two", 2);
	 if(interp_order == 2) {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 8, "standard_name", "grid_contact_spec",
	    "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
	    "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area",
	    "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
	 }
	 else {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 6, "standard_name", "grid_contact_spec",
	    "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
	    "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area");
	 }
                                                                                                                                                          
	 dims[0] = dim_ncells; dims[1] = dim_two;
	 id_tile1_cell = mpp_def_var(fid, "tile1_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic1");
	 id_tile2_cell = mpp_def_var(fid, "tile2_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic2");
	 id_xgrid_area = mpp_def_var(fid, "xgrid_area", MPP_DOUBLE, 1, &dim_ncells, 2, "standard_name",
	 "exchange_grid_area", "units", "m2");
	 if(interp_order == 2) {
            id_tile1_dist = mpp_def_var(fid, "tile1_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent1_cell_centroid");
            id_tile2_dist = mpp_def_var(fid, "tile2_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent2_cell_centroid");
	 }
	 mpp_end_def(fid);

	 for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 1;}
	 nwrite[0] = strlen(contact);
	 mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
	 nwrite[0] = naxo;

	 atmxocn_area = (double *)malloc(naxo*sizeof(double)); 
	 atmxocn_ia   = (int    *)malloc(naxo*sizeof(int   )); 
	 atmxocn_ja   = (int    *)malloc(naxo*sizeof(int   )); 
	 atmxocn_io   = (int    *)malloc(naxo*sizeof(int   )); 
	 atmxocn_jo   = (int    *)malloc(naxo*sizeof(int   )); 
	 vid = mpp_get_varid(fid_old, "AREA_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_area);
	 vid = mpp_get_varid(fid_old, "I_ATM_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_ia);
	 vid = mpp_get_varid(fid_old, "J_ATM_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_ja);
	 vid = mpp_get_varid(fid_old, "I_OCN_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_io);
	 vid = mpp_get_varid(fid_old, "J_OCN_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_jo);
	  
	 for(i=0;i<naxo;i++)atmxocn_area[i] = atmxocn_area[i] * 4.0 * PI * RADIUS * RADIUS; 

	 mpp_put_var_value(fid, id_xgrid_area, atmxocn_area);
	 mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, atmxocn_ia);
	 mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, atmxocn_io);
	 start[1] = 1;
	 mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, atmxocn_ja);
	 mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, atmxocn_jo);
	 if(interp_order == 2) {
            atmxocn_di = (double *)malloc(naxo*sizeof(double)); 
            atmxocn_dj = (double *)malloc(naxo*sizeof(double)); 
	    vid = mpp_get_varid(fid_old, "DI_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_di);
	    vid = mpp_get_varid(fid_old, "DJ_ATMxOCN"); mpp_get_var_value(fid_old, vid, atmxocn_dj);
            start[1] = 0;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, atmxocn_di);
            start[1] = 1;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, atmxocn_dj);
	 }
	 mpp_close(fid);
	 free(atmxocn_area);
      }
      mpp_close(fid_old);
   }
/* ...................... landXocean.nc .................*/
   if(is_coupled_grid)  {
      const char version[] = "0.2";
      char lxo_file[STRING];
      char axo_file[STRING];
      char axl_file[STRING];
      char amosaic_file[STRING];
      char lmosaic_file[STRING];
      char omosaic_file[STRING];
      char otopog_file[STRING];

      char atile_name[255], ltile_name[255], otile_name[255];
      char amosaic_name[255], lmosaic_name[255], omosaic_name[255];

      strcpy(atile_name,"tile1"); strcpy(ltile_name,"tile1"); strcpy(otile_name,"tile1");
      strcpy(amosaic_name,"atmos"); strcpy(lmosaic_name,"land"); strcpy(omosaic_name,"ocean");

//---------------
      int no, nl, na, n;
      size_t  naxl, naxo, nlxo, naxl_step;
      int     *atmxlnd_ia, *atmxlnd_ja, *atmxlnd_il, *atmxlnd_jl;
      int     *atmxocn_ia, *atmxocn_ja, *atmxocn_io, *atmxocn_jo;
      int     *lndxocn_il, *lndxocn_jl, *lndxocn_io, *lndxocn_jo;
      double  *atmxlnd_area, *atmxlnd_di, *atmxlnd_dj;
      double  *atmxocn_area, *atmxocn_di, *atmxocn_dj;
      double  *lndxocn_area, *lndxocn_di, *lndxocn_dj;
      int     fid_old, vid;

      fid_old = mpp_open(old_file, MPP_READ);
      nlxo = mpp_get_dimlen(fid_old , "i_lndXocn");
//----------------
 
      /* write out lndXocn data*/
      if(nlxo>0) {
	 size_t start[4], nwrite[4];
                                                                                                                                                          
	 int fid, dim_string, dim_ncells, dim_two, dims[4];
	 int id_xgrid_area, id_contact, n;
	 int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;
	 char contact[STRING];
                                                                                                                                                          
	 for(i=0; i<4; i++) {
            start[i] = 0; nwrite[i] = 1;
	 }
	 sprintf(lxo_file, "%s_mosaicX%s_mosaic.nc", lmosaic_name, omosaic_name);
	 printf("Writing %s_mosaicX%s_mosaic.nc\n", lmosaic_name, omosaic_name);
	 sprintf(contact, "%s_mosaic:%s::%s_mosaic:%s", lmosaic_name, ltile_name, omosaic_name, otile_name);
	 fid = mpp_open(lxo_file, MPP_WRITE);
	 //mpp_def_global(fid, "history", history);
	 dim_string = mpp_def_dim(fid, "string", STRING);
	 dim_ncells = mpp_def_dim(fid, "ncells", nlxo);
	 dim_two    = mpp_def_dim(fid, "two", 2);
	 if(interp_order == 2) {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 8, "standard_name", "grid_contact_spec",
	    "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
	    "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area",
	    "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
	 }
	 else {
            id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 6, "standard_name", "grid_contact_spec",
	    "contact_spec_version", version, "contact_type", "exchange", "parent1_cell",
	    "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area");
	 }
                                                                                                                                                          
	 dims[0] = dim_ncells; dims[1] = dim_two;
	 id_tile1_cell = mpp_def_var(fid, "tile1_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic1");
	 id_tile2_cell = mpp_def_var(fid, "tile2_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic2");
	 id_xgrid_area = mpp_def_var(fid, "xgrid_area", MPP_DOUBLE, 1, &dim_ncells, 2, "standard_name",
	 "exchange_grid_area", "units", "m2");
	 if(interp_order == 2) {
            id_tile1_dist = mpp_def_var(fid, "tile1_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent1_cell_centroid");
            id_tile2_dist = mpp_def_var(fid, "tile2_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent2_cell_centroid");
	 }
	 mpp_end_def(fid);

	 for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 1;}
	 nwrite[0] = strlen(contact);
	 mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
	 nwrite[0] = nlxo;

	 lndxocn_area = (double *)malloc(nlxo*sizeof(double)); 
	 lndxocn_il   = (int    *)malloc(nlxo*sizeof(int   )); 
	 lndxocn_jl   = (int    *)malloc(nlxo*sizeof(int   )); 
	 lndxocn_io   = (int    *)malloc(nlxo*sizeof(int   )); 
	 lndxocn_jo   = (int    *)malloc(nlxo*sizeof(int   )); 
	 vid = mpp_get_varid(fid_old, "AREA_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_area);
	 vid = mpp_get_varid(fid_old, "I_LND_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_il);
	 vid = mpp_get_varid(fid_old, "J_LND_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_jl);
	 vid = mpp_get_varid(fid_old, "I_OCN_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_io);
	 vid = mpp_get_varid(fid_old, "J_OCN_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_jo);
	  
	 for(i=0;i<nlxo;i++)lndxocn_area[i] = lndxocn_area[i] * 4.0 * PI * RADIUS * RADIUS; 

	 mpp_put_var_value(fid, id_xgrid_area, lndxocn_area);
	 mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, lndxocn_il);
	 mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, lndxocn_io);
	 start[1] = 1;
	 mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, lndxocn_jl);
	 mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, lndxocn_jo);
	 if(interp_order == 2) {
            lndxocn_di = (double *)malloc(nlxo*sizeof(double)); 
            lndxocn_dj = (double *)malloc(nlxo*sizeof(double)); 
	    vid = mpp_get_varid(fid_old, "DI_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_di);
	    vid = mpp_get_varid(fid_old, "DJ_LNDxOCN"); mpp_get_var_value(fid_old, vid, lndxocn_dj);
            start[1] = 0;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, lndxocn_di);
            start[1] = 1;
            mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, lndxocn_dj);
	 }
	 mpp_close(fid);
	 free(lndxocn_area);
      }
      mpp_close(fid_old);
   }

/* ...................... mosaic.nc .................*/
   {
      const char version[] = "0.2";
      char lxo_file[STRING], axo_file[STRING], axl_file[STRING], mosaic_file[STRING];
      char amosaic_file[STRING], lmosaic_file[STRING], omosaic_file[STRING];
      char amosaic_name[STRING], lmosaic_name[STRING], omosaic_name[STRING];
      char otopog_file[STRING];

      int fid, dim_string, dim_axo, dim_lxo, dim_axl, dims[4], n;
      size_t start[4], nwrite[4];
      int id_lmosaic_dir, id_lmosaic_file, id_omosaic_dir, id_omosaic_file;
      int id_amosaic_dir, id_amosaic_file, id_otopog_dir, id_otopog_file;
      int id_xgrids_dir, id_axo_file, id_lxo_file, id_axl_file;
      int id_amosaic, id_lmosaic, id_omosaic;

      int nfile_axo = 1, nfile_axl = 1, nfile_lxo = 1; 

      strcpy(mosaic_file,"mosaic.nc");
      printf("Writing %s\n", mosaic_file);

      fid = mpp_open(mosaic_file, MPP_WRITE);
      //  mpp_def_global(fid, "history", history);
      dim_string = mpp_def_dim(fid, "string", STRING);
      dim_axo = mpp_def_dim(fid, "nfile_aXo", nfile_axo);
      dim_axl = mpp_def_dim(fid, "nfile_aXl", nfile_axl);
      dim_lxo = mpp_def_dim(fid, "nfile_lXo", nfile_lxo);
      id_amosaic_dir  = mpp_def_var(fid, "atm_mosaic_dir", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "directory_storing_atmosphere_mosaic");
      id_amosaic_file = mpp_def_var(fid, "atm_mosaic_file", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "atmosphere_mosaic_file_name");
      id_amosaic      = mpp_def_var(fid, "atm_mosaic", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "atmosphere_mosaic_name");
      id_lmosaic_dir  = mpp_def_var(fid, "lnd_mosaic_dir", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "directory_storing_land_mosaic");
      id_lmosaic_file = mpp_def_var(fid, "lnd_mosaic_file", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "land_mosaic_file_name");
      id_lmosaic      = mpp_def_var(fid, "lnd_mosaic", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "land_mosaic_name");
      id_omosaic_dir  = mpp_def_var(fid, "ocn_mosaic_dir", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "directory_storing_ocean_mosaic");
      id_omosaic_file = mpp_def_var(fid, "ocn_mosaic_file", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "ocean_mosaic_file_name");
      id_omosaic      = mpp_def_var(fid, "ocn_mosaic", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "ocean_mosaic_name");
      id_otopog_dir   = mpp_def_var(fid, "ocn_topog_dir", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "directory_storing_ocean_topog");
      id_otopog_file  = mpp_def_var(fid, "ocn_topog_file", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "ocean_topog_file_name");
      /* since exchange grid is created in this tool, we may add command line option to specify where to store the output */
      /*    id_xgrids_dir = mpp_def_var(fid, "xgrids_dir", MPP_CHAR, 1, &dim_string,
	    1, "standard_name", "directory_storing_xgrids"); */
      dims[0] = dim_axo; dims[1] = dim_string;
      id_axo_file = mpp_def_var(fid, "aXo_file", MPP_CHAR, 2, dims, 1, "standard_name", "atmXocn_exchange_grid_file");
      dims[0] = dim_axl; dims[1] = dim_string;
      id_axl_file = mpp_def_var(fid, "aXl_file", MPP_CHAR, 2, dims, 1, "standard_name", "atmXlnd_exchange_grid_file");
      dims[0] = dim_lxo; dims[1] = dim_string;
      id_lxo_file = mpp_def_var(fid, "lXo_file", MPP_CHAR, 2, dims, 1, "standard_name", "lndXocn_exchange_grid_file");
      mpp_end_def(fid);

      strcpy(otopog_file,"topog.nc");

      sprintf(amosaic_name,"%s_mosaic",atmos_name); sprintf(lmosaic_name,"%s_mosaic",land_name); sprintf(omosaic_name,"%s_mosaic",ocean_name);
      sprintf(amosaic_file,"%s.nc",amosaic_name); sprintf(lmosaic_file,"%s.nc",lmosaic_name); sprintf(omosaic_file,"%s.nc",omosaic_name);

      sprintf(axl_file,"%sX%s.nc", amosaic_name, lmosaic_name); 
      sprintf(axo_file,"%sX%s.nc", amosaic_name, omosaic_name); 
      sprintf(lxo_file,"%sX%s.nc", lmosaic_name, omosaic_name); 

      for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 1; }
      nwrite[0] = strlen(mosaic_dir);
      mpp_put_var_value_block(fid, id_amosaic_dir, start, nwrite, mosaic_dir);
      nwrite[0] = strlen(amosaic_file);
      mpp_put_var_value_block(fid, id_amosaic_file, start, nwrite, amosaic_file);
      nwrite[0] = strlen(amosaic_name);
      mpp_put_var_value_block(fid, id_amosaic, start, nwrite, amosaic_name);
      nwrite[0] = strlen(mosaic_dir);
      mpp_put_var_value_block(fid, id_lmosaic_dir, start, nwrite, mosaic_dir);
      nwrite[0] = strlen(lmosaic_file);
      mpp_put_var_value_block(fid, id_lmosaic_file, start, nwrite, lmosaic_file);
      nwrite[0] = strlen(lmosaic_name);
      mpp_put_var_value_block(fid, id_lmosaic, start, nwrite, lmosaic_name);
      nwrite[0] = strlen(mosaic_dir);
      mpp_put_var_value_block(fid, id_omosaic_dir, start, nwrite, mosaic_dir);
      nwrite[0] = strlen(omosaic_file);
      mpp_put_var_value_block(fid, id_omosaic_file, start, nwrite, omosaic_file);
      nwrite[0] = strlen(omosaic_name);
      mpp_put_var_value_block(fid, id_omosaic, start, nwrite, omosaic_name);
      nwrite[0] = strlen(mosaic_dir);
      mpp_put_var_value_block(fid, id_otopog_dir, start, nwrite, mosaic_dir);
      nwrite[0] = strlen(otopog_file);
      mpp_put_var_value_block(fid, id_otopog_file, start, nwrite, otopog_file);
      for(i=0; i<4; i++) {
	 start[i] = 0; nwrite[i] = 1;
      }
      start[0] = 0; nwrite[1] = strlen(axo_file);
      mpp_put_var_value_block(fid, id_axo_file, start, nwrite, axo_file);

      start[0] = 0; nwrite[1] = strlen(axl_file);
      mpp_put_var_value_block(fid, id_axl_file, start, nwrite, axl_file);

      start[0] = 0; nwrite[1] = strlen(lxo_file);
      mpp_put_var_value_block(fid, id_lxo_file, start, nwrite, lxo_file);
      mpp_close(fid);
   }

   return 0;
}

int 
gs_make_vgrid(char *history, int nbnds, int *bnds, int n1, 
	      int n2, int *nz, char *gridname, char *center)
{
   int i, nk;
   char filename[128];
   double *zeta;
   int c, option_index = 0;

   /* check the command-line arguments to make sure the value are suitable */
   if( nbnds < 2 ) mpp_error("number of bounds specified through -nbnd should be an integer greater than 1");
   if( nbnds != n1 ) mpp_error("nbnds does not equal number entry specified through -bnd");
   if( nbnds-1 != n2 ) mpp_error("nbnds-1 does not match number entry specified through -nz");

   /* generate grid */
   nk = 0;
   for(i=0; i<nbnds-1; i++) nk += nz[i];
   zeta = (double *)malloc((nk+1)*sizeof(double));
   create_vgrid(nbnds, bnds, nz, zeta, center);

   sprintf(filename, "%s.nc", gridname);
  
   /* write out vertical grid into a netcdf file */
   {
      int fid, dim, varid;
    
      fid = mpp_open(filename, MPP_WRITE);
      dim  = mpp_def_dim(fid, "nzv", nk+1);
      varid = mpp_def_var(fid, "zeta", NC_DOUBLE, 1, &dim, 2, "standard_name", "vertical_grid_vertex",
      "units", "meters");
      mpp_def_global_att(fid, "grid_version", GRID_VERSION);
      mpp_def_global_att(fid, "code_version", TAGNAME);
      mpp_def_global_att(fid, "history", history);    
      mpp_end_def(fid);
      mpp_put_var_value(fid, varid, zeta);
  
      mpp_close(fid);
   }

   if(mpp_pe() == mpp_root_pe()) printf("Successfully generate vertical grid file %s\n", filename);
   return 0;
}


/* This file will get the directory that stores the file and the file name (without the dir path) */
void get_file_dir_and_name(char *file, char *filedir, char *filename)
{
   char *fptr=NULL;
   int siz;
  
   fptr = strrchr(file, '/');

   if(!fptr) {
      strcpy(filename, file);
      strcpy(filedir, "./");
   }
   else {
      ++fptr;
      siz = fptr - file;
      strcpy(filename, fptr);
      strncpy(filedir, file, siz);      
   }
};

#define D2R (M_PI/180.)
#define MAXXGRIDFILE 100 
#define MX 2000  
#define AREA_RATIO_THRESH (1.e-6)
#define TINY_VALUE (1.e-7)
#define TOLORENCE (1.e-4)

int gs_make_coupler_mosaic(char *history, char *amosaic, char *lmosaic, 
                           char *omosaic, char *otopog, int interp_order, 
                           double sea_level, char *mosaic_name, int check)
{  
   int c, i, same_mosaic;
   char mosaic_file[STRING];
   char omosaic_name[STRING], amosaic_name[STRING], lmosaic_name[STRING];
   char **otile_name=NULL, **atile_name=NULL, **ltile_name=NULL;
   int x_refine = 2, y_refine = 2;
   int nfile_lxo=0, nfile_axo=0, nfile_axl=0;
   char lxo_file[MAXXGRIDFILE][STRING];
   char axo_file[MAXXGRIDFILE][STRING];
   char axl_file[MAXXGRIDFILE][STRING];  
   char amosaic_dir[STRING], amosaic_file[STRING];
   char lmosaic_dir[STRING], lmosaic_file[STRING];
   char omosaic_dir[STRING], omosaic_file[STRING];
   char otopog_dir[STRING], otopog_file[STRING];
   int    ntile_ocn, ntile_atm, ntile_lnd;
   int    *nxo = NULL, *nyo = NULL, *nxa = NULL, *nya = NULL, *nxl = NULL, *nyl = NULL;
   double **xocn = NULL, **yocn = NULL, **xatm = NULL, **yatm = NULL, **xlnd = NULL, **ylnd = NULL;
   double **area_ocn = NULL, **area_lnd = NULL, **area_atm = NULL;
   double **omask = NULL;
   int    lnd_same_as_atm = 0;
   int    option_index = 0;
   double axo_area_sum = 0, axl_area_sum = 0;
   int    ocn_south_ext = 0;

   /*mosaic_file can not have the same name as amosaic, lmosaic or omosaic, also the file name of
     amosaic, lmosaic, omosaic can not be "mosaic.nc"
   */
   sprintf(mosaic_file, "%s.nc", mosaic_name);
   get_file_dir_and_name(amosaic, amosaic_dir, amosaic_file);
   get_file_dir_and_name(lmosaic, lmosaic_dir, lmosaic_file);
   get_file_dir_and_name(omosaic, omosaic_dir, omosaic_file);
   get_file_dir_and_name(otopog, otopog_dir, otopog_file);
   if( !strcmp(mosaic_file, amosaic_file) || !strcmp(mosaic_file, lmosaic_file) || !strcmp(mosaic_file, omosaic_file) ) 
      mpp_error("make_coupler_mosaic: mosaic_file can not have the same name as amosaic, lmosaic or omosaic"); 
   if( !strcmp(amosaic_file, "mosaic.nc") || !strcmp(lmosaic_file, "mosaic.nc") || !strcmp(omosaic_file, "mosaic.nc") ) 
      mpp_error("make_coupler_mosaic: the file name of amosaic, lmosaic or omosaic can not be mosaic.nc"); 

  
   /*
    * Read atmosphere grid
    */
   {
      int n, m_fid, g_fid, vid, gid, tid;
      size_t start[4], nread[4];
      char dir[STRING], filename[STRING], file[2*STRING];    

      for(n=0; n<4; n++) {
	 start[n] = 0;
	 nread[n] = 1;
      }

      m_fid = mpp_open(amosaic, MPP_READ);
      vid = mpp_get_varid(m_fid, "mosaic");    
      mpp_get_var_value(m_fid, vid, amosaic_name);
      ntile_atm  = mpp_get_dimlen(m_fid, "ntiles");
      nxa        = (int *) malloc (ntile_atm*sizeof(int));
      nya        = (int *) malloc (ntile_atm*sizeof(int));
      xatm       = (double **) malloc( ntile_atm*sizeof(double *));
      yatm       = (double **) malloc( ntile_atm*sizeof(double *));
      area_atm   = (double **) malloc( ntile_atm*sizeof(double *));
      atile_name = (char **)malloc(ntile_atm*sizeof(char *));
      /* grid should be located in the same directory of mosaic file */
      get_file_path(amosaic, dir);
      gid = mpp_get_varid(m_fid, "gridfiles");
      tid = mpp_get_varid(m_fid, "gridtiles");
      for(n=0; n<ntile_atm; n++) {
	 double *tmpx, *tmpy;
	 int i, j;
      
	 start[0] = n; start[1] = 0; nread[0] = 1; nread[1] = STRING;
	 mpp_get_var_value_block(m_fid, gid, start, nread, filename);
	 atile_name[n] = (char *)malloc(STRING*sizeof(char));
	 mpp_get_var_value_block(m_fid, tid, start, nread,  atile_name[n]);
	 sprintf(file, "%s/%s", dir, filename);
	 g_fid = mpp_open(file, MPP_READ);
	 nxa[n] = mpp_get_dimlen(g_fid, "nx");
	 nya[n] = mpp_get_dimlen(g_fid, "ny");
	 if(nxa[n]%x_refine != 0 ) mpp_error("make_coupler_mosaic: atmos supergrid x-size can not be divided by x_refine");
	 if(nya[n]%y_refine != 0 ) mpp_error("make_coupler_mosaic: atmos supergrid y-size can not be divided by y_refine");
	 nxa[n] /= x_refine;
	 nya[n] /= y_refine;
	 xatm[n]     = (double *)malloc((nxa[n]+1)*(nya[n]+1)*sizeof(double));
	 yatm[n]     = (double *)malloc((nxa[n]+1)*(nya[n]+1)*sizeof(double));
	 area_atm[n] = (double *)malloc((nxa[n]  )*(nya[n]  )*sizeof(double));
	 tmpx        = (double *)malloc((nxa[n]*x_refine+1)*(nya[n]*y_refine+1)*sizeof(double));
	 tmpy        = (double *)malloc((nxa[n]*x_refine+1)*(nya[n]*y_refine+1)*sizeof(double));
	 vid = mpp_get_varid(g_fid, "x");
	 mpp_get_var_value(g_fid, vid, tmpx);
	 vid = mpp_get_varid(g_fid, "y");
	 mpp_get_var_value(g_fid, vid, tmpy);      
	 for(j = 0; j < nya[n]+1; j++) for(i = 0; i < nxa[n]+1; i++) {
	       xatm[n][j*(nxa[n]+1)+i] = tmpx[(j*y_refine)*(nxa[n]*x_refine+1)+i*x_refine];
	       yatm[n][j*(nxa[n]+1)+i] = tmpy[(j*y_refine)*(nxa[n]*x_refine+1)+i*x_refine];
	    }
	 free(tmpx);
	 free(tmpy);      
	 /*scale grid from degree to radian, because create_xgrid assume the grid is in radians */
	 for(i=0; i<(nxa[n]+1)*(nya[n]+1); i++) {
	    xatm[n][i] *= D2R;
	    yatm[n][i] *= D2R;
	 }
	 get_grid_area(nxa+n, nya+n, xatm[n], yatm[n], area_atm[n]);
	 mpp_close(g_fid);
      }
      mpp_close(m_fid);
   }

   /*
    * Read land grid
    */
   if (strcmp(lmosaic, amosaic) ) { /* land mosaic is different from atmosphere mosaic */
      int n, m_fid, g_fid, vid, gid, tid;
      size_t start[4], nread[4];
      char dir[STRING], filename[STRING], file[2*STRING];   

      for(n=0; n<4; n++) {
	 start[n] = 0;
	 nread[n] = 1;
      }
      m_fid = mpp_open(lmosaic, MPP_READ);
      vid = mpp_get_varid(m_fid, "mosaic");    
      mpp_get_var_value(m_fid, vid, lmosaic_name);
      ntile_lnd  = mpp_get_dimlen(m_fid, "ntiles");
      nxl        = (int *) malloc (ntile_lnd*sizeof(int));
      nyl        = (int *) malloc (ntile_lnd*sizeof(int));
      xlnd       = (double **) malloc( ntile_lnd*sizeof(double *));
      ylnd       = (double **) malloc( ntile_lnd*sizeof(double *));
      area_lnd   = (double **) malloc( ntile_lnd*sizeof(double *));
      ltile_name = (char **)malloc(ntile_lnd*sizeof(char *));
      /* grid should be located in the same directory of mosaic file */
      get_file_path(lmosaic, dir);
      gid = mpp_get_varid(m_fid, "gridfiles");
      tid = mpp_get_varid(m_fid, "gridtiles");    
      for(n=0; n<ntile_lnd; n++) {
	 double *tmpx, *tmpy;
	 int i, j;
      
	 start[0] = n; start[1] = 0; nread[0] = 1; nread[1] = STRING;
	 mpp_get_var_value_block(m_fid, gid, start, nread, filename);
	 ltile_name[n] = (char *)malloc(STRING*sizeof(char));
	 mpp_get_var_value_block(m_fid, tid, start, nread,  ltile_name[n]);
	 sprintf(file, "%s/%s", dir, filename);
	 g_fid = mpp_open(file, MPP_READ);
	 nxl[n] = mpp_get_dimlen(g_fid, "nx");
	 nyl[n] = mpp_get_dimlen(g_fid, "ny");
	 if(nxl[n]%x_refine != 0 ) mpp_error("make_coupler_mosaic: land supergrid x-size can not be divided by x_refine");
	 if(nyl[n]%y_refine != 0 ) mpp_error("make_coupler_mosaic: land supergrid y-size can not be divided by y_refine");
	 nxl[n]      /= x_refine;
	 nyl[n]      /= y_refine;
	 xlnd[n]     = (double *)malloc((nxl[n]+1)*(nyl[n]+1)*sizeof(double));
	 ylnd[n]     = (double *)malloc((nxl[n]+1)*(nyl[n]+1)*sizeof(double));
	 area_lnd[n] = (double *)malloc((nxl[n]  )*(nyl[n]  )*sizeof(double));
	 tmpx        = (double *)malloc((nxl[n]*x_refine+1)*(nyl[n]*y_refine+1)*sizeof(double));
	 tmpy        = (double *)malloc((nxl[n]*x_refine+1)*(nyl[n]*y_refine+1)*sizeof(double));
	 vid = mpp_get_varid(g_fid, "x");
	 mpp_get_var_value(g_fid, vid, tmpx);
	 vid = mpp_get_varid(g_fid, "y");
	 mpp_get_var_value(g_fid, vid, tmpy);     
	 for(j = 0; j < nyl[n]+1; j++) for(i = 0; i < nxl[n]+1; i++) {
	       xlnd[n][j*(nxl[n]+1)+i] = tmpx[(j*y_refine)*(nxl[n]*x_refine+1)+i*x_refine];
	       ylnd[n][j*(nxl[n]+1)+i] = tmpy[(j*y_refine)*(nxl[n]*x_refine+1)+i*x_refine];
	    }
	 free(tmpx);
	 free(tmpy);
	 /*scale grid from degree to radian, because create_xgrid assume the grid is in radians */
	 for(i=0; i<(nxl[n]+1)*(nyl[n]+1); i++) {
	    xlnd[n][i] *= D2R;
	    ylnd[n][i] *= D2R;
	 }
	 get_grid_area(nxl+n, nyl+n, xlnd[n], ylnd[n], area_lnd[n]);
	 mpp_close(g_fid);
      }
      mpp_close(m_fid);
   }
   else { /* land mosaic is same as atmosphere mosaic */
      ntile_lnd = ntile_atm;
      nxl = nxa;
      nyl = nya;
      xlnd = xatm;
      ylnd = yatm;
      area_lnd = area_atm;
      lnd_same_as_atm = 1;
      strcpy(lmosaic_name, amosaic_name);
      ltile_name = atile_name;
   }

   /*
    * Read ocean grid boundaries and mask (where water is) for each tile within the mosaic.
    */
   {
      int n, ntiles, m_fid, g_fid, t_fid, vid, gid, tid;
      size_t start[4], nread[4];
      char dir[STRING], filename[STRING], file[2*STRING];
    
      for(n=0; n<4; n++) {
	 start[n] = 0;
	 nread[n] = 1;
      }
      m_fid = mpp_open(omosaic, MPP_READ);
      vid = mpp_get_varid(m_fid, "mosaic");    
      mpp_get_var_value(m_fid, vid, omosaic_name);
      ntile_ocn  = mpp_get_dimlen(m_fid, "ntiles");
      nxo      = (int     *) malloc(ntile_ocn*sizeof(int));
      nyo      = (int     *) malloc(ntile_ocn*sizeof(int));
      xocn     = (double **) malloc(ntile_ocn*sizeof(double *));
      yocn     = (double **) malloc(ntile_ocn*sizeof(double *));
      area_ocn = (double **) malloc(ntile_ocn*sizeof(double *));
      otile_name = (char **) malloc(ntile_ocn*sizeof(char *));
      /* grid should be located in the same directory of mosaic file */
      get_file_path(omosaic, dir);
      gid = mpp_get_varid(m_fid, "gridfiles");
      tid = mpp_get_varid(m_fid, "gridtiles");    

      /* For the purpose of reproducing between processor count, the layout
	 is set to (1, npes). */

      for(n=0; n<ntile_ocn; n++) {
	 double *tmpx, *tmpy;
	 int i, j;
	 double min_atm_lat, min_lat;
      
	 start[0] = n; start[1] = 0; nread[0] = 1; nread[1] = STRING;
	 mpp_get_var_value_block(m_fid, gid, start, nread, filename);
	 otile_name[n] = (char *)malloc(STRING*sizeof(char));
	 mpp_get_var_value_block(m_fid, tid, start, nread,  otile_name[n]);
	 sprintf(file, "%s/%s", dir, filename);
	 g_fid = mpp_open(file, MPP_READ);
	 nxo[n] = mpp_get_dimlen(g_fid, "nx");
	 nyo[n] = mpp_get_dimlen(g_fid, "ny");
	 if(nxo[n]%x_refine != 0 ) mpp_error("make_coupler_mosaic: ocean supergrid x-size can not be divided by x_refine");
	 if(nyo[n]%y_refine != 0 ) mpp_error("make_coupler_mosaic: ocean supergrid y-size can not be divided by y_refine");
	 tmpx        = (double *)malloc((nxo[n]+1)*(nyo[n]+1)*sizeof(double));
	 tmpy        = (double *)malloc((nxo[n]+1)*(nyo[n]+1)*sizeof(double));
	 vid = mpp_get_varid(g_fid, "x");
	 mpp_get_var_value(g_fid, vid, tmpx);
	 vid = mpp_get_varid(g_fid, "y");
	 mpp_get_var_value(g_fid, vid, tmpy);     
   
	 /* sometimes the ocean is only covered part of atmosphere, especially not cover
	    the south pole region. In order to get all the exchange grid between atmosXland,
	    we need to extend one point to cover the whole atmosphere. This need the
	    assumption of one-tile ocean. Also we assume the latitude is the along j=0
	 */
	 if(ntile_ocn == 1) {
	    int na;
	    for(i=1; i<=nxo[n]; i++) 
	       if(tmpy[i] != tmpy[i-1]) mpp_error("make_coupler_mosaic: latitude is not uniform along j=0");
	    /* calculate the minimum of latitude of atmosphere grid */
	    min_atm_lat = 9999; /* dummy large value */
	    for(na=0; na<ntile_atm; na++) {
	       min_lat = minval_double((nxa[na]+1)*(nya[na]+1), yatm[na]);
	       if(min_atm_lat > min_lat) min_atm_lat = min_lat;
	    }
	    if(tmpy[0]*D2R > min_atm_lat + TINY_VALUE) { /* extend one point in south direction*/
	       ocn_south_ext = 1;
	    }
	 }      
	 nxo[n] /= x_refine;
	 nyo[n] /= y_refine;
	 nyo[n] += ocn_south_ext;
	 xocn[n]     = (double *)malloc((nxo[n]+1)*(nyo[n]+1)*sizeof(double));
	 yocn[n]     = (double *)malloc((nxo[n]+1)*(nyo[n]+1)*sizeof(double));
	 area_ocn[n] = (double *)malloc((nxo[n]  )*(nyo[n]  )*sizeof(double));

	 for(j = 0; j < nyo[n]+1; j++) for(i = 0; i < nxo[n]+1; i++) {
	       xocn[n][(j+ocn_south_ext)*(nxo[n]+1)+i] = tmpx[(j*y_refine)*(nxo[n]*x_refine+1)+i*x_refine] * D2R;
	       yocn[n][(j+ocn_south_ext)*(nxo[n]+1)+i] = tmpy[(j*y_refine)*(nxo[n]*x_refine+1)+i*x_refine] * D2R;
	    }
	 if(ocn_south_ext==1) {
	    for(i=0; i<nxo[n]+1; i++) {
	       xocn[n][i] = xocn[n][nxo[n]+1+i];
	       yocn[n][i] = min_atm_lat;
	    }
	 }
	 free(tmpx);
	 free(tmpy);
	 get_grid_area(nxo+n, nyo+n, xocn[n], yocn[n], area_ocn[n]);
	 mpp_close(g_fid);
      }
      mpp_close(m_fid);
    
      /* read ocean topography */
      t_fid = mpp_open(otopog, MPP_READ);
      ntiles = mpp_get_dimlen(t_fid, "ntiles");
      if(ntile_ocn != ntiles) mpp_error("make_coupler_mosaic: dimlen ntiles in mosaic file is not the same as dimlen in topog file");
      omask = (double **)malloc(ntile_ocn*sizeof(double *));
      for(n=0; n<ntile_ocn; n++) {
	 char name[128];
	 int nx, ny, i, j;
	 double *depth;

	 if(ntiles == 1)
	    strcpy(name, "nx");
	 else
	    sprintf(name, "nx_tile%d", n+1);
	 nx = mpp_get_dimlen(t_fid, name);
	 if(ntiles == 1)
	    strcpy(name, "ny");
	 else
	    sprintf(name, "ny_tile%d", n+1);
	 ny = mpp_get_dimlen(t_fid, name);
	 if( nx != nxo[n] || ny+ocn_south_ext != nyo[n]) mpp_error("make_coupler_mosaic: grid size mismatch between mosaic file and topog file");
	 if(ntiles == 1)
	    strcpy(name, "depth");
	 else
	    sprintf(name, "depth_tile%d", n+1);
	 depth    = (double *)malloc(nx*ny*sizeof(double));
	 omask[n] = (double *)malloc(nxo[n]*nyo[n]*sizeof(double));
	 vid = mpp_get_varid(t_fid, name);
	 mpp_get_var_value(t_fid, vid, depth);
	 for(i=0; i<nxo[n]*nyo[n]; i++) omask[n][i] = 0;
	 for(j=0; j<ny; j++) for(i=0; i<nx; i++) {
	       if(depth[j*nx+i] >sea_level) omask[n][(j+ocn_south_ext)*nx+i] = 1;
	    }
	 free(depth);
      }
      mpp_close(t_fid);
   }    

  
   /* Either omosaic is different from both lmosaic and amosaic,
      or all the three mosaic are the same
   */
   if(strcmp(omosaic_name, amosaic_name)) { /* omosaic is different from amosaic */
      if(!strcmp(omosaic_name, lmosaic_name)) mpp_error("make_coupler_mosaic: omosaic is the same as lmosaic, "
      "but different from amosaic.");
      same_mosaic = 0;
   }
   else { /* omosaic is same as amosaic */
      if(strcmp(omosaic_name, lmosaic_name)) mpp_error("make_coupler_mosaic: omosaic is the same as amosaic, "
      "but different from lmosaic.");
      same_mosaic = 1;
   }
    
    
   /***************************************************************************************
     First generate the exchange grid between atmos mosaic and land/ocean mosaic              
   ***************************************************************************************/
   nfile_axo = 0;
   nfile_axl = 0;
   nfile_lxo = 0;
   {
      int no, nl, na, n;
      size_t  **naxl, **naxo;
      int     ***atmxlnd_ia,   ***atmxlnd_ja,   ***atmxlnd_il,   ***atmxlnd_jl;
      int     ***atmxocn_ia,   ***atmxocn_ja,   ***atmxocn_io,   ***atmxocn_jo;
      double  ***atmxlnd_area, ***atmxlnd_dia,  ***atmxlnd_dja,  ***atmxlnd_dil,  ***atmxlnd_djl;
      double  ***atmxocn_area, ***atmxocn_dia,  ***atmxocn_dja,  ***atmxocn_dio,  ***atmxocn_djo;
      double  ***atmxocn_clon, ***atmxocn_clat, ***atmxlnd_clon, ***atmxlnd_clat;
      double   min_area; 

      naxl         = (size_t ** )malloc(ntile_atm*sizeof(size_t *));
      naxo         = (size_t ** )malloc(ntile_atm*sizeof(size_t *));
      atmxlnd_area = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxlnd_ia   = (int    ***)malloc(ntile_atm*sizeof(int    **));
      atmxlnd_ja   = (int    ***)malloc(ntile_atm*sizeof(int    **));
      atmxlnd_il   = (int    ***)malloc(ntile_atm*sizeof(int    **));
      atmxlnd_jl   = (int    ***)malloc(ntile_atm*sizeof(int    **));
      atmxocn_area = (double ***)malloc(ntile_atm*sizeof(double **));
      atmxocn_ia   = (int    ***)malloc(ntile_atm*sizeof(int    **));
      atmxocn_ja   = (int    ***)malloc(ntile_atm*sizeof(int    **));
      atmxocn_io   = (int    ***)malloc(ntile_atm*sizeof(int    **));
      atmxocn_jo   = (int    ***)malloc(ntile_atm*sizeof(int    **));

      if(interp_order == 2 ) {
	 atmxlnd_dia  = (double ***)malloc(ntile_atm*sizeof(double **));
	 atmxlnd_dja  = (double ***)malloc(ntile_atm*sizeof(double **));
	 atmxlnd_dil  = (double ***)malloc(ntile_atm*sizeof(double **));
	 atmxlnd_djl  = (double ***)malloc(ntile_atm*sizeof(double **));
	 atmxocn_dia  = (double ***)malloc(ntile_atm*sizeof(double **));
	 atmxocn_dja  = (double ***)malloc(ntile_atm*sizeof(double **));
	 atmxocn_dio  = (double ***)malloc(ntile_atm*sizeof(double **));
	 atmxocn_djo  = (double ***)malloc(ntile_atm*sizeof(double **));      
	 atmxlnd_clon = (double ***)malloc(ntile_atm*sizeof(double **));
	 atmxlnd_clat = (double ***)malloc(ntile_atm*sizeof(double **));
	 atmxocn_clon = (double ***)malloc(ntile_atm*sizeof(double **));
	 atmxocn_clat = (double ***)malloc(ntile_atm*sizeof(double **));      
      }
    
      for(na=0; na<ntile_atm; na++) {
	 naxl[na]         = (size_t * )malloc(ntile_lnd*sizeof(size_t));
	 naxo[na]         = (size_t * )malloc(ntile_ocn*sizeof(size_t));
	 atmxlnd_area[na] = (double **)malloc(ntile_lnd*sizeof(double *));
	 atmxlnd_ia[na]   = (int    **)malloc(ntile_lnd*sizeof(int    *));
	 atmxlnd_ja[na]   = (int    **)malloc(ntile_lnd*sizeof(int    *));
	 atmxlnd_il[na]   = (int    **)malloc(ntile_lnd*sizeof(int    *));
	 atmxlnd_jl[na]   = (int    **)malloc(ntile_lnd*sizeof(int    *));
	 atmxocn_area[na] = (double **)malloc(ntile_ocn*sizeof(double *));
	 atmxocn_ia[na]   = (int    **)malloc(ntile_ocn*sizeof(int    *));
	 atmxocn_ja[na]   = (int    **)malloc(ntile_ocn*sizeof(int    *));
	 atmxocn_io[na]   = (int    **)malloc(ntile_ocn*sizeof(int    *));
	 atmxocn_jo[na]   = (int    **)malloc(ntile_ocn*sizeof(int    *));
    
	 if(interp_order == 2 ) {
	    atmxlnd_dia [na] = (double **)malloc(ntile_lnd*sizeof(double *));
	    atmxlnd_dja [na] = (double **)malloc(ntile_lnd*sizeof(double *));
	    atmxlnd_dil [na] = (double **)malloc(ntile_lnd*sizeof(double *));
	    atmxlnd_djl [na] = (double **)malloc(ntile_lnd*sizeof(double *));
	    atmxocn_dia [na] = (double **)malloc(ntile_ocn*sizeof(double *));
	    atmxocn_dja [na] = (double **)malloc(ntile_ocn*sizeof(double *));
	    atmxocn_dio [na] = (double **)malloc(ntile_ocn*sizeof(double *));
	    atmxocn_djo [na] = (double **)malloc(ntile_ocn*sizeof(double *));
	    atmxlnd_clon[na] = (double **)malloc(ntile_lnd*sizeof(double *));
	    atmxlnd_clat[na] = (double **)malloc(ntile_lnd*sizeof(double *));
	    atmxocn_clon[na] = (double **)malloc(ntile_ocn*sizeof(double *));
	    atmxocn_clat[na] = (double **)malloc(ntile_ocn*sizeof(double *));      
	 }

	 for(nl=0; nl<ntile_lnd; nl++) {
	    atmxlnd_area[na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	    atmxlnd_ia  [na][nl] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	    atmxlnd_ja  [na][nl] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	    atmxlnd_il  [na][nl] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	    atmxlnd_jl  [na][nl] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	    if(interp_order == 2 ) {
	       atmxlnd_clon[na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	       atmxlnd_clat[na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	       atmxlnd_dia [na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	       atmxlnd_dja [na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	       atmxlnd_dil [na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	       atmxlnd_djl [na][nl] = (double *)malloc(MAXXGRID*sizeof(double));
	    }
	 }
 
	 for(no=0; no<ntile_ocn; no++) {
	    atmxocn_area[na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	    atmxocn_ia  [na][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	    atmxocn_ja  [na][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	    atmxocn_io  [na][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	    atmxocn_jo  [na][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));          
	    if(interp_order == 2 ) {
	       atmxocn_clon[na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	       atmxocn_clat[na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	       atmxocn_dia [na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	       atmxocn_dja [na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	       atmxocn_dio [na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	       atmxocn_djo [na][no] = (double *)malloc(MAXXGRID*sizeof(double));
	    }
	 }
      }
      
      for(na=0; na<ntile_atm; na++) {
      
	 int      l, is, ie, js, je, la, ia, ja, il, jl, io, jo, layout[2];
	 int      n0, n1, n2, n3, na_in, nl_in, no_in, n_out, n_out2;
	 double   xa_min, ya_min, xo_min, yo_min, xl_min, yl_min, xa_avg;
	 double   xa_max, ya_max, xo_max, yo_max, xl_max, yl_max;
	 double   xarea;
	 double   xa[MV], ya[MV], xl[MV], yl[MV], xo[MV], yo[MV];
	 double   x_out[MV], y_out[MV];
	 double   atmxlnd_x[MX][MV], atmxlnd_y[MX][MV];
	 int      num_v[MX];
	 int      axl_i[MX], axl_j[MX], axl_t[MX];
	 double   axl_xmin[MX], axl_xmax[MX], axl_ymin[MX], axl_ymax[MX];
	 double   axl_area[MX], axl_clon[MX], axl_clat[MX];
	 size_t   count;
	 domain2D Dom;
      
	 for(nl=0; nl<ntile_lnd; nl++) naxl[na][nl] = 0;
	 for(no=0; no<ntile_ocn; no++) naxo[na][no] = 0;
	 layout[0] = mpp_npes();
	 layout[1] = 1;
        
	 mpp_define_domain2d(nxa[na]*nya[na], 1, layout, 0, 0, &Dom);
	 mpp_get_compute_domain2d(Dom, &is, &ie, &js, &je );
	 for(la=is;la<=ie;la++) {
	
	    ia = la%nxa[na];
	    ja = la/nxa[na];
	    n0 = ja    *(nxa[na]+1) + ia;
	    n1 = ja    *(nxa[na]+1) + ia+1;
	    n2 = (ja+1)*(nxa[na]+1) + ia+1;
	    n3 = (ja+1)*(nxa[na]+1) + ia;
	    xa[0] = xatm[na][n0]; ya[0] = yatm[na][n0];
	    xa[1] = xatm[na][n1]; ya[1] = yatm[na][n1];
	    xa[2] = xatm[na][n2]; ya[2] = yatm[na][n2];
	    xa[3] = xatm[na][n3]; ya[3] = yatm[na][n3];
	    ya_min  = minval_double(4, ya);
	    ya_max  = maxval_double(4, ya);
	    na_in   = fix_lon(xa, ya, 4, M_PI);
	    xa_min  = minval_double(na_in, xa);
	    xa_max  = maxval_double(na_in, xa);
	    xa_avg  = avgval_double(na_in, xa);
	    count = 0;
	    for(nl=0; nl<ntile_lnd; nl++) {
	       for(jl = 0; jl < nyl[nl]; jl ++) for(il = 0; il < nxl[nl]; il++) {
		     n0 = jl    *(nxl[nl]+1) + il;
		     n1 = jl    *(nxl[nl]+1) + il+1;
		     n2 = (jl+1)*(nxl[nl]+1) + il+1;
		     n3 = (jl+1)*(nxl[nl]+1) + il;
		     xl[0] = xlnd[nl][n0]; yl[0] = ylnd[nl][n0];
		     xl[1] = xlnd[nl][n1]; yl[1] = ylnd[nl][n1];
		     xl[2] = xlnd[nl][n2]; yl[2] = ylnd[nl][n2];
		     xl[3] = xlnd[nl][n3]; yl[3] = ylnd[nl][n3];
		     yl_min = minval_double(4, yl);
		     yl_max = maxval_double(4, yl);
		     if(yl_min >= ya_max || yl_max <= ya_min ) continue;	    
		     nl_in  = fix_lon(xl, yl, 4, xa_avg);
		     xl_min = minval_double(nl_in, xl);
		     xl_max = maxval_double(nl_in, xl);
		     /* xl should in the same range as xa after lon_fix, so no need to
			consider cyclic condition
		     */
	      	    
		     if(xa_min >= xl_max || xa_max <= xl_min ) continue;	
		     if (  (n_out = clip_2dx2d( xa, ya, na_in, xl, yl, nl_in, x_out, y_out )) > 0 ) {
			xarea = poly_area(x_out, y_out, n_out);
			min_area = min(area_lnd[nl][jl*nxl[nl]+il], area_atm[na][la]);
			if( xarea/min_area > AREA_RATIO_THRESH ) {
			   /*  remember the exchange grid vertices */
			   for(n=0; n<n_out; n++) {
			      atmxlnd_x[count][n] = x_out[n];
			      atmxlnd_y[count][n] = y_out[n];
			   }
			   axl_i[count]    = il;
			   axl_j[count]    = jl;
			   axl_t[count]    = nl;
			   num_v[count]    = n_out;
			   axl_xmin[count] = minval_double(n_out, x_out);
			   axl_xmax[count] = maxval_double(n_out, x_out);
			   axl_ymin[count] = minval_double(n_out, y_out);
			   axl_ymax[count] = maxval_double(n_out, y_out);
			   axl_area[count] = 0;
			   if(interp_order == 2) {
			      axl_clon[count] = 0;
			      axl_clat[count] = 0;
			   }
			   ++count;
			   if(count>MX) mpp_error("make_coupler_mosaic: count is greater than MX, increase MX");
			}
		     }
		  }
	    }

	    /* calculate atmos/ocean x-cells */
	    for(no=0; no<ntile_ocn; no++) {
	       for(jo = 0; jo < nyo[no]; jo++) for(io = 0; io < nxo[no]; io++) {	
		     n0 = jo    *(nxo[no]+1) + io;
		     n1 = jo    *(nxo[no]+1) + io+1;
		     n2 = (jo+1)*(nxo[no]+1) + io+1;
		     n3 = (jo+1)*(nxo[no]+1) + io;
		     xo[0] = xocn[no][n0]; yo[0] = yocn[no][n0];
		     xo[1] = xocn[no][n1]; yo[1] = yocn[no][n1];
		     xo[2] = xocn[no][n2]; yo[2] = yocn[no][n2];
		     xo[3] = xocn[no][n3]; yo[3] = yocn[no][n3];
		     yo_min = minval_double(4, yo);
		     yo_max = maxval_double(4, yo);
		     no_in  = fix_lon(xo, yo, 4, xa_avg);
		     xo_min = minval_double(no_in, xo);
		     xo_max = maxval_double(no_in, xo);
		     if(omask[no][jo*nxo[no]+io] > 0.5) { /* over sea/ice */
			/* xo should in the same range as xa after lon_fix, so no need to
			   consider cyclic condition
			*/
			if(xa_min >= xo_max || xa_max <= xo_min || yo_min >= ya_max || yo_max <= ya_min ) continue;	    

			if (  (n_out = clip_2dx2d( xa, ya, na_in, xo, yo, no_in, x_out, y_out )) > 0) {
			   xarea = poly_area(x_out, y_out, n_out );
			   min_area = min(area_ocn[no][jo*nxo[no]+io], area_atm[na][la]);
			   if(xarea/min_area > AREA_RATIO_THRESH) {
	    
			      atmxocn_area[na][no][naxo[na][no]] = xarea;
			      atmxocn_io[na][no][naxo[na][no]]   = io;
			      atmxocn_jo[na][no][naxo[na][no]]   = jo;
			      atmxocn_ia[na][no][naxo[na][no]]   = ia;
			      atmxocn_ja[na][no][naxo[na][no]]   = ja;
			      if(interp_order == 2) {
				 atmxocn_clon[na][no][naxo[na][no]] = poly_ctrlon ( x_out, y_out, n_out, xa_avg);
				 atmxocn_clat[na][no][naxo[na][no]] = poly_ctrlat ( x_out, y_out, n_out );		
			      }
			      ++(naxo[na][no]);
			      if(naxo[na][no] > MAXXGRID) mpp_error("naxo is greater than MAXXGRID, increase MAXXGRID");
			   }
			}
		     }
		     else { /* over land */
			/* find the overlap of atmxlnd and ocean cell */
			for(l=0; l<count; l++) {
			   if(axl_xmin[l] >= xo_max || axl_xmax[l] <= xo_min || axl_ymin[l] >= ya_max || axl_ymax[l] <= ya_min ) continue;	  
			   if((n_out = clip_2dx2d( atmxlnd_x[l], atmxlnd_y[l], num_v[l], xo, yo, no_in, x_out, y_out )) > 0) {
			      xarea = poly_area(x_out, y_out, n_out );
			      min_area = min(area_lnd[axl_t[l]][axl_j[l]*nxl[axl_t[l]]+axl_i[l]], area_atm[na][la]);
			      if(xarea/min_area > AREA_RATIO_THRESH) {
				 axl_area[l] += xarea;
				 if(interp_order == 2) {
				    axl_clon[l] += poly_ctrlon ( x_out, y_out, n_out, xa_avg);
				    axl_clat[l] += poly_ctrlat ( x_out, y_out, n_out);
				 }
			      }
			   }
			}
		     }
		  }
	    }
	    /* get the exchange grid between land and atmos. */
	    for(l=0; l<count; l++) {
	       nl = axl_t[l];
	       min_area = min(area_lnd[nl][axl_j[l]*nxl[nl]+axl_i[l]], area_atm[na][la]);
	       if(axl_area[l]/min_area > AREA_RATIO_THRESH) {
		  atmxlnd_area[na][nl][naxl[na][nl]] = axl_area[l];
		  atmxlnd_ia  [na][nl][naxl[na][nl]] = ia;
		  atmxlnd_ja  [na][nl][naxl[na][nl]] = ja;
		  atmxlnd_il  [na][nl][naxl[na][nl]] = axl_i[l];
		  atmxlnd_jl  [na][nl][naxl[na][nl]] = axl_j[l];
		  if(interp_order == 2) {
		     atmxlnd_clon[na][nl][naxl[na][nl]] = axl_clon[l];
		     atmxlnd_clat[na][nl][naxl[na][nl]] = axl_clat[l];
		  }
		  ++(naxl[na][nl]);
		  if(naxl[na][nl] > MAXXGRID) mpp_error("naxl is greater than MAXXGRID, increase MAXXGRID");
	       }
	    }   
	 }/* end of la loop */

	 mpp_delete_domain2d(&Dom);
      } /* end of na loop */

      /* calculate the centroid of model grid, as well as land_mask and ocean_mask */
      {
	 double **l_area, **o_area;
	 int    nl, no, ll, lo;
	 l_area = (double **)malloc(ntile_lnd*sizeof(double *));
	 o_area = (double **)malloc(ntile_ocn*sizeof(double *));
	 for(nl =0; nl<ntile_lnd; nl++) {
	    l_area[nl] = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	    for(ll=0; ll<nxl[nl]*nyl[nl]; ll++) {
	       l_area[nl][ll] = 0;
	    }
	 }
	 for(no =0; no<ntile_ocn; no++) {
	    o_area[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	    for(lo=0; lo<nxo[no]*nyo[no]; lo++) {
	       o_area[no][lo] = 0;
	    }
	 }
            
	 if(interp_order == 1) {
	    for(na=0; na<ntile_atm; na++) {
	       for(nl=0; nl<ntile_lnd; nl++) {
		  int nxgrid;
	  
		  nxgrid = naxl[na][nl];
		  mpp_sum_int(1, &nxgrid);
		  if(nxgrid > 0) {
		     double *g_area;
		     int    *g_il, *g_jl;
		     int    ii;
		     g_il = (int    *)malloc(nxgrid*sizeof(int   ));
		     g_jl = (int    *)malloc(nxgrid*sizeof(int   ));	
		     g_area = (double *)malloc(nxgrid*sizeof(double));
		     mpp_gather_field_int   (naxl[na][nl], atmxlnd_il[na][nl], g_il);
		     mpp_gather_field_int   (naxl[na][nl], atmxlnd_jl[na][nl], g_jl);
		     mpp_gather_field_double(naxl[na][nl], atmxlnd_area[na][nl], g_area);
		     for(i=0; i<nxgrid; i++) {
			ii = g_jl[i]*nxl[nl]+g_il[i];
			l_area[nl][ii] += g_area[i];
		     }
		     free(g_il);
		     free(g_jl);
		     free(g_area);
		  }
	       }

	       for(no=0; no<ntile_ocn; no++) {
		  int nxgrid;
		  nxgrid = naxo[na][no];
		  mpp_sum_int(1, &nxgrid);
		  if(nxgrid > 0) {
		     double *g_area;
		     int    *g_io, *g_jo;
		     int    ii;
		     g_io = (int    *)malloc(nxgrid*sizeof(int   ));
		     g_jo = (int    *)malloc(nxgrid*sizeof(int   ));	
		     g_area = (double *)malloc(nxgrid*sizeof(double));
		     mpp_gather_field_int   (naxo[na][no], atmxocn_io[na][no], g_io);
		     mpp_gather_field_int   (naxo[na][no], atmxocn_jo[na][no], g_jo);
		     mpp_gather_field_double(naxo[na][no], atmxocn_area[na][no], g_area);
		     for(i=0; i<nxgrid; i++) {	      
			ii = g_jo[i]*nxo[no]+g_io[i];
			o_area[no][ii] += g_area[i];
		     }
		     free(g_io);
		     free(g_jo);
		     free(g_area);
		  }
	       }
	    }
	 }
	 else { /* interp_order == 2 */
	    double **l_clon, **l_clat;
	    double **o_clon, **o_clat;
	    double  *a_area,  *a_clon,  *a_clat;
	    int la;
      
	    l_clon = (double **)malloc(ntile_lnd*sizeof(double *));
	    l_clat = (double **)malloc(ntile_lnd*sizeof(double *));
	    for(nl =0; nl<ntile_lnd; nl++) {
	       l_clon[nl] = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	       l_clat[nl] = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	       for(ll=0; ll<nxl[nl]*nyl[nl]; ll++) {
		  l_clon[nl][ll] = 0;
		  l_clat[nl][ll] = 0;
	       }
	    }
	    o_clon = (double **)malloc(ntile_ocn*sizeof(double *));
	    o_clat = (double **)malloc(ntile_ocn*sizeof(double *));
	    for(no =0; no<ntile_ocn; no++) {
	       o_clon[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	       o_clat[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	       for(lo=0; lo<nxo[no]*nyo[no]; lo++) {
		  o_clon[no][lo] = 0;
		  o_clat[no][lo] = 0;
	       }
	    }	
	    for(na=0; na<ntile_atm; na++) {
	       //	double *area, *clon, *clat;
      
	       a_area = (double *)malloc(nxa[na]*nya[na]*sizeof(double));
	       a_clon = (double *)malloc(nxa[na]*nya[na]*sizeof(double));
	       a_clat = (double *)malloc(nxa[na]*nya[na]*sizeof(double));
	       for(la=0; la<nxa[na]*nya[na]; la++) {
		  a_area[la] = 0;
		  a_clon[la] = 0;
		  a_clat[la] = 0;
	       }

	       for(nl=0; nl<ntile_lnd; nl++) {
		  int nxgrid;
	  
		  nxgrid = naxl[na][nl];
		  mpp_sum_int(1, &nxgrid);
		  if(nxgrid > 0) {
		     double *g_area, *g_clon, *g_clat;
		     int    *g_ia,   *g_ja,   *g_il, *g_jl;
		     int    ii;
		     g_ia = (int    *)malloc(nxgrid*sizeof(int   ));
		     g_ja = (int    *)malloc(nxgrid*sizeof(int   ));
		     g_il = (int    *)malloc(nxgrid*sizeof(int   ));
		     g_jl = (int    *)malloc(nxgrid*sizeof(int   ));	
		     g_area = (double *)malloc(nxgrid*sizeof(double));
		     g_clon = (double *)malloc(nxgrid*sizeof(double));
		     g_clat = (double *)malloc(nxgrid*sizeof(double));
		     mpp_gather_field_int   (naxl[na][nl], atmxlnd_ia[na][nl], g_ia);
		     mpp_gather_field_int   (naxl[na][nl], atmxlnd_ja[na][nl], g_ja);
		     mpp_gather_field_int   (naxl[na][nl], atmxlnd_il[na][nl], g_il);
		     mpp_gather_field_int   (naxl[na][nl], atmxlnd_jl[na][nl], g_jl);
		     mpp_gather_field_double(naxl[na][nl], atmxlnd_area[na][nl], g_area);
		     mpp_gather_field_double(naxl[na][nl], atmxlnd_clon[na][nl], g_clon);
		     mpp_gather_field_double(naxl[na][nl], atmxlnd_clat[na][nl], g_clat);
		     for(i=0; i<nxgrid; i++) {
			ii = g_ja[i]*nxa[na]+g_ia[i];
			a_area[ii] += g_area[i];
			a_clon[ii] += g_clon[i];
			a_clat[ii] += g_clat[i];
			ii = g_jl[i]*nxl[nl]+g_il[i];
			l_area[nl][ii] += g_area[i];
			l_clon[nl][ii] += g_clon[i];
			l_clat[nl][ii] += g_clat[i];
		     }
		     free(g_ia);
		     free(g_ja);
		     free(g_il);
		     free(g_jl);
		     free(g_area);
		     free(g_clon);
		     free(g_clat);
		  }
	       }

	       for(no=0; no<ntile_ocn; no++) {
		  int nxgrid;
		  nxgrid = naxo[na][no];
		  mpp_sum_int(1, &nxgrid);
		  if(nxgrid > 0) {
		     double *g_area, *g_clon, *g_clat;
		     int    *g_ia,   *g_ja,   *g_io, *g_jo;
		     int    ii;
		     g_ia = (int    *)malloc(nxgrid*sizeof(int   ));
		     g_ja = (int    *)malloc(nxgrid*sizeof(int   ));
		     g_io = (int    *)malloc(nxgrid*sizeof(int   ));
		     g_jo = (int    *)malloc(nxgrid*sizeof(int   ));	
		     g_area = (double *)malloc(nxgrid*sizeof(double));
		     g_clon = (double *)malloc(nxgrid*sizeof(double));
		     g_clat = (double *)malloc(nxgrid*sizeof(double));
		     mpp_gather_field_int   (naxo[na][no], atmxocn_ia[na][no], g_ia);
		     mpp_gather_field_int   (naxo[na][no], atmxocn_ja[na][no], g_ja);
		     mpp_gather_field_int   (naxo[na][no], atmxocn_io[na][no], g_io);
		     mpp_gather_field_int   (naxo[na][no], atmxocn_jo[na][no], g_jo);
		     mpp_gather_field_double(naxo[na][no], atmxocn_area[na][no], g_area);
		     mpp_gather_field_double(naxo[na][no], atmxocn_clon[na][no], g_clon);
		     mpp_gather_field_double(naxo[na][no], atmxocn_clat[na][no], g_clat);
		     for(i=0; i<nxgrid; i++) {	      
			ii = g_ja[i]*nxa[na]+g_ia[i];
			a_area[ii] += g_area[i];
			a_clon[ii] += g_clon[i];
			a_clat[ii] += g_clat[i];
			ii = g_jo[i]*nxo[no]+g_io[i];
			o_area[no][ii] += g_area[i];
			o_clon[no][ii] += g_clon[i];
			o_clat[no][ii] += g_clat[i];
		     }
		     free(g_ia);
		     free(g_ja);
		     free(g_io);
		     free(g_jo);
		     free(g_area);
		     free(g_clon);
		     free(g_clat);
		  }
	       }

	       for(la=0; la<nxa[na]*nya[na]; la++) {
		  if(a_area[la] > 0) {
		     a_clon[la] /= a_area[la];
		     a_clat[la] /= a_area[la];
		  }
	       }
	
	       /* substract atmos centroid to get the centroid distance between atmos grid and exchange grid. */
	       for(nl=0; nl<ntile_lnd; nl++) {
		  for(i=0; i<naxl[na][nl]; i++) {
		     la = atmxlnd_ja[na][nl][i]*nxa[na] + atmxlnd_ia[na][nl][i];
		     atmxlnd_dia[na][nl][i] = atmxlnd_clon[na][nl][i]/atmxlnd_area[na][nl][i] - a_clon[la];
		     atmxlnd_dja[na][nl][i] = atmxlnd_clat[na][nl][i]/atmxlnd_area[na][nl][i] - a_clat[la];
		  }
	       }
	       for(no=0; no<ntile_ocn; no++) {
		  for(i=0; i<naxo[na][no]; i++) {
		     la = atmxocn_ja[na][no][i]*nxa[na] + atmxocn_ia[na][no][i];
		     atmxocn_dia[na][no][i] = atmxocn_clon[na][no][i]/atmxocn_area[na][no][i] - a_clon[la];
		     atmxocn_dja[na][no][i] = atmxocn_clat[na][no][i]/atmxocn_area[na][no][i] - a_clat[la];
		  }
	       }
	
	       free(a_area);
	       free(a_clon);
	       free(a_clat);
	    }

      
	    /* centroid distance from exchange grid to land grid */
	    for(nl=0; nl<ntile_lnd; nl++) {
	       for(ll=0; ll<nxl[nl]*nyl[nl]; ll++) {
		  if(l_area[nl][ll] > 0) {
		     l_clon[nl][ll] /= l_area[nl][ll];
		     l_clat[nl][ll] /= l_area[nl][ll];
		  }
	       }
	       for(na=0; na<ntile_atm; na++) {
		  for(i=0; i<naxl[na][nl]; i++) {
		     ll = atmxlnd_jl[na][nl][i]*nxl[nl] + atmxlnd_il[na][nl][i];
		     atmxlnd_dil[na][nl][i] = atmxlnd_clon[na][nl][i]/atmxlnd_area[na][nl][i] - l_clon[nl][ll];
		     atmxlnd_djl[na][nl][i] = atmxlnd_clat[na][nl][i]/atmxlnd_area[na][nl][i] - l_clat[nl][ll];
		  }
	       }
	       free(l_clon[nl]);
	       free(l_clat[nl]);
	    }

	    /* centroid distance from exchange grid to ocean grid */
	    for(no=0; no<ntile_ocn; no++) {
	       for(lo=0; lo<nxo[no]*nyo[no]; lo++) {
		  if(o_area[no][lo] > 0) {
		     o_clon[no][lo] /= o_area[no][lo];
		     o_clat[no][lo] /= o_area[no][lo];
		  }
	       }
	       for(na=0; na<ntile_atm; na++) {
		  for(i=0; i<naxo[na][no]; i++) {
		     lo = atmxocn_jo[na][no][i]*nxo[no] + atmxocn_io[na][no][i];
		     atmxocn_dio[na][no][i] = atmxocn_clon[na][no][i]/atmxocn_area[na][no][i] - o_clon[no][lo];
		     atmxocn_djo[na][no][i] = atmxocn_clat[na][no][i]/atmxocn_area[na][no][i] - o_clat[no][lo];
		  }
	       }
	       free(o_clon[no]);
	       free(o_clat[no]);
	    }
	    free(o_clon);
	    free(o_clat);
	    free(l_clon);
	    free(l_clat);  
	 }

	 /* calculate ocean_frac and compare ocean_frac with omask */
	 /* also write out ocn_frac */
	 {
	    int    io, jo;
	    double ocn_frac;
	    int    id_mask, fid, dims[2];
	    char ocn_mask_file[STRING];
	    double *mask;
	    int ny;


	    for(no=0; no<ntile_ocn; no++) {
	       ny = nyo[no]-ocn_south_ext;
	       mask = (double *)malloc(nxo[no]*ny*sizeof(double));
	       for(jo=0; jo<ny; jo++) for(io=0; io<nxo[no]; io++) {
		     i = (jo+ocn_south_ext)*nxo[no]+io;
		     ocn_frac = o_area[no][i]/area_ocn[no][i];
		     if( fabs(omask[no][i] - ocn_frac) > TOLORENCE ) {
			printf("at ocean point (%d,%d), omask = %f, ocn_frac = %f, diff = %f\n",
			io, jo, omask[no][i], ocn_frac, omask[no][i] - ocn_frac);
			mpp_error("make_coupler_mosaic: omask is not equal ocn_frac");
		     }
		     mask[jo*nxo[no]+io] = ocn_frac;
		  }
	       if(ntile_ocn > 1)
		  sprintf(ocn_mask_file, "ocean_mask_tile%d.nc", no+1);
	       else
		  strcpy(ocn_mask_file, "ocean_mask.nc");
	       fid = mpp_open(ocn_mask_file, MPP_WRITE);
	       mpp_def_global_att(fid, "grid_version", GRID_VERSION);
	       mpp_def_global_att(fid, "code_version", TAGNAME);
	       mpp_def_global_att(fid, "history", history);
          	  

	       dims[1] = mpp_def_dim(fid, "nx", nxo[no]); 
	       dims[0] = mpp_def_dim(fid, "ny", ny);
	       id_mask = mpp_def_var(fid, "mask", MPP_DOUBLE, 2, dims,  2, "standard_name",
	       "ocean fraction at T-cell centers", "units", "none");
	       mpp_end_def(fid);
	       mpp_put_var_value(fid, id_mask, mask);
	       mpp_close(fid);
	       free(mask);
	    }
	 }

	 /* calculate land_frac and  write out land_frac */
	 {
	    int    il, jl;
	    int    id_mask, fid, dims[2];
	    char lnd_mask_file[STRING];
	    double *mask;
	
	    for(nl=0; nl<ntile_lnd; nl++) {
	       mask = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	       for(jl=0; jl<nyl[nl]; jl++) for(il=0; il<nxl[nl]; il++) {
		     i = jl*nxl[nl]+il;
		     mask[i] = l_area[nl][i]/area_lnd[nl][i];
		  }
	       if(ntile_lnd > 1)
		  sprintf(lnd_mask_file, "land_mask_tile%d.nc", nl+1);
	       else
		  strcpy(lnd_mask_file, "land_mask.nc");
	       fid = mpp_open(lnd_mask_file, MPP_WRITE);
	       mpp_def_global_att(fid, "grid_version", GRID_VERSION);
	       mpp_def_global_att(fid, "code_version", TAGNAME);
	       mpp_def_global_att(fid, "history", history);
	       dims[1] = mpp_def_dim(fid, "nx", nxl[nl]); 
	       dims[0] = mpp_def_dim(fid, "ny", nyl[nl]);
	       id_mask = mpp_def_var(fid, "mask", MPP_DOUBLE, 2, dims,  2, "standard_name",
	       "land fraction at T-cell centers", "units", "none");
	       mpp_end_def(fid);
	       mpp_put_var_value(fid, id_mask, mask);
	       free(mask);
	       mpp_close(fid);
	    }
	 }        
      
	 for(nl=0; nl<ntile_lnd; nl++) free(l_area[nl]);
	 for(no=0; no<ntile_ocn; no++) free(o_area[no]);      
	 free(o_area);
	 free(l_area);
      }
    
  
      for(na=0; na<ntile_atm; na++) {
	 /* write out atmXlnd data*/
	 for(nl = 0; nl < ntile_lnd; nl++) {
	    int nxgrid;
	    nxgrid = naxl[na][nl];
	    mpp_sum_int(1, &nxgrid);
	    if(nxgrid>0) {
	       size_t start[4], nwrite[4];
	       int *gdata_int;
	       double *gdata_dbl;
	  
	       int fid, dim_string, dim_ncells, dim_two, dims[4];
	       int id_xgrid_area, id_contact, n;
	       int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;
	       char contact[STRING];

	       for(i=0; i<4; i++) {
		  start[i] = 0; nwrite[i] = 1;
	       }	  	  
	       if(same_mosaic)
		  sprintf(axl_file[nfile_axl], "atm_%s_%sXlnd_%s_%s.nc", amosaic_name, atile_name[na], lmosaic_name, ltile_name[nl]);
	       else
		  sprintf(axl_file[nfile_axl], "%s_%sX%s_%s.nc", amosaic_name, atile_name[na], lmosaic_name, ltile_name[nl]);
	       sprintf(contact, "%s:%s::%s:%s", amosaic_name, atile_name[na], lmosaic_name, ltile_name[nl]);
	       fid = mpp_open(axl_file[nfile_axl], MPP_WRITE);
	       mpp_def_global_att(fid, "grid_version", GRID_VERSION);
	       mpp_def_global_att(fid, "code_version", TAGNAME);
	       mpp_def_global_att(fid, "history", history);
	       dim_string = mpp_def_dim(fid, "string", STRING);
	       dim_ncells = mpp_def_dim(fid, "ncells", nxgrid);
	       dim_two    = mpp_def_dim(fid, "two", 2);
	       if(interp_order == 2) {
		  id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 7, "standard_name", "grid_contact_spec",
		  "contact_type", "exchange", "parent1_cell",
		  "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area", 
		  "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
	       }
	       else {
		  id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 5, "standard_name", "grid_contact_spec",
		  "contact_type", "exchange", "parent1_cell",
		  "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area");
	       }
	    
	       dims[0] = dim_ncells; dims[1] = dim_two;
	       id_tile1_cell = mpp_def_var(fid, "tile1_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic1");
	       id_tile2_cell = mpp_def_var(fid, "tile2_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic2");
	       id_xgrid_area = mpp_def_var(fid, "xgrid_area", MPP_DOUBLE, 1, &dim_ncells, 2, "standard_name",
	       "exchange_grid_area", "units", "m2");
	       if(interp_order == 2) {
		  id_tile1_dist = mpp_def_var(fid, "tile1_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent1_cell_centroid");
		  id_tile2_dist = mpp_def_var(fid, "tile2_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent2_cell_centroid");
	       }
	       mpp_end_def(fid);

	       /* the index will start from 1, instead of 0 ( fortran index) */
	       for(i = 0;i < naxl[na][nl]; i++) {
		  ++(atmxlnd_ia[na][nl][i]);
		  ++(atmxlnd_ja[na][nl][i]);
		  ++(atmxlnd_il[na][nl][i]);
		  ++(atmxlnd_jl[na][nl][i]);
	       }
	       nwrite[0] = strlen(contact);
	       mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
	       nwrite[0] = nxgrid;

	       gdata_int = (int *)malloc(nxgrid*sizeof(int));
	       gdata_dbl = (double *)malloc(nxgrid*sizeof(double));

	       mpp_gather_field_double(naxl[na][nl], atmxlnd_area[na][nl], gdata_dbl);
	       if(check) {
		  for(n=0; n<nxgrid; n++) axl_area_sum += gdata_dbl[n];
	       }
	       mpp_put_var_value(fid, id_xgrid_area, gdata_dbl);
	       mpp_gather_field_int(naxl[na][nl], atmxlnd_ia[na][nl], gdata_int);
	       mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	       mpp_gather_field_int(naxl[na][nl], atmxlnd_il[na][nl], gdata_int);
	       mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	       start[1] = 1;
	       mpp_gather_field_int(naxl[na][nl], atmxlnd_ja[na][nl], gdata_int);
	       mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	       mpp_gather_field_int(naxl[na][nl], atmxlnd_jl[na][nl], gdata_int);
	       mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	       if(interp_order == 2) {
		  start[1] = 0;
		  mpp_gather_field_double(naxl[na][nl], atmxlnd_dia[na][nl], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
		  mpp_gather_field_double(naxl[na][nl], atmxlnd_dil[na][nl], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
		  start[1] = 1;
		  mpp_gather_field_double(naxl[na][nl], atmxlnd_dja[na][nl], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
		  mpp_gather_field_double(naxl[na][nl], atmxlnd_djl[na][nl], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
	       }
	       mpp_close(fid);
	       free(gdata_int);
	       free(gdata_dbl);
	       ++nfile_axl;
	    }
	 } /* end of nl loop */

	 /* write out atmXocn data */
	 for(no = 0; no < ntile_ocn; no++) {
	    int nxgrid;
	
	    nxgrid = naxo[na][no];
	    mpp_sum_int(1, &nxgrid);
	    if(nxgrid>0) {
	       size_t start[4], nwrite[4];
	       int *gdata_int;
	       double *gdata_dbl;
	       int fid, dim_string, dim_ncells, dim_two, dims[4];
	       int id_xgrid_area, id_contact, n;
	       int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;	  
	       char contact[STRING];

	       for(i=0; i<4; i++) {
		  start[i] = 0; nwrite[i] = 1;
	       }
	  
	       if(same_mosaic)
		  sprintf(axo_file[nfile_axo], "atm_%s_%sXocn_%s_%s.nc", amosaic_name, atile_name[na], omosaic_name, otile_name[no]);
	       else
		  sprintf(axo_file[nfile_axo], "%s_%sX%s_%s.nc", amosaic_name, atile_name[na], omosaic_name, otile_name[no]);
	  
	       sprintf(contact, "%s:%s::%s:%s", amosaic_name, atile_name[na], omosaic_name, otile_name[no]);
	       fid = mpp_open(axo_file[nfile_axo], MPP_WRITE);
	       mpp_def_global_att(fid, "grid_version", GRID_VERSION);
	       mpp_def_global_att(fid, "code_version", TAGNAME);
	       mpp_def_global_att(fid, "history", history);
	       dim_string = mpp_def_dim(fid, "string", STRING);
	       dim_ncells = mpp_def_dim(fid, "ncells", nxgrid);
	       dim_two    = mpp_def_dim(fid, "two", 2);
	       if(interp_order == 2) {
		  id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 7, "standard_name", "grid_contact_spec",
		  "contact_type", "exchange", "parent1_cell",
		  "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area", 
		  "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
	       }
	       else {
		  id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 5, "standard_name", "grid_contact_spec",
		  "contact_type", "exchange", "parent1_cell",
		  "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area" );
	       }
	       dims[0] = dim_ncells; dims[1] = dim_two;
	       id_tile1_cell = mpp_def_var(fid, "tile1_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic1");
	       id_tile2_cell = mpp_def_var(fid, "tile2_cell", MPP_INT, 2, dims, 1, "standard_name", "parent_cell_indices_in_mosaic2");
	       id_xgrid_area = mpp_def_var(fid, "xgrid_area", MPP_DOUBLE, 1, &dim_ncells, 2, "standard_name",
	       "exchange_grid_area", "units", "m2");
	       if(interp_order == 2) {
		  id_tile1_dist = mpp_def_var(fid, "tile1_distance", MPP_DOUBLE, 2, dims, 1, "standard_name",
		  "distance_from_parent1_cell_centroid");
		  id_tile2_dist = mpp_def_var(fid, "tile2_distance", MPP_DOUBLE, 2, dims, 1, "standard_name",
		  "distance_from_parent2_cell_centroid");
	       }
	       mpp_end_def(fid);

	       /* the index will start from 1, instead of 0 ( fortran index) */
	       for(i = 0;i < naxo[na][no]; i++) {
		  ++(atmxocn_ia[na][no][i]);
		  ++(atmxocn_ja[na][no][i]);
		  ++(atmxocn_io[na][no][i]);
		  atmxocn_jo[na][no][i] += 1-ocn_south_ext; /* possible one artificial j-level is added at south end */
	       }

	       nwrite[0] = strlen(contact);
	       mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
	
	       nwrite[0] = nxgrid;

	       gdata_int = (int *)malloc(nxgrid*sizeof(int));
	       gdata_dbl = (double *)malloc(nxgrid*sizeof(double));

	       mpp_gather_field_double(naxo[na][no], atmxocn_area[na][no], gdata_dbl);
	       if(check) {
		  for(n=0; n<nxgrid; n++) axo_area_sum += gdata_dbl[n];
	       }
	       mpp_put_var_value_block(fid, id_xgrid_area, start, nwrite, gdata_dbl);
	       mpp_gather_field_int(naxo[na][no], atmxocn_ia[na][no], gdata_int);
	       mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	       mpp_gather_field_int(naxo[na][no], atmxocn_io[na][no], gdata_int);
	       mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	       start[1] = 1;
	       mpp_gather_field_int(naxo[na][no], atmxocn_ja[na][no], gdata_int);
	       mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	       mpp_gather_field_int(naxo[na][no], atmxocn_jo[na][no], gdata_int);
	       mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	       if(interp_order == 2) {
		  start[1] = 0;
		  mpp_gather_field_double(naxo[na][no], atmxocn_dia[na][no], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
		  mpp_gather_field_double(naxo[na][no], atmxocn_dio[na][no], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
		  start[1] = 1;
		  mpp_gather_field_double(naxo[na][no], atmxocn_dja[na][no], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
		  mpp_gather_field_double(naxo[na][no], atmxocn_djo[na][no], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
	       }
	       mpp_close(fid);
	       free(gdata_int);
	       free(gdata_dbl);
	       ++nfile_axo;
	    }
	 } /* end of no loop */
      
      } /* end of na loop */

      /*release the memory */
      for(na=0; na<ntile_atm; na++) {
	 for(nl=0; nl<ntile_lnd; nl++) {
	    free(atmxlnd_area[na][nl]);
	    free(atmxlnd_ia  [na][nl]);
	    free(atmxlnd_ja  [na][nl]);
	    free(atmxlnd_il  [na][nl]);
	    free(atmxlnd_jl  [na][nl]);
	    if(interp_order == 2) {
	       free(atmxlnd_clon[na][nl]);
	       free(atmxlnd_clat[na][nl]);
	       free(atmxlnd_dia [na][nl]);
	       free(atmxlnd_dja [na][nl]);
	       free(atmxlnd_dil [na][nl]);
	       free(atmxlnd_djl [na][nl]);
	    }
	 }
	 free(atmxlnd_area[na]);
	 free(atmxlnd_ia  [na]);
	 free(atmxlnd_ja  [na]);
	 free(atmxlnd_il  [na]);
	 free(atmxlnd_jl  [na]);
	 if(interp_order == 2) {
	    free(atmxlnd_clon[na]);
	    free(atmxlnd_clat[na]);
	    free(atmxlnd_dia [na]);
	    free(atmxlnd_dja [na]);
	    free(atmxlnd_dil [na]);
	    free(atmxlnd_djl [na]);
	 }
	 for(no=0; no<ntile_ocn; no++) {
	    free(atmxocn_area[na][no]);
	    free(atmxocn_ia  [na][no]);
	    free(atmxocn_ja  [na][no]);
	    free(atmxocn_io  [na][no]);
	    free(atmxocn_jo  [na][no]);
	    if(interp_order == 2) {
	       free(atmxocn_clon[na][no]);
	       free(atmxocn_clat[na][no]);
	       free(atmxocn_dia [na][no]);
	       free(atmxocn_dja [na][no]);
	       free(atmxocn_dio [na][no]);
	       free(atmxocn_djo [na][no]);
	    }
	 }
	 free(atmxocn_area[na]);
	 free(atmxocn_ia  [na]);
	 free(atmxocn_ja  [na]);
	 free(atmxocn_io  [na]);
	 free(atmxocn_jo  [na]);
	 if(interp_order == 2) {
	    free(atmxocn_clon[na]);
	    free(atmxocn_clat[na]);
	    free(atmxocn_dia [na]);
	    free(atmxocn_dja [na]);
	    free(atmxocn_dio [na]);
	    free(atmxocn_djo [na]);
	 }
	 free(naxl[na]);
	 free(naxo[na]);
      }    
      free(atmxlnd_area);
      free(atmxlnd_ia  );
      free(atmxlnd_ja  );
      free(atmxlnd_il  );
      free(atmxlnd_jl  );
      free(atmxocn_area);
      free(atmxocn_ia  );
      free(atmxocn_ja  );
      free(atmxocn_io  );
      free(atmxocn_jo  );   
      if(interp_order == 2) {
	 free(atmxlnd_clon);
	 free(atmxlnd_clat);
	 free(atmxlnd_dja );
	 free(atmxlnd_dia );
	 free(atmxlnd_dil );
	 free(atmxlnd_djl );
	 free(atmxocn_clon);
	 free(atmxocn_clat);
	 free(atmxocn_dia );
	 free(atmxocn_dja );
	 free(atmxocn_dio );
	 free(atmxocn_djo );
      }
      free(naxl);
      free(naxo);
   }
   if(mpp_pe() == mpp_root_pe()) printf("\nNOTE from make_coupler_mosaic: Complete the process to create exchange grids "
   "for fluxes between atmosphere and surface (sea ice and land)\n" );
  
   /***************************************************************************************
     Then generate the exchange grid between land mosaic and ocean mosaic
     if land mosaic is different from atmos mosaic
   ***************************************************************************************/
   nfile_lxo = 0;
   if( !lnd_same_as_atm ) {
      int     no, nl, ll, lo;
      size_t  **nlxo;
      int     ***lndxocn_il, ***lndxocn_jl, ***lndxocn_io, ***lndxocn_jo;
      double  ***lndxocn_area, ***lndxocn_dil, ***lndxocn_djl, ***lndxocn_dio, ***lndxocn_djo;
      double  ***lndxocn_clon, ***lndxocn_clat;
      double  min_area;

      nlxo         = (size_t ** )malloc(ntile_lnd*sizeof(size_t * ));
      lndxocn_area = (double ***)malloc(ntile_lnd*sizeof(double **));
      lndxocn_il   = (int    ***)malloc(ntile_lnd*sizeof(int    **));
      lndxocn_jl   = (int    ***)malloc(ntile_lnd*sizeof(int    **));
      lndxocn_io   = (int    ***)malloc(ntile_lnd*sizeof(int    **));
      lndxocn_jo   = (int    ***)malloc(ntile_lnd*sizeof(int    **));
      if(interp_order == 2) {
	 lndxocn_dil  = (double ***)malloc(ntile_lnd*sizeof(double **));
	 lndxocn_djl  = (double ***)malloc(ntile_lnd*sizeof(double **));
	 lndxocn_dio  = (double ***)malloc(ntile_lnd*sizeof(double **));
	 lndxocn_djo  = (double ***)malloc(ntile_lnd*sizeof(double **));
	 lndxocn_clon = (double ***)malloc(ntile_lnd*sizeof(double **));
	 lndxocn_clat = (double ***)malloc(ntile_lnd*sizeof(double **));
      }
      for(nl=0; nl<ntile_lnd; nl++) {
	 nlxo        [nl] = (size_t * )malloc(ntile_ocn*sizeof(size_t  ));
	 lndxocn_area[nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	 lndxocn_il  [nl] = (int    **)malloc(ntile_ocn*sizeof(int    *));
	 lndxocn_jl  [nl] = (int    **)malloc(ntile_ocn*sizeof(int    *));
	 lndxocn_io  [nl] = (int    **)malloc(ntile_ocn*sizeof(int    *));
	 lndxocn_jo  [nl] = (int    **)malloc(ntile_ocn*sizeof(int    *));
	 if(interp_order == 2) {
	    lndxocn_dil [nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	    lndxocn_djl [nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	    lndxocn_dio [nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	    lndxocn_djo [nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	    lndxocn_clon[nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	    lndxocn_clat[nl] = (double **)malloc(ntile_ocn*sizeof(double *));
	 }
	 for(no=0; no<ntile_ocn; no++) {
	    lndxocn_area[nl][no] = (double *)malloc(MAXXGRID*sizeof(double));
	    lndxocn_il  [nl][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	    lndxocn_jl  [nl][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	    lndxocn_io  [nl][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));
	    lndxocn_jo  [nl][no] = (int    *)malloc(MAXXGRID*sizeof(int   ));          
	    if(interp_order == 2 ) {  
	       lndxocn_dil[nl][no]  = (double *)malloc(MAXXGRID*sizeof(double));
	       lndxocn_djl[nl][no]  = (double *)malloc(MAXXGRID*sizeof(double));
	       lndxocn_dio[nl][no]  = (double *)malloc(MAXXGRID*sizeof(double));
	       lndxocn_djo[nl][no]  = (double *)malloc(MAXXGRID*sizeof(double));
	       lndxocn_clon[nl][no] = (double *)malloc(MAXXGRID*sizeof(double *));
	       lndxocn_clat[nl][no] = (double *)malloc(MAXXGRID*sizeof(double *));
	    }
	 }
      }
 
      for(nl=0; nl<ntile_lnd; nl++) {
	 int      il, jl, io, jo, is, ie, js, je, layout[2];
	 int      n0, n1, n2, n3, nl_in, no_in, n_out, nxgrid;
	 double   xarea, xctrlon, xctrlat;
	 double   xl_min, yl_min, xo_min, yo_min, xl_avg;
	 double   xl_max, yl_max, xo_max, yo_max;
	 double   xl[MV], yl[MV], xo[MV], yo[MV], x_out[MV], y_out[MV];
	 domain2D Dom;

	 for(no=0; no<ntile_ocn; no++) nlxo[nl][no] = 0;
      
	 layout[0] = mpp_npes();
	 layout[1] = 1;
        
	 mpp_define_domain2d(nxl[nl]*nyl[nl], 1, layout, 0, 0, &Dom);
	 mpp_get_compute_domain2d(Dom, &is, &ie, &js, &je );
	 for(ll=is;ll<=ie;ll++) {
	    il = ll%nxl[nl];
	    jl = ll/nxl[nl];
	    n0 = jl    *(nxl[nl]+1) + il;
	    n1 = jl    *(nxl[nl]+1) + il+1;
	    n2 = (jl+1)*(nxl[nl]+1) + il+1;
	    n3 = (jl+1)*(nxl[nl]+1) + il;
	    xl[0] = xlnd[nl][n0]; yl[0] = ylnd[nl][n0];
	    xl[1] = xlnd[nl][n1]; yl[1] = ylnd[nl][n1];
	    xl[2] = xlnd[nl][n2]; yl[2] = ylnd[nl][n2];
	    xl[3] = xlnd[nl][n3]; yl[3] = ylnd[nl][n3];
	    yl_min  = minval_double(4, yl);
	    yl_max  = maxval_double(4, yl);
	    nl_in   = fix_lon(xl, yl, 4, M_PI);
	    xl_min  = minval_double(nl_in, xl);
	    xl_max  = maxval_double(nl_in, xl);
	    xl_avg  = avgval_double(nl_in, xl);      
	    for(no=0; no<ntile_ocn; no++) {
	       for(jo = 0; jo < nyo[no]; jo++) for(io = 0; io < nxo[no]; io++) if(omask[no][jo*nxo[no]+io] > 0.5) {	
			n0 = jo    *(nxo[no]+1) + io;
			n1 = jo    *(nxo[no]+1) + io+1;
			n2 = (jo+1)*(nxo[no]+1) + io+1;
			n3 = (jo+1)*(nxo[no]+1) + io;
			xo[0] = xocn[no][n0]; yo[0] = yocn[no][n0];
			xo[1] = xocn[no][n1]; yo[1] = yocn[no][n1];
			xo[2] = xocn[no][n2]; yo[2] = yocn[no][n2];
			xo[3] = xocn[no][n3]; yo[3] = yocn[no][n3];
			yo_min = minval_double(4, yo);
			yo_max = maxval_double(4, yo);
			if(yo_min >= yl_max || yo_max <= yl_min ) continue;	    
			no_in  = fix_lon(xo, yo, 4, xl_avg);
			xo_min = minval_double(no_in, xo);
			xo_max = maxval_double(no_in, xo);
			/* xo should in the same range as xa after lon_fix, so no need to
			   consider cyclic condition
			*/
			if(xl_min >= xo_max || xl_max <= xo_min ) continue;
			if (  (n_out = clip_2dx2d( xl, yl, nl_in, xo, yo, no_in, x_out, y_out )) > 0 ){
			   xarea = poly_area(x_out, y_out, n_out );
			   min_area = min(area_ocn[no][jo*nxo[no]+io], area_lnd[nl][ll] );
			   if(xarea/min_area > AREA_RATIO_THRESH ) {
			      lndxocn_area[nl][no][nlxo[nl][no]] = xarea;
			      lndxocn_io[nl][no][nlxo[nl][no]]   = io;
			      lndxocn_jo[nl][no][nlxo[nl][no]]   = jo;
			      lndxocn_il[nl][no][nlxo[nl][no]]   = il;
			      lndxocn_jl[nl][no][nlxo[nl][no]]   = jl;
			      if(interp_order == 2) {
				 lndxocn_clon[nl][no][nlxo[nl][no]] = poly_ctrlon ( x_out, y_out, n_out, xl_avg);
				 lndxocn_clat[nl][no][nlxo[nl][no]] = poly_ctrlat ( x_out, y_out, n_out );
			      }
			      ++(nlxo[nl][no]);
			      if(nlxo[nl][no] > MAXXGRID) mpp_error("nlxo is greater than MAXXGRID, increase MAXXGRID");
			   }
			}
		     } /* end of io, jo loop */
	    } 
	 } /* end of ll( or il, jl) loop */
	 mpp_delete_domain2d(&Dom);
      }/* for(nl=0; nl<ntile_lnd; nl++) */

      /* calculate the centroid of model grid. */
      if(interp_order == 2) {
	 double *l_area, *l_clon, *l_clat;
	 double **o_area, **o_clon, **o_clat;
 
	 o_area = (double **)malloc(ntile_ocn*sizeof(double *));
	 o_clon = (double **)malloc(ntile_ocn*sizeof(double *));
	 o_clat = (double **)malloc(ntile_ocn*sizeof(double *));
	 for(no =0; no<ntile_ocn; no++) {
	    o_area[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	    o_clon[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	    o_clat[no] = (double *)malloc(nxo[no]*nyo[no]*sizeof(double));
	    for(lo=0; lo<nxo[no]*nyo[no]; lo++) {
	       o_area[no][lo] = 0;
	       o_clon[no][lo] = 0;
	       o_clat[no][lo] = 0;
	    }
	 }

	 for(nl=0; nl<ntile_lnd; nl++) {
	    l_area = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	    l_clon = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	    l_clat = (double *)malloc(nxl[nl]*nyl[nl]*sizeof(double));
	    for(ll=0; ll<nxl[nl]*nyl[nl]; ll++) {
	       l_area[ll] = 0;
	       l_clon[ll] = 0;
	       l_clat[ll] = 0;
	    }

	    for(no=0; no<ntile_ocn; no++) {
	       int nxgrid;
	       nxgrid = nlxo[nl][no];
	       mpp_sum_int(1, &nxgrid);
	       if(nxgrid > 0) {
		  double *g_area, *g_clon, *g_clat;
		  int    *g_il,   *g_jl,   *g_io, *g_jo;
		  int    ii;
		  g_il = (int    *)malloc(nxgrid*sizeof(int   ));
		  g_jl = (int    *)malloc(nxgrid*sizeof(int   ));
		  g_io = (int    *)malloc(nxgrid*sizeof(int   ));
		  g_jo = (int    *)malloc(nxgrid*sizeof(int   ));	
		  g_area = (double *)malloc(nxgrid*sizeof(double));
		  g_clon = (double *)malloc(nxgrid*sizeof(double));
		  g_clat = (double *)malloc(nxgrid*sizeof(double));
		  mpp_gather_field_int   (nlxo[nl][no], lndxocn_il[nl][no], g_il);
		  mpp_gather_field_int   (nlxo[nl][no], lndxocn_jl[nl][no], g_jl);
		  mpp_gather_field_int   (nlxo[nl][no], lndxocn_io[nl][no], g_io);
		  mpp_gather_field_int   (nlxo[nl][no], lndxocn_jo[nl][no], g_jo);
		  mpp_gather_field_double(nlxo[nl][no], lndxocn_area[nl][no], g_area);
		  mpp_gather_field_double(nlxo[nl][no], lndxocn_clon[nl][no], g_clon);
		  mpp_gather_field_double(nlxo[nl][no], lndxocn_clat[nl][no], g_clat);
		  for(i=0; i<nxgrid; i++) {	      
		     ii = g_jl[i]*nxl[nl]+g_il[i];
		     l_area[ii] += g_area[i];
		     l_clon[ii] += g_clon[i];
		     l_clat[ii] += g_clat[i];
		     ii = g_jo[i]*nxo[no]+g_io[i];
		     o_area[no][ii] += g_area[i];
		     o_clon[no][ii] += g_clon[i];
		     o_clat[no][ii] += g_clat[i];
		  }
		  free(g_il);
		  free(g_jl);
		  free(g_io);
		  free(g_jo);
		  free(g_area);
		  free(g_clon);
		  free(g_clat);
	       }
	    }
	    for(ll=0; ll<nxl[nl]*nyl[nl]; ll++) {
	       if(l_area[ll] > 0) {
		  l_clon[ll] /= l_area[ll];
		  l_clat[ll] /= l_area[ll];
	       }
	    }
	    /* substract land centroid to get the centroid distance between land grid and exchange grid. */
	    for(no=0; no<ntile_ocn; no++) {
	       for(i=0; i<nlxo[nl][no]; i++) {
		  ll = lndxocn_jl[nl][no][i]*nxl[nl] + lndxocn_il[nl][no][i];
		  lndxocn_dil[nl][no][i] = lndxocn_clon[nl][no][i]/lndxocn_area[nl][no][i] - l_clon[ll];
		  lndxocn_djl[nl][no][i] = lndxocn_clat[nl][no][i]/lndxocn_area[nl][no][i] - l_clat[ll];
	       }
	    }
	
	    free(l_area);
	    free(l_clon);
	    free(l_clat);	
	 }

	 /* centroid distance from exchange grid to ocean grid */
	 for(no=0; no<ntile_ocn; no++) {
	    for(lo=0; lo<nxo[no]*nyo[no]; lo++) {
	       if(o_area[no][lo] > 0) {
		  o_clon[no][lo] /= o_area[no][lo];
		  o_clat[no][lo] /= o_area[no][lo];
	       }
	    }
	    for(nl=0; nl<ntile_lnd; nl++) {
	       for(i=0; i<nlxo[nl][no]; i++) {
		  lo = lndxocn_jo[nl][no][i]*nxo[no] + lndxocn_io[nl][no][i];
		  lndxocn_dio[nl][no][i] = lndxocn_clon[nl][no][i]/lndxocn_area[nl][no][i] - o_clon[no][lo];
		  lndxocn_djo[nl][no][i] = lndxocn_clat[nl][no][i]/lndxocn_area[nl][no][i] - o_clat[no][lo];
	       }
	    }
	    free(o_area[no]);
	    free(o_clon[no]);
	    free(o_clat[no]);
	 }

	 free(o_area);
	 free(o_clon);
	 free(o_clat);
      }

      /* write out lndXocn data */
      for(nl = 0; nl < ntile_lnd; nl++) {
	 for(no = 0; no < ntile_ocn; no++) {
	    int nxgrid;
	 
	    /* get total number of exchange grid on all the pes */
	    nxgrid = nlxo[nl][no];
	    mpp_sum_int(1, &nxgrid);
	
	    if(nxgrid >0) {
	       size_t start[4], nwrite[4];
	       int *gdata_int;
	       double *gdata_dbl;
	  
	       char contact[STRING];
	       int fid, dim_string, dim_ncells, dim_two, dims[4];
	       int id_contact, id_xgrid_area, n;
	       int id_tile1_cell, id_tile2_cell, id_tile1_dist, id_tile2_dist;

	       for(i=0; i<4; i++) {
		  start[i] = 0; nwrite[i] = 1;
	       }	  	  
	  
	       if(same_mosaic)
		  sprintf(lxo_file[nfile_lxo], "lnd_%s_%sXocn_%s_%s.nc", lmosaic_name, ltile_name[nl], omosaic_name, otile_name[no]);
	       else
		  sprintf(lxo_file[nfile_lxo], "%s_%sX%s_%s.nc", lmosaic_name, ltile_name[nl], omosaic_name, otile_name[no]);
	       sprintf(contact, "%s:%s::%s:%s", lmosaic_name, ltile_name[nl], omosaic_name, otile_name[no]);

	       fid = mpp_open(lxo_file[nfile_lxo], MPP_WRITE);
	       mpp_def_global_att(fid, "grid_version", GRID_VERSION);
	       mpp_def_global_att(fid, "code_version", TAGNAME);
	       mpp_def_global_att(fid, "history", history);
	       dim_string = mpp_def_dim(fid, "string", STRING);
	       dim_ncells = mpp_def_dim(fid, "ncells", nxgrid);
	       dim_two    = mpp_def_dim(fid, "two", 2);
	       if(interp_order == 2) {
		  id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 7, "standard_name", "grid_contact_spec",
		  "contact_type", "exchange", "parent1_cell",
		  "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area", 
		  "distant_to_parent1_centroid", "tile1_distance", "distant_to_parent2_centroid", "tile2_distance");
	       }
	       else {
		  id_contact = mpp_def_var(fid, "contact", MPP_CHAR, 1, &dim_string, 5, "standard_name", "grid_contact_spec",
		  "contact_type", "exchange", "parent1_cell",
		  "tile1_cell", "parent2_cell", "tile2_cell", "xgrid_area_field", "xgrid_area" );
	       }
	       dims[0] = dim_ncells; dims[1] = dim_two;
	       id_tile1_cell = mpp_def_var(fid, "tile1_cell", MPP_INT, 2, dims, 1, "standard_name", "parent1_cell_indices");
	       id_tile2_cell = mpp_def_var(fid, "tile2_cell", MPP_INT, 2, dims, 1, "standard_name", "parent2_cell_indices");
	       id_xgrid_area = mpp_def_var(fid, "xgrid_area", MPP_DOUBLE, 1, &dim_ncells, 2, "standard_name",
	       "exchange_grid_area", "units", "m2");

	       if(interp_order == 2) {
		  id_tile1_dist = mpp_def_var(fid, "tile1_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent1_cell_centroid");
		  id_tile2_dist = mpp_def_var(fid, "tile2_distance", MPP_DOUBLE, 2, dims, 1, "standard_name", "distance_from_parent2_cell_centroid");
	       }
	       mpp_end_def(fid);

	       /* the index will start from 1, instead of 0 ( fortran index) */
	       for(i = 0;i < nlxo[nl][no]; i++) {
		  ++(lndxocn_il[nl][no][i]);
		  ++(lndxocn_jl[nl][no][i]);
		  ++(lndxocn_io[nl][no][i]);
		  lndxocn_jo[nl][no][i] += 1 - ocn_south_ext; /* one artificial j-level may be added in the south end */
	       }

	       nwrite[0] = strlen(contact);
	       mpp_put_var_value_block(fid, id_contact, start, nwrite, contact);
	       nwrite[0] = nxgrid;

	       gdata_int = (int *)malloc(nxgrid*sizeof(int));
	       gdata_dbl = (double *)malloc(nxgrid*sizeof(double));

	       mpp_gather_field_double(nlxo[nl][no], lndxocn_area[nl][no], gdata_dbl);
	       mpp_put_var_value(fid, id_xgrid_area, gdata_dbl);
	       mpp_gather_field_int(nlxo[nl][no], lndxocn_il[nl][no], gdata_int);
	       mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	       mpp_gather_field_int(nlxo[nl][no], lndxocn_io[nl][no], gdata_int);
	       mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	       start[1] = 1;
	       mpp_gather_field_int(nlxo[nl][no], lndxocn_jl[nl][no], gdata_int);
	       mpp_put_var_value_block(fid, id_tile1_cell, start, nwrite, gdata_int);
	       mpp_gather_field_int(nlxo[nl][no], lndxocn_jo[nl][no], gdata_int);
	       mpp_put_var_value_block(fid, id_tile2_cell, start, nwrite, gdata_int);
	       if(interp_order == 2) {
		  start[1] = 0;
		  mpp_gather_field_double(nlxo[nl][no], lndxocn_dil[nl][no], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
		  mpp_gather_field_double(nlxo[nl][no], lndxocn_dio[nl][no], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
		  start[1] = 1;
		  mpp_gather_field_double(nlxo[nl][no], lndxocn_djl[nl][no], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile1_dist, start, nwrite, gdata_dbl);
		  mpp_gather_field_double(nlxo[nl][no], lndxocn_djo[nl][no], gdata_dbl);
		  mpp_put_var_value_block(fid, id_tile2_dist, start, nwrite, gdata_dbl);
	       }
	       mpp_close(fid);
	       free(gdata_int);
	       free(gdata_dbl);
	       ++nfile_lxo;
	    }
	 } /* for(no=0; no<ntile_ocn; no++) */
      } /* for(nl=0; nl<ntile_lnd; nl++) */

      /* release the memory */
      for(nl=0; nl<ntile_lnd; nl++) {
	 for(no=0; no<ntile_ocn; no++) {
	    free(lndxocn_area[nl][no]);
	    free(lndxocn_il  [nl][no]);
	    free(lndxocn_jl  [nl][no]);
	    free(lndxocn_io  [nl][no]);
	    free(lndxocn_jo  [nl][no]);
	    if(interp_order == 2) {
	       free(lndxocn_clon[nl][no]);
	       free(lndxocn_clat[nl][no]);
	       free(lndxocn_dil [nl][no]);
	       free(lndxocn_djl [nl][no]);
	       free(lndxocn_dio [nl][no]);
	       free(lndxocn_djo [nl][no]);
	    }
	 }
	 free(lndxocn_area[nl]);
	 free(lndxocn_il  [nl]);
	 free(lndxocn_jl  [nl]);
	 free(lndxocn_io  [nl]);
	 free(lndxocn_jo  [nl]);
	 if(interp_order == 2) {
	    free(lndxocn_clon[nl]);
	    free(lndxocn_clat[nl]);
	    free(lndxocn_dil [nl]);
	    free(lndxocn_djl [nl]);
	    free(lndxocn_dio [nl]);
	    free(lndxocn_djo [nl]);
	 }
	 free(nlxo[nl]);
      }
      free(nlxo);
      free(lndxocn_area);
      free(lndxocn_il  );
      free(lndxocn_jl  );
      free(lndxocn_io  );
      free(lndxocn_jo  );   
      if(interp_order == 2) {
	 free(lndxocn_clon);
	 free(lndxocn_clat);
	 free(lndxocn_dil );
	 free(lndxocn_djl );
	 free(lndxocn_dio );
	 free(lndxocn_djo );
      }

      if(mpp_pe() == mpp_root_pe()) printf("\nNOTE from make_coupler_mosaic: Complete the process to create exchange grids "
      "for runoff between land and sea ice.\n" );
   }
   else {
      nfile_lxo = nfile_axo;
      for(i=0; i<nfile_axo; i++) strcpy(lxo_file[i], axo_file[i]);
      if(mpp_pe() == mpp_root_pe()) printf("\nNOTE from make_coupler_mosaic: Since lmosaic is the same as amosaic, "
      "no need to compute the exchange grid between lmosaic and omosaic, "
      "simply use the exchange grid between amosaic and omosaic.\n");    
   }

   if(mpp_pe() == mpp_root_pe()) {
      int n, i;
      double axo_area_frac;
      double axl_area_frac;
      double tiling_area;
      double *atm_area;
      double atm_area_sum;

      if(check) {
	 /* for cubic grid, when number of model points is odd, some grid cell area will be negative,
	    need to think about how to solve this issue in the future */
	 /*      atm_area_sum = 4*M_PI*RADIUS*RADIUS; */
	 atm_area_sum = 0;
	 for(n=0; n<ntile_atm; n++) {
	    atm_area = (double *)malloc(nxa[n]*nya[n]*sizeof(double));
	    get_grid_area(&nxa[n], &nya[n], xatm[n], yatm[n], atm_area);
	    for(i=0; i<nxa[n]*nya[n]; i++) atm_area_sum += atm_area[i];
	    free(atm_area);
	 }
	 axo_area_frac = axo_area_sum/atm_area_sum*100;
	 axl_area_frac = axl_area_sum/atm_area_sum*100;
	 tiling_area   = (atm_area_sum-axo_area_sum-axl_area_sum)/atm_area_sum*100;    
	 printf("\nNOTE: axo_area_sum is %f and ocean fraction is %f%%\n", axo_area_sum, axo_area_frac);
	 printf("NOTE: axl_area_sum is %f and land  fraction is %f%%\n", axl_area_sum, axl_area_frac);
	 printf("NOTE: tiling error is %f%%\n", tiling_area );
      }
   }
  
   /*Fianlly create the coupler mosaic file mosaic_name.nc */
   {
      int fid, dim_string, dim_axo, dim_lxo, dim_axl, dims[4], n;
      size_t start[4], nwrite[4];
      int id_lmosaic_dir, id_lmosaic_file, id_omosaic_dir, id_omosaic_file;
      int id_amosaic_dir, id_amosaic_file, id_otopog_dir, id_otopog_file;
      int id_xgrids_dir, id_axo_file, id_lxo_file, id_axl_file;
      int id_amosaic, id_lmosaic, id_omosaic;
    
      fid = mpp_open(mosaic_file, MPP_WRITE);
      mpp_def_global_att(fid, "grid_version", GRID_VERSION);
      mpp_def_global_att(fid, "code_version", TAGNAME);
      mpp_def_global_att(fid, "history", history);
      dim_string = mpp_def_dim(fid, "string", STRING);
      dim_axo = mpp_def_dim(fid, "nfile_aXo", nfile_axo);
      dim_axl = mpp_def_dim(fid, "nfile_aXl", nfile_axl);
      dim_lxo = mpp_def_dim(fid, "nfile_lXo", nfile_lxo);
      id_amosaic_dir  = mpp_def_var(fid, "atm_mosaic_dir", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "directory_storing_atmosphere_mosaic");
      id_amosaic_file = mpp_def_var(fid, "atm_mosaic_file", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "atmosphere_mosaic_file_name");
      id_amosaic      = mpp_def_var(fid, "atm_mosaic", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "atmosphere_mosaic_name");
      id_lmosaic_dir  = mpp_def_var(fid, "lnd_mosaic_dir", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "directory_storing_land_mosaic");
      id_lmosaic_file = mpp_def_var(fid, "lnd_mosaic_file", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "land_mosaic_file_name");
      id_lmosaic      = mpp_def_var(fid, "lnd_mosaic", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "land_mosaic_name");        
      id_omosaic_dir  = mpp_def_var(fid, "ocn_mosaic_dir", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "directory_storing_ocean_mosaic");
      id_omosaic_file = mpp_def_var(fid, "ocn_mosaic_file", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "ocean_mosaic_file_name");
      id_omosaic      = mpp_def_var(fid, "ocn_mosaic", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "ocean_mosaic_name");    
      id_otopog_dir   = mpp_def_var(fid, "ocn_topog_dir", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "directory_storing_ocean_topog");
      id_otopog_file  = mpp_def_var(fid, "ocn_topog_file", MPP_CHAR, 1, &dim_string,
      1, "standard_name", "ocean_topog_file_name");
      /* since exchange grid is created in this tool, we may add command line option to specify where to store the output */
      /*    id_xgrids_dir = mpp_def_var(fid, "xgrids_dir", MPP_CHAR, 1, &dim_string,
	    1, "standard_name", "directory_storing_xgrids"); */
      dims[0] = dim_axo; dims[1] = dim_string;
      id_axo_file = mpp_def_var(fid, "aXo_file", MPP_CHAR, 2, dims, 1, "standard_name", "atmXocn_exchange_grid_file");
      dims[0] = dim_axl; dims[1] = dim_string;
      id_axl_file = mpp_def_var(fid, "aXl_file", MPP_CHAR, 2, dims, 1, "standard_name", "atmXlnd_exchange_grid_file");
      dims[0] = dim_lxo; dims[1] = dim_string;
      id_lxo_file = mpp_def_var(fid, "lXo_file", MPP_CHAR, 2, dims, 1, "standard_name", "lndXocn_exchange_grid_file");
      mpp_end_def(fid);
      for(i=0; i<4; i++) { start[i] = 0; nwrite[i] = 1; }
    
      nwrite[0] = strlen(amosaic_dir);
      mpp_put_var_value_block(fid, id_amosaic_dir, start, nwrite, amosaic_dir);
      nwrite[0] = strlen(amosaic_file);
      mpp_put_var_value_block(fid, id_amosaic_file, start, nwrite, amosaic_file);
      nwrite[0] = strlen(amosaic_name);
      mpp_put_var_value_block(fid, id_amosaic, start, nwrite, amosaic_name);
      nwrite[0] = strlen(lmosaic_dir);
      mpp_put_var_value_block(fid, id_lmosaic_dir, start, nwrite, lmosaic_dir);
      nwrite[0] = strlen(lmosaic_file);
      mpp_put_var_value_block(fid, id_lmosaic_file, start, nwrite, lmosaic_file);
      nwrite[0] = strlen(lmosaic_name);
      mpp_put_var_value_block(fid, id_lmosaic, start, nwrite, lmosaic_name);
      nwrite[0] = strlen(omosaic_dir);
      mpp_put_var_value_block(fid, id_omosaic_dir, start, nwrite, omosaic_dir);
      nwrite[0] = strlen(omosaic_file);
      mpp_put_var_value_block(fid, id_omosaic_file, start, nwrite, omosaic_file);
      nwrite[0] = strlen(omosaic_name);
      mpp_put_var_value_block(fid, id_omosaic, start, nwrite, omosaic_name);
      nwrite[0] = strlen(otopog_dir);
      mpp_put_var_value_block(fid, id_otopog_dir, start, nwrite, otopog_dir);
      nwrite[0] = strlen(otopog_file);
      mpp_put_var_value_block(fid, id_otopog_file, start, nwrite, otopog_file);
      nwrite[0] = 1;

      for(n=0; n<nfile_axo; n++) {
	 start[0] = n; nwrite[1] = strlen(axo_file[n]);
	 mpp_put_var_value_block(fid, id_axo_file, start, nwrite, axo_file[n]);
      }
      for(n=0; n<nfile_axl; n++) {
	 start[0] = n; nwrite[1] = strlen(axl_file[n]);
	 mpp_put_var_value_block(fid, id_axl_file, start, nwrite, axl_file[n]);
      }
      for(n=0; n<nfile_lxo; n++) {
	 start[0] = n; nwrite[1] = strlen(lxo_file[n]);
	 mpp_put_var_value_block(fid, id_lxo_file, start, nwrite, lxo_file[n]);
      }    
      mpp_close(fid);
   }
   return 0;
}
  
