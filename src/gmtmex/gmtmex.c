/*
 *	Copyright (c) 2015-2024 by P. Wessel and J. Luis
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * This is the MATLAB/Octave(mex) GMT application, which can do the following:
 * 1) Create a new session and optionally return the API pointer. We provide for
 *    storing the pointer as a global variable (persistent) between calls.
 * 2) Destroy a GMT session, either given the API pointer or by fetching it from
 *    the global (persistent) variable.
 * 3) Call any of the GMT modules while passing data in and out of GMT.
 *
 * First argument to the gmt function is the API pointer, but it is optional once created.
 * Next argument is the module name
 * Third argument is the option string
 * Finally, there are optional comma-separated MATLAB array entities required by the command.
 * Information about the options of each program is provided via GMT_Encode_Options.
 *
 * GMT Version:	6.x
 * Created:	20-OCT-2017
 * Updated:	1-APR-2021 requires GMT 6.2.x
 *
 *
 * GMT convenience functions used by MATLAB/OCTAVE mex/oct API
 * We also define the MEX structures used to pass data in/out of GMT.
 */

#define GMTMEX_MAJOR_VERSION 2
#define GMTMEX_MINOR_VERSION 1
#define GMTMEX_PATCH_VERSION 0

/* Define the minimum GMT version suitable for this GMTMEX version */

#define GMTMEX_GMT_MAJOR_VERSION	6
#define GMTMEX_GMT_MINOR_VERSION	2
#define GMTMEX_GMT_PATCH_VERSION	0

#define STDC_FORMAT_MACROS
#define GMTMEX_LIB

#include "gmt.h"
#include "gmt_version.h"
#include <math.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

#define gmt_M_unused(x) (void)(x)

#ifdef GMT_OCTOCT
#	include <oct.h>
#else
#	include <mex.h>
#	define mxIsScalar_(mx) \
		( (2 == mxGetNumberOfDimensions(mx)) \
		&&  (1 == mxGetM(mx))&&  (1 == mxGetN(mx)) )
#endif	/* Matlab and Octave(mex) */

/* Matlab and Octave (in -mex mode) are identical, oct files are different and not yet tested */

/* Older Ml versions don't have mwSize */
#ifndef GMT_OCTMEX
#if !defined(MATLAB_VERSION)
#if !defined(MWSIZE_MAX)
#    define MATLAB_VERSION 0x2006a /* R2006a or earlier */
#elif MX_API_VER < 0x07040000
#    define MATLAB_VERSION 0x2006b /* R2006b */
#elif !defined(FMT_PTRDIFF_T)
#    define MATLAB_VERSION 0x2007a /* R2007a */
#elif !defined(CUINT64_T)
#    define MATLAB_VERSION 0x2007b /* R2007b */
#elif defined(mxSetLogical)
#    define MATLAB_VERSION 0x2008a /* R2008a */
#else
#    if !defined(blas_h)
#        include "blas.h"
#    endif
#    if !defined(lapack_h)
#        include "lapack.h"
#    endif
#    if !defined(MATHWORKS_MATRIX_MATRIX_PUB_FWD_H)
#        if defined(CHAR16_T)
#            if !defined(COMPLEX_TYPES)
#                define MATLAB_VERSION 0x2008b /* R2008b */
#            elif !defined(cgeqr2p)
#                define MATLAB_VERSION 0x2010b /* R2010b */
#            else
#                define MATLAB_VERSION 0x2011a /* R2011a */
#            endif
#        else
#            include "emlrt.h"
#            define MATLAB_VERSION EMLRT_VERSION_INFO /* R2011b or later */
#        endif
#    else
#        if !defined(COMPLEX_TYPES)
#            define MATLAB_VERSION 0x2009a /* R2009a */
#        elif !defined(cgbequb)
#            define MATLAB_VERSION 0x2009b /* R2009b */
#        else
#            define MATLAB_VERSION 0x2010a /* R2010a */
#        endif
#    endif
#endif
#endif /* if !defined(MATLAB_VERSION) */

#if MATLAB_VERSION < 0x2006b
typedef int mwSize;
#endif
#endif

#ifdef GMT_OCTOCT	/* Octave oct files only */
#	define MEX_PROG "Octave(oct)"
#	define MEX_COL_ORDER GMT_IS_ROW_FORMAT
	/* Macros for getting the Octave(oct) ij that correspond to (row,col) [no pad involved] */
	/* This one operates on GMT_MATRIX */
#	define MEXM_IJ(M,row,col) ((row)*M->n_columns + (col))
	/* And this on GMT_GRID */
#	define MEXG_IJ(M,row,col) ((row)*M->header->n_columns + (col))
	/* And this on GMT_CUBE */
#	define MEXU_IJK(M,layer,row,col) ((layer)*M->header->nm + (row)*M->header->n_columns + (col))
#else	/* Here we go for Matlab or Octave(mex) */
#	ifdef GMT_MATLAB
#		define MEX_PROG "Matlab"
#	else
#		define MEX_PROG "Octave(mex)"
#	endif
#	define MEX_COL_ORDER GMT_IS_COL_FORMAT
	/* Macros for getting the Matlab/Octave(mex) ij that correspond to (row,col) [no pad involved] */
	/* This one operates on GMT_MATRIX */
#	define MEXM_IJ(M,row,col) ((col)*M->n_rows + (row))
	/* And this on GMT_GRID */
#	define MEXG_IJ(M,row,col) ((col)*M->header->n_rows + M->header->n_rows - (row) - 1)
	/* And this on GMT_CUBE */
#	define MEXU_IJK(M,layer,row,col) ((layer)*M->header->nm + (col)*M->header->n_rows + M->header->n_rows - (row) - 1)
#endif

/* These 4 functions are used by gmtmex.c: */
EXTERN_MSC char   GMTMEX_objecttype (const mxArray *ptr);
EXTERN_MSC int    GMTMEX_print_func (FILE *fp, const char *message);
EXTERN_MSC void   GMTMEX_Set_Object (void *API, struct GMT_RESOURCE *X, const mxArray *ptr);
EXTERN_MSC void * GMTMEX_Get_Object (void *API, struct GMT_RESOURCE *X);

/* Definitions of MEX structures used to hold GMT objects.
 * DO NOT MODIFY THE ORDER OF THE FIELDNAMES */

/* GMT_IS_DATASET:
 * Returned by GMT via the parser as a MEX structure with the
 * fields listed below.  Pure datasets will only set the data
 * matrix and leave the text cell array empty, while datasets
 * with trailing text will fill out both.  Only the first segment
 * will have any information in the info and projection_ref_* items.  */

#define N_MEX_FIELDNAMES_DATASET	6
static const char *gmtmex_fieldname_dataset[N_MEX_FIELDNAMES_DATASET] =
	{"data", "text", "header", "comment", "proj4", "wkt"};

/* GMT_IS_CUBE:
 * Returned by GMT via the parser as a MEX structure with the
 * fields listed below. */

#define N_MEX_FIELDNAMES_CUBE	19
static const char *gmtmex_fieldname_cube[N_MEX_FIELDNAMES_CUBE] =
	{"v", "x", "y", "z", "range", "inc", "registration", "nodata", "title", "comment",
	 "command", "datatype", "x_unit", "y_unit", "z_unit", "v_unit", "layout", "proj4", "wkt"};

/* GMT_IS_GRID:
 * Returned by GMT via the parser as a MEX structure with the
 * fields listed below. */

#define N_MEX_FIELDNAMES_GRID	17
static const char *gmtmex_fieldname_grid[N_MEX_FIELDNAMES_GRID] =
	{"z", "x", "y", "range", "inc", "registration", "nodata", "title", "comment",
	 "command", "datatype", "x_unit", "y_unit", "z_unit", "layout", "proj4", "wkt"};

/* GMT_IS_IMAGE:
 * Returned by GMT via the parser as a MEX structure with the
 * fields listed below.  */

#define N_MEX_FIELDNAMES_IMAGE	19
static const char *gmtmex_fieldname_image[N_MEX_FIELDNAMES_IMAGE] =
	{"image", "x", "y", "range", "inc", "registration", "nodata", "title", "comment", "command",
	 "datatype", "x_unit", "y_unit", "z_unit", "colormap", "alpha", "layout", "proj4", "wkt"};

/* GMT_IS_PALETTE:
 * Returned by GMT via the parser as a MEX structure with the
 * fields listed below.  */

#define N_MEX_FIELDNAMES_CPT	11
static const char *gmtmex_fieldname_cpt[N_MEX_FIELDNAMES_CPT] =
	{"colormap", "alpha", "range", "minmax", "bfn", "depth", "hinge", "cpt", "model", "mode", "comment"};
	
/* GMT_IS_POSTSCRIPT:
 * Returned by GMT via the parser as a MEX structure with the
 * fields listed below.  */

#define N_MEX_FIELDNAMES_PS	4
static const char *gmtmex_fieldname_ps[N_MEX_FIELDNAMES_PS] =
	{"postscript", "length", "mode", "comment"};

/* Macro for indexing into a GMT grid [with pad] */
#define GMT_IJP(h,row,col) ((uint64_t)(((int64_t)(row)+(int64_t)h->pad[GMT_YHI])*((int64_t)h->mx)+(int64_t)(col)+(int64_t)h->pad[GMT_XLO]))

#define MODULE_LEN 	32	/* Max length of a GMT module name */

#ifndef rint
	#define rint(x) (floor((x)+0.5f)) //does not work reliable.
#endif

#if defined(WIN32)
#if !defined(lrint)
#	define lrint (int64_t)rint
#endif
#endif

enum MEX_dim {
	DIM_COL	= 0,	/* Holds the number of columns for vectors and x-nodes for matrix */
	DIM_ROW = 1};	/* Holds the number of rows for vectors and y-nodes for matrix */

/* Note: Wherever we say "MATLAB" below we mean "MATLAB or Octave"
 *
 * Reading and writing occur inside gmt_api.c via the standard GMT containers.
 * The packing up of GMT structures into MATLAB structures and vice versa happens after getting the
 * results out of the GMT API and before passing them back to MATLAB.
 *
 * All 6 GMT Resources are supported in this API, according to these rules:
 *  GMT_CUBE:	Handled with a MATLAB cube structure and we use GMT's GMT_CUBE for the passing
 *		  + Basic header array of length 12 [xmin, xmax, ymin, ymax, zmin, zmax, vmin, vmax, reg, xinc, yinc, zinc]
 *		  + The 3-D grid array w (single precision)
 *		  + An x-array of coordinates
 *		  + An y-array of coordinates
 *		  + An z-array of coordinates
 *		  + Various Proj4 strings
 *  GMT_GRID:	Handled with a MATLAB grid structure and we use GMT's GMT_GRID for the passing
 *		  + Basic header array of length 9 [xmin, xmax, ymin, ymax, zmin, zmax, reg, xinc, yinc]
 *		  + The 2-D grid array z (single precision)
 *		  + An x-array of coordinates
 *		  + An y-array of coordinates
 *		  + Various Proj4 strings
 *  GMT_IMAGE:	Handled with a MATLAB image structure and we use GMT's GMT_IMAGE for the passing
 *		  + Basic header array of length 9 [xmin, xmax, ymin, ymax, zmin, zmax, reg, xinc, yinc]
 *		  + The 2-D grid array z (single precision)
 *		  + An x-array of coordinates
 *		  + An y-array of coordinates
 *		  + Various Proj4 strings
 * GMT_DATASET: Handled with an array of MATLAB data structures and we use GMT's GMT_DATASET for passing in and out of GMT.
 *		Each MEX structure contain data for one segment:
 *		  + A text string for the segment header
 *		  + A 2-D matrix with rows and columns (double precision)
 *		  + An optional cell array with strings from trailing columns that could not be deciphered as data.
 *		  + First segment may also have dataset comment and proj4/wtk strings
 * GMT_PALETTE: Handled with a MATLAB structure and we use GMT's native GMT_PALETTE for the passing.
 *		  + colormap is the N*3 matrix for MATLAB colormaps
 *		  + range is a N-element array with z-values at color changes
 *		  + alpha is a N-element array with transparencies
 *		  + minmax is a 2-element array with zmin and zmax
 *		  + bnf is a 3x3-element matrix with back,fore,NaN colors
 *		  + depth is a 1-element matrix with color depth (1, 8, 24 bits)
 *		  + hinge is a 1-element matrix with hinge z-value (or NaN)
 *		  + cpt is a N*6-element matrix with original CPT slice values
 *		  + comment holds any Palette comments
 * GMT_POSTSCRIPT: Handled with a MATLAB structure and we use GMT's native GMT_POSTSCRIPT for the passing.
 *		  + postscript is the single string with all the PostScript code
 *		  + length is the number of bytes in the string
 *		  + mode is the overlay/trailer indicator
 *		  + comment holds any PostScript comments
 */

static int gmtmex_print_func (FILE *fp, const char *message) {
	/* Replacement for GMT's gmt_print_func.  It is being used indirectly via
	 * API->print_func.  Purpose of this is to allow MATLAB (which cannot use
	 * printf) to reset API->print_func to this function via GMT_Create_Session.
	 * This allows GMT's errors and warnings to appear in MATLAB console. */
	gmt_M_unused (fp);
	mexPrintf (message);
	return 0;
}

static uint64_t gmtmex_getMNK (const mxArray *p, int which) {
	/* Get number of columns or number of bands of a mxArray.
	   which = 0 to inquire n_rows
	         = 1 to inquire n_columns
	         = 2 to inquire n_bands
	         = ? ERROR
	*/
	uint64_t nx, ny, nBands, nDims;
	const mwSize *dim_array = NULL;

	nDims     = mxGetNumberOfDimensions(p);
	dim_array = mxGetDimensions(p);
	ny = dim_array[0];
	nx = dim_array[1];
	nBands = dim_array[2];
	if (nDims == 2) 	/* Otherwise it would stay undefined */
		nBands = 1;

	if (which == 0)
		return ny;
	else if (which == 1)
		return nx;
	else if (which == 2)
		return nBands;
	else
		mexErrMsgTxt("gmtmex_getMNK: Bad dimension number!");
	return 0;
}

static void gmtmex_quit_if_missing (const char *function, const char *field) {
	char buffer[128] = {""};
	sprintf (buffer , "%s: Could not find structure field %s\n", function, field);
	mexErrMsgTxt (buffer);
}

