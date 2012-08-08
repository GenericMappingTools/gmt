#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#ifdef use_libMPI 
#include <mpi.h>
#endif
#include "mosaic_util.h"
#include "constant.h"

#define HPI (0.5*M_PI)
#define TPI (2.0*M_PI)
#define TOLORENCE (1.e-6)
#define EPSLN (1.e-10)
/***********************************************************
    void error_handler(char *str)
    error handler: will print out error message and then abort
***********************************************************/

void error_handler(const char *msg)
{
  fprintf(stderr, "FATAL Error: %s\n", msg );
#ifdef use_libMPI      
  MPI_Abort(MPI_COMM_WORLD, -1);
#else
  exit(1);
#endif  
}; /* error_handler */

/*********************************************************************

   int nearest_index(double value, const double *array, int ia)

   return index of nearest data point within "array" corresponding to "value".
   if "value" is outside the domain of "array" then nearest_index = 0
   or = size(array)-1 depending on whether array(0) or array(ia-1) is
   closest to "value"

   Arguments:
     value:  arbitrary data...same units as elements in "array"
     array:  array of data points  (must be monotonically increasing)
     ia   :  size of array.

 ********************************************************************/
int nearest_index(double value, const double *array, int ia)
{
  int index, i;
  int keep_going;

  for(i=1; i<ia; i++){
    if (array[i] < array[i-1]) 
      error_handler("nearest_index: array must be monotonically increasing"); 
  }
  if (value < array[0] )
    index = 0;
  else if ( value > array[ia-1]) 
    index = ia-1;
  else
    {
      i=0;
      keep_going = 1;
      while (i < ia && keep_going) {
	i = i+1;
	if (value <= array[i]) {
	  index = i;
	  if (array[i]-value > value-array[i-1]) index = i-1;
	  keep_going = 0;
	}
      }
    }
  return index;

};

/******************************************************************/

void tokenize(const char * const string, const char *tokens, unsigned int varlen,
	      unsigned int maxvar, char * pstring, unsigned int * const nstr)
{
  size_t i, j, nvar, len, ntoken;
  int found, n;
  
  nvar = 0; j = 0;
  len = strlen(string);
  ntoken = strlen(tokens);
  /* here we use the fact that C array [][] is contiguous in memory */
  if(string[0] == 0)error_handler("Error from tokenize: to-be-parsed string is empty");
  
  for(i = 0; i < len; i ++){
    if(string[i] != ' ' && string[i] != '\t'){
      found = 0;
      for(n=0; n<ntoken; n++) {
	if(string[i] == tokens[n] ) {
	  found = 1;
	  break;
	}
      }
      if(found) {
	if( j != 0) { /* remove :: */
	  *(pstring + (nvar++)*varlen + j) = 0;
	  j = 0;
	  if(nvar >= maxvar) error_handler("Error from tokenize: number of variables exceeds limit");
	}
      }
      else {
        *(pstring + nvar*varlen + j++) = string[i];
        if(j >= varlen ) error_handler("error from tokenize: variable name length exceeds limit during tokenization");
      }
    }
  }
  *(pstring + nvar*varlen + j) = 0;
  
  *nstr = ++nvar;

}

/*******************************************************************************
  double maxval_double(int size, double *data)
  get the maximum value of double array
*******************************************************************************/
double maxval_double(int size, const double *data)
{
  int n;
  double maxval;

  maxval = data[0];
  for(n=1; n<size; n++){
    if( data[n] > maxval ) maxval = data[n];
  }

  return maxval;
  
}; /* maxval_double */


/*******************************************************************************
  double minval_double(int size, double *data)
  get the minimum value of double array
*******************************************************************************/
double minval_double(int size, const double *data)
{
  int n;
  double minval;

  minval = data[0];
  for(n=1; n<size; n++){
    if( data[n] < minval ) minval = data[n];
  }

  return minval;
  
}; /* minval_double */

/*******************************************************************************
  double avgval_double(int size, double *data)
  get the average value of double array
*******************************************************************************/
double avgval_double(int size, const double *data)
{
  int n;
  double avgval;

  avgval = 0;
  for(n=0; n<size; n++) avgval += data[n];
  avgval /= size;
  
  return avgval;
  
}; /* avgval_double */


