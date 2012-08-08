/*******************************************************************************
                             create_hgrid.h
  This header file provide interface to create different types of horizontal 
  grid. geographical grid location, cell length, cell area and rotation
  angle are returned. All the returned data are on supergrid.

  contact: Zhi.Liang@noaa.gov

*******************************************************************************/
#ifdef CREATE_HGRID_H_
#define CREATE_HGRID_H_
void create_regular_lonlat_grid( int *nxbnds, int *nybnds, double *xbnds, double *ybnds,
		         	 int *nlon, int *nlat, int *isc, int *iec,
				 int *jsc, int *jec, double *x, double *y, double *dx,
				 double *dy, double *area, double *angle_dx, char *center_cell );

void create_simple_cartesian_grid( double *xbnds, double *ybnds, int *nlon, int *nlat,
				   double *simple_dx, double *simple_dy, int *isc, int *iec,
				   int *jsc, int *jec, double *x, double *y,
				   double *dx, double *dy, double *area, double *angle_dx, char *center_cell );

void create_grid_from_file( char *file, int *nlon, int *nlat, double *x, double *y, double *dx, double *dy,
           		    double *area, double *angle_dx );

void create_spectral_grid( int *nlon, int *nlat, int *isc, int *iec,
			   int *jsc, int *jec, double *x, double *y, double *dx,
			   double *dy, double *area, double *angle_dx );

void create_tripolar_grid( int *nxbnds, int *nybnds, double *xbnds, double *ybnds,
			   int *nlon, int *nlat, double *lat_join_in, int *isc, int *iec,
			   int *jsc, int *jec, double *x, double *y, double *dx, double *dy,
			   double *area, double *angle_dx );

void create_conformal_cubic_grid( int *npts, int *nratio, char *method, char *orientation, double *x,
			          double *y, double *dx, double *dy, double *area, double *angle_dx,
			          double *angle_dy );

void create_gnomonic_cubic_grid( char* grid_type, int *npts, double *x, double *y,
				double *dx, double *dy, double *area, double *angle_dx,
			        double *angle_dy );
#endif