static void *gmtmex_get_grid (void *API, struct GMT_GRID *G) {
	/* Given an incoming GMT grid G, build a MATLAB structure and assign the output components.
 	 * Note: Incoming GMT grid has standard padding while MATLAB grid has none. */

	unsigned int k;
	uint64_t row, col, gmt_ij;
	float  *f = NULL;
	double *d = NULL, *G_x = NULL, *G_y = NULL, *x = NULL, *y = NULL;
	mxArray *G_struct = NULL, *mxptr[N_MEX_FIELDNAMES_GRID];

	if (!G->data)	/* Safety valve */
		mexErrMsgTxt ("gmtmex_get_grid: programming error, output matrix G is empty\n");

	/* Create a MATLAB struct to hold this grid [matrix will be a float (mxSINGLE_CLASS)]. */
	G_struct = mxCreateStructMatrix (1, 1, N_MEX_FIELDNAMES_GRID, gmtmex_fieldname_grid);

	/* Get pointers and populate structure from the information in G */
	mxptr[0]  = mxCreateNumericMatrix (G->header->n_rows, G->header->n_columns, mxSINGLE_CLASS, mxREAL);
	mxptr[1]  = mxCreateNumericMatrix (1, G->header->n_columns, mxDOUBLE_CLASS, mxREAL);
	mxptr[2]  = mxCreateNumericMatrix (1, G->header->n_rows,    mxDOUBLE_CLASS, mxREAL);
	mxptr[3]  = mxCreateNumericMatrix (1, 6, mxDOUBLE_CLASS, mxREAL);
	mxptr[4]  = mxCreateNumericMatrix (1, 2, mxDOUBLE_CLASS, mxREAL);
	mxptr[5]  = mxCreateDoubleScalar ((double)G->header->registration);
	mxptr[6]  = mxCreateDoubleScalar ((double)G->header->nan_value);
	mxptr[7]  = mxCreateString (G->header->title);
	mxptr[8]  = mxCreateString (G->header->remark);
	mxptr[9]  = mxCreateString (G->header->command);
	mxptr[10] = mxCreateString ("float32");
	mxptr[11] = mxCreateString (G->header->x_units);
	mxptr[12] = mxCreateString (G->header->y_units);
	mxptr[13] = mxCreateString (G->header->z_units);
	mxptr[14] = (G->header->mem_layout[0]) ? mxCreateString(G->header->mem_layout) : mxCreateString ("TCB");
	mxptr[15] = mxCreateString (G->header->ProjRefPROJ4);
	mxptr[16] = mxCreateString (G->header->ProjRefWKT);

	d = mxGetPr (mxptr[3]);	/* Range */
	for (k = 0; k < 4; k++) d[k] = G->header->wesn[k];
	d[4] = G->header->z_min;	d[5] = G->header->z_max;

	d = mxGetPr (mxptr[4]);	/* Increments */
	for (k = 0; k < 2; k++) d[k] = G->header->inc[k];

	/* Load the real grd array into a float MATLAB array by transposing
           from padded GMT grd format to unpadded MATLAB format */
	f = mxGetData (mxptr[0]);
	for (row = 0; row < G->header->n_rows; row++) {
		for (col = 0; col < G->header->n_columns; col++) {
			gmt_ij = GMT_IJP (G->header, row, col);
			f[MEXG_IJ(G,row,col)] = G->data[gmt_ij];
		}
	}

	/* Also return the convenient x and y arrays */
	G_x = GMT_Get_Coord (API, GMT_IS_GRID, GMT_X, G);	/* Get array of x coordinates */
	G_y = GMT_Get_Coord (API, GMT_IS_GRID, GMT_Y, G);	/* Get array of y coordinates */
	x = mxGetData (mxptr[1]);
	y = mxGetData (mxptr[2]);
	memcpy (x, G_x, G->header->n_columns * sizeof (double));
	for (k = 0; k < G->header->n_rows; k++)
		y[G->header->n_rows-1-k] = G_y[k];	/* Must reverse the y-array */
	if (GMT_Destroy_Data (API, &G_x))
		mexPrintf("Warning: Failure to delete G_x (x coordinate vector)\n");
	if (GMT_Destroy_Data (API, &G_y))
		mexPrintf("Warning: Failure to delete G_y (y coordinate vector)\n");
	for (k = 0; k < N_MEX_FIELDNAMES_GRID; k++)
		mxSetField (G_struct, 0, gmtmex_fieldname_grid[k], mxptr[k]);
	return (G_struct);
}

static void *gmtmex_get_cube (void *API, struct GMT_CUBE *U) {
	/* Given an incoming GMT cube U, build a MATLAB structure and assign the output components.
 	 * Note: Incoming GMT cube has standard padding while MATLAB cube has none. */

	unsigned int k;
	uint64_t row, col, layer, gmt_ij, offset = 0;
	mwSize ndim = 3, dim[3];
	float  *f = NULL;
	double *d = NULL, *U_x = NULL, *U_y = NULL, *x = NULL, *y = NULL, *z = NULL;
	mxArray *U_struct = NULL, *mxptr[N_MEX_FIELDNAMES_CUBE];

	if (!U->data)	/* Safety valve */
		mexErrMsgTxt ("gmtmex_get_cube: programming error, output cube U is empty\n");

	if (U->header->n_bands == 1) {	/* One layer cube is just a grid */
		struct GMT_GRID *G = NULL;
		static void *ptr = NULL;
		if ((G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY,
		        NULL, U->header->wesn, U->header->inc, U->header->registration, 2, NULL)) == NULL)
			mexErrMsgTxt ("gmtmex_get_cube: Failure to alloc GMT source grid for input\n");
		G->data = U->data;	/* Temporarily read from the single cube layer */
		ptr = gmtmex_get_grid (API, G);
		G->data = NULL;		/* Undo this since nothing was allocated for G above */
		if (GMT_Destroy_Data (API, &G))
			mexPrintf("Warning: Failure to delete G (grid layer in gmtmex_get_cube)\n");

		return (ptr);
	}

	/* Create a MATLAB struct to hold this cube [matrix will be a float (mxSINGLE_CLASS)]. */
	U_struct = mxCreateStructMatrix (1, 1, N_MEX_FIELDNAMES_CUBE, gmtmex_fieldname_cube);
	dim[0] = U->header->n_rows;	dim[1] = U->header->n_columns;	dim[2] = U->header->n_bands;

	/* Get pointers and populate structure from the information in U */
	mxptr[0]  = mxCreateNumericArray (ndim, dim, mxSINGLE_CLASS, mxREAL);
	mxptr[1]  = mxCreateNumericMatrix (1, U->header->n_columns, mxDOUBLE_CLASS, mxREAL);
	mxptr[2]  = mxCreateNumericMatrix (1, U->header->n_rows,    mxDOUBLE_CLASS, mxREAL);
	mxptr[3]  = mxCreateNumericMatrix (1, U->header->n_bands,   mxDOUBLE_CLASS, mxREAL);
	mxptr[4]  = mxCreateNumericMatrix (1, 8, mxDOUBLE_CLASS, mxREAL);
	mxptr[5]  = mxCreateNumericMatrix (1, 3, mxDOUBLE_CLASS, mxREAL);
	mxptr[6]  = mxCreateDoubleScalar ((double)U->header->registration);
	mxptr[7]  = mxCreateDoubleScalar ((double)U->header->nan_value);
	mxptr[8]  = mxCreateString (U->header->title);
	mxptr[9]  = mxCreateString (U->header->remark);
	mxptr[10]  = mxCreateString (U->header->command);
	mxptr[11] = mxCreateString ("float32");
	mxptr[12] = mxCreateString (U->header->x_units);
	mxptr[13] = mxCreateString (U->header->y_units);
	mxptr[14] = mxCreateString (U->units);
	mxptr[15] = mxCreateString (U->header->z_units);
	mxptr[16] = (U->header->mem_layout[0]) ? mxCreateString(U->header->mem_layout) : mxCreateString ("TCB");
	mxptr[17] = mxCreateString (U->header->ProjRefPROJ4);
	mxptr[18] = mxCreateString (U->header->ProjRefWKT);

	d = mxGetPr (mxptr[4]);	/* Range */
	for (k = 0; k < 4; k++) d[k] = U->header->wesn[k];
	d[4] = U->z_range[0];	d[5] = U->z_range[1];
	d[6] = U->header->z_min;	d[7] = U->header->z_max;

	d = mxGetPr (mxptr[5]);	/* Increments */
	for (k = 0; k < 2; k++) d[k] = U->header->inc[k];
	d[2] = U->z_inc;

	/* Load the real grd array into a float MATLAB array by transposing
           from padded GMT grd format to unpadded MATLAB format */
	f = mxGetData (mxptr[0]);
	for (layer = 0; layer < U->header->n_bands; layer++) {
		for (row = 0; row < U->header->n_rows; row++) {
			for (col = 0; col < U->header->n_columns; col++) {
				gmt_ij = GMT_IJP (U->header, row, col) + offset;
				f[MEXU_IJK(U,layer,row,col)] = U->data[gmt_ij];
			}
		}
		offset += U->header->size;
	}

	/* Also return the convenient x and y arrays */
	U_x = GMT_Get_Coord (API, GMT_IS_GRID, GMT_X, U);	/* Get array of x coordinates */
	U_y = GMT_Get_Coord (API, GMT_IS_GRID, GMT_Y, U);	/* Get array of y coordinates */
	x = mxGetData (mxptr[1]);
	y = mxGetData (mxptr[2]);
	z = mxGetData (mxptr[3]);
	memcpy (z, U->z, U->header->n_bands  * sizeof (double));
	memcpy (x, U_x, U->header->n_columns * sizeof (double));
	for (k = 0; k < U->header->n_rows; k++)
		y[U->header->n_rows-1-k] = U_y[k];	/* Must reverse the y-array */
	for (k = 0; k < N_MEX_FIELDNAMES_CUBE; k++)
		mxSetField (U_struct, 0, gmtmex_fieldname_cube[k], mxptr[k]);
	return (U_struct);
}

static void *gmtmex_get_dataset (void *API, struct GMT_DATASET *D) {
	/* Given a GMT DATASET D, build a MATLAB array of segment structure and assign values.
	 * Each segment will have 6 items:
	 * header:	Text string with the segment header (could be empty)
	 * data:	Matrix with the data for this segment (n_rows by n_columns)
	 * text:	Optional cell array for trailing text
	 * comment:	Cell array with any comments
	 * proj4:	String with any proj4 information
	 * wkt:		String with any WKT information
	 */

	int n_headers;
	uint64_t tbl, seg, seg_out, col, row, start, k, n_items = 1;
	double *data = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	mxArray *D_struct = NULL, *mxheader = NULL, *mxdata = NULL, *mxtext = NULL, *mxstring = NULL;
	gmt_M_unused (API);

	if (D == NULL) {	/* No output produced (?) - return a null data set */
		D_struct = mxCreateStructMatrix (0, 0, N_MEX_FIELDNAMES_DATASET, gmtmex_fieldname_dataset);
		return (D_struct);
	}

	for (tbl = seg_out = 0; tbl < D->n_tables; tbl++)	/* Count non-zero segments */
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++)
			if (D->table[tbl]->segment[seg]->n_rows)
				seg_out++;
	if (seg_out == 0) n_items = 0;
	D_struct = mxCreateStructMatrix ((mwSize)seg_out, (mwSize)n_items, N_MEX_FIELDNAMES_DATASET, gmtmex_fieldname_dataset);

	n_headers = (D->n_tables) ? D->table[0]->n_headers : 0;	/* Number of header records in first table */
	for (tbl = seg_out = 0; tbl < D->n_tables; tbl++) {
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
			S = D->table[tbl]->segment[seg];	/* Shorthand */
			if (S->n_rows == 0) continue;		/* Skip empty segments */
			if (S->header) {	/* Has segment header */
				mxheader = mxCreateString (S->header);
				mxSetField (D_struct, (mwSize)seg_out, "header", mxheader);
			}
			if (S->text) {	/* Has trailing text */
				mxtext   = mxCreateCellMatrix (S->n_rows, 1);
				for (row = 0; row < S->n_rows; row++) {
					mxstring = mxCreateString (S->text[row]);
					mxSetCell (mxtext, (int)row, mxstring);
				}
				mxSetField (D_struct, (mwSize)seg_out, "text", mxtext);
			}
			if (S->n_columns) {	/* Has numerical data */
				mxdata   = mxCreateNumericMatrix ((mwSize)S->n_rows, (mwSize)S->n_columns, mxDOUBLE_CLASS, mxREAL);
				data      = mxGetPr (mxdata);
				for (col = start = 0; col < S->n_columns; col++, start += S->n_rows) /* Copy the data columns */
					memcpy (&data[start], S->data[col], S->n_rows * sizeof (double));
				mxSetField (D_struct, (mwSize)seg_out, "data", mxdata);
			}
			if (n_headers) {	/* First segment will get any headers, the rest nothing */
				mxtext = mxCreateCellMatrix (n_headers, n_headers ? 1 : 0);
				for (k = 0; k < (uint64_t)n_headers; k++) {
					mxstring = mxCreateString (D->table[0]->header[k]);
					mxSetCell (mxtext, (int)k, mxstring);
				}
				mxSetField (D_struct, (mwSize)seg_out, "comment", mxtext);
				n_headers = 0;	/* No other segment will have a non-empty comment cell array */
			}
			seg_out++;
		}
	}
	return (D_struct);
}

static void *gmtmex_get_postscript (void *API, struct GMT_POSTSCRIPT *P) {
	/* Given a GMT GMT_POSTSCRIPT P, build a MATLAB array of segment structure and assign values.
	 * Each segment will have 4 items:
	 * postscript:	Text string with the entire PostScript plot
	 * length:	Byte length of postscript
	 * mode:	1 has header, 2 has trailer, 3 is complete
	 * comment:	Cell array with any comments
	 */
	uint64_t k, *length = NULL;
	unsigned int *mode = NULL;
	mxArray *P_struct = NULL, *mxptr[N_MEX_FIELDNAMES_PS], *mxstring = NULL;
	gmt_M_unused (API);

	if (P == NULL)	/* Safety valve */
		mexErrMsgTxt ("gmtmex_get_postscript: programming error, input POSTSCRIPT struct P is NULL or data string is empty\n");

	if (!P->data) {
		/* Return empty PS struct */
		P_struct = mxCreateStructMatrix (0, 0, N_MEX_FIELDNAMES_PS, gmtmex_fieldname_ps);
		return P_struct;
	}

	/* Return PS with postscript and length in a struct */
	P_struct = mxCreateStructMatrix (1, 1, N_MEX_FIELDNAMES_PS, gmtmex_fieldname_ps);

	mxptr[0] = mxCreateString (P->data);
	mxptr[1] = mxCreateNumericMatrix (1, 1, mxUINT64_CLASS, mxREAL);
	mxptr[2] = mxCreateNumericMatrix (1, 1, mxUINT32_CLASS, mxREAL);
	mxptr[3] = mxCreateCellMatrix (P->n_headers, P->n_headers ? 1 : 0);
	length   = (uint64_t *)mxGetData(mxptr[1]);
	mode     = (uint32_t *)mxGetData(mxptr[2]);

	length[0] = (uint64_t)P->n_bytes;	/* Set length of the PS string */
	mode[0]   = (uint32_t)P->mode;		/* Set mode of the PS string */

	for (k = 0; k < P->n_headers; k++) {
		mxstring = mxCreateString (P->header[k]);
		mxSetCell (mxptr[3], (int)k, mxstring);
	}

	for (k = 0; k < N_MEX_FIELDNAMES_PS; k++)
		mxSetField (P_struct, 0, gmtmex_fieldname_ps[k], mxptr[k]);

	return P_struct;
}