/*******************************************************************************
  void latlon2xyz
  Routine to map (lon, lat) to (x,y,z)
******************************************************************************/
void latlon2xyz(int size, const double *lon, const double *lat, double *x, double *y, double *z)
{
  int n;
  
  for(n=0; n<size; n++) {
    x[n] = cos(lat[n])*cos(lon[n]);
    y[n] = cos(lat[n])*sin(lon[n]);
    z[n] = sin(lat[n]);
  }

} /* latlon2xyz */

/*------------------------------------------------------------
       void xyz2laton(np, p, xs, ys)
   Transfer cartesian coordinates to spherical coordinates
   ----------------------------------------------------------*/
void xyz2latlon( int np, const double *x, const double *y, const double *z, double *lon, double *lat)
{

  double xx, yy, zz;
  double dist, sinp;
  int i;

  for(i=0; i<np; i++) {
    xx = x[i];
    yy = y[i];
    zz = z[i];
    dist = sqrt(xx*xx+yy*yy+zz*zz);
    xx /= dist;
    yy /= dist;
    zz /= dist;
    
    if ( fabs(xx)+fabs(yy)  < EPSLN ) 
       lon[i] = 0;
     else
       lon[i] = atan2(yy, xx);
     lat[i] = asin(zz);
    
     if ( lon[i] < 0.) lon[i] = 2.*M_PI + lon[i];
  }

} /* xyz2latlon */

/*------------------------------------------------------------------------------
  double box_area(double ll_lon, double ll_lat, double ur_lon, double ur_lat)
  return the area of a lat-lon grid box. grid is in radians.
  ----------------------------------------------------------------------------*/
double box_area(double ll_lon, double ll_lat, double ur_lon, double ur_lat)
{
  double dx = ur_lon-ll_lon;
  double area;
  
  if(dx > M_PI)  dx = dx - 2.0*M_PI;
  if(dx < -M_PI) dx = dx + 2.0*M_PI;

  return (dx*(sin(ur_lat)-sin(ll_lat))*RADIUS*RADIUS ) ;
  
}; /* box_area */


/*------------------------------------------------------------------------------
  double poly_area(const x[], const y[], int n)
  obtains area of input polygon by line integrating -sin(lat)d(lon)
  Vertex coordinates must be in degrees.
  Vertices must be listed counter-clockwise around polygon.
  grid is in radians.
  ----------------------------------------------------------------------------*/
double poly_area(const double x[], const double y[], int n)
{
  double area = 0.0;
  int    i;

  for (i=0;i<n;i++) {
    int ip = (i+1) % n;
    double dx = (x[ip]-x[i]);
    double lat1, lat2;

    lat1 = y[ip];
    lat2 = y[i];
    if(dx > M_PI)  dx = dx - 2.0*M_PI;
    if(dx < -M_PI) dx = dx + 2.0*M_PI;
    if (dx==0.0) continue;
    
    if ( fabs(lat1-lat2) < SMALL_VALUE) /* cheap area calculation along latitude */
      area -= dx*sin(0.5*(lat1+lat2));
    else
      area += dx*(cos(lat1)-cos(lat2))/(lat1-lat2);
  }
  return area*RADIUS*RADIUS;

}; /* poly_area */

double poly_area_no_adjust(const double x[], const double y[], int n)
{
  double area = 0.0;
  int    i;

  for (i=0;i<n;i++) {
    int ip = (i+1) % n;
    double dx = (x[ip]-x[i]);
    double lat1, lat2;

    lat1 = y[ip];
    lat2 = y[i];
    if (dx==0.0) continue;
    
    if ( fabs(lat1-lat2) < SMALL_VALUE) /* cheap area calculation along latitude */
      area -= dx*sin(0.5*(lat1+lat2));
    else
      area += dx*(cos(lat1)-cos(lat2))/(lat1-lat2);
  }
  return area*RADIUS*RADIUS;

}; /* poly_area_no_adjust */

int delete_vtx(double x[], double y[], int n, int n_del)
{
  for (;n_del<n-1;n_del++) {
    x[n_del] = x[n_del+1];
    y[n_del] = y[n_del+1];
  }
  
  return (n-1);
} /* delete_vtx */

