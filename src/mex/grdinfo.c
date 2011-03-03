/*
 *	$Id: grdinfo.c,v 1.10 2011-03-03 21:02:51 guru Exp $
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
 
#include "gmt.h"
#include "mex.h"

void grdinfo (double info[], char *filein, struct GRD_HEADER *grd)
{
/* info		:	info: array for xmin, xmax, ymin, ymax, zmin, zmax, node-offset dx dy */
/* filein	:	input filename */
/* grd		:	GMT grdfile header structure */

	if (info) {
		info[0] = grd->x_min;
		info[1] = grd->x_max;
		info[2] = grd->y_min;
		info[3] = grd->y_max;
		info[4] = grd->z_min;
		info[5] = grd->z_max;
		info[6] = grd->node_offset;
		info[7] = grd->x_inc;
		info[8] = grd->y_inc;
	}

	mexPrintf("%s: Title: %s\n", filein, grd->title);
	mexPrintf("%s: Command: %s\n", filein, grd->command);
	mexPrintf("%s: Remark: %s\n", filein, grd->remark);
	if (grd->node_offset)
		mexPrintf("%s: Pixel registration used\n", filein);
	else
		mexPrintf("%s: Normal node registration used\n", filein);
	mexPrintf("%s: x_min: %g x_max: %g x_inc: %g name: %s nx: %d\n",
		filein, grd->x_min, grd->x_max, grd->x_inc, grd->x_units, grd->nx);
	mexPrintf("%s: y_min: %g y_max: %g y_inc: %g name: %s ny: %d\n",
		filein, grd->y_min, grd->y_max, grd->y_inc, grd->y_units, grd->ny);
	mexPrintf("%s: z_min: %g z_max: %g name: %s\n",
		filein, grd->z_min, grd->z_max, grd->z_units);
	mexPrintf("%s: scale_factor: %g add_offset: %g\n",
		filein, grd->z_scale_factor, grd->z_add_offset);
}


 /* Gateway routine */
   
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	struct GRD_HEADER grd;
	double *info = (double *)NULL;
	char *filein, *argv = "grdinfo-mex";
	int ns, ssz;

	GMT_begin (0, &argv);

	GMT_grdio_init ();
 
	/* Load the file name into a char string */

	ns = mxGetN (prhs[0]) + 1;
	ssz = ns * sizeof (mxChar);

	if (ssz > BUFSIZ)
		mexErrMsgTxt ("grdread: filename too long\n");

	filein = mxMalloc (ssz);
  
	if (nrhs != 1) {
		mexPrintf (" usage: [info = ]grdinfo('filename'); \n");
		return;
	}

	if (mxGetString (prhs[0], filein, ns + 1)) {
		mexPrintf ("%s\n", filein);
		mexErrMsgTxt ("grdinfo: Failure to decode string \n");
	}
	
	/* Make sure file is readable */
	
	if (GMT_access (filein, R_OK))
		mexErrMsgTxt ("grdinfo: Cannot find or open file\n");

	/* Read the header */
 
	GMT_grd_init (&grd, 0, NULL, FALSE);
	if (GMT_read_grd_info (filein, &grd))
		mexErrMsgTxt ("grdinfo: Failure to read header\n");

	/* Create scalars for return arguments */

	if (nlhs == 1) {
		plhs[0] = mxCreateDoubleMatrix (1, 9, mxREAL);
		info = mxGetPr (plhs[0]);
 	}
 	
	/* Do the actual computations in a subroutine */
 
	grdinfo (info, filein, &grd);
	
	mxFree (filein);
	return;
}