static void *gmtmex_get_palette (void *API, struct GMT_PALETTE *C) {
	/* Given a GMT GMT_PALETTE C, build a MATLAB structure and assign values.
	 * Each segment will have 10 items:
	 * colormap:	Nx3 array of colors usable in Matlab' colormap
	 * alpha:	Nx1 array with transparency values
	 * range:	Nx1 arran with z-values at color changes
	 * minmax:	2x1 array with min/max zvalues
	 * bfn:		3x3 array with colors for background, forground, nan
	 * depth	Color depth 24, 8, 1
	 * hinge:	Z-value at discontinuous color break, or NaN
	 * cpt:		Nx6 full GMT CPT array
	 * model:	String with color model rgb, hsv, or cmyk [rgb]
	 * comment:	Cell array with any comments
	 *
	 * Limitation: MATLAB's colormap format can either hold discrete
	 * or continuous colormaps, but not a mixture of these, which GMT
	 * can do.  Thus, mixed-mode GMT cpts being used in MATLAB or passed
	 * out from MATLAB cannot represent these changes accurately. */

	unsigned int k, j, n_colors, *depth = NULL;
	double *color = NULL, *cpt = NULL, *alpha = NULL, *minmax = NULL, *range = NULL, *hinge = NULL, *cyclic = NULL, *bfn = NULL;
	mxArray *C_struct = NULL, *mxptr[N_MEX_FIELDNAMES_CPT], *mxstring = NULL;
	gmt_M_unused (API);

	if (C == NULL)	/* Safety valve */
		mexErrMsgTxt ("gmtmex_get_palette: programming error, output CPT C is empty\n");

	if (!C->data) {	/* Return empty struct */
		C_struct = mxCreateStructMatrix (0, 0, N_MEX_FIELDNAMES_CPT, gmtmex_fieldname_cpt);
		return (C_struct);
	}

	/* Return CPT via colormap, range, and alpha arrays in a struct */
	/* Create a MATLAB struct for this CPT */
	C_struct = mxCreateStructMatrix (1, 1, N_MEX_FIELDNAMES_CPT, gmtmex_fieldname_cpt);

	n_colors = (C->is_continuous) ? C->n_colors + 1 : C->n_colors;
	mxptr[0] = mxCreateNumericMatrix (n_colors, 3, mxDOUBLE_CLASS, mxREAL);
	mxptr[1] = mxCreateNumericMatrix (n_colors, 1, mxDOUBLE_CLASS, mxREAL);
	mxptr[2] = mxCreateNumericMatrix (C->n_colors, 2, mxDOUBLE_CLASS, mxREAL);
	mxptr[3] = mxCreateNumericMatrix (2, 1, mxDOUBLE_CLASS, mxREAL);
	mxptr[4] = mxCreateNumericMatrix (3, 3, mxDOUBLE_CLASS, mxREAL);
	mxptr[5] = mxCreateNumericMatrix (1, 1, mxUINT32_CLASS, mxREAL);
	mxptr[6] = mxCreateNumericMatrix (1, 1, mxDOUBLE_CLASS, mxREAL);
	mxptr[7] = mxCreateNumericMatrix (C->n_colors, 6, mxDOUBLE_CLASS, mxREAL);
	mxptr[8] = NULL;	/* Set below */
	mxptr[9] = mxCreateNumericMatrix (1, 1, mxDOUBLE_CLASS, mxREAL);
	mxptr[10] = mxCreateCellMatrix (C->n_headers, C->n_headers ? 1 : 0);

	color    = mxGetPr (mxptr[0]);
	alpha    = mxGetPr (mxptr[1]);
	range    = mxGetPr (mxptr[2]);
	minmax   = mxGetPr (mxptr[3]);
	bfn      = mxGetPr (mxptr[4]);
	depth    = (uint32_t *)mxGetData (mxptr[5]);
	hinge    = mxGetPr (mxptr[6]);
	cpt      = mxGetPr (mxptr[7]);
	cyclic   = mxGetPr (mxptr[9]);
	depth[0] = (C->is_bw) ? 1 : ((C->is_gray) ? 8 : 24);
	hinge[0] = (C->has_hinge) ? C->hinge : mxGetNaN ();
	cyclic[0] = (C->is_wrapping) ? 1.0 : 0.0;
	for (j = 0; j < 3; j++)	/* Copy r/g/b from palette bfn to MATLAB array */
		for (k = 0; k < 3; k++) bfn[j+3*k] = C->bfn[j].rgb[k];
	for (j = 0; j < C->n_colors; j++) {	/* Copy r/g/b from palette to MATLAB colormap and cpt */
		for (k = 0; k < 3; k++) {
			color[j+k*n_colors] = cpt[j+k*C->n_colors] = C->data[j].rgb_low[k];
			cpt[j+(k+3)*C->n_colors] = C->data[j].rgb_high[k];
		}
		alpha[j] = C->data[j].rgb_low[3];
		range[j] = C->data[j].z_low;
		range[j+C->n_colors] = C->data[j].z_high;
	}
	if (C->is_continuous) {	/* Add last color/alpha to colormap */
		for (k = 0; k < 3; k++) color[j+k*n_colors] = C->data[C->n_colors-1].rgb_high[k];
		alpha[j] = C->data[j].rgb_low[3];
	}
	minmax[0] = C->data[0].z_low;	/* Set min/max limits */
	minmax[1] = C->data[C->n_colors-1].z_high;
	if (C->n_headers) {
		for (k = 0; k < C->n_headers; k++) {
			mxstring = mxCreateString (C->header[k]);
			mxSetCell (mxptr[10], (int)k, mxstring);
		}
	}
	if (C->model & GMT_HSV)
		mxptr[8] = mxCreateString ("hsv");
	else if (C->model & GMT_CMYK)
		mxptr[8] = mxCreateString ("cmyk");
	else
		mxptr[8] = mxCreateString ("rgb");

	for (k = 0; k < N_MEX_FIELDNAMES_CPT; k++)	/* Update all fields */
		mxSetField (C_struct, 0, gmtmex_fieldname_cpt[k], mxptr[k]);
	return (C_struct);
}

static void *gmtmex_get_image (void *API, struct GMT_IMAGE *I) {
	unsigned int k;
	mwSize   dim[3];
	uint8_t *u = NULL, *alpha = NULL;
	double  *d = NULL, *I_x = NULL, *I_y = NULL, *x = NULL, *y = NULL, *color = NULL;
	mxArray *I_struct = NULL, *mxptr[N_MEX_FIELDNAMES_IMAGE];

	if (I == NULL || !I->data)	/* Safety valve */
		mexErrMsgTxt ("gmtmex_get_image: programming error, output image I is empty\n");

	/* Return image via a uint8_t (mxUINT8_CLASS) matrix in a struct */
	/* Create a MATLAB struct for this image */
	I_struct = mxCreateStructMatrix (1, 1, N_MEX_FIELDNAMES_IMAGE, gmtmex_fieldname_image);
	/* Create the various fields with information from I */
	mxptr[0]  = NULL;	/* Set below */
	mxptr[1]  = mxCreateNumericMatrix (1, I->header->n_columns, mxDOUBLE_CLASS, mxREAL);
	mxptr[2]  = mxCreateNumericMatrix (1, I->header->n_rows, mxDOUBLE_CLASS, mxREAL);
	mxptr[3]  = mxCreateNumericMatrix (1, 6, mxDOUBLE_CLASS, mxREAL);
	mxptr[4]  = mxCreateNumericMatrix (1, 2, mxDOUBLE_CLASS, mxREAL);
	mxptr[5]  = mxCreateDoubleScalar ((double)I->header->registration);
	mxptr[6]  = mxCreateDoubleScalar ((double)I->header->nan_value);
	mxptr[7]  = mxCreateString (I->header->title);
	mxptr[8]  = mxCreateString (I->header->remark);
	mxptr[9]  = mxCreateString (I->header->command);
	mxptr[10] = mxCreateString ("uint8");
	mxptr[11] = mxCreateString (I->header->x_units);
	mxptr[12] = mxCreateString (I->header->y_units);
	mxptr[13] = mxCreateString (I->header->z_units);
	mxptr[14] = mxptr[15] = NULL;	/* Set below */
	mxptr[16] = (I->header->mem_layout[0]) ? mxCreateString(I->header->mem_layout) : mxCreateString ("TCBa");
	mxptr[17] = mxCreateString (I->header->ProjRefPROJ4);
	mxptr[18] = mxCreateString (I->header->ProjRefWKT);

	/* Fill in values */
	d = mxGetPr (mxptr[3]);	/* Range */
	for (k = 0; k < 4; k++) d[k] = I->header->wesn[k];
	d[4] = I->header->z_min;	d[5] = I->header->z_max;

	d = mxGetPr(mxptr[4]);	/* Increments */
	for (k = 0; k < 2; k++) d[k] = I->header->inc[k];

	if (I->colormap != NULL) {	/* Indexed image has a color map */
		mxptr[14] = mxCreateNumericMatrix (I->n_indexed_colors, 3, mxDOUBLE_CLASS, mxREAL);
		mxptr[0]  = mxCreateNumericMatrix (I->header->n_rows, I->header->n_columns, mxUINT8_CLASS, mxREAL);
		u     = mxGetData (mxptr[0]);
		color = mxGetPr (mxptr[14]);
		for (k = 0; k < 4 * (unsigned int)I->n_indexed_colors && I->colormap[k] >= 0; k++)
			color[k] = (uint8_t)I->colormap[k];
		k /= 4;
		memcpy (u, I->data, I->header->nm * sizeof (uint8_t));
	}
	else if (I->header->n_bands == 1) {	/* gray image */
		mxptr[0] = mxCreateNumericMatrix (I->header->n_rows, I->header->n_columns, mxUINT8_CLASS, mxREAL);
		u = mxGetData (mxptr[0]);
		memcpy (u, I->data, I->header->nm * sizeof (uint8_t));
	}
	else if (I->header->n_bands == 3) {	/* RGB image */
		dim[0] = I->header->n_rows;	dim[1] = I->header->n_columns; dim[2] = 3;
		mxptr[0] = mxCreateNumericArray (3, dim, mxUINT8_CLASS, mxREAL);
		u = mxGetData (mxptr[0]);
		if (!strncmp(I->header->mem_layout, "TCBa", 4))
			memcpy (u, I->data, 3 * I->header->nm * sizeof (uint8_t));
		else if (!strncmp(I->header->mem_layout, "TRPa", 4)) {
			GMT_Change_Layout (API, GMT_IS_IMAGE, "TCB", 0, I, u, alpha);		/* Convert from TRP to TCB */
			mxptr[16] = mxCreateString ("TCBa");	/* Because we just converted to it above */
		}
		else {
			mexPrintf("WarnError: this image's' memory layout, %s, is not implemented. Expect random art.\n");
		}
		if (I->alpha) {
			mxptr[15] = mxCreateNumericMatrix (I->header->n_rows, I->header->n_columns, mxUINT8_CLASS, mxREAL);
			alpha = mxGetData (mxptr[15]);
			memcpy (alpha, I->alpha, I->header->nm * sizeof (uint8_t));
		}
	}
	else if (I->header->n_bands == 4) {	/* RGBA image, with a color map */
		dim[0] = I->header->n_rows;	dim[1] = I->header->n_columns; dim[2] = 3;
		mxptr[0] = mxCreateNumericArray (3, dim, mxUINT8_CLASS, mxREAL);
		u = mxGetData (mxptr[0]);
		mxptr[15] = mxCreateNumericMatrix (I->header->n_rows, I->header->n_columns, mxUINT8_CLASS, mxREAL);
		alpha = mxGetData (mxptr[15]);
		memcpy (u, I->data, 3 * I->header->nm * sizeof (uint8_t));
		memcpy (alpha, &(I->data)[3 * I->header->nm], I->header->nm * sizeof (uint8_t));
		/*
		for (k = 0; k < I->header->nm; k++) {
			for (m = 0; m < 3; m++)
				u[k+m*I->header->nm] = (uint8_t)I->data[4*k+m];
			alpha[k] = (uint8_t)I->data[4*k+3];
		}
		*/
	}

	/* Also return the convenient x and y arrays */
	I_x = GMT_Get_Coord (API, GMT_IS_IMAGE, GMT_X, I);	/* Get array of x coordinates */
	I_y = GMT_Get_Coord (API, GMT_IS_IMAGE, GMT_Y, I);	/* Get array of y coordinates */
	x = mxGetData (mxptr[1]);
	y = mxGetData (mxptr[2]);
	memcpy (x, I_x, I->header->n_columns * sizeof (double));
	for (k = 0; k < I->header->n_rows; k++)	/* Must reverse the y-array */
		y[I->header->n_rows-1-k] = I_y[k];
	if (GMT_Destroy_Data (API, &I_x))
		mexPrintf("Warning: Failure to delete I_x (x coordinate vector)\n");
	if (GMT_Destroy_Data (API, &I_y))
		mexPrintf("Warning: Failure to delete I_y (y coordinate vector)\n");
	for (k = 0; k < N_MEX_FIELDNAMES_IMAGE; k++) {	/* Update fields */
		if (mxptr[k]) mxSetField (I_struct, 0, gmtmex_fieldname_image[k], mxptr[k]);
	}
	return (I_struct);
}

static struct GMT_CUBE *gmtmex_cube_init (void *API, unsigned int direction, unsigned int module_input, const mxArray *ptr) {
	/* Used to Create an empty Cube container to hold a GMT cube.
 	 * If direction is GMT_IN then we are given a MATLAB cube and can determine its size, etc.
	 * If direction is GMT_OUT then we allocate an empty GMT cube as a destination. */
	unsigned int row, col, layer;
	uint64_t gmt_ij, offset = 0;
	struct GMT_CUBE *U = NULL;

