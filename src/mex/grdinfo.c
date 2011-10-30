/*
 *	$Id$
 *
 *      Copyright (c) 1999-2011 by P. Wessel
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
/* Program:	grdinfo.c
 * Purpose:	matlab callable routine to read a grd file header
 * Author:	P Wessel, modified from D Sandwell's original version
 * Date:	07/01/93
 * Update:	09/15/97 Phil Sharfstein: modified to Matlab 5 API
 *		10/06/98 P Wessel, upgrade to GMT 3.1 function calls
 *		11/12/98 P Wessel, ANSI-C and calls GMT_begin()
 *		10/20/03 P Wessel, longer path names [R Mueller]
 *		4/22/08 P Wessel, Now works with either Matlab or Octave
 */
 
#include "gmt_mex.h"

/* Matlab Gateway routine for grdinfo */
   
void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	struct GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
	struct GMT_GRID *G = NULL;
	double *info = NULL;
	char *filein = NULL;

	if (nrhs != 1) {
		GMT5MEX_banner;
		mexPrintf (" usage: [info = ]grdinfo('filename'); \n");
		return;
	}

	filein = mxArrayToString (prhs[0]);	/* Load the file name into a char string */
	
	/* 1. Initializing new GMT session */
	if ((API = GMT_Create_Session ("GMT/MEX-API", GMTAPI_GMT)) == NULL) mexErrMsgTxt ("Failure to create GMT Session\n");

	/* 2. READING IN A GRID */
	if (GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET)) mexErrMsgTxt ("GMT: (grdinfo) Failure to Begin IO\n");
	if ((G = GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, filein, NULL)) == NULL) mexErrMsgTxt ("GMT: (grdinfo) Read failure\n");
	if (GMT_End_IO (API, GMT_IN, 0)) mexErrMsgTxt ("GMT: (grdinfo) Failure to End IO\n");
	
	mexPrintf("%s: Title: %s\n", filein, G->header->title);
	mexPrintf("%s: Command: %s\n", filein, G->header->command);
	mexPrintf("%s: Remark: %s\n", filein, G->header->remark);
	if (G->header->registration)
		mexPrintf("%s: Pixel registration used\n", filein);
	else
		mexPrintf("%s: Normal node registration used\n", filein);
	mexPrintf("%s: x_min: %g x_max: %g x_inc: %g name: %s nx: %d\n",
		filein, G->header->wesn[XLO], G->header->wesn[XHI], G->header->inc[GMT_X], G->header->x_units, G->header->nx);
	mexPrintf("%s: y_min: %g y_max: %g y_inc: %g name: %s ny: %d\n",
		filein, G->header->wesn[YLO], G->header->wesn[YHI], G->header->inc[GMT_Y], G->header->y_units, G->header->ny);
	mexPrintf("%s: z_min: %g z_max: %g name: %s\n",
		filein, G->header->z_min, G->header->z_max, G->header->z_units);
	mexPrintf("%s: scale_factor: %g add_offset: %g\n",
		filein, G->header->z_scale_factor, G->header->z_add_offset);

	/* Create scalars for return arguments */

	if (nlhs == 1) GMTMEX_grdheader2info (plhs, G, 0);	/* Return info array */
 	
	if (GMT_Destroy_Data (API, GMT_ALLOCATED, &G)) mexErrMsgTxt ("Run-time error\n");

	/* 3. Destroy GMT session */
	if (GMT_Destroy_Session (&API))  mexErrMsgTxt ("GMT: (grdinfo) Failure to destroy GMT Session\n");

	return;
}
