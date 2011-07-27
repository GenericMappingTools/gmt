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
/* Program:	grdread.c
 * Purpose:	matlab/octave callable routine to read a GMT grid file
 */
 
#include "gmt_mex.h"

/* Matlab Gateway routine */

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
	struct GMT_GRID *G = NULL;
	float	*z = NULL;
	char *filein = NULL;
	GMT_LONG row, col, gmt_node;
	int	px = -1, py = -1, pz = -1, pi = -1;

	if (nrhs != 1 || nlhs < 1 || nlhs > 4) {	/* Give usage message and return */
		GMT5MEX_banner;
		mexPrintf ("usage: z = grdread ('filename');\n");
		mexPrintf ("	[z info] = grdread ('filename');\n");
		mexPrintf ("	[x y z] = grdread ('filename');\n");
		mexPrintf ("	[x y z info] = grdread ('filename');\n");
		return;
	}
	if (!mxIsChar(prhs[nrhs-1])) mexErrMsgTxt ("Input must contain the filename string\n");

	/* Load the file name into a char string */

	filein = (char *) mxArrayToString (prhs[0]);	/* Load the file name into a char string */

	/* 1. Initializing new GMT session */
	if (GMT_Create_Session (&API, "MEX", GMTAPI_GMT)) mexErrMsgTxt ("GMT: (grdread) Failure to create GMT Session\n");

	/* 2. READING IN A GRID */
	if (GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET)) mexErrMsgTxt ("GMT: (grdinfo) Failure to Begin IO\n");
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&filein, (void **)&G))
		mexErrMsgTxt ("GMT: (grdread) Read failure\n");
	if (GMT_End_IO (API, GMT_IN, 0)) mexErrMsgTxt ("GMT: (grdinfo) Failure to End IO\n");
	
	/* Create a matrix for the return array */

	pz = (nlhs >= 3) ? 2 : 0;
	if (nlhs == 2 || nlhs == 4) pi = nlhs - 1;
	if (nlhs > 2) {px = 0; py = 1;}

	plhs[pz] = mxCreateNumericMatrix (G->header->ny, G->header->nx, mxSINGLE_CLASS, mxREAL);
	z = (float *)mxGetData (plhs[pz]);
	
	/*  Load the real grd array into a double matlab array
	    by transposing from padded GMT grd format to unpadded matlab format */
    
	GMT_grd_loop (API->GMT, G, row, col, gmt_node) z[MEX_IJ(G,row,col)] = G->data[gmt_node];
	    
	/* Create scalars for return arguments */

	if (pi >= 0) GMTMEX_grdheader2info (plhs, G, pi);	/* Also return info array */
	if (px >= 0) GMTMEX_grdxy (API, plhs, G, px, py);	/* Return x,y arrays also */
	
	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&G);
	
	/* 10. Destroy GMT API session */
	if (GMT_Destroy_Session (&API)) mexErrMsgTxt ("GMT: (surface) Failure to destroy GMT Session\n");

	return;
}
