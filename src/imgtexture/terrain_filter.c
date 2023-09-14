/*
 * terrain_filter.c
 *
 * Created by Leland Brown on 2010 Oct 30.
 *
 * Copyright (c) 2010-2013 Leland Brown.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define THIS_MODULE_MODERN_NAME	"terrain_filter"
#define THIS_MODULE_CLASSIC_NAME	"terrain_filter"
#define THIS_MODULE_LIB		"imgtexture"
#define THIS_MODULE_PURPOSE	"Callable function to do image texture"

#define _USE_MATH_DEFINES

#include "terrain_filter.h"
#include "transpose_inplace.h"
#include "dct.h"

#include "compatibility.h"

#include <stddef.h> // for ptrdiff_t
#include <stdlib.h>
#include <math.h>

// For a 64-bit compile we need LONG to be 64 bits, even if the compiler uses an LLP64 model
#define LONG ptrdiff_t

static const double equatorial_radius = 6378137.0;      // WGS84 value in meters
static const double flattening = 1.0 / 298.257223563;   // WGS84 value
static const double ecc = 0.0818191908426215;           // first eccentricity
					 // = sqrt( (2.0 - flattening) * flattening )

// Utility function:

static int flt_isnan(float x) {
	volatile float y = x;
	return y != y;
}

static double conformal_lat( double lat );
static double isometric_lat( double lat );
static double tan_lat_from_tan_conformal( double tan_conlat );
static double tan_lat_from_isometric( double isolat );
static double mercator_relscale_from_tan_lat( double tan_lat );

void terrain_image_data(
	float *data,        // input/output: array of data to convert (row-major order)
	int    nrows,       // input: number of rows    in data array
	int    ncols,       // input: number of columns in data array
	double vertical_enhancement,
						// input: positive numbers give more contrast in the midrange
						//        but less detail in the lightest & darkest areas;
						//        negative numbers the opposite
	double image_min,   // input: minimum value for output pixels
	double image_max    // input: maximum value for output pixels
) {
// Converts output of terrain_filter() to grayscale image pixels;
// selects tone curve based on vertical_enhancement parameter.
	int i, j;
	float *ptr;
	double factor, half_span, image_mean;
	
	// Transform data values using the requested vertical enhancement:
	factor = pow(2.0, vertical_enhancement * 0.5 - 1.0);
	half_span  = 0.5 * (image_max - image_min);
	image_mean = 0.5 * (image_max + image_min);

	// CONCURRENCY NOTE: The iterations of the loops below will be
	// independent and can be executed in parallel, if each thread has
	// its own "ptr" variable initialized as in the comment below.
	for (i = 0, ptr = data; i < nrows; i++, ptr += ncols) {
		//float *ptr = data + (LONG)i * (LONG)ncols;    // for concurrency
		for (j = 0; j < ncols; ++j) {
			double z = ptr[j] * factor;		// scale values to set contrast tradeoff
			z = tanh(z);					// nonlinear mapping to range (-1,1)
			ptr[j] = (float)(z * half_span + image_mean);	// fit to desired range of image pixel values
		}
	}
}


static double conformal_lat(double lat) {
	// exact formula based on isometric latitude:
//  double isolat = isometric_lat( lat );
//  double temp = exp( fabs(isolat) );
//  double tan_conlat = copysign( temp - 1.0/temp, isolat ) * 0.5;
//  double conlat = atan( tan_conlat );
//  double conlat = lat;    // spherical earth approximation

	double geocentric_lat = atan( ((1.0-ecc)*(1.0+ecc)) * tan(lat) );
	double conlat = geocentric_lat; // better approximation

	return conlat;
}

static double isometric_lat(double lat) {
	// exact formula, but difficult to invert:
//  double sin_lat = sin( lat );
//  double isolat =
//      ( log(1.0+    sin_lat) - log(1.0-    sin_lat) ) * 0.5 +
//      ( log(1.0-ecc*sin_lat) - log(1.0+ecc*sin_lat) ) * 0.5 * ecc

	double conlat = conformal_lat(lat);
	double sin_conlat = sin(conlat);
	double cos_conlat = cos(conlat);
	double isolat = copysign(log(cos_conlat / (1.0+fabs(sin_conlat) ) ), sin_conlat);
			   // = ( log(1.0+sin_conlat) - log(1.0-sin_conlat) ) * 0.5
			   // = atanh( tan(conlat*0.5) ) * 2.0
	
	return isolat;
}

static double tan_lat_from_tan_conformal( double tan_conlat ) {
//  double tan_lat = tan_conlat;    // spherical earth approximation

	double tan_geocentric_lat = tan_conlat; // better approximation
	double tan_lat = tan_geocentric_lat / ( (1.0-ecc) * (1.0+ecc) );

	return tan_lat;
}

static double tan_lat_from_isometric( double isolat ) {
	double temp = exp( fabs(isolat) );
	double tan_conlat = copysign( temp - 1.0/temp, isolat ) * 0.5;
	
	return tan_lat_from_tan_conformal( tan_conlat );
}

static double mercator_relscale_from_tan_lat( double tan_lat ) {
	double relscale = sqrt( 1.0 + ((1.0-ecc)*(1.0+ecc)) * tan_lat * tan_lat );
				 // = sqrt( (1.0-ecc*sin(lat)) * (1.0+ecc*sin(lat)) ) / cos(lat)
				 // = secant of parametric (reduced) latitude
	return relscale;
}

void geographic_scale(
	double  latdeg, // input:  latitude in degrees
	double *xsize,  // output: meters per degree of longitude
	double *ysize   // output: meters per degree of latitude
) {
// Determines X and Y scales at given latitude for geographic projection
	double lat = (M_PI/180.0) * latdeg;

	double temp1 = ecc * sin( lat );
	double temp2 = (1.0 - temp1) * (1.0 + temp1);
	double temp3 = (1.0 - ecc)   * (1.0 + ecc);
	
	double normal_radius     = equatorial_radius / sqrt( temp2 );
	double meridional_radius = normal_radius * temp3 / temp2;

	*xsize = normal_radius * cos( lat );
	*ysize = meridional_radius;

	*xsize *= M_PI/180.0;
	*ysize *= M_PI/180.0;
}

double geographic_aspect(double latdeg) {
// Determines graticule aspect ratio at given latitude
	double xsize, ysize;
	
	geographic_scale(latdeg, &xsize, &ysize);
	
	// Return graticule aspect ratio (width/height in meters)
	return xsize / ysize;
}

void fix_mercator(
	float *data,    // input/output: array of data to process (row-major order)
	double detail,  // input: "detail" exponent to be applied
	int    nrows,   // input: number of rows    in data array
	int    ncols,   // input: number of columns in data array
	double lat1deg, // input: latitude at bottom edge (or center) of bottom pixels, degrees
	double lat2deg  // input: latitude at top    edge (or center) of top    pixels, degrees
//  enum Terrain_Reg registration   // feature not yet implemented
) {
// Corrects output of terrain_filter() for scale variation of Mercator-projected data.
// Assumes scale is true at the equator.
	enum Terrain_Reg registration = TERRAIN_REG_CELL;
	int i, j;
	float *ptr;
	double ypix1, ypix2, pix2merc, isolat0, ypix0;

	// convert latitudes from degrees to radians
	double lat1 = (M_PI/180.0) * lat1deg;
	double lat2 = (M_PI/180.0) * lat2deg;

	double isolat1 = isometric_lat( lat1 );
	double isolat2 = isometric_lat( lat2 );

	switch (registration) {
		case TERRAIN_REG_GRID:
			ypix1 = (double)(nrows - 1);    // center of bottom row of pixels
			ypix2 = 0.0;                    // center of top    row of pixels
			break;
		case TERRAIN_REG_CELL:
			ypix1 = (double)nrows - 0.5;    // bottom edge of bottom row of pixels
			ypix2 = -0.5;                   // top    edge of top    row of pixels
			break;
		default:
			ypix1 = ypix2 = 0.0;            // invalid data registration type
	}

	pix2merc = (isolat2 - isolat1) / (ypix2 - ypix1);
	isolat0  = (isolat1 + isolat2) / 2;
	ypix0    = (ypix1   + ypix2)   / 2;

	for (i=0, ptr=data; i<nrows; ++i, ptr+=ncols) {
		//float *ptr = data + (LONG)i * (LONG)ncols;
		double ypix = (double)i;
		double isolat = isolat0 + (ypix - ypix0) * pix2merc;
		double tan_lat = tan_lat_from_isometric( isolat );
		double relscale = mercator_relscale_from_tan_lat( tan_lat );
		
		double zfactor = pow( relscale, detail );
		for (j=0; j<ncols; ++j)
			ptr[j] *= (float)zfactor;
	}
}

void fix_polar_stereographic(
	float *data,        // input/output: array of data to process (row-major order)
	double detail,      // input: "detail" exponent to be applied
	int    nrows,       // input: number of rows    in data array
	int    ncols,       // input: number of columns in data array
	double center_res   // input: meters/pixel at pole (assumed to be center of array)
) {
// Corrects output of terrain_filter() for scale variation of polar stereographic projection
// (either North or South Pole). Assumes scale is true at the pole.
	const double rfactor = sqrt( ((1+ecc)*(1-ecc)) * pow( (1+ecc)/(1-ecc), ecc ) ) * 0.5;

	int i, j;
	float *ptr;
	double relscale, zfactor;

	double xres = center_res;
	double yres = center_res;

	// assume pole is in center of array
	double ipole = 0.5 * (double)(nrows-1);
	double jpole = 0.5 * (double)(ncols-1);

	double xfactor = xres / equatorial_radius;
	double yfactor = yres / equatorial_radius;

	for (i=0, ptr=data; i<nrows; ++i, ptr+=ncols) {
		//float *ptr = data + (LONG)i * (LONG)ncols;
		double idiff = ((double)i - ipole) * yfactor;
		for (j=0; j<ncols; ++j) {
			double jdiff = ((double)j - jpole) * xfactor;
			double r = sqrt( idiff*idiff + jdiff*jdiff );
			if (r > 0.0) {
				double temp = r * rfactor;
						 // = exp( -isometric_lat ) for north polar projection
						 // = exp( +isometric_lat ) for south polar projection
				double tan_conlat = ( 1.0/temp - temp ) * 0.5;
				double tan_lat = tan_lat_from_tan_conformal( tan_conlat );
				relscale = r * mercator_relscale_from_tan_lat( tan_lat );
			}
			else
				relscale = 1.0;
			
			zfactor = pow( relscale, detail );
			ptr[j] *= (float)zfactor;
		}
	}
}

double polar_stereographic_center_res(
	int    nrows,           // input: number of rows    in data array
	int    ncols,           // input: number of columns in data array
	double corner_latdeg    // input: latitude at outer corner (or center) of corner pixels
//  enum Terrain_Reg registration   // feature not yet implemented
) {
// Returns meters/pixel at pole (assumed to be center of array), if data is in
// polar stereographic projection.
// Note: data is assumed to span no more than 90 degrees latitude - i.e., the center
// of the array must be in the same hemisphere (north or south) as the corners.
	enum Terrain_Reg registration = TERRAIN_REG_CELL;

	const double rfactor = sqrt( ((1+ecc)*(1-ecc)) * pow( (1+ecc)/(1-ecc), ecc ) ) * 0.5;

	int idiff, jdiff;
	double conlat, temp, rcorner, center_res;
	double corner_lat = (M_PI/180.0) * corner_latdeg;
	
	switch (registration) {
		case TERRAIN_REG_GRID:
			// assume pole is in center of array
			idiff = (int)(0.5 * (double)(nrows-1));
			jdiff = (int)(0.5 * (double)(ncols-1));
			break;
		case TERRAIN_REG_CELL:
			// assume pole is in center of array
			idiff = (int)(0.5 * (double)nrows);
			jdiff = (int)(0.5 * (double)ncols);
			break;
		default:
			// invalid data registration type
			return 0.0;
	}
	
	rcorner = sqrt( idiff*idiff + jdiff*jdiff );
	conlat = conformal_lat( corner_lat );
	temp = cos(conlat) / ( 1.0 + fabs( sin(conlat) ) );
			 // = exp( -fabs(isometric_lat) )
	center_res = temp / rcorner * (equatorial_radius / rfactor);
	
	return center_res;
}

void fix_terrain_output(
	float *data,        // input/output: array of data to process (row-major order)
	double detail,      // input: "detail" exponent to be applied
	int    nrows,       // input: number of rows    in data array
	int    ncols,       // input: number of columns in data array
	double xdim,        // input: xdim value passed to terrain_filter()
	double ydim,        // input: ydim value passed to terrain_filter()
	enum Terrain_Coord_Type
		   coord_type,  // input: coord_type passed to terrain_filter()
	double center_lat,  // input: center_lat passed to terrain_filter()
	struct Terrain_Scale_Callback
		   scale        // input: functor to compute scale at each pixel
) {
// Corrects output of terrain_filter() for scale variation of arbitrary projection.
// For small-scale maps, conformal projections are preferred in order to avoid artifacts
// from shape distortion.
// Callback function should return scale = 1/(distance between pixels);
// for nonconformal projections or non-square pixels use 1/sqrt(pixel area).
	int i, j;
	float *ptr;
	double xres, yres, xsize, ysize;
	
	if (coord_type == TERRAIN_DEGREES) {
		geographic_scale( center_lat, &xsize, &ysize );
		// convert degrees to meters (approximately)
		xres = xdim * xsize;
		yres = ydim * ysize;
	} else {
		xres = xdim;
		yres = ydim;
	}
	double old_res = sqrt( fabs( xres * yres ) );

	for (i=0, ptr=data; i<nrows; ++i, ptr+=ncols) {
		//float *ptr = data + (LONG)i * (LONG)ncols;
		for (j=0; j<ncols; ++j) {
			double relscale = scale.callback( i, j, scale.state ) * old_res;
			double zfactor = pow( relscale, detail );
			ptr[j] *= (float)zfactor;
		}
	}
}


// Fractional Laplacian operator:

// How to use the functions setup_operator(), apply_operator(), and cleanup_operator():
//
//      float *data;    // transposed array ncols x nrows
//
//      Terrain_Operator_Info info;
//      int error = setup_operator( detail, ncols, nrows, yscale, registration, &info );
//      if (error) {
//          // handle error here
//      }
//      for (int i=0; i<ncols; ++i) {
//          apply_operator( data, i, nrows, info );
//      }
//      cleanup_operator( info );
//

#define TERRAIN_OPERATOR_METHOD 0
//#define TERRAIN_OPERATOR_METHOD 1

struct Terrain_Operator_Info {
	double *separablex;
	double *separabley;
	double *splinex;
	double *spliney;
	double *xx;
	double *yy;
	double  power;
	double  factor;
};

static int setup_operator(double detail, int ncols, int nrows, double xscale, double yscale,
                          enum Terrain_Reg registration, struct Terrain_Operator_Info *info) {
// Note: data array is in transposed layout (ncols x nrows)
	int m2, n2, i, j;
	double *storage;
	double xfactor, yfactor, nux, xfreq, cosx, tempx, nuy, yfreq, cosy, tempy;

	switch (registration) {
		case TERRAIN_REG_GRID:
			m2 = 2*ncols-2;
			n2 = 2*nrows-2;
			info->factor = 1.00 / ((double)n2 * (double)m2);    // DCT normalization factor
			break;
		case TERRAIN_REG_CELL:
			m2 = ncols+ncols;
			n2 = nrows+nrows;
			info->factor = 0.25 / ((double)n2 * (double)m2);    // DCT normalization factor
			break;
		default:
			return TERRAIN_FILTER_INVALID_PARAM;
	}
	
	info->factor *= pow( 2.0*M_PI, detail );    // constant factor for fractional Laplacian
	info->power = detail * 0.5;

	xfactor = 1.0 / (double)m2;
	yfactor = 1.0 / (double)n2;
	
	// Allocate array storage
	
	storage = (double *)malloc( sizeof( double ) * ((m2+n2+2) * 3 + ncols + nrows) );
	if (!storage)
		return TERRAIN_FILTER_MALLOC_ERROR;

	info->separablex = storage, storage += ncols;
	info->separabley = storage, storage += nrows;
	info->splinex    = storage, storage += (m2+1);
	info->spliney    = storage, storage += (n2+1);
	info->xx         = storage, storage += (m2+1);
	info->yy         = storage, storage += (n2+1);

	// Use approximation to bicubic spline interpolator

	for (i=0; i<=m2; ++i) {
		nux   = xfactor * (double)i;
		xfreq = xscale * nux;
		cosx  = cos(M_PI * nux);
		tempx = cosx * 0.5 + 0.5;
		info->xx[i] = xfreq * xfreq;
		info->splinex[i] = (tempx*tempx) * ( (cosx+2.0) / (cosx*cosx*2.0+1.0) );
	}

	for (j=0; j<=n2; ++j) {
		nuy   = yfactor * (double)j;
		yfreq = yscale * nuy;
		cosy  = cos(M_PI * nuy);
		tempy = cosy * 0.5 + 0.5;
		info->yy[j] = yfreq * yfreq;
		info->spliney[j] = (tempy*tempy) * ( (cosy+2.0) / (cosy*cosy*2.0+1.0) );
	}

	for (i=0; i<ncols; ++i) {
		info->separablex[i] = info->splinex[i] * info->xx[i] + info->splinex[m2-i] * info->xx[m2-i];
	}    
	for (j=0; j<nrows; ++j) {
		info->separabley[j] = info->spliney[j] * info->yy[j] + info->spliney[n2-j] * info->yy[n2-j];
	}
	
	return TERRAIN_FILTER_SUCCESS;
}

static void apply_operator(float *data, int col, int nrows, struct Terrain_Operator_Info info) {
// Note: data array is in transposed layout (ncols x nrows)
	int j, i = col;
	float *ptr = data + (LONG)i * (LONG)nrows;
	
	// Fractional Laplacian operator

	#if TERRAIN_OPERATOR_METHOD == 1
		for (j=0; j<nrows; ++j) {
			double log1 = info.splinex[i]    * info.spliney[j]    * log( info.xx[i]    + info.yy[j]    );
			double log2 = info.splinex[i]    * info.spliney[n2-j] * log( info.xx[i]    + info.yy[n2-j] );
			double log3 = info.splinex[m2-i] * info.spliney[j]    * log( info.xx[m2-i] + info.yy[j]    );
			double log4 = info.splinex[m2-i] * info.spliney[n2-j] * log( info.xx[m2-i] + info.yy[n2-j] );
			ptr[j] *= info.factor * exp( ( (log1 + log4) + (log2 + log3) ) * info.power );
		}
	#else
		for (j=0; j<nrows; ++j)
			ptr[j] *= (float)(info.factor * pow(info.separablex[i] + info.separabley[j], info.power));
	#endif

	if (i == 0)
		ptr[0] = 0.0;   // set "DC" component to zero
}

static void cleanup_operator(struct Terrain_Operator_Info info) {
	free( info.separablex );
}


static void two_dcts( float *ptr, int    length, const struct Dct_Plan *plan ) {
	int j;
	float *ptr2 = ptr + length;

	for (j=0; j<length; ++j) {
		plan->in_data[0][j] = (double)ptr[j];
	}
	for (j=0; j<length; ++j) {
		plan->in_data[1][j] = (double)ptr2[j];
	}
	
	perform_dcts( plan );

	for (j=0; j<length; ++j) {
		ptr[j] = (float)plan->out_data[0][j];
	}
	for (j=0; j<length; ++j) {
		ptr2[j] = (float)plan->out_data[1][j];
	}
}

// Note: single_dct() takes the same time as two_dcts() but does half the work,
// so it is preferred to use two_dcts() whenever possible.
static void single_dct( float *ptr, int length, const struct Dct_Plan *plan ) {
	int j;
	
	for (j=0; j<length; ++j)
		plan->in_data[0][j] = plan->in_data[1][j] = (double)ptr[j];
	
	perform_dcts( plan );

	for (j=0; j<length; ++j)
		ptr[j] = (float)plan->out_data[0][j];
}


// Progress info structure used by init_progress(), set_progress(),
// report_progress(), update_progress(), and relay_progress().
struct Terrain_Progress_Info {
	const struct Terrain_Progress_Callback
		  *progress;        // overall progress callback functor
	const float
		  *step_times;      // array of relative time to complete each step
	int    total_steps;     // total number of main steps
	int    steps_done;      // number of whole main steps completed so far
	float  portion_done;    // overall progress as of last complete step
	float  this_portion;    // current step's portion of overall work
	float  total_time;      // sum of step_times
};


// Initialize progress info structure to be used by set_progress(),
// report_progress(), update_progress(), and relay_progress().
static struct Terrain_Progress_Info
init_progress(
	const struct Terrain_Progress_Callback
		  *progress,    // overall progress callback functor
	const float
		  *step_times,  // array of relative time to complete each step
	int    total_steps  // size of step_portions array
) {
// Note: This function retains a copy of the step_times pointer
// in the returned structure.
	struct Terrain_Progress_Info info;
	int i;
	
	info.progress    = progress;
	info.step_times  = step_times;
	info.total_steps = total_steps;
	info.total_time  = 0.0;
	for (i=0; i<total_steps; ++i) {
		info.total_time += step_times[i];
	}
	info.steps_done   = 0;
	info.portion_done = 0.0;
	info.this_portion = step_times[0] / info.total_time;

	return info;
}

// Set progress info at start of new step
static void set_progress(struct Terrain_Progress_Info *info,
	int steps_done)     // number of whole steps completed
{
	if (steps_done >= info->total_steps) {
		info->steps_done   = info->total_steps;
		info->portion_done = 1.0;
		info->this_portion = 0.0;
		return;
	}
	for (; info->steps_done < steps_done; ++info->steps_done) {
		info->portion_done += info->step_times[info->steps_done] / info->total_time;
	}
	info->this_portion = info->step_times[info->steps_done] / info->total_time;
}

// Reports overall progress between major steps
static INLINE int report_progress(const struct Terrain_Progress_Info *info) {  // current progress info
	return info->progress->callback(info->portion_done, (float)info->steps_done, info->total_steps, info->progress->state);
}

// Reports overall progress at each substep of main steps
static INLINE int update_progress(
	const struct Terrain_Progress_Info
	   *info,           // progress info at end of last main step
	int substep_count,  // number of substeps completed so far
	int substep_total)  // total number of substeps in this step
{
	float step_progress   = (float)substep_count / (float)substep_total;
	float steps_done      = info->steps_done   + step_progress;
	float overall_portion = info->portion_done + step_progress * info->this_portion;
	return info->progress->callback(overall_portion, steps_done, info->total_steps, info->progress->state );
}

// Reports overall progress based on % progress reported by individual steps
static int relay_progress(
	float step_progress,    // portion of step completed so far (0.00 to 1.00)
	void *state)            // pointer to Terrain_Progress_Info structure
{
	struct Terrain_Progress_Info *info = (struct Terrain_Progress_Info *)state;
	float steps_done      = info->steps_done   + step_progress;
	float overall_portion = info->portion_done + step_progress * info->this_portion;
	return info->progress->callback(overall_portion, steps_done, info->total_steps, info->progress->state );
}


// Main terrain_filter function:

int terrain_filter(
	float *data,        // input/output: array of data to process (row-major order)
	double detail,      // input: "detail" exponent to be applied
	int    nrows,       // input: number of rows    in data array
	int    ncols,       // input: number of columns in data array
	double xdim,        // input: spacing between pixel columns (in degrees or meters)
	double ydim,        // input: spacing between pixel rows    (in degrees or meters)
	enum Terrain_Coord_Type
		   coord_type,  // input: coordinate type for xdim & ydim (degrees or meters)
	double center_lat,  // input: latitude in degrees at center of data array
						//        (ignored if coord_type == TERRAIN_METERS)
	const struct Terrain_Progress_Callback
		  *progress     // optional callback functor for status; NULL for none
//  enum Terrain_Reg registration   // feature not yet implemented
) {
// Computes operator (-Laplacian)^(detail/2) applied to data array.
// Returns 0 on success, nonzero if an error occurred (see enum Terrain_Filter_Errors).
// Mean of data array is always (approximately) zero on output.
// On input, vertical units (data array values) should be in meters.
	enum Terrain_Reg registration = TERRAIN_REG_CELL;
	
	int num_threads = 1;    // number of threads used to parallelize the three DCT loops
	
	// approximate relative amount of time spent in each step
	// (actual times vary with data array size, memory size, and DCT algorithms chosen):
	const float step_times[6] = {0.5, 2.0f/num_threads, 1.0, 4.0f/num_threads, 1.0, 2.0f/num_threads};
	
	const int total_steps = sizeof( step_times ) / sizeof( *step_times );
	
	struct Terrain_Progress_Info progress_info = init_progress( progress, step_times, total_steps );
	struct Transpose_Progress_Callback sub_progress = { relay_progress, &progress_info };

	int error, type_fwd, type_bwd, i, j;
	float *ptr;
	float data_min, data_max;
	double normalizer, xres, yres, xscale, yscale, xsize, ysize;
	const double steepness = 2.0;
	
	struct Terrain_Operator_Info info;

	// Determine pixel dimensions:
	
	if (coord_type == TERRAIN_DEGREES) {
		geographic_scale(center_lat, &xsize, &ysize);
		xres = xdim * xsize;		// convert degrees to meters (approximately)
		yres = ydim * ysize;
	} else {
		xres = xdim;
		yres = ydim;
	}

	xscale = fabs(1.0 / xres);
	yscale = fabs(1.0 / yres);

	switch (registration) {
		case TERRAIN_REG_GRID:
			type_fwd = type_bwd = 1;
			break;
		case TERRAIN_REG_CELL:
			type_fwd = 2;
			type_bwd = 3;
			break;
		default:
			return TERRAIN_FILTER_INVALID_PARAM;
	}

	if (progress && report_progress( &progress_info ))
		return TERRAIN_FILTER_CANCELED;

	data_min = data[0];
	data_max = data[0];
	
	for (i=0, ptr=data; i<nrows; ++i, ptr+=ncols) {
		//float *ptr = data + (LONG)i * (LONG)ncols;
		for (j=0; j<ncols; ++j) {
			if (ptr[j] < data_min)
				data_min = ptr[j];
			else if (ptr[j] > data_max)
				data_max = ptr[j];
		}
	}
	
	normalizer  = pow( 2.0 / (data_max - data_min), 1.0 - detail );
	normalizer *= pow( steepness, -detail );

	for (i=0, ptr=data; i<nrows; ++i, ptr+=ncols) {
		//float *ptr = data + (LONG)i * (LONG)ncols;
		for (j=0; j<ncols; ++j)
			ptr[j] *= (float)normalizer;
	}

	if ((error = setup_operator(detail, ncols, nrows, xscale, yscale, registration, &info)))
		return error;

	set_progress( &progress_info, 1 );
	
	if (progress && report_progress( &progress_info ))
		return TERRAIN_FILTER_CANCELED;

	// CONCURRENCY NOTE: The iterations of the loop below will be
	// independent and can be executed in parallel, if each thread has
	// its own dct_plan with separate calls to setup_dcts() and cleanup_dcts().
	{
		struct Dct_Plan dct_plan = setup_dcts( type_fwd, ncols );
		
		if (!dct_plan.dct_buffer)
			return TERRAIN_FILTER_MALLOC_ERROR;
		
		for (i=0; i<nrows-1; i+=2) {
			float *ptr = data + (LONG)i * (LONG)ncols;
			two_dcts( ptr, ncols, &dct_plan );
			if (progress && update_progress( &progress_info, i+2, nrows ))
				return TERRAIN_FILTER_CANCELED;
		}
		if (nrows & 1) {
			float *ptr = data + (LONG)(nrows - 1) * (LONG)ncols;
			single_dct( ptr, ncols, &dct_plan );
		}
		
		cleanup_dcts(&dct_plan);
	}

	set_progress( &progress_info, 2 );
	
	if (progress && report_progress( &progress_info ))
		return TERRAIN_FILTER_CANCELED;

	error = transpose_inplace(data, nrows, ncols, progress ? &sub_progress : NULL);
	if (error) {
		if (error > 0)
			return TERRAIN_FILTER_MALLOC_ERROR;
		else
			return TERRAIN_FILTER_CANCELED;
	}
	//if ((error = transpose_inplace(data, nrows, ncols, progress ? &sub_progress : NULL)))
		//return (error > 0) ? TERRAIN_FILTER_MALLOC_ERROR : TERRAIN_FILTER_CANCELED;

	set_progress(&progress_info, 3);
	
	if (progress && report_progress( &progress_info ))
		return TERRAIN_FILTER_CANCELED;

	// CONCURRENCY NOTE: The iterations of the loop below will be
	// independent and can be executed in parallel, if each thread has
	// its own fwd_plan and bwd_plan with separate calls to setup_dcts()
	// and cleanup_dcts().
	{
		struct Dct_Plan fwd_plan = setup_dcts( type_fwd, nrows );
		struct Dct_Plan bwd_plan = setup_dcts( type_bwd, nrows );
				
		if (!fwd_plan.dct_buffer || !bwd_plan.dct_buffer)
			return TERRAIN_FILTER_MALLOC_ERROR;

		for (i=0; i<ncols-1; i+=2) {
			float *ptr = data + (LONG)i * (LONG)nrows;
			
			two_dcts( ptr, nrows, &fwd_plan );
			apply_operator( data, i,   nrows, info );
			apply_operator( data, i+1, nrows, info );
			two_dcts( ptr, nrows, &bwd_plan );
			
			if (progress && update_progress( &progress_info, i+2, ncols ))
				return TERRAIN_FILTER_CANCELED;
		}
		
		if (ncols & 1) {
			float *ptr = data + (LONG)(ncols - 1) * (LONG)nrows;

			single_dct( ptr, nrows, &fwd_plan );
			apply_operator( data, ncols-1, nrows, info );
			single_dct( ptr, nrows, &bwd_plan );
		}
	
		cleanup_dcts( &bwd_plan );
		cleanup_dcts( &fwd_plan );
	}
	
	if (flt_isnan( data[0] ))
		return TERRAIN_FILTER_NULL_VALUES;

	set_progress( &progress_info, 4 );
	
	if (progress && report_progress( &progress_info ))
		return TERRAIN_FILTER_CANCELED;

	error = transpose_inplace(data, ncols, nrows, progress ? &sub_progress : NULL);
	if (error) {
		if (error > 0)
			return TERRAIN_FILTER_MALLOC_ERROR;
		else
			return TERRAIN_FILTER_CANCELED;
	}

	set_progress( &progress_info, 5 );
	
	if (progress && report_progress( &progress_info ))
		return TERRAIN_FILTER_CANCELED;

	// CONCURRENCY NOTE: The iterations of the loop below will be
	// independent and can be executed in parallel, if each thread has
	// its own dct_plan with separate calls to setup_dcts() and cleanup_dcts().
	{
		struct Dct_Plan dct_plan = setup_dcts( type_bwd, ncols );

		if (!dct_plan.dct_buffer)
			return TERRAIN_FILTER_MALLOC_ERROR;

		for (i=0; i<nrows-1; i+=2) {
			float *ptr = data + (LONG)i * (LONG)ncols;
			two_dcts( ptr, ncols, &dct_plan );
			if (progress && update_progress( &progress_info, i+2, nrows ))
				return TERRAIN_FILTER_CANCELED;
		}
		if (nrows & 1) {
			float *ptr = data + (LONG)(nrows - 1) * (LONG)ncols;
			single_dct( ptr, ncols, &dct_plan );
		}

		cleanup_dcts( &dct_plan );
	}

	cleanup_operator( info );
	set_progress( &progress_info, 6 );
	
	if (progress)		// report final progress; ignore any cancel request at this point
		report_progress(&progress_info);

	return TERRAIN_FILTER_SUCCESS;
}
