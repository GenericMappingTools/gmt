/*
 *	$Id: grdread.c,v 1.17 2011-03-03 21:02:51 guru Exp $
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
/* Program:	grdread.c
 * Purpose:	matlab callable routine to read a grd file
 * Author:	P Wessel, modified from D Sandwell's original version
 * Date:	07/01/93
 * Update:	06/04/96: P Wessel: Now can return [x,y,z] as option.
 *		09/15/97 Phil Sharfstein: modified to Matlab 5 API
 *		10/06/98 P Wessel, upgrade to GMT 3.1 function calls
 *		11/12/98 P Wessel, ANSI-C and calls GMT_begin()
 *		10/07/99 P Wessel, Did not set x,y if [x,y,z,d] was used
 *		10/20/03 P Wessel, longer path names [R Mueller]
 *		4/22/08 P Wessel, Now works with either Matlab or Octave
 */
 
#include "gmt.h"
#include "mex.h"

int grdread (double z_8[], double info[], char *filein, struct GRD_HEADER *grd)
{
	/* info contains xmin, xmax, ymin, ymax, zmin, zmax, node-offset, dx, dy */

	GMT_LONG i, j, pad[4];
	float *z_4;          /* real array for output */

	pad[0] = pad[1] = pad[2] = pad[3] = 0;

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

	/*  Allocate memory */

	if ((z_4 = (float *) malloc (sizeof (float) * grd->nx * grd->ny)) == (float *)NULL) return (1);
 
 	/* Check for file access here since the exit returned by the read routine shuts down Matlab... */
	
	if (GMT_access (filein, R_OK)) return (2);
		
	/*  Read the grid */
 
	if (GMT_read_grd (filein, grd, z_4, 0.0, 0.0, 0.0, 0.0, pad, 0)) {
		free ((void *)z_4); 
  	  	return (2);
	}
 
	/*  Load the real grd array into a double matlab array
	    by transposing from grd format to matlab format */
    
	for (i = 0; i < grd->ny; i++) for (j = 0; j < grd->nx; j++) z_8[j*grd->ny+grd->ny-i-1] = z_4[i*grd->nx+j];

	/*  Free memory */

	free ((void *)z_4);
	
	return (0);
}


/* Matlab Gateway routine */

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	struct GRD_HEADER grd;
	double *z_8, *info = (double *)NULL, *x = NULL, *y = NULL, off;
	char *filein, *argv = "grdread-mex";
	int error, ns, ssz, pz, i;
 
	GMT_begin (0, &argv);

	GMT_grdio_init ();

	if (nrhs != 1 || nlhs < 1 || nlhs > 4) {
		mexPrintf ("usage: z = grdread('filename');\n");
		mexPrintf (" 	[z,info] = grdread('filename');\n");
		mexPrintf ("	[x,y,z] = grdread('filename');\n");
		mexPrintf ("	[x,y,z,info] = grdread('filename');\n");
		return;
	}

	/* Load the file name into a char string */

	ns = mxGetN (prhs[0]) + 1;
	ssz = ns * sizeof (mxChar);

	if (ssz > BUFSIZ)
		mexErrMsgTxt ("grdread: filename too long\n");

	filein = mxMalloc (ssz);

	if (mxGetString (prhs[0], filein, ns + 1)) {
		mexPrintf ("%s\n", filein);
		mexErrMsgTxt ("grdread: failure to decode string \n");
	}

	/* Read the header */
 
	GMT_grd_init (&grd, 0, NULL, FALSE);
	if (GMT_read_grd_info (filein, &grd))
		mexErrMsgTxt ("grdread: failure to read header\n");

	/* Create a matrix for the return array */

	pz = (nlhs >= 3) ? 2 : 0;

	plhs[pz] = mxCreateDoubleMatrix (grd.ny, grd.nx, mxREAL);
    
	z_8 = mxGetPr (plhs[pz]);

	/* Create scalars for return arguments */

	if (nlhs == 2) {	/* Also return info array */
		plhs[1] = mxCreateDoubleMatrix (1, 9, mxREAL);
		info = mxGetPr (plhs[1]);
	}
	else if (nlhs >= 3) {	/* Return x,y arrays instead */
		plhs[0] = mxCreateDoubleMatrix (1, grd.nx, mxREAL);
		plhs[1] = mxCreateDoubleMatrix (1, grd.ny, mxREAL);
		x = mxGetPr (plhs[0]);
		y = mxGetPr (plhs[1]);
		if (nlhs == 4) {	/* Also return info array */
			plhs[3] = mxCreateDoubleMatrix (1, 9, mxREAL);
			info = mxGetPr (plhs[3]);
		}
	}
 
	/* Call grdread to get the contents of the file */
 
	if ((error = grdread (z_8, info, filein, &grd))) {
		if (error == 1)
			mexErrMsgTxt ("grdread: failure to allocate memory\n");
		else
			mexErrMsgTxt ("grdread: failure to read file\n");
	}
	if (nlhs >= 3) {	/* Fill in the x and y arrayx */
		off = (grd.node_offset) ? 0.5 : 0.0;
		for (i = 0; i < grd.nx; i++) x[i] = grd.x_min + (i + off) * grd.x_inc;
		for (i = 0; i < grd.ny; i++) y[i] = grd.y_min + (i + off) * grd.y_inc;
	}
	mxFree (filein);
	return;
}