	if (direction == GMT_IN) {	/* Dimensions are known from the input pointer */
		unsigned int registration, flag = (module_input) ? GMT_VIA_MODULE_INPUT : 0;
		mxArray *mx_ptr = NULL, *mxCube = NULL;

		if (mxIsEmpty (ptr))
			mexErrMsgTxt ("gmtmex_cube_init: The input that was supposed to contain the Cube, is empty\n");

		if (mxIsStruct(ptr)) {	/* Passed a regular MEX Cube structure */
			double *inc = NULL, *range = NULL, *z = NULL, *reg = NULL;
			unsigned int pad = (unsigned int)GMT_NOTSET;
			uint64_t *this_dim = NULL, dims[3] = {0, 0, 0};
			char x_unit[GMT_GRID_VARNAME_LEN80] = { "" }, y_unit[GMT_GRID_VARNAME_LEN80] = { "" },
			     z_unit[GMT_GRID_VARNAME_LEN80] = { "" }, v_unit[GMT_GRID_VARNAME_LEN80] = { "" }, layout[3];
			mx_ptr = mxGetField (ptr, 0, "inc");
			if (mx_ptr == NULL)
				mexErrMsgTxt ("gmtmex_cube_init: Could not find inc array with Cube increments\n");
			inc = mxGetData (mx_ptr);

			mx_ptr = mxGetField (ptr, 0, "range");
			if (mx_ptr == NULL)
				mexErrMsgTxt ("gmtmex_cube_init: Could not find range array for Cube range\n");
			range = mxGetData (mx_ptr);

			mxCube = mxGetField(ptr, 0, "v");
			if (mxCube == NULL)
				mexErrMsgTxt ("gmtmex_cube_init: Could not find data array for Cube\n");
			if (!mxIsSingle(mxCube) && !mxIsDouble(mxCube))
				mexErrMsgTxt ("gmtmex_cube_init: data array must be either single or double.\n");

			mx_ptr = mxGetField (ptr, 0, "registration");
			if (mx_ptr == NULL)
				mexErrMsgTxt ("gmtmex_cube_init: Could not find registration array for Cube registration\n");
			reg = mxGetData (mx_ptr);
			registration = (unsigned int)lrint(reg[0]);


			mx_ptr = mxGetField(ptr, 0, "pad");
			if (mx_ptr != NULL) {
				double *dpad = mxGetData(mx_ptr);
				pad = (unsigned int)dpad[0];
				if (pad > 2)
					mexPrintf("gmtmex_cube_init:  This pad value (%d) is very probably wrong.\n");
			}

			if (inc[2] == 0.0) {	/* Non-equidistant cube layering, must allocate based on dimensions */
				mexPrintf("gmtmex_cube_init:  Detected non-equidistant z-spacing.\n");
				if ((mx_ptr = mxGetField (ptr, 0, "z")) == NULL)
					mexErrMsgTxt ("gmtmex_cube_init: Could not find z array for Cube z-nodes\n");
				dims[2] = mxGetN(mx_ptr);	/* Number of output levels */
				this_dim = dims;	/* Pointer to the dims instead of NULL */
				z = mxGetData (mx_ptr);	/* Get the non-equidistant z nodes */
			}
			if ((U = GMT_Create_Data (API, GMT_IS_CUBE|flag, GMT_IS_VOLUME, GMT_CONTAINER_AND_DATA,
			                          this_dim, range, inc, registration, pad, NULL)) == NULL)
				mexErrMsgTxt ("gmtmex_cube_init: Failure to alloc GMT source Cube for input\n");

			if (this_dim) {	/* If not equidistant we must duplicate the level array into the Cube manually */
				if (U->z == NULL && GMT_Put_Levels (API, U, z, dims[2]))
					mexErrMsgTxt ("gmtmex_cube_init: Failure to put non-equidistant z-nodes into the cube structure\n");
			}

			U->z_range[0] = range[4];
			U->z_range[1] = range[5];
			U->header->z_min = range[6];
			U->header->z_max = range[7];

			U->header->registration = registration;

			mx_ptr = mxGetField (ptr, 0, "nodata");
			if (mx_ptr != NULL)
				U->header->nan_value = *(float *)mxGetData (mx_ptr);

			mx_ptr = mxGetField (ptr, 0, "proj4");
			if (mx_ptr != NULL && mxGetN(mx_ptr) > 6) {		/* A true proj4 string will have at least this length */
				char *str = malloc(mxGetN(mx_ptr) + 1);
				mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
				U->header->ProjRefPROJ4 = GMT_Duplicate_String (API, str);
				free (str);
			}
			mx_ptr = mxGetField (ptr, 0, "wkt");
			if (mx_ptr != NULL && mxGetN(mx_ptr) > 20) {	/* A true WKT string will have more than this length */
				char *str = malloc(mxGetN(mx_ptr) + 1);
				mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
				U->header->ProjRefWKT = GMT_Duplicate_String (API, str);
				free (str);
			}
			mx_ptr = mxGetField (ptr, 0, "title");
			if (mx_ptr != NULL) {
				char *str = malloc(mxGetN(mx_ptr) + 1);
				mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(U->header->title, str, GMT_GRID_VARNAME_LEN80 - 1);
				free (str);
			}
			mx_ptr = mxGetField (ptr, 0, "command");
			if (mx_ptr != NULL) {
				char *str = malloc(mxGetN(mx_ptr) + 1);
				mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(U->header->command, str, GMT_GRID_COMMAND_LEN320 - 1);
				free (str);
			}
			mx_ptr = mxGetField (ptr, 0, "comment");
			if (mx_ptr != NULL) {
				char *str = malloc(mxGetN(mx_ptr)+2);
				mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(U->header->remark, str, GMT_GRID_REMARK_LEN160 - 1);
				free (str);
			}
			mx_ptr = mxGetField (ptr, 0, "x_unit");
			if (mx_ptr != NULL) {
				mxGetString(mx_ptr, x_unit, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(U->header->x_units, x_unit, GMT_GRID_VARNAME_LEN80 - 1);
			}
			mx_ptr = mxGetField (ptr, 0, "y_unit");
			if (mx_ptr != NULL) {
				mxGetString(mx_ptr, y_unit, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(U->header->y_units, y_unit, GMT_GRID_VARNAME_LEN80 - 1);
			}
			mx_ptr = mxGetField (ptr, 0, "z_unit");
			if (mx_ptr != NULL) {
				mxGetString(mx_ptr, z_unit, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(U->units, z_unit, GMT_GRID_VARNAME_LEN80 - 1);
			}
			mx_ptr = mxGetField (ptr, 0, "v_unit");
			if (mx_ptr != NULL) {
				mxGetString(mx_ptr, v_unit, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(U->header->z_units, z_unit, GMT_GRID_VARNAME_LEN80 - 1);
			}
			mx_ptr = mxGetField (ptr, 0, "layout");
			if (mx_ptr != NULL) {
				mxGetString(mx_ptr, layout, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(U->header->mem_layout, layout, 3);
			}
			else
				strncpy(U->header->mem_layout, "TRS", 3);
		}

		if (mxIsSingle(mxCube)) {
			float *f4 = mxGetData(mxCube);
			if (f4 == NULL)
				mexErrMsgTxt("gmtmex_cube_init: Cube pointer is NULL where it absolutely could not be.");
			for (layer = 0; layer < U->header->n_bands; layer++) {
				for (row = 0; row < U->header->n_rows; row++) {
					for (col = 0; col < U->header->n_columns; col++) {
						gmt_ij = GMT_IJP (U->header, row, col) + offset;
						U->data[gmt_ij] = f4[MEXU_IJK(U,layer,row,col)];
					}
				}
				offset += U->header->size;
			}
		}
		else {
			double *f8 = mxGetData(mxCube);
			if (f8 == NULL)
				mexErrMsgTxt("gmtmex_cube_init: Cube pointer is NULL where it absolutely could not be.");
			for (layer = 0; layer < U->header->n_bands; layer++) {
				for (row = 0; row < U->header->n_rows; row++) {
					for (col = 0; col < U->header->n_columns; col++) {
						gmt_ij = GMT_IJP (U->header, row, col) + offset;
						U->data[gmt_ij] = (float)f8[MEXU_IJK(U,layer,row,col)];
					}
				}
				offset += U->header->size;
			}
		}
		GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_cube_init: Allocated GMT Cube %lx\n", (long)U);
		GMT_Report (API, GMT_MSG_DEBUG,
		            "gmtmex_cube_init: Registered GMT Grid array %lx via memory reference from MATLAB\n",
		            (long)U->data);
	}
	else {	/* Just allocate an empty container to hold an output grid (signal this by passing 0s and NULLs [mode == GMT_IS_OUTPUT from 5.4]) */
		if ((U = GMT_Create_Data (API, GMT_IS_CUBE, GMT_IS_VOLUME, GMT_IS_OUTPUT,
		                          NULL, NULL, NULL, 0, 0, NULL)) == NULL)
			mexErrMsgTxt ("gmtmex_cube_init: Failure to alloc GMT blank Cube container for holding output cube\n");
	}
	return (U);
}

static struct GMT_GRID *gmtmex_grid_init (void *API, unsigned int direction, unsigned int module_input, const mxArray *ptr) {
	/* Used to Create an empty Grid container to hold a GMT grid.
 	 * If direction is GMT_IN then we are given a MATLAB grid and can determine its size, etc.
	 * If direction is GMT_OUT then we allocate an empty GMT grid as a destination. */
	unsigned int row, col;
	uint64_t gmt_ij;
	struct GMT_GRID *G = NULL;

	if (direction == GMT_IN) {	/* Dimensions are known from the input pointer */
		unsigned int registration, flag = (module_input) ? GMT_VIA_MODULE_INPUT : 0;
		mxArray *mx_ptr = NULL, *mxGrid = NULL, *mxHdr = NULL;

		if (mxIsEmpty (ptr))
			mexErrMsgTxt ("gmtmex_grid_init: The input that was supposed to contain the Grid, is empty\n");
		if (!mxIsStruct (ptr)) {
			if (!mxIsCell (ptr))
				mexErrMsgTxt ("gmtmex_grid_init: Expected a Grid structure or Cell array for input\n");
			else {		/* Test that we have a {MxN,1x9} cell array */
				if (mxGetM(ptr) != 2 && mxGetN(ptr) != 2)
					mexErrMsgTxt ("gmtmex_grid_init: Cell array must contain two elements\n");
				else {
					mxGrid = mxGetCell(ptr, 0);
					mxHdr  = mxGetCell(ptr, 1);
					if (mxGetM(mxGrid) < 2 || mxGetN(mxGrid) < 2)
						mexErrMsgTxt ("gmtmex_grid_init: First element of grid's cell array must contain a decent matrix\n");
					if (mxGetM(mxHdr) != 1 || mxGetN(mxHdr) != 9)
						mexErrMsgTxt ("gmtmex_grid_init: grid's cell array second element must contain a 1x9 vector\n");
					if (!mxIsSingle(mxGrid) && !mxIsDouble(mxGrid))
						mexErrMsgTxt ("gmtmex_grid_init: grid's cell matrix must be either single or double.\n");
				}
			}
		}

		if (mxIsStruct(ptr)) {	/* Passed a regular MEX Grid structure */
			double *inc = NULL, *range = NULL, *reg = NULL;
			unsigned int pad = (unsigned int)GMT_NOTSET;
			char x_unit[GMT_GRID_VARNAME_LEN80] = { "" }, y_unit[GMT_GRID_VARNAME_LEN80] = { "" },
			     z_unit[GMT_GRID_VARNAME_LEN80] = { "" }, layout[3];
			mx_ptr = mxGetField (ptr, 0, "inc");
			if (mx_ptr == NULL)
				mexErrMsgTxt ("gmtmex_grid_init: Could not find inc array with Grid increments\n");
			inc = mxGetData (mx_ptr);

			mx_ptr = mxGetField (ptr, 0, "range");
			if (mx_ptr == NULL)
				mexErrMsgTxt ("gmtmex_grid_init: Could not find range array for Grid range\n");
			range = mxGetData (mx_ptr);

			mxGrid = mxGetField(ptr, 0, "z");
			if (mxGrid == NULL)
				mexErrMsgTxt ("gmtmex_grid_init: Could not find data array for Grid\n");
			if (!mxIsSingle(mxGrid) && !mxIsDouble(mxGrid))
				mexErrMsgTxt ("gmtmex_grid_init: data array must be either single or double.\n");

			mx_ptr = mxGetField (ptr, 0, "registration");
			if (mx_ptr == NULL)
				mexErrMsgTxt ("gmtmex_grid_init: Could not find registration array for Grid registration\n");
			reg = mxGetData (mx_ptr);
			registration = (unsigned int)lrint(reg[0]);


			mx_ptr = mxGetField(ptr, 0, "pad");
			if (mx_ptr != NULL) {
				double *dpad = mxGetData(mx_ptr);
				pad = (unsigned int)dpad[0];
				if (pad > 2)
					mexPrintf("gmtmex_grid_init:  This pad value (%d) is very probably wrong.\n");
			}

			if ((G = GMT_Create_Data (API, GMT_IS_GRID|flag, GMT_IS_SURFACE, GMT_GRID_ALL,
			                          NULL, range, inc, registration, pad, NULL)) == NULL)
				mexErrMsgTxt ("gmtmex_grid_init: Failure to alloc GMT source matrix for input\n");

			G->header->z_min = range[4];
			G->header->z_max = range[5];

			G->header->registration = registration;

			mx_ptr = mxGetField (ptr, 0, "nodata");
			if (mx_ptr != NULL)
				G->header->nan_value = *(float *)mxGetData (mx_ptr);

			mx_ptr = mxGetField (ptr, 0, "proj4");
			if (mx_ptr != NULL && mxGetN(mx_ptr) > 6) {		/* A true proj4 string will have at least this lenght */
				char *str = malloc(mxGetN(mx_ptr) + 1);
				mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
				G->header->ProjRefPROJ4 = GMT_Duplicate_String (API, str);
				free (str);
			}
			mx_ptr = mxGetField (ptr, 0, "wkt");
			if (mx_ptr != NULL && mxGetN(mx_ptr) > 20) {	/* A true WTT string will have more thna this lenght */
				char *str = malloc(mxGetN(mx_ptr) + 1);
				mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
				G->header->ProjRefWKT = GMT_Duplicate_String (API, str);
				free (str);
			}
			mx_ptr = mxGetField (ptr, 0, "title");
			if (mx_ptr != NULL) {
				char *str = malloc(mxGetN(mx_ptr) + 1);
				mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(G->header->title, str, GMT_GRID_VARNAME_LEN80 - 1);
				free (str);
			}
			mx_ptr = mxGetField (ptr, 0, "command");
			if (mx_ptr != NULL) {
				char *str = malloc(mxGetN(mx_ptr) + 1);
				mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(G->header->command, str, GMT_GRID_COMMAND_LEN320 - 1);
				free (str);
			}
			mx_ptr = mxGetField (ptr, 0, "comment");
			if (mx_ptr != NULL) {
				char *str = malloc(mxGetN(mx_ptr)+2);
				mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(G->header->remark, str, GMT_GRID_REMARK_LEN160 - 1);
				free (str);
			}
			mx_ptr = mxGetField (ptr, 0, "x_unit");
			if (mx_ptr != NULL) {
				mxGetString(mx_ptr, x_unit, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(G->header->x_units, x_unit, GMT_GRID_VARNAME_LEN80 - 1);
			}
			mx_ptr = mxGetField (ptr, 0, "y_unit");
			if (mx_ptr != NULL) {
				mxGetString(mx_ptr, y_unit, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(G->header->y_units, y_unit, GMT_GRID_VARNAME_LEN80 - 1);
			}
			mx_ptr = mxGetField (ptr, 0, "z_unit");
			if (mx_ptr != NULL) {
				mxGetString(mx_ptr, z_unit, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(G->header->z_units, z_unit, GMT_GRID_VARNAME_LEN80 - 1);
			}
			mx_ptr = mxGetField (ptr, 0, "layout");
			if (mx_ptr != NULL) {
				mxGetString(mx_ptr, layout, (mwSize)mxGetN(mx_ptr) + 1);
				strncpy(G->header->mem_layout, layout, 3);
			}
			else
				strncpy(G->header->mem_layout, "TRS", 3);
		}
		else {	/* Passed header and grid separately */
			double *h = mxGetData(mxHdr);
			registration = (unsigned int)lrint(h[6]);
			if ((G = GMT_Create_Data (API, GMT_IS_GRID|flag, GMT_IS_SURFACE, GMT_GRID_ALL,
			                          NULL, h, &h[7], registration, GMT_NOTSET, NULL)) == NULL)
				mexErrMsgTxt ("gmtmex_grid_init: Failure to alloc GMT source matrix for input\n");
			G->header->z_min = h[4];
			G->header->z_max = h[5];
		}

		if (mxIsSingle(mxGrid)) {
			float *f4 = mxGetData(mxGrid);
			if (f4 == NULL)
				mexErrMsgTxt("gmtmex_grid_init: Grid pointer is NULL where it absolutely could not be.");
			for (row = 0; row < G->header->n_rows; row++) {
				for (col = 0; col < G->header->n_columns; col++) {
					gmt_ij = GMT_IJP (G->header, row, col);
					G->data[gmt_ij] = f4[MEXG_IJ(G,row,col)];
				}
			}
		}
		else {
			double *f8 = mxGetData(mxGrid);
			if (f8 == NULL)
				mexErrMsgTxt("gmtmex_grid_init: Grid pointer is NULL where it absolutely could not be.");
			for (row = 0; row < G->header->n_rows; row++) {
				for (col = 0; col < G->header->n_columns; col++) {
					gmt_ij = GMT_IJP (G->header, row, col);
					G->data[gmt_ij] = (float)f8[MEXG_IJ(G,row,col)];
				}
			}
		}
		GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_grid_init: Allocated GMT Grid %lx\n", (long)G);
		GMT_Report (API, GMT_MSG_DEBUG,
		            "gmtmex_grid_init: Registered GMT Grid array %lx via memory reference from MATLAB\n",
		            (long)G->data);
	}
	else {	/* Just allocate an empty container to hold an output grid (signal this by passing 0s and NULLs [mode == GMT_IS_OUTPUT from 5.4]) */
		if ((G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_IS_OUTPUT,
		                          NULL, NULL, NULL, 0, 0, NULL)) == NULL)
			mexErrMsgTxt ("gmtmex_grid_init: Failure to alloc GMT blank grid container for holding output grid\n");
	}
	return (G);
}

static struct GMT_IMAGE *gmtmex_image_init (void *API, unsigned int direction, unsigned int module_input, const mxArray *ptr) {
	/* Used to Create an empty Image container to hold a GMT image.
 	 * If direction is GMT_IN then we are given a MATLAB image and can determine its size, etc.
	 * If direction is GMT_OUT then we allocate an empty GMT image as a destination. */
	struct GMT_IMAGE *I = NULL;
	if (direction == GMT_IN) {	/* Dimensions are known from the input pointer */
		uint64_t dim[3];
		unsigned int flag = (module_input) ? GMT_VIA_MODULE_INPUT : 0, pad = 0;
		char x_unit[GMT_GRID_VARNAME_LEN80] = { "" }, y_unit[GMT_GRID_VARNAME_LEN80] = { "" },
		     z_unit[GMT_GRID_VARNAME_LEN80] = { "" }, layout[4];
		double  *reg = NULL, *inc = NULL, *range = NULL;
		mxArray *mx_ptr = NULL;

		if (mxIsEmpty (ptr))
			mexErrMsgTxt ("gmtmex_image_init: The input that was supposed to contain the Image, is empty\n");

		if (!mxIsStruct (ptr))
			mexErrMsgTxt ("gmtmex_image_init: Expected a Image structure for input\n");

		mx_ptr = mxGetField (ptr, 0, "range");
		if (mx_ptr == NULL)
			mexErrMsgTxt ("gmtmex_image_init: Could not find range array for Image range\n");
		range = mxGetData (mx_ptr);

		mx_ptr = mxGetField (ptr, 0, "inc");
		if (mx_ptr == NULL)
			mexErrMsgTxt ("gmtmex_image_init: Could not find inc array with Image increments\n");
		inc = mxGetData (mx_ptr);

		mx_ptr = mxGetField(ptr, 0, "registration");
		if (mx_ptr == NULL)
			mexErrMsgTxt("gmtmex_image_init: Could not find registration info in Image struct\n");
		reg = mxGetData(mx_ptr);

		mx_ptr = mxGetField(ptr, 0, "pad");
		if (mx_ptr != NULL) {
			double *dpad = mxGetData(mx_ptr);
			pad = (unsigned int)dpad[0];
			if (pad > 2)
				mexPrintf("gmtmex_grid_init:  This pad value (%d) is very probably wrong.\n");
		}

		mx_ptr = mxGetField (ptr, 0, "image");
		if (mx_ptr == NULL)
			mexErrMsgTxt ("gmtmex_image_init: Could not find data array for Image\n");

		if (!mxIsUint8(mx_ptr))
			mexErrMsgTxt("gmtmex_image_init: The only data type supported by now is UInt8, and this image is not.\n");

		dim[0] = gmtmex_getMNK (mx_ptr, 1);	dim[1] = gmtmex_getMNK (mx_ptr, 0);	dim[2] = gmtmex_getMNK (mx_ptr, 2);
		if ((I = GMT_Create_Data (API, GMT_IS_IMAGE|flag, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, dim,
			                      range, inc, (unsigned int)reg[0], pad, NULL)) == NULL)
			mexErrMsgTxt ("gmtmex_image_init: Failure to alloc GMT source image for input\n");

		I->data = (unsigned char *)mxGetData (mx_ptr);				/* Send in the Matlab owned memory. */
		GMT_Set_AllocMode (API, GMT_IS_IMAGE, I);
		//I->alloc_mode = GMT_ALLOC_EXTERNALLY;

/*
		memcpy (I->data, (unsigned char *)mxGetData (mx_ptr), I->header->nm * I->header->n_bands * sizeof (char));
		for (row = 0; row < I->header->n_rows; row++) {
			for (col = 0; col < I->header->n_columns; col++) {
				gmt_ij = GMT_IJP (I->header, row, col);
				I->data [gmt_ij] = f[MEXG_IJ(I,row,col)];
			}
		}
*/
		mx_ptr = mxGetField (ptr, 0, "alpha");
		I->alpha = NULL;
		if (mx_ptr != NULL) {
			if (mxGetNumberOfDimensions(mx_ptr) == 2)
				I->alpha = (unsigned char *)mxGetData (mx_ptr);		/* Send in the Matlab owned memory. */
		}

		I->header->z_min = range[4];
		I->header->z_max = range[5];

		mx_ptr = mxGetField(ptr, 0, "x");
		if (mx_ptr == NULL)
			mexErrMsgTxt("gmtmex_image_init: Could not find x-coords vector for Image\n");
		I->x = mxGetData(mx_ptr);

		mx_ptr = mxGetField(ptr, 0, "y");
		if (mx_ptr == NULL)
			mexErrMsgTxt("gmtmex_image_init: Could not find y-coords vector for Image\n");
		I->y = mxGetData(mx_ptr);

		mx_ptr = mxGetField (ptr, 0, "nodata");
		if (mx_ptr != NULL)
			I->header->nan_value = *(float *)mxGetData (mx_ptr);

		mx_ptr = mxGetField (ptr, 0, "proj4");
		if (mx_ptr != NULL && mxGetN(mx_ptr) > 6) {		/* A true proj4 string will have at least this length */
			char *str = malloc(mxGetN(mx_ptr) + 1);
			mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
			I->header->ProjRefPROJ4 = GMT_Duplicate_String (API, str);
			free (str);
		}
		mx_ptr = mxGetField (ptr, 0, "wkt");
		if (mx_ptr != NULL && mxGetN(mx_ptr) > 20) {	/* A true WTT string will have more than this length */
			char *str = malloc(mxGetN(mx_ptr) + 1);
			mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
			I->header->ProjRefWKT = GMT_Duplicate_String (API, str);
			free (str);
		}

		mx_ptr = mxGetField (ptr, 0, "title");
		if (mx_ptr != NULL) {
			char *str = malloc(mxGetN(mx_ptr) + 1);
			mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
			strncpy(I->header->title, str, GMT_GRID_VARNAME_LEN80 - 1);
			free (str);
		}
		mx_ptr = mxGetField (ptr, 0, "command");
		if (mx_ptr != NULL) {
			char *str = malloc(mxGetN(mx_ptr) + 1);
			mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
			strncpy(I->header->command, str, GMT_GRID_COMMAND_LEN320 - 1);
			free (str);
		}
		mx_ptr = mxGetField (ptr, 0, "comment");
		if (mx_ptr != NULL) {
			char *str = malloc(mxGetN(mx_ptr) + 1);
			mxGetString(mx_ptr, str, (mwSize)mxGetN(mx_ptr) + 1);
			strncpy(I->header->remark, str, GMT_GRID_REMARK_LEN160 - 1);
			free (str);
		}
		mx_ptr = mxGetField (ptr, 0, "x_unit");
		if (mx_ptr != NULL) {
			mxGetString(mx_ptr, x_unit, (mwSize)mxGetN(mx_ptr) + 1);
			strncpy(I->header->x_units, x_unit, GMT_GRID_VARNAME_LEN80 - 1);
		}
		mx_ptr = mxGetField (ptr, 0, "y_unit");
		if (mx_ptr != NULL) {
			mxGetString(mx_ptr, y_unit, (mwSize)mxGetN(mx_ptr) + 1);
			strncpy(I->header->y_units, y_unit, GMT_GRID_VARNAME_LEN80 - 1);
		}
		mx_ptr = mxGetField (ptr, 0, "z_unit");
		if (mx_ptr != NULL) {
			mxGetString(mx_ptr, z_unit, (mwSize)mxGetN(mx_ptr) + 1);
			strncpy(I->header->z_units, z_unit, GMT_GRID_VARNAME_LEN80 - 1);
		}
		mx_ptr = mxGetField (ptr, 0, "layout");
		if (mx_ptr != NULL) {
			mxGetString(mx_ptr, layout, (mwSize)mxGetN(mx_ptr) + 1);
			strncpy(I->header->mem_layout, layout, 4);
		}
		else
			strncpy(I->header->mem_layout, "TCBa", 4);

		I->colormap = NULL;		/* BUT IT SHOULD BE PROPERLY ASSIGNED IF IMAGE IS INDEXED */
		if (dim[2] == 1)
			I->color_interp = "Gray";
		else
			I->color_interp = "Unknown";	/* BUT WE CAN */

		GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_image_init: Allocated GMT Image %lx\n", (long)I);
		GMT_Report (API, GMT_MSG_DEBUG,
		            "gmtmex_image_init: Registered GMT Image array %lx via memory reference from MATLAB\n",
		            (long)I->data);
	}
	else {	/* Just allocate an empty container to hold an output image (signal this by passing 0s and NULLs [mode == GMT_IS_OUTPUT from 5.4]) */
		if ((I = GMT_Create_Data (API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_IS_OUTPUT, NULL, NULL, NULL, 0, 0, NULL)) == NULL)
			mexErrMsgTxt ("gmtmex_image_init: Failure to alloc GMT blank image container for holding output image\n");

		GMT_Set_Default (API, "API_IMAGE_LAYOUT", "TCBa");	/* State how we wish to receive images from GDAL */
	}
	return (I);
}

static void *gmtmex_dataset_init (void *API, unsigned int direction, unsigned int module_input, const mxArray *ptr, unsigned int *actual_family) {
	/* Create containers to hold or receive data tables:
	 * direction == GMT_IN:  Create empty GMT_DATASET container, fill from Mex, and use as GMT input.
	 *	Input from MATLAB may be a MEX data structure, a plain matrix, a cell array of strings or a single string.
	 * direction == GMT_OUT: Create empty GMT_DATASET container, let GMT fill it out, and use for Mex output.
 	 * If direction is GMT_IN then we are given a MATLAB struct and can determine dimension.
	 * If output then we don't know size so we set dimensions to zero. */
	struct GMT_DATASET *D = NULL;

	*actual_family = GMT_IS_DATASET;	/* Default but may change to matrix below */
	if (direction == GMT_IN) {	/* Data given, dimensions are know, create container for GMT */
		uint64_t seg, col, row, start, k, n_headers, dim[4] = {1, 0, 0, 0}, n, m, n_rows;	/* We only return a single table */
		size_t length = 0;
		bool got_single_record = false;
		unsigned int mode;
		char buffer[BUFSIZ] = {""},  *txt = NULL;
		mxArray *mx_ptr = NULL, *mx_ptr_d = NULL, *mx_ptr_t = NULL;
		double *data = NULL;
		struct GMT_DATASEGMENT *S = NULL;

		if (!ptr) mexErrMsgTxt ("gmtmex_dataset_init: Input is empty where it can't be.\n");
		if (mxIsNumeric (ptr)) {	/* Got a MATLAB matrix as input - pass data pointers via MATRIX to save memory */
			struct GMT_MATRIX *M = NULL;
			unsigned int flag = (module_input) ? GMT_VIA_MODULE_INPUT : 0;
			flag |= GMT_VIA_MATRIX;
			*actual_family |= GMT_VIA_MATRIX;
			mxClassID type = mxGetClassID (ptr);	/* Storage type for this matrix */
			dim[DIM_ROW] = mxGetM (ptr);		/* Number of rows */
			dim[DIM_COL] = mxGetN (ptr);		/* Number of columns */
			/* Create matrix container but do not allocate any matrix memory */
			if ((M = GMT_Create_Data (API, GMT_IS_DATASET|flag, GMT_IS_PLP, GMT_CONTAINER_ONLY, dim, NULL, NULL, 0, 0, NULL)) == NULL)
				mexErrMsgTxt ("gmtmex_dataset_init: Failure to alloc GMT source matrix\n");
			GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_dataset_init: Allocated GMT Matrix %lx\n", (long)M);
			switch (type) {	/* Assign ML type pointer to the corresponding GMT matrix union pointer */
				case mxDOUBLE_CLASS: M->type = GMT_DOUBLE; M->data.f8  =             mxGetData (ptr); break;
				case mxSINGLE_CLASS: M->type = GMT_FLOAT;  M->data.f4  =    (float *)mxGetData (ptr); break;
				case mxUINT64_CLASS: M->type = GMT_ULONG;  M->data.ui8 = (uint64_t *)mxGetData (ptr); break;
				case mxINT64_CLASS:  M->type = GMT_LONG;   M->data.si8 =  (int64_t *)mxGetData (ptr); break;
				case mxUINT32_CLASS: M->type = GMT_UINT;   M->data.ui4 = (uint32_t *)mxGetData (ptr); break;
				case mxINT32_CLASS:  M->type = GMT_INT;    M->data.si4 =  (int32_t *)mxGetData (ptr); break;
				case mxUINT16_CLASS: M->type = GMT_USHORT; M->data.ui2 = (uint16_t *)mxGetData (ptr); break;
				case mxINT16_CLASS:  M->type = GMT_SHORT;  M->data.si2 =  (int16_t *)mxGetData (ptr); break;
				case mxUINT8_CLASS:  M->type = GMT_UCHAR;  M->data.uc1 =  (uint8_t *)mxGetData (ptr); break;
				case mxINT8_CLASS:   M->type = GMT_CHAR;   M->data.sc1 =   (int8_t *)mxGetData (ptr); break;
				default:
					mexErrMsgTxt ("gmtmex_dataset_init: Unsupported MATLAB data type in GMT matrix input.");
					break;
			}
			/* Data from MATLAB and Octave(mex) is in col format and data from Octave(oct) is in row format */
#ifdef GMT_OCTOCT
			M->dim = M->n_columns;
#else
			M->dim = M->n_rows;
#endif
			//M->alloc_mode = GMT_ALLOC_EXTERNALLY;	/* Since matrix was allocated by MATLAB/Octave we cannot free it in GMT */
			GMT_Set_AllocMode (API, GMT_IS_MATRIX, M);
			M->shape = MEX_COL_ORDER;		/* Either col or row order, depending on MATLAB/Octave setting in gmtmex.h */
			return (M);
		}
		/* We come here if we did not receive a matrix,  There are three options: */
		/* 1. A dataset MATLAB structure or array of structures.
		 * 2. A Cell array of plain text strings for a text-only file.
		 * 3. A single text string instead of a one-item cell array of strings. */

		if (mxIsStruct (ptr)) {	/* Got the dataset structure */
			dim[GMT_SEG] = mxGetM (ptr);	/* Number of segments */
			if (dim[GMT_SEG] == 0) mexErrMsgTxt ("gmtmex_dataset_init: Input has zero segments where it can't be.\n");
			mx_ptr_d = mxGetField (ptr, 0, "data");	/* Get first segment's data matrix [if available] */
			if (mx_ptr_d && mxIsEmpty(mx_ptr_d)) mx_ptr_d = NULL;		/* Got one but was empty */
			mx_ptr_t = mxGetField (ptr, 0, "text");	/* Get first segment's text matrix [if available] */
			if (mx_ptr_t && mxIsEmpty(mx_ptr_t)) mx_ptr_t = NULL;		/* Got one but was empty */

			if (mx_ptr_d == NULL && mx_ptr_t == NULL)
				mexErrMsgTxt("gmtmex_dataset_init: Both 'data' array and 'text' array are NULL!\n");
			if (mx_ptr_d)
				dim[GMT_COL] = mxGetN (mx_ptr_d);	/* Number of columns */
			if (dim[GMT_COL] == 0 && mx_ptr_t == NULL)
				mexErrMsgTxt ("gmtmex_dataset_init: Input has zero columns where it can't be.\n");

			if (mx_ptr_t)	/* This segment also has a cell array of strings */
				mode = GMT_WITH_STRINGS;
			else
				mode = GMT_NO_STRINGS;

			if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_PLP, mode, dim, NULL, NULL, 0, 0, NULL)) == NULL)
				mexErrMsgTxt ("gmtmex_dataset_init: Failure to alloc GMT destination dataset\n");
			GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_dataset_init: Allocated GMT dataset %lx\n", (long)D);

			for (seg = 0; seg < dim[GMT_SEG]; seg++) {	/* Each incoming structure is a new data segment */
				mx_ptr = mxGetField (ptr, (mwSize)seg, "header");		/* Get pointer to MEX segment header */
				buffer[0] = 0;							/* Reset our temporary text buffer */
				if (mx_ptr && (length = mxGetN (mx_ptr)) != 0)			/* These is a non-empty segment header to keep */
					mxGetString (mx_ptr, buffer, (mwSize)(length+1));
				mx_ptr_d = mxGetField (ptr, (mwSize)seg, "data");		/* Data matrix for this segment */
				if (mx_ptr_d && mxIsEmpty(mx_ptr_d)) mx_ptr_d = NULL;		/* Got one but was empty */
				mx_ptr_t = mxGetField (ptr, (mwSize)seg, "text");		/* text cell array for this segment */
				if (mx_ptr_t && mxIsEmpty(mx_ptr_t)) mx_ptr_t = NULL;		/* Got one but was empty */

				if (mx_ptr_t) {	/* This segment also has a cell array of strings or possibly a single string (if n_rows == 1) */
					got_single_record = false;
					m = mxGetM (mx_ptr_t);	n = mxGetN (mx_ptr_t);
					if (!mxIsCell (mx_ptr_t) && (m == 1 || n == 1)) {
						got_single_record = true;
						m = n = 1;
					}
				}
				else	/* No trailing text */
					m = n = 0;
				dim[GMT_ROW] = (mx_ptr_d == NULL) ? m : mxGetM (mx_ptr_d);	/* Number of rows in matrix (or strings if no data) */
				if ((m == dim[GMT_ROW] && n == 1) || (n == dim[GMT_ROW] && m == 1))
					mode = GMT_WITH_STRINGS;
				else
					mode = GMT_NO_STRINGS;
				/* Allocate a new data segment and hook up to to our single table */
				S = GMT_Alloc_Segment (API, mode, dim[GMT_ROW], dim[GMT_COL], buffer, D->table[0]->segment[seg]);
				if (mx_ptr_d != NULL) data = mxGetData (mx_ptr_d);
				for (col = start = 0; col < S->n_columns; col++, start += S->n_rows) /* Copy the data columns */
					memcpy (S->data[col], &data[start], S->n_rows * sizeof (double));
				if (mode == GMT_WITH_STRINGS) {	/* Add in the trailing strings */
					if (got_single_record) {	/* Only true when we got a single row with a single string instead of a cell array */
						txt = mxArrayToString (mx_ptr_t);
						S->text[0] = GMT_Duplicate_String (API, txt);
					}
					else {	/* Must extract text from the cell array */
						for (row = 0; row < S->n_rows; row++) {
							mx_ptr = mxGetCell (mx_ptr_t, (mwSize)row);
							txt = mxArrayToString (mx_ptr);
							S->text[row] = GMT_Duplicate_String (API, txt);
						}
					}
				}
				D->table[0]->n_records += S->n_rows;	/* Must manually keep track of totals */
				if (seg == 0) {	/* First segment may have table information */
					mx_ptr_t = mxGetField (ptr, (mwSize)seg, "comment");	/* Table headers */
					if (mx_ptr_t && (n_headers = mxGetM (mx_ptr_t)) != 0) {	/* Number of headers found */
						for (k = 0; k < n_headers; k++) {	/* Extract the headers and insert into dataset */
							mx_ptr = mxGetCell (mx_ptr_t, (mwSize)k);
							txt = mxArrayToString (mx_ptr);
							if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_TEXT, txt, D))
								mexErrMsgTxt("gmtmex_dataset_init: Failed to set a dataset header\n");
						}
					}
				}
				if (mode == GMT_WITH_STRINGS) D->type = (D->n_columns) ? GMT_READ_MIXED : GMT_READ_TEXT;
				else D->type = GMT_READ_DATA;
			}
		}
		else if (mxIsCell (ptr)) {	/* Got a cell array of strings and no numerical data */
			uint64_t k2 = 0;
			m = mxGetM (ptr);	n = mxGetN (ptr);
			n_rows = (m > n) ? m : n;	/* Number of items in cell array */
			mode = GMT_WITH_STRINGS;	/* Since that is all we have */
			/* Determine number of segments up front since user may use '>' to indicate segment header */
			for (k = 0; k < n_rows; k++) {
				mx_ptr = mxGetCell (ptr, (mwSize)k);
				txt = mxArrayToString (mx_ptr);
				if (txt[0] == '>') dim[GMT_SEG]++;	/* Found start of a new segment */
			}
			if (dim[GMT_SEG] == 0) dim[GMT_SEG] = 1;	/* No segment headers given a single segment */
			if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_WITH_STRINGS, dim, NULL, NULL, 0, 0, NULL)) == NULL)
				mexErrMsgTxt ("gmtmex_dataset_init: Failure to alloc GMT destination dataset\n");
			GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_dataset_init: Allocated GMT dataset %lx\n", (long)D);
			k = seg = 0;
			while (k < n_rows) {	/* Examine the input records and look for segment breaks */
				mx_ptr = mxGetCell (ptr, (mwSize)k);
				txt = mxArrayToString (mx_ptr);
				buffer[0] = '\0';
				if (txt[0] == '>' || (k == 0 && txt[0] != '>')) {	/* Found start of a new (or first and only) segment */
					if (txt[0] == '>') strcpy (buffer, txt), k++;	/* Segment header */
					k2 = k;	/* k and k2 initially point to the first row of the current segment */
					dim[GMT_ROW] = 0;	/* Have no rows so far */
					while (k2 < n_rows && dim[GMT_ROW] == 0) {	/* While not reached end of current segment */
						mx_ptr = mxGetCell (ptr, (mwSize)k2);
						txt = mxArrayToString (mx_ptr);
						if (txt[0] == '>')	/* Got next segment header, must end current segment */
							dim[GMT_ROW] = k2 - k;
						else	/* Keep going */
							k2++;
					}
					if (dim[GMT_ROW] == 0) dim[GMT_ROW] = k2 - k;	/* Happens for last segment when there is no "next" segment header */
				}
				/* Now we have the length of this segment */
				S = GMT_Alloc_Segment (API, GMT_WITH_STRINGS, dim[GMT_ROW], 0, buffer, D->table[0]->segment[seg]);
				for (row = 0; row < S->n_rows; row++) {	/* Hook up the string records */
					mx_ptr = mxGetCell (ptr, (mwSize)(k+row));	/* k is the offset to 1st record of current segment in input cell array */
					txt = mxArrayToString (mx_ptr);
					S->text[row] = GMT_Duplicate_String (API, txt);
				}
				D->table[0]->n_records += S->n_rows;	/* Must manually keep track of total records */
				seg++;	/* Got ourselves a new segment, increment counter */
				/* Move to next unused record or we have reached the end */
				k = k2;
			}
			D->type = GMT_READ_TEXT;
		}
		else if (mxIsChar (ptr)) {	/* Got a single string only */
			dim[GMT_ROW] = dim[GMT_SEG] = 1;	/* Put string into a single segment */
			mode = GMT_WITH_STRINGS;		/* Since that is all we have */
			if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_TEXT, mode, dim, NULL, NULL, 0, 0, NULL)) == NULL)
				mexErrMsgTxt ("gmtmex_dataset_init: Failure to alloc GMT destination dataset\n");
			GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_dataset_init: Allocated GMT dataset %lx\n", (long)D);
			S = D->table[0]->segment[0];	/* The lone segment */
			txt = mxArrayToString (ptr);	/* The lone string */
			S->text[0] = GMT_Duplicate_String (API, txt);
			D->type = GMT_READ_TEXT;
		}
		else
			mexErrMsgTxt ("gmtmex_dataset_init: Expected a data structure, cell array with strings, or a single string for input\n");
		D->n_records = D->table[0]->n_records;
	}
	else {	/* Here we set up an empty container to receive data from GMT (signal this by passing 0s and NULLs) */
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_IS_OUTPUT, NULL, NULL, NULL, 0, 0, NULL)) == NULL)
			mexErrMsgTxt ("gmtmex_dataset_init: Failure to alloc GMT source dataset\n");
		GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_dataset_init: Allocated GMT Dataset %lx\n", (long)D);
	}
	return (D);
}

