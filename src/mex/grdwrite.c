/*
 *	$Id$
 *
 *      Copyright (c) 1999-2012 by P. Wessel
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
 *      Contact info: www.soest.hawaii.edu/pwessel
 *--------------------------------------------------------------------*/
/* Program:	grdwrite.c
 * Purpose:	matlab callable routine to write a grd file
 * Author:	P Wessel, modified from D Sandwell's original version
 * Date:	07/01/93
 * Updates:	06/04/96: Now can take [x,y,z,file,title] instead, assuming node-format
 *		09/15/97 Phil Sharfstein: modified to Matlab 5 API
 *		10/06/98 P Wessel, upgrade to GMT 3.1 function calls
 *		11/12/98 P Wessel, ANSI-C and calls GMT_begin()
 *		10/20/03 P Wessel, longer path names [R Mueller]
 *		4/22/08 P Wessel, Now works with either Matlab or Octave
*/
 
#include "gmt_mex.h"

 /* Gateway routine */

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
	double *z = NULL;
	char *fileout = NULL, *title = NULL;
	struct GMT_GRID *G = NULL;
	GMT_LONG row, col, error = 0, k;
	uint64_t gmt_ij;

	if (nrhs < 3 || nrhs > 6) {
		GMT5MEX_banner;
		mexPrintf ("usage: grdwrite(Z, D, 'filename'[, 'title']);\n");
		mexPrintf ("       grdwrite(X, Y, Z, 'filename', 'title', [1]);\n");
		return;
	}

	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session ("GMT/MEX-API", GMTAPI_GMT)) == NULL) mexErrMsgTxt ("Failure to create GMT Session\n");

	/* 2. Prepare the GMT grid */
	if ((G = GMT_Create_Data (API, GMT_IS_GRID, NULL)) == NULL) mexErrMsgTxt ("Allocation failure\n");
	GMT_grd_init (API->GMT, G->header, NULL, FALSE);

	/* 3. Load the file name and title (if given) into char strings */
	k = (nrhs >= 5) ? 3 : 2;
	fileout = mxArrayToString (prhs[k]);	/* Load the file name into a char string */

	if (nrhs >= 4) {	/* Load the title into a char string */
		k = (nrhs >= 5) ? 4 : 3;
		title = mxArrayToString (prhs[k]);	/* Load the file name into a char string */
		strcpy (G->header->title, title);
	}

	/*  4. Get the Z array and fill in the header info */
	z = GMTMEX_info2grdheader (API, prhs, nrhs, G);
	
	sprintf (G->header->remark, "File written from %s with grdwrite", MEX_PROG);

	/*  5. Allocate memory for the grid */
	G->data = GMT_memory (API->GMT, NULL, G->header->size, float);

	/* 6. Transpose from Matlab orientation to grd orientation */
	GMT_grd_loop (API->GMT, G, row, col, gmt_ij) G->data[gmt_ij] = z[MEX_IJ(G,row,col)];
	
	/* 7. Write the grid */
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, fileout, G) != GMT_OK) mexErrMsgTxt ("Read failure\n");
	
	/* 8. Destroy the temporary grid */
	if (GMT_Destroy_Data (API, GMT_ALLOCATED, &G)) mexErrMsgTxt ("Run-time error\n");

	/* 9. Destroy GMT API session */
	if (GMT_Destroy_Session (&API)) mexErrMsgTxt ("Failure to destroy GMT Session\n");

	return;
}
