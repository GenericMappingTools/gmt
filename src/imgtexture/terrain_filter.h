/*
 * terrain_filter.h
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

#ifndef TERRAIN_FILTER_H
#define TERRAIN_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "extern_msc.h"

// DATA TYPE DEFINITIONS:
// =====================

enum Terrain_Coord_Type {
    TERRAIN_DEGREES = 0,    // geographic coordinates in degrees (latitude/longitude)
    TERRAIN_METERS  = 1     // projected  coordinates in meters  (northing/easting)
};

enum Terrain_Reg {
    TERRAIN_REG_GRID = 1,   // nodes  - pixel centers on grid (adjacent regions overlap 1 pixel)
    TERRAIN_REG_CELL = 2    // pixels - pixel edges   on grid (adjacent regions don't overlap)
};

enum Terrain_Filter_Errors {
    TERRAIN_FILTER_SUCCESS       = 0,
    TERRAIN_FILTER_MALLOC_ERROR  = 1,   // memory allocation error occurred
    TERRAIN_FILTER_NULL_VALUES   = 2,   // input data contains NaN values
    TERRAIN_FILTER_INVALID_PARAM = 3,   // invalid data registration type
    TERRAIN_FILTER_CANCELED      = -1   // cancellation requested by progress callback function
};

struct Terrain_Progress_Callback {
    // callback function - return nonzero value to cancel operation:
    int (*callback)(
        float portion_complete, // overall portion completed so far (0.00 to 1.00)
        float steps_done,       // number of steps (or partial steps) completed so far
        int   total_steps,      // total number of steps
        void *state);           // copy of state information pointer
    void *state;    // pointer to optional state information for use by callback() function:
};

struct Terrain_Scale_Callback {
    // callback function - return scale of projection at given pixel
    // (i.e., reciprocal of pixel spacing) in units of 1/meters:
    double (*callback)(int row, int col, void *state);
    void *state;    // pointer to optional state information for use by callback() function:
};


// PRIMARY TEXTURE SHADING FUNCTION:
// ================================

// Computes operator (-Laplacian)^(detail/2) applied to data array.
// Returns 0 on success, nonzero if an error occurred (see enum Terrain_Filter_Errors).
// Mean of data array is always (approximately) zero on output.
// On input, vertical units (data array values) should be in meters.
EXTERN_MSC int terrain_filter(
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
);


// AUXILIARY FUNCTIONS FOR TEXTURE SHADING:
// =======================================

// Corrects output of terrain_filter() for scale variation of Mercator-projected data.
// Assumes scale is true at the equator.
EXTERN_MSC void fix_mercator(
    float *data,    // input/output: array of texture shading data (row-major order)
    double detail,  // input: "detail" exponent used to create texture shading
    int    nrows,   // input: number of rows    in data array
    int    ncols,   // input: number of columns in data array
    double lat1deg, // input: latitude at bottom edge (or center) of bottom pixels, degrees
    double lat2deg  // input: latitude at top    edge (or center) of top    pixels, degrees
//  enum Terrain_Reg registration   // feature not yet implemented
);

// Corrects output of terrain_filter() for scale variation of polar stereographic projection
// (either North or South Pole). Assumes scale is true at the pole.
EXTERN_MSC void fix_polar_stereographic(
    float *data,        // input/output: array of data to process (row-major order)
    double detail,      // input: "detail" exponent to be applied
    int    nrows,       // input: number of rows    in data array
    int    ncols,       // input: number of columns in data array
    double center_res   // input: meters/pixel at pole (assumed to be center of array)
                        //        (see also polar_stereographic_center_res() function)
);

// Corrects output of terrain_filter() for scale variation of arbitrary projection.
// For small-scale maps, conformal projections are preferred in order to avoid artifacts
// from shape distortion.
// Callback function should return scale = 1/(distance between pixels in meters);
// for nonconformal projections or non-square pixels use 1/sqrt(pixel area).
EXTERN_MSC void fix_terrain_output(
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
);

// Converts output of terrain_filter() to grayscale image pixels;
// selects tone curve based on vertical_enhancement parameter.
EXTERN_MSC void terrain_image_data(
    float *data,        // input/output: array of data to convert (row-major order)
    int    nrows,       // input: number of rows    in data array
    int    ncols,       // input: number of columns in data array
    double vertical_enhancement,
                        // input: positive numbers give more contrast in the midrange
                        //        but less detail in the lightest & darkest areas;
                        //        negative numbers the opposite
    double image_min,   // input: minimum value for output pixels
    double image_max    // input: maximum value for output pixels
);


// MISCELLANEOUS UTILITY FUNCTIONS:
// ===============================

// Returns meters/pixel at pole (assumed to be center of array) for data in
// polar stereographic projection.
// Note: data is assumed to span no more than 90 degrees latitude - i.e., the center
// of the array must be in the same hemisphere (north or south) as the corners.
double polar_stereographic_center_res(
    int    nrows,           // input: number of rows    in data array
    int    ncols,           // input: number of columns in data array
    double corner_latdeg    // input: latitude at outer corner (or center) of corner pixels
//  enum Terrain_Reg registration   // feature not yet implemented
);

// Determines X and Y scales at given latitude for geographic projection
void geographic_scale(
    double  latdeg, // input:  latitude in degrees
    double *xsize,  // output: meters per degree of longitude
    double *ysize   // output: meters per degree of latitude
);

// Determines graticule aspect ratio at given latitude
double geographic_aspect( double latdeg );

#ifdef __cplusplus
}
#endif

#endif