static struct GMT_PALETTE *gmtmex_palette_init (void *API, unsigned int direction, unsigned int module_input, const mxArray *ptr) {
	/* Used to create an empty CPT container to hold a GMT Color Palette.
 	 * If direction is GMT_IN then we are given a MATLAB CPT struct and can determine its size, etc.
	 * If direction is GMT_OUT then we allocate an empty GMT CPT as a destination. */
	struct GMT_PALETTE *P = NULL;
	if (direction == GMT_IN) {	/* Dimensions are known from the input pointer */
		unsigned int k, j, one = 1, n_headers, *depth = NULL;
		uint64_t dim[2] = {0, 0};
		unsigned int flag = (module_input) ? GMT_VIA_MODULE_INPUT : 0;
		char model[8] = {""};
		mxArray *mx_ptr[N_MEX_FIELDNAMES_CPT];
		double *colormap = NULL, *range = NULL, *minmax = NULL, *alpha = NULL, *bfn = NULL, *hinge = NULL, *cpt = NULL, *cyclic = NULL;

		if (mxIsEmpty (ptr))
			mexErrMsgTxt ("gmtmex_palette_init: The input that was supposed to contain the CPT, is empty\n");
		if (!mxIsStruct (ptr))
			mexErrMsgTxt ("gmtmex_palette_init: Expected a CPT structure for input\n");
		for (k = 0; k < N_MEX_FIELDNAMES_CPT; k++) {
			if ((mx_ptr[k] = mxGetField (ptr, 0, gmtmex_fieldname_cpt[k])) == NULL)
				gmtmex_quit_if_missing ("gmtmex_palette_init", gmtmex_fieldname_cpt[k]);
		}

		dim[0] = mxGetM (mx_ptr[0]);	/* Number of rows in colormap */
		if (dim[0] < 1)
			mexErrMsgTxt ("gmtmex_palette_init: Colormap array has no CPT values\n");
		colormap = mxGetData (mx_ptr[0]);
		alpha    = mxGetData (mx_ptr[1]);
		range    = mxGetData (mx_ptr[2]);
		minmax   = mxGetData (mx_ptr[3]);
		bfn      = mxGetData (mx_ptr[4]);
		depth    = mxGetData (mx_ptr[5]);
		hinge    = mxGetData (mx_ptr[6]);
		cpt      = mxGetData (mx_ptr[7]);
		cyclic   = mxGetData (mx_ptr[9]);

		/* Disable unused-but-set-variable warnings */
		(void)(minmax);
		(void)(colormap);

		dim[1] = mxGetM (mx_ptr[2]);	/* Length of range array */
		if (dim[0] > dim[1]) {	/* This only happens when we have a continuous color table */
			dim[1] = dim[0];    /* Actual length of colormap array */
			dim[0]--;           /* Number of CPT slices */
		}
		else	/* Discrete, so the one offset needs to be zero */
			one = 0;

		if ((P = GMT_Create_Data (API, GMT_IS_PALETTE|flag, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
			mexErrMsgTxt ("gmtmex_palette_init: Failure to alloc GMT source CPT for input\n");

		if ((n_headers = (unsigned int)mxGetM (mx_ptr[10])) != 0) {	/* Number of headers found */
			char *txt = NULL;
			mxArray *ptr = NULL;
			for (k = 0; k < n_headers; k++) {
				ptr = mxGetCell (mx_ptr[10], (mwSize)k);
				txt = mxArrayToString (ptr);
				if (GMT_Set_Comment (API, GMT_IS_PALETTE, GMT_COMMENT_IS_TEXT, txt, P))
					mexErrMsgTxt("gmtmex_palette_init: Failed to set a CPT header\n");
			}
		}
		for (j = 0; j < 3; j++) {	/* Do the bfn first */
			for (k = 0; k < 3; k++)
				P->bfn[j].rgb[k] = bfn[j+k*3];
		}
		for (j = 0; j < P->n_colors; j++) {	/* OK to access j+1'th element since length of colormap is P->n_colors+1 */
			for (k = 0; k < 3; k++) {
				P->data[j].rgb_low[k]  = cpt[j+k*dim[0]];
				P->data[j].rgb_high[k] = cpt[j+(k+3)*dim[0]];
			}
			P->data[j].rgb_low[3]  = alpha[j];
			P->data[j].rgb_high[3] = alpha[j+one];
			P->data[j].z_low  = range[j];
			P->data[j].z_high = range[j+P->n_colors];
			P->data[j].annot = 3;	/* Enforce annotations for now */
		}
		P->is_continuous = one;
		P->is_bw = P->is_gray = 0;
		P->is_wrapping = (unsigned int)rint (cyclic[0]);
		if (depth[0] == 1)
			P->is_bw = 1;
		else if (depth[0] == 8)
			P->is_gray = 1;
		GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_palette_init: Allocated GMT CPT %lx\n", (long)P);
		if (!mxIsNaN (hinge[0])) {
			P->has_hinge = 1;
			P->mode |= GMT_CPT_HINGED;
			P->hinge = hinge[0];
		}
		mxGetString (mx_ptr[8], model, (mwSize)mxGetN(mx_ptr[8])+1);
		if (!strncmp (model, "hsv", 3U))
			P->model = GMT_HSV;
		else if (!strncmp (model, "cmyk", 4U))
			P->model = GMT_CMYK;
		else
			P->model = GMT_RGB;
	}
	else {	/* Just allocate an empty container to hold an output grid (signal this by passing 0s and NULLs [mode == GMT_IS_OUTPUT from 5.4]) */
		if ((P = GMT_Create_Data (API, GMT_IS_PALETTE, GMT_IS_NONE, GMT_IS_OUTPUT, NULL, NULL, NULL, 0, 0, NULL)) == NULL)
			mexErrMsgTxt ("gmtmex_palette_init: Failure to alloc GMT blank CPT container for holding output CPT\n");
	}
	return (P);
}

static struct GMT_POSTSCRIPT *gmtmex_ps_init (void *API, unsigned int direction, unsigned int module_input, const mxArray *ptr) {
	/* Used to Create an empty POSTSCRIPT container to hold a GMT POSTSCRIPT object.
 	 * If direction is GMT_IN then we are given a MATLAB structure with known sizes.
	 * If direction is GMT_OUT then we allocate an empty GMT POSTSCRIPT as a destination. */
	struct GMT_POSTSCRIPT *P = NULL;
	if (direction == GMT_IN) {	/* Dimensions are known from the MATLAB input pointer */
		uint64_t dim[1] = {0}, *length = NULL;
		unsigned int k, n_headers, *mode = NULL, flag = (module_input) ? GMT_VIA_MODULE_INPUT : 0;
		mxArray *mx_ptr[N_MEX_FIELDNAMES_PS];
		char *PS = NULL;

		if (mxIsEmpty (ptr))
			mexErrMsgTxt ("gmtmex_ps_init: The input that was supposed to contain the PostScript structure is empty\n");
		if (!mxIsStruct (ptr))
			mexErrMsgTxt ("gmtmex_ps_init: Expected a MATLAB PostScript structure for input\n");
		for (k = 0; k < N_MEX_FIELDNAMES_PS; k++) {
			if ((mx_ptr[k] = mxGetField (ptr, 0, gmtmex_fieldname_ps[k])) == NULL)
				gmtmex_quit_if_missing ("gmtmex_ps_init", gmtmex_fieldname_ps[k]);
		}

		length = mxGetData (mx_ptr[1]);
		if (length[0] == 0)
			mexErrMsgTxt ("gmtmex_ps_init: Dimension of PostScript given as zero\n");
		PS = malloc (mxGetN(mx_ptr[0])+1);
		mxGetString (mx_ptr[0], PS, (mwSize)mxGetN(mx_ptr[0]));
		mode = mxGetData (mx_ptr[2]);
		/* Passing dim[0] = 0 since we dont want any allocation of a PS string */
		if ((P = GMT_Create_Data (API, GMT_IS_POSTSCRIPT|flag, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
			mexErrMsgTxt ("gmtmex_ps_init: Failure to alloc GMT POSTSCRIPT source for input\n");
		P->data = PS;	/* PostScript string instead is coming from MATLAB */
		GMT_Set_AllocMode (API, GMT_IS_POSTSCRIPT, P);
		//P->alloc_mode = GMT_ALLOC_EXTERNALLY;	/* Hence we are not allowed to free it */
		P->n_bytes = length[0];	/* Length of the actual PS string */
		//P->n_alloc = 0;		/* But nothing was actually allocated here - just passing pointer from MATLAB */
		P->mode = mode[0];	/* Inherit the mode */
		if ((n_headers = (unsigned int)mxGetM (mx_ptr[3])) != 0) {	/* Number of headers found */
			char *txt = NULL;
			mxArray *ptr = NULL;
			for (k = 0; k < n_headers; k++) {
				ptr = mxGetCell (mx_ptr[3], (mwSize)k);
				txt = mxArrayToString (ptr);
				if (GMT_Set_Comment (API, GMT_IS_POSTSCRIPT, GMT_COMMENT_IS_TEXT, txt, P))
					mexErrMsgTxt("gmtmex_ps_init: Failed to set a PostScript header\n");
			}
		}
		GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_ps_init: Allocated GMT POSTSCRIPT %lx\n", (long)P);
	}
	else {	/* Just allocate an empty container to hold an output PS object (signal this by passing 0s and NULLs [mode == GMT_IS_OUTPUT from 5.4]) */
		if ((P = GMT_Create_Data (API, GMT_IS_POSTSCRIPT, GMT_IS_NONE, GMT_IS_OUTPUT, NULL, NULL, NULL, 0, 0, NULL)) == NULL)
			mexErrMsgTxt ("gmtmex_ps_init: Failure to alloc GMT POSTSCRIPT container for holding output PostScript\n");
	}
	return (P);
}

static char gmtmex_objecttype (const mxArray *ptr) {
	/* Determine what we are returning so gmt write can pass the correct -T? flag */
	mxArray *mx_ptr = NULL;
	if (mxIsEmpty (ptr))
		mexErrMsgTxt ("gmtmex_objecttype: Pointer is empty\n");
	if (mxIsStruct (ptr)) {	/* This means either a cube, dataset, grid, image, cpt, or PS, so must check for fields */
		mx_ptr = mxGetField (ptr, 0, "v_unit");
		if (mx_ptr) return 'u';
		mx_ptr = mxGetField (ptr, 0, "data");
		if (mx_ptr) return 'd';
		mx_ptr = mxGetField (ptr, 0, "postscript");
		if (mx_ptr) return 'p';
		mx_ptr = mxGetField (ptr, 0, "hinge");
		if (mx_ptr) return 'c';
		mx_ptr = mxGetField (ptr, 0, "image");
		if (mx_ptr) return 'i';
		mx_ptr = mxGetField (ptr, 0, "z");
		if (mx_ptr) return 'g';
		mexErrMsgTxt ("gmtmex_objecttype: Could not recognize the structure\n");
	}
	else if (mxIsCell (ptr))	/* This is a dataset with text only */
		return 'd';
	else	/* We have to assume it is a numerical matrix */
		return 'd';
	return '-';	/* Can never get here you would think */
}

static void gmtmex_Set_Object (void *API, struct GMT_RESOURCE *X, const mxArray *ptr) {
	/* Create the GMT container and hook onto resource array as X->object */
	unsigned int module_input = (X->option->option == GMT_OPT_INFILE), actual_family = X->family;

	switch (X->family) {
		case GMT_IS_CUBE:	/* Get a cube from Matlab or a dummy one to hold GMT output */
			X->object = gmtmex_cube_init (API, X->direction, module_input, ptr);
			GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_Set_Object: Got Cube\n");
			break;
		case GMT_IS_GRID:	/* Get a grid from Matlab or a dummy one to hold GMT output */
			X->object = gmtmex_grid_init (API, X->direction, module_input, ptr);
			GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_Set_Object: Got Grid\n");
			break;
		case GMT_IS_IMAGE:	/* Get an image from Matlab or a dummy one to hold GMT output */
			X->object = gmtmex_image_init (API, X->direction, module_input, ptr);
			GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_Set_Object: Got Image\n");
			break;
		case GMT_IS_DATASET:	/* Get a dataset from Matlab or a dummy one to hold GMT output */
			/* Because a GMT_DATASET may appears as a GMT_MATRIX or GMT_VECTOR we need the actual_family to open the virtual file later */
			X->object = gmtmex_dataset_init (API, X->direction, module_input, ptr, &actual_family);
			break;
		case GMT_IS_PALETTE:	/* Get a palette from Matlab or a dummy one to hold GMT output */
			X->object = gmtmex_palette_init (API, X->direction, module_input, ptr);
			GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_Set_Object: Got CPT\n");
			break;
		case GMT_IS_POSTSCRIPT:	/* Get a PostScript struct from Matlab or a dummy one to hold GMT output */
			X->object = gmtmex_ps_init (API, X->direction, module_input, ptr);
			GMT_Report (API, GMT_MSG_DEBUG, "gmtmex_Set_Object: Got POSTSCRIPT\n");
			break;
		default:
			GMT_Report (API, GMT_MSG_NORMAL, "gmtmex_Set_Object: Bad data type (%d)\n", X->family);
			break;
	}
	if (X->object == NULL)
		mexErrMsgTxt("GMT: Failure to register the resource\n");
	if (GMT_Open_VirtualFile (API, actual_family, X->geometry, X->direction|GMT_IS_REFERENCE, X->object, X->name) != GMT_NOERROR) 	/* Make filename with embedded object ID */
		mexErrMsgTxt ("GMT: Failure to open virtual file\n");
	if (GMT_Expand_Option (API, X->option, X->name) != GMT_NOERROR)	/* Replace ? in argument with name */
		mexErrMsgTxt ("GMT: Failure to expand filename marker (?)\n");
}

static void *gmtmex_Get_Object (void *API, struct GMT_RESOURCE *X) {
	mxArray *ptr = NULL;
	/* In line-by-line modules it is possible no output is produced, hence we make an exception for DATASET: */
	if ((X->object = GMT_Read_VirtualFile (API, X->name)) == NULL && X->family != GMT_IS_DATASET)
		mexErrMsgTxt ("GMT: Error reading virtual file from GMT\n");
	switch (X->family) {	/* Determine what container we got */
		case GMT_IS_CUBE:	/* A GMT cube; make it the pos'th output item */
			ptr = gmtmex_get_cube (API, X->object);
			break;
		case GMT_IS_GRID:	/* A GMT grid; make it the pos'th output item */
			ptr = gmtmex_get_grid (API, X->object);
			break;
		case GMT_IS_DATASET:	/* A GMT table; make it a data structure and the pos'th output item */
			ptr = gmtmex_get_dataset (API, X->object);
			break;
		case GMT_IS_PALETTE:	/* A GMT CPT; make it a colormap and the pos'th output item  */
			ptr = gmtmex_get_palette (API, X->object);
			break;
		case GMT_IS_IMAGE:	/* A GMT Image; make it the pos'th output item  */
			ptr = gmtmex_get_image (API, X->object);
			break;
		case GMT_IS_POSTSCRIPT:		/* A GMT PostScript string; make it the pos'th output item  */
			ptr = gmtmex_get_postscript (API, X->object);
#if 0
			{
			char cmd[32] = {""};
			strcpy(cmd, name);		strcat(cmd, " -A -Tf");
			GMT_Call_Module(API, "psconvert", GMT_MODULE_CMD, cmd);
			}
#endif
			break;
		default:
			mexErrMsgTxt ("GMT: Internal Error - unsupported data type\n");
			break;
	}
	return ptr;
}

#if GMT_MAJOR_VERSION == 6 && GMT_MINOR_VERSION > 1
extern int gmt_get_V (char arg);	/* Temporary here to allow full debug messaging */
#else
extern int GMT_get_V (char arg);	/* Temporary here to allow full debug messaging */
#define gmt_get_V GMT_get_V	/* Old name */
#endif

#ifndef SINGLE_SESSION
/* Being declared external we can access it between MEX calls */
static uintptr_t *pPersistent;    /* To store API address back and forth within a single MATLAB session */

/* Here is the exit function, which gets run when the MEX-file is
   cleared and when the user exits MATLAB. The mexAtExit function
   should always be declared as static. */
static void force_Destroy_Session (void) {
	void *API = (void *)pPersistent[0];	/* Get the GMT API pointer */
	if (API != NULL) {		/* Otherwise just silently ignore this call */
		if (GMT_Destroy_Session (API)) mexErrMsgTxt ("Failure to destroy GMT session\n");
		*pPersistent = 0;	/* Wipe the persistent memory */
	}
}
#endif

static void usage (int nlhs, int nrhs) {
	/* Basic usage message */
	if (nrhs == 0) {	/* No arguments at all results in the GMT banner message */
		mexPrintf("\nGMT - The Generic Mapping Tools, %s API, Version %d.%d.%d\n",
		          MEX_PROG, GMTMEX_MAJOR_VERSION, GMTMEX_MINOR_VERSION, GMTMEX_PATCH_VERSION);
		mexPrintf("Copyright 1991-2021 Paul Wessel, Walter H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe\n\n");
		mexPrintf("This program comes with NO WARRANTY, to the extent permitted by law.\n");
		mexPrintf("You may redistribute copies of this program under the terms of the\n");
		mexPrintf("GNU Lesser General Public License.\n");
		mexPrintf("For more information about these matters, see the file named LICENSE.TXT.\n");
		mexPrintf("For a brief description of GMT modules, type gmt ('help')\n\n");
	}
	else {
		mexPrintf("Usage is:\n\tgmt ('module_name', 'options'[, <matlab arrays>]); %% Run a GMT module\n");
		if (nlhs != 0)
			mexErrMsgTxt ("But meanwhile you already made an error by asking help and an output.\n");
	}
}

static void *Initiate_Session (unsigned int verbose) {
	/* Initialize the GMT Session and store the API pointer in a persistent variable */
	void *API = NULL;
	/* Initializing new GMT session with a MATLAB-acceptable replacement for the printf function */
	/* For debugging with verbose we pass the specified verbose shifted by 10 bits - this is decoded in API */
	if ((API = GMT_Create_Session (MEX_PROG, 2U, (verbose << 10) + GMT_SESSION_NOEXIT + GMT_SESSION_EXTERNAL +
	                               GMT_SESSION_COLMAJOR, gmtmex_print_func)) == NULL)
		mexErrMsgTxt ("GMT: Failure to create new GMT session\n");

#ifndef SINGLE_SESSION
	if (!pPersistent) pPersistent = mxMalloc(sizeof(uintptr_t));
	pPersistent[0] = (uintptr_t)(API);
	mexMakeMemoryPersistent (pPersistent);
#endif
	return (API);
}

static void *alloc_default_plhs (void *API, struct GMT_RESOURCE *X) {
	/* Allocate a default plhs when it was not stated in command line. That is, mimic the Matlab behavior
	   when we do for example (i.e. no lhs):  sqrt([4 9])
	*/
	void *ptr = NULL;
	gmt_M_unused (API);

	switch (X->family) {
		case GMT_IS_CUBE:
			ptr = (void *)mxCreateStructMatrix (0, 0, N_MEX_FIELDNAMES_CUBE, gmtmex_fieldname_cube);
			break;
		case GMT_IS_GRID:
			ptr = (void *)mxCreateStructMatrix (0, 0, N_MEX_FIELDNAMES_GRID, gmtmex_fieldname_grid);
			break;
		case GMT_IS_IMAGE:
			ptr = (void *)mxCreateStructMatrix (0, 0, N_MEX_FIELDNAMES_IMAGE, gmtmex_fieldname_image);
			break;
		case GMT_IS_DATASET:
			ptr = (void *)mxCreateStructMatrix (0, 0, N_MEX_FIELDNAMES_DATASET, gmtmex_fieldname_dataset);
			break;
		case GMT_IS_PALETTE:
			ptr = (void *)mxCreateStructMatrix (0, 0, N_MEX_FIELDNAMES_CPT, gmtmex_fieldname_cpt);
			break;
		case GMT_IS_POSTSCRIPT:
			ptr = (void *)mxCreateStructMatrix (0, 0, N_MEX_FIELDNAMES_PS, gmtmex_fieldname_ps);
			break;
		default:
			break;
	}
	return ptr;
}

/* This is the function that is called when we type gmt in MATLAB/Octave */
void GMT_mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	int status = 0;                 /* Status code from GMT API */
	int n_in_objects = 0;           /* Number of input objects passed to module */
	unsigned int first = 0;         /* Array ID of first command argument (not 0 when API-ID is first) */
	unsigned int verbose = 0;       /* Default verbose setting */
	unsigned int n_items = 0, pos = 0; /* Number of MATLAB arguments (left and right) */
	size_t str_length = 0, k = 0;   /* Misc. counters */
	void *API = NULL;               /* GMT API control structure */
	struct GMT_OPTION *options = NULL; /* Linked list of module options */
	struct GMT_RESOURCE *X = NULL;  /* Array of information about MATLAB args */
	char *cmd = NULL;               /* Pointer used to get the user's MATLAB command */
	char *gtxt = NULL;              /* For debug printing of revised command */
	char *opt_args = NULL;          /* Pointer to the user's module options */
	char module[MODULE_LEN] = {""}; /* Name of GMT module to call */
	char opt_buffer[BUFSIZ] = {""}; /* Local copy of command line options */
	void *ptr = NULL;
#ifndef SINGLE_SESSION
	uintptr_t *pti = NULL;          /* To locally store the API address */
#endif

	/* -1. Check that the GMT library is of a suitable version for this GMTMEX version */

	if (GMT_MAJOR_VERSION > GMTMEX_GMT_MAJOR_VERSION) {	/* This may not work if, for instance, we want >= 6.1.x but is using GMT 7.0.0 */
		char message[128] = {""};
		sprintf (message, "Warning: Your GMT version (%d.%d.%d) may be too new to work with GMTMEX %d.%d.%d.\n",
			GMT_MAJOR_VERSION, GMT_MINOR_VERSION, GMT_RELEASE_VERSION, GMTMEX_GMT_MAJOR_VERSION, GMTMEX_GMT_MINOR_VERSION, GMTMEX_GMT_PATCH_VERSION);
		mexPrintf (message);
	}
	else if (GMT_MAJOR_VERSION < GMTMEX_GMT_MAJOR_VERSION || GMT_MINOR_VERSION < GMTMEX_GMT_MINOR_VERSION || (GMT_MINOR_VERSION == GMTMEX_GMT_MINOR_VERSION && GMT_RELEASE_VERSION < GMTMEX_GMT_PATCH_VERSION)) {
		char message[128] = {""};
		sprintf (message, "Error: The GMT shared library must be at least version %d.%d.%d but you have %d.%d.%d.\n",
			GMTMEX_GMT_MAJOR_VERSION, GMTMEX_GMT_MINOR_VERSION, GMTMEX_GMT_PATCH_VERSION,
			GMT_MAJOR_VERSION, GMT_MINOR_VERSION, GMT_RELEASE_VERSION);
		mexErrMsgTxt (message);
	}

	/* 0. No arguments at all results in the GMT banner message */
	if (nrhs == 0) {
		usage (nlhs, nrhs);
		return;
	}

	/* 1. Check for the special commands create and help */

	if (nrhs == 1) {	/* This may be create or help */
		cmd = mxArrayToString (prhs[0]);
		if (!cmd) mexErrMsgTxt("GMT: First input argument must be a string. Maybe a composition of a string and a cell array?\n");
		if (!strncmp (cmd, "help", 4U) || !strncmp (cmd, "--help", 6U)) {
			usage (nlhs, 1);
			return;
		}
#ifndef SINGLE_SESSION
		if (!strncmp (cmd, "create", 6U)) {	/* Asked to create a new GMT session */
			if (nlhs > 1)	/* Asked for too much output, only 1 or 0 is allowed */
				mexErrMsgTxt ("GMT: Usage: gmt ('create') or API = gmt ('create');\n");
			if (pPersistent)                        /* See if have a GMT API pointer */
				API = (void *)pPersistent[0];
			if (API != NULL) {                      /* If another session still exists */
				GMT_Report (API, GMT_MSG_VERBOSE,
				            "GMT: A previous GMT session is still active. Ignoring your 'create' request.\n");
				if (nlhs) /* Return nothing */
					plhs[0] = mxCreateNumericMatrix (1, 0, mxUINT64_CLASS, mxREAL);
				return;
			}
			if ((gtxt = strstr (cmd, "-V")) != NULL) verbose = gmt_get_V (gtxt[2]);
			API = Initiate_Session (verbose);	/* Initializing a new GMT session */

			if (nlhs) {	/* Return the API address as an integer (nlhs == 1 here) )*/
				plhs[0] = mxCreateNumericMatrix (1, 1, mxUINT64_CLASS, mxREAL);
				pti = mxGetData(plhs[0]);
				*pti = *pPersistent;
			}

			mexAtExit(force_Destroy_Session);	/* Register an exit function. */
			return;
		}

		/* OK, neither create nor help, must be a single command with no arguments nor the API. So get it: */
		if (!pPersistent || (API = (void *)pPersistent[0]) == NULL) {	/* No session yet, create one under the hood */
			API = Initiate_Session(verbose);    /* Initializing a new GMT session */
			mexAtExit(force_Destroy_Session);   /* Register an exit function. */
		}
		else
			API = (void *)pPersistent[0];       /* Get the GMT API pointer */
		if (API == NULL) mexErrMsgTxt ("GMT: This GMT5 session has is corrupted. Better to start from scratch.\n");
	}
	else if (mxIsScalar_(prhs[0]) && mxIsUint64(prhs[0])) {
		/* Here, nrhs > 1 . If first arg is a scalar int, we assume it is the API memory address */
		pti = (uintptr_t *)mxGetData(prhs[0]);
		API = (void *)pti[0];	/* Get the GMT API pointer */
		first = 1;		/* Commandline args start at prhs[1] since prhs[0] had the API id argument */
	}
	else {		/* We still don't have the API, so we must get it from the past or initiate a new session */
		if (!pPersistent || (API = (void *)pPersistent[0]) == NULL) {
			API = Initiate_Session (verbose);	/* Initializing new GMT session */
			mexAtExit(force_Destroy_Session);	/* Register an exit function. */
		}
#endif
	}

#ifdef SINGLE_SESSION
	/* Initiate a new session */
	API = Initiate_Session (verbose);	/* Initializing new GMT session */
#endif

	if (!cmd) {	/* First argument is the command string, e.g., 'blockmean -R0/5/0/5 -I1' or just 'destroy' */
		cmd = mxArrayToString(prhs[first]);
		if (!cmd) mexErrMsgTxt("GMT: First input argument must be a string but is probably a cell array of strings.\n");
	}

	if (!strncmp (cmd, "destroy", 7U)) {	/* Destroy the session */
#ifndef SINGLE_SESSION
		if (nlhs != 0)
			mexErrMsgTxt ("GMT: Usage is gmt ('destroy');\n");

		if (GMT_Destroy_Options (API, &options)) mexErrMsgTxt ("GMT: Failure to destroy GMT5 options\n");
		if (GMT_Destroy_Session (API)) mexErrMsgTxt ("GMT: Failure to destroy GMT5 session\n");
		*pPersistent = 0;	/* Wipe the persistent memory */
#endif
		return;
	}

	/* 2. Get module name and separate out args */

	/* Here we have a GMT module call. The documented use is to give the module name separately from
	 * the module options, but users may forget and combine the two.  So we check both cases. */

	n_in_objects = nrhs - 1;
	str_length = strlen (cmd);				/* Length of module (or command) argument */
	for (k = 0; k < str_length && cmd[k] != ' '; k++);	/* Determine first space in command */

	if (k == str_length) {	/* Case 2a): No spaces found: User gave 'module' separately from 'options' */
		strcpy (module, cmd);				/* Isolate the module name in this string */
		if (nrhs > 1 && mxIsChar (prhs[first+1])) {	/* Got option string */
			first++;	/* Since we have a 2nd string to skip now */
			opt_args = mxArrayToString (prhs[first]);
			n_in_objects--;
		}
		/* Else we got no options, just input objects */
	}
	else {	/* Case b2. Get mex arguments, if any, and extract the GMT module name */
		if (k >= MODULE_LEN)
			mexErrMsgTxt ("GMT: Module name in command is too long\n");
		strncpy (module, cmd, k);	/* Isolate the module name in this string */

		while (cmd[k] == ' ') k++;	/* Skip any spaces between module name and start of options */
		if (cmd[k]) opt_args = &cmd[k];
	}


	/* See if info about installation is required */
	if (!strcmp(module, "gmt")) {
		char t[256] = {""};
		if (!opt_args) {
			mexPrintf("Warning: calling the 'gmt' program by itself does nothing here.\n");
			return;
		}
		if (!strcmp(opt_args, "--show-bindir")) 	/* Show the directory that contains the 'gmt' executable */
			GMT_Get_Default (API, "BINDIR", t);
		else if (!strcmp(opt_args, "--show-sharedir"))	/* Show share directory */
			GMT_Get_Default (API, "SHAREDIR", t);
		else if (!strcmp(opt_args, "--show-datadir"))	/* Show the data directory */
			GMT_Get_Default (API, "DATADIR", t);
		else if (!strcmp(opt_args, "--show-plugindir"))	/* Show the plugin directory */
			GMT_Get_Default (API, "PLUGINDIR", t);
		else if (!strcmp(opt_args, "--show-cores"))	/* Show number of cores */
			GMT_Get_Default (API, "CORES", t);

		if (t[0] != '\0') {
			if (nlhs)
				plhs[0] = mxCreateString (t);
			else
				mexPrintf ("%s\n", t);
		}
		else
			mexPrintf ("Warning: called the 'gmt' program with unknown option.\n");
		return;
	}

	/* Make sure this is a valid module */
	if ((status = GMT_Call_Module (API, module, GMT_MODULE_EXIST, NULL)) != GMT_NOERROR) 	/* No, not found */
		mexErrMsgTxt ("GMT: No module by that name was found.\n");

	/* Below here we may actually wish to add options to the opt_args, but it is a pointer.  So we duplicate to
	 * another string with enough space. */

	if (opt_args) strcpy (opt_buffer, opt_args);	/* opt_buffer has lots of space for additions */
	/* 2+ Add -F to psconvert if user requested a return image but did not explicitly give -F */
	if (!strncmp (module, "psconvert", 9U) && nlhs == 1 && (!opt_args || !strstr ("-F", opt_args))) {	/* OK, add -F */
		if (opt_args)
			strcat (opt_buffer, " -F");
		else
			strcpy (opt_buffer, "-F");
	}

	/* 2++ If gmtwrite then add -T? with correct object type */
	if (strstr(module, "write") && opt_args && !strstr(opt_args, "-T") && n_in_objects == 1) {	/* Add type for writing to disk */
		char targ[5] = {" -T?"};
		targ[3] = gmtmex_objecttype (prhs[nrhs-1]);
		strcat (opt_buffer, targ);
	}
	/* 2+++ If gmtread -Ti then temporarily set pad to 0 since we don't want padding in image arrays */
	else if (strstr(module, "read") && opt_args && strstr(opt_args, "-Ti"))
		GMT_Set_Default(API, "API_PAD", "0");

	/* 3. Convert mex command line arguments to a linked GMT option list */
	if (opt_buffer[0] && (options = GMT_Create_Options (API, 0, opt_buffer)) == NULL)
		mexErrMsgTxt ("GMT: Failure to parse GMT5 command options\n");

	if (!options && nlhs == 0 && nrhs == 1 && strcmp (module, "end")) 	/* Just requesting usage message, so add -? to options */
		options = GMT_Create_Options (API, 0, "-?");

	/* 4. Preprocess to update GMT option lists and return info array X */
	if ((X = GMT_Encode_Options (API, module, n_in_objects, &options, &n_items)) == NULL) {
		if (n_items == UINT_MAX)	/* Just got usage/synopsis option */
			n_items = 0;
		else
			mexErrMsgTxt ("GMT: Failure to encode mex command options\n");
	}

	if (options) {	/* Only for debugging - remove this section when stable */
		gtxt = GMT_Create_Cmd (API, options);
		GMT_Report (API, GMT_MSG_DEBUG, "GMT_Encode_Options: Revised command after memory-substitution: %s\n", gtxt);
		GMT_Destroy_Cmd (API, &gtxt);	/* Only needed it for the above verbose */
	}

	/* 5. Assign input sources (from MATLAB to GMT) and output destinations (from GMT to MATLAB) */

	for (k = 0; k < n_items; k++) {	/* Number of GMT containers involved in this module call */
		if (X[k].direction == GMT_IN) {
			if ((X[k].pos+first+1) < (unsigned int)nrhs)
				ptr = (void *)prhs[X[k].pos+first+1];
			else
				mexErrMsgTxt ("GMT: Attempting to address a prhs entry that does not exist\n");
		}
		else {
			if ((X[k].pos) < nlhs)
				ptr = (void *)plhs[X[k].pos];
			else {
				//mexErrMsgTxt ("GMT: Attempting to address a plhs entry that does not exist\n");
				ptr = alloc_default_plhs (API, &X[k]);
			}
		}
		gmtmex_Set_Object (API, &X[k], ptr);	/* Set object pointer */
	}

	/* 6. Run GMT module; give usage message if errors arise during parsing */
	status = GMT_Call_Module (API, module, GMT_MODULE_OPT, options);
	if (status != GMT_NOERROR) {
		if (status <= GMT_MODULE_PURPOSE)
			return;
		else {
			mexPrintf("GMT: Module return with failure while executing the command\n%s\n", cmd);
			mexErrMsgTxt("GMT: exiting\n");
		}
	}

	/* 7. Hook up any GMT outputs to MATLAB plhs array */

	for (k = 0; k < n_items; k++) {	/* Get results from GMT into MATLAB arrays */
		if (X[k].direction == GMT_IN) continue;	/* Only looking for stuff coming OUT of GMT here */
		pos = X[k].pos;		/* Short-hand for index into the plhs[] array being returned to MATLAB */
		plhs[pos] = gmtmex_Get_Object (API, &X[k]);	/* Hook mex object onto rhs list */
	}

	/* 2++- If gmtread -Ti then reset the sessions pad value that was temporarily changed above (2+++) */
	if (strstr(module, "read") && opt_args && strstr(opt_args, "-Ti"))
		GMT_Set_Default (API, "API_PAD", "2");

	/* 8. Free all GMT containers involved in this module call */

	for (k = 0; k < n_items; k++) {
		void *ppp = X[k].object;
		if (GMT_Close_VirtualFile (API, X[k].name) != GMT_NOERROR)
			mexErrMsgTxt ("GMT: Failed to close virtual file\n");
		if (GMT_Destroy_Data (API, &X[k].object) != GMT_NOERROR)
			mexErrMsgTxt ("GMT: Failed to destroy object used in the interface between GMT and MATLAB\n");
		else {	/* Success, now make sure we don't destroy the same pointer more than once */
			for (size_t kk = k+1; kk < n_items; kk++)
				if (X[kk].object == ppp) X[kk].object = NULL;
		}
	}
#if GMT_MAJOR_VERSION == 6 && GMT_MINOR_VERSION > 1
	/* Before we just let the memory leak... */
	GMT_Free (API, &X);
#endif

	/* 9. Destroy linked option list */

	if (GMT_Destroy_Options (API, &options) != GMT_NOERROR)
		mexErrMsgTxt ("GMT: Failure to destroy GMT5 options\n");
#ifdef SINGLE_SESSION
	if (GMT_Destroy_Session (API))
		mexErrMsgTxt ("GMT: Failure to destroy GMT5 session\n");
#endif
	return;
}

EXTERN_MSC void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	GMT_mexFunction (nlhs, plhs, nrhs, prhs);
}