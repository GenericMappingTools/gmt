/************************************************************************
* readgrd routine to read and write  a grd file in pixel registration   *
************************************************************************/
/************************************************************************
* Creator: David T. Sandwell    Scripps Institution of Oceanography    *
* Date   : 06/23/95             Copyright, David T. Sandwell           *
************************************************************************/
/************************************************************************
* Modification history:                                                 *
*   Revised for GMT3.4 December 28, 2002 - David Sandwell               *
*   Revised for GMT4.2 May 10, 2007      - David Sandwell               *
*   Revised for GMT5.x March 20, 2013    - Paul Wessel                  *
************************************************************************/

/* Sandwell/SIO-specific front end to generic F77 GMT grid i/o functions:

1. GMT_F77_readgrdinfo_ (unsigned int dim[], double limit[], double inc[], \
      char *title, char *remark, char *file)
   When returning, dim[2] holds the registration (0 = gridline, 1 = pixel).
   limit[4-5] holds zmin/zmax.

2. GMT_F77_readgrd_ (float *array, unsigned int dim[], double limit[], \
      double inc[], char *title, char *remark, char *file)
   If dim[2] is 1 we allocate the array, otherwise we assume it has space
   When returning, dim[2] holds the registration (0 = gridline, 1 = pixel).
   limit[4-5] holds zmin/zmax.

3. GMT_F77_writegrd_ (float *array, unsigned int dim[], double limit[], \
      double inc[], char *title, char *remark, char *file)
   When called, dim[2] holds the registration (0 = gridline, 1 = pixel).
*/

# include "gmt.h"
# include <math.h>
# include <string.h>

char    *argsav[2000]; /* a hack to make gips stuff link */

/************************************************************************/
int readgrd_(float *rdat, int *nx, int *ny, double *rlt0, double *rln0, \
    double *dlt, double *dln, double *rdum, char *title, char *filein)
{
	/* dat	  = real array for input
	 * nx	  = number of x points
	 * ny	  = number of y points
	 * rlt0	  = starting latitude
	 * rln0	  = starting longitude
	 * dlt	  = latitude spacing
	 * dln	  = longitude spacing
	 * rdum	  = dummy value
	 * title  = title
	 * filein = filename of input file
	 */
	uint64_t node, nm;
	unsigned int dim[4] = {0, 0, 0, 0};
	double wesn[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, inc[2] = {0.0, 0.0};
	char remark[GMT_GRID_REMARK_LEN160];

	/* Obtain the grid and header info */
	if (GMT_F77_readgrd_ (rdat, dim, wesn, inc, title, remark, filein))
		return EXIT_FAILURE;

	/* Assing output value from header parameters */

	*nx = dim[GMT_X];
	*ny = dim[GMT_Y];
	*dlt = -inc[GMT_Y];
	*dln =  inc[GMT_X];
	if (dim[GMT_Z] == GMT_GRID_PIXEL_REG) {
		*rlt0 = wesn[GMT_YHI];
		*rln0 = wesn[GMT_XLO]; 
	}
	else {
		*rlt0 = wesn[GMT_YHI] + 0.5 * inc[GMT_Y];
		*rln0 = wesn[GMT_XLO] - 0.5 * inc[GMT_X]; 
	}
	*rdum = floor (wesn[GMT_ZHI] + 1.0);

	/* Calculate rdum and reset NaNs to the dummy value */
	
	nm = ((uint64_t)dim[GMT_X]) * ((uint64_t)dim[GMT_Y]);
	for (node = 0; node < nm; node++) {
		if(isnan ((double)rdat[node])) rdat[node] = (float)*rdum; 
	}
	return EXIT_SUCCESS;
}
/************************************************************************/
int writegrd_(float *rdat, int *nx, int *ny, double *rlt0, double *rln0, \
    double *dlt, double *dln, double *rland, double *rdum, char *title, \
    char *fileout)
{
	/* dat	   = real array for output
	 * nx	   = number of x points
	 * ny	   = number of y points
	 * rlt0	   = starting latitude
	 * rln0	   = starting longitude
	 * dlt	   = latitude spacing
	 * dln	   = longitude spacing
	 * rlan	   = land value
	 * rdum	   = dummy value
	 * title   = title
	 * fileout = filename of output file
	 */
	
	uint64_t node, nm;
	unsigned int dim[4] = {0, 0, 0, 0};
	double wesn[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, inc[2] = {0.0, 0.0};

	/* Calculate header parameters */
	
	dim[GMT_X] = *nx;
	dim[GMT_Y] = *ny;
	dim[GMT_Z] = GMT_GRID_PIXEL_REG;
	wesn[GMT_XHI] = *rln0 + *nx * *dln;
	wesn[GMT_XLO] = *rln0;
	if (wesn[GMT_XHI] < wesn[GMT_XLO]) {
		wesn[GMT_XLO] = wesn[GMT_XHI];
		wesn[GMT_XHI] = *rln0;
	}
	inc[GMT_X] = fabs (*dln);
	inc[GMT_Y] = fabs (*dlt);
	wesn[GMT_YLO] = *rlt0 + *ny * *dlt;
	wesn[GMT_YHI] = *rlt0;
	if (wesn[GMT_YHI] < wesn[GMT_YLO]) {
		wesn[GMT_YLO] = wesn[GMT_YHI];
		wesn[GMT_YHI] = *rlt0;
	}
	
	/*  Set dummy and land values to NaN. */
	
	nm = ((uint64_t)dim[GMT_X]) * ((uint64_t)dim[GMT_Y]);
	for (node = 0; node < nm; node++) {
		if ((rdat[node] == *rdum) || (rdat[node] == *rland))
			rdat[node] = (float)NAN;
	}
	
	if (GMT_F77_writegrd_ (rdat, dim, wesn, inc, title, "", fileout))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
/************************************************************************/
