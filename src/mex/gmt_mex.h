/*
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      Contact info: www.soest.hawaii.edu/pwessel
 *--------------------------------------------------------------------*/
/* GMT convenience functions used by MATLAB/OCTAVE mex functions
 */

#ifndef GMT_MEX_H
#define GMT_MEX_H

#include "gmt.h"
#include <mex.h>

#ifdef GMT_MATLAB
#define MEX_PROG "Matlab"
#else
#define MEX_PROG "Octave"
#endif

/* Macro for getting the Matlab/Octave ij that correspond to (row,col) [no pad involved] */
#define MEX_IJ(G,row,col) ((col)*G->header->ny + G->header->ny - (row) - 1)

#define GMT5MEX_banner mexPrintf("The Generic Mapping Tools v. 5 %s interface\n",MEX_PROG)
#define GMT_IS_PS	9	/* Use for PS output; use GMT_IS_GRID or GMT_IS_DATASET for data */

void GMTMEX_grdheader2info (mxArray *plhs[], struct GMT_GRID *G, int item);
void GMTMEX_grdxy (struct GMTAPI_CTRL *API, mxArray *plhs[], struct GMT_GRID *G, int px, int py);
void GMTMEX_prep_mexgrd (struct GMTAPI_CTRL *API, mxArray *plhs[], int nlhs, struct GMT_GRID *G);
void GMTMEX_prep_mextbl (struct GMTAPI_CTRL *API, mxArray *plhs[], int nlhs, struct GMT_VECTOR *V);
double *GMTMEX_info2grdheader (struct GMTAPI_CTRL *API, const mxArray *prhs[], int nrhs, struct GMT_GRID *G);
char *GMTMEX_src_grid_init (struct GMTAPI_CTRL *API, const mxArray *prhs[], int nrhs, struct GMT_GRID **G);
char *GMTMEX_src_vector_init (struct GMTAPI_CTRL *API, const mxArray *prhs[], int n_cols, int n_start, struct GMT_VECTOR **V);
char *GMTMEX_dest_grid_init (struct GMTAPI_CTRL *API, struct GMT_GRID **G, int nlhs, char *options);
char *GMTMEX_dest_vector_init (struct GMTAPI_CTRL *API, GMT_LONG n_cols, struct GMT_VECTOR **V, int nlhs, char *options);
char *GMTMEX_options_init (struct GMTAPI_CTRL *API, const mxArray *prhs[], int nrhs);
char *GMTMEX_build_cmd (struct GMTAPI_CTRL *API, char *src, char *options, char *dest, GMT_LONG mode);
void GMTMEX_free (char *input, char *output, char *options, char *cmd);

#endif