int insert_vtx(double x[], double y[], int n, int n_ins, double lon_in, double lat_in)
{
  int i;

  for (i=n-1;i>=n_ins;i--) {
    x[i+1] = x[i];
    y[i+1] = y[i];
  }
  
  x[n_ins] = lon_in;
  y[n_ins] = lat_in;
  return (n+1);
} /* insert_vtx */

void v_print(double x[], double y[], int n)
{
  int i;

  for (i=0;i<n;i++) printf(" %20g   %20g\n", x[i], y[i]);
} /* v_print */

int fix_lon(double x[], double y[], int n, double tlon)
{
  double x_sum, dx;
  int i, nn = n, pole = 0;

  for (i=0;i<nn;i++) if (fabs(y[i])>=HPI-TOLORENCE) pole = 1;
  if (0&&pole) {
    printf("fixing pole cell\n");
    v_print(x, y, nn);
    printf("---------");
  }

  /* all pole points must be paired */
  for (i=0;i<nn;i++) if (fabs(y[i])>=HPI-TOLORENCE) {
    int im=(i+nn-1)%nn, ip=(i+1)%nn;

    if (y[im]==y[i] && y[ip]==y[i]) {
      nn = delete_vtx(x, y, nn, i);
      i--;
    } else if (y[im]!=y[i] && y[ip]!=y[i]) {
      nn = insert_vtx(x, y, nn, i, x[i], y[i]);
      i++;
    }
  }
  /* first of pole pair has longitude of previous vertex */
  /* second of pole pair has longitude of subsequent vertex */
  for (i=0;i<nn;i++) if (fabs(y[i])>=HPI-TOLORENCE) {
    int im=(i+nn-1)%nn, ip=(i+1)%nn;

    if (y[im]!=y[i]) x[i] = x[im];
    if (y[ip]!=y[i]) x[i] = x[ip];
  }

  if (nn) x_sum = x[0]; else return(0);
  for (i=1;i<nn;i++) {
    double dx = x[i]-x[i-1];

    if      (dx < -M_PI) dx = dx + TPI;
    else if (dx >  M_PI) dx = dx - TPI;
    x_sum += (x[i] = x[i-1] + dx);
  }

  dx = (x_sum/nn)-tlon;
  if      (dx < -M_PI) for (i=0;i<nn;i++) x[i] += TPI;
  else if (dx >  M_PI) for (i=0;i<nn;i++) x[i] -= TPI;

  if (0&&pole) {
    printf("area=%g\n", poly_area(x, y,nn));
    v_print(x, y, nn);
    printf("---------");
  }

  return (nn);
} /* fix_lon */


/*------------------------------------------------------------------------------
  double great_circle_distance()
  computes distance between two points along a great circle
  (the shortest distance between 2 points on a sphere)
  returned in units of meter
  ----------------------------------------------------------------------------*/
double great_circle_distance(double *p1, double *p2)
{
  double dist, beta;
  
  /* This algorithm is not accurate for small distance 
  dist = RADIUS*ACOS(SIN(p1[1])*SIN(p2[1]) + COS(p1[1])*COS(p2[1])*COS(p1[0]-p2[0]));
  */
  beta = 2.*asin( sqrt( sin((p1[1]-p2[1])/2.)*sin((p1[1]-p2[1])/2.) + 
                               cos(p1[1])*cos(p2[1])*(sin((p1[0]-p2[0])/2.)*sin((p1[0]-p2[0])/2.)) ) );
  dist = RADIUS*beta;
  return dist;

}; /* great_circle_distance */


/*------------------------------------------------------------------------------
  double spherical_angle(const double *p1, const double *p2, const double *p3)
           p3
         /
        /
       p1 ---> angle
         \
          \
           p2
 -----------------------------------------------------------------------------*/
double spherical_angle(const double *v1, const double *v2, const double *v3)
{
  double angle;
  double px, py, pz, qx, qy, qz, ddd;

  /* vector product between v1 and v2 */
  px = v1[1]*v2[2] - v1[2]*v2[1];
  py = v1[2]*v2[0] - v1[0]*v2[2];
  pz = v1[0]*v2[1] - v1[1]*v2[0];
  /* vector product between v1 and v3 */
  qx = v1[1]*v3[2] - v1[2]*v3[1];
  qy = v1[2]*v3[0] - v1[0]*v3[2];
  qz = v1[0]*v3[1] - v1[1]*v3[0];
    
  /* angle between p and q */
  ddd = sqrt( (px*px+py*py+pz*pz)*(qx*qx+qy*qy+qz*qz) );
  if (ddd > 0) {
    angle = acos((px*qx+py*qy+pz*qz)/ddd);
  }
  else
    angle = 0.;

  return angle;
}; /* spherical_angle */

/*------------------------------------------------------------------------------
  double spherical_excess_area(p_lL, p_uL, p_lR, p_uR) 
  get the surface area of a cell defined as a quadrilateral 
  on the sphere. Area is computed as the spherical excess
  [area units are m^2]
  ----------------------------------------------------------------------------*/
double spherical_excess_area(const double* p_ll, const double* p_ul,
			     const double* p_lr, const double* p_ur, double radius)
{
  double area, ang1, ang2, ang3, ang4;
  double v1[3], v2[3], v3[3];

  /*   S-W: 1   */  
  latlon2xyz(1, p_ll, p_ll+1, v1, v1+1, v1+2);
  latlon2xyz(1, p_lr, p_lr+1, v2, v2+1, v2+2);
  latlon2xyz(1, p_ul, p_ul+1, v3, v3+1, v3+2);
  ang1 = spherical_angle(v1, v2, v3);

  /*   S-E: 2   */  
  latlon2xyz(1, p_lr, p_lr+1, v1, v1+1, v1+2);
  latlon2xyz(1, p_ur, p_ur+1, v2, v2+1, v2+2);
  latlon2xyz(1, p_ll, p_ll+1, v3, v3+1, v3+2);
  ang2 = spherical_angle(v1, v2, v3);

  /*   N-E: 3   */  
  latlon2xyz(1, p_ur, p_ur+1, v1, v1+1, v1+2);
  latlon2xyz(1, p_ul, p_ul+1, v2, v2+1, v2+2);
  latlon2xyz(1, p_lr, p_lr+1, v3, v3+1, v3+2);
  ang3 = spherical_angle(v1, v2, v3);
  
  /*   N-W: 4   */  
  latlon2xyz(1, p_ul, p_ul+1, v1, v1+1, v1+2);
  latlon2xyz(1, p_ur, p_ur+1, v2, v2+1, v2+2);
  latlon2xyz(1, p_ll, p_ll+1, v3, v3+1, v3+2);
  ang4 = spherical_angle(v1, v2, v3);

  area = (ang1 + ang2 + ang3 + ang4 - 2.*M_PI) * radius* radius;

  return area;
  
}; /* spherical_excess_area */


/*----------------------------------------------------------------------
    void vect_cross(e, p1, p2)
    Perform cross products of 3D vectors: e = P1 X P2
    -------------------------------------------------------------------*/
    
void vect_cross(const double *p1, const double *p2, double *e )
{
  
  e[0] = p1[1]*p2[2] - p1[2]*p2[1];
  e[1] = p1[2]*p2[0] - p1[0]*p2[2];
  e[2] = p1[0]*p2[1] - p1[1]*p2[0];

}; /* vect_cross */

/* ----------------------------------------------------------------
   make a unit vector
   --------------------------------------------------------------*/
void normalize_vect(double *e)
{
  double pdot;
  int k;

  pdot = e[0]*e[0] + e[1] * e[1] + e[2] * e[2];
  pdot = sqrt( pdot ); 

  for(k=0; k<3; k++) e[k] /= pdot;
};


/*------------------------------------------------------------------
  void unit_vect_latlon(int size, lon, lat, vlon, vlat)

  calculate unit vector for latlon in cartesian coordinates

  ---------------------------------------------------------------------*/
void unit_vect_latlon(int size, const double *lon, const double *lat, double *vlon, double *vlat)
{
  double sin_lon, cos_lon, sin_lat, cos_lat;
  int n;
  
  for(n=0; n<size; n++) {
    sin_lon = sin(lon[n]);
    cos_lon = cos(lon[n]);
    sin_lat = sin(lat[n]);
    cos_lat = cos(lat[n]);
    
    vlon[3*n] = -sin_lon;
    vlon[3*n+1] =  cos_lon;
    vlon[3*n+2] =  0.;
    
    vlat[3*n]   = -sin_lat*cos_lon;
    vlat[3*n+1] = -sin_lat*sin_lon;
    vlat[3*n+2] =  cos_lat;
  }
}; /* unit_vect_latlon */
