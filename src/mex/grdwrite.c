/*
 *	$Id: grdwrite.c,v 1.16 2011-03-03 21:02:51 guru Exp $
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
 
#include "gmt.h"
#include "mex.h"

#ifdef GMT_MATLAB
#define MEX_PROG "Matlab"
#else
#define MEX_PROG "Octave"
#endif

int grdwrite (double z_8[], double x[], double y[], double info[], char *fileout, char *title, int nx, int ny, int pix)
{
/* z_8[]	:	double array for input */
/* x[], y[]	:	arrays for x/y */
/* info[]	:	array for xmin, xmax, ymin, ymax, zmin, zmax, node-offset dx dy */
/* *fileout	:	output filename */
/* *title	:	title */
/* nx		:	number of x points */
/* ny		:	number of y points */
/* pix		:	1 if pixel reg, 0 if gridline registered */

	GMT_LONG i, j, i2, pad[4], error = 0;
	float *z_4;           /* real array for output */
	struct GRD_HEADER grd; 

	pad[0] = pad[1] = pad[2] = pad[3] = 0;

	/* Initialize the header with default values */

	GMT_grd_init (&grd, 0, (char **)NULL, 0);
	
	grd.nx = nx;
	grd.ny = ny;
	if (info) {
		grd.x_min = info[0];
		grd.x_max = info[1];
		grd.y_min = info[2];
		grd.y_max = info[3];
		grd.node_offset = (int) info[6];
		if (grd.node_offset) {
			grd.x_inc = (grd.x_max - grd.x_min) / grd.nx;
			grd.y_inc = (grd.y_max - grd.y_min) / grd.ny;
		}
		else {
			grd.x_inc = (grd.x_max - grd.x_min) / (grd.nx - 1);
			grd.y_inc = (grd.y_max - grd.y_min) / (grd.ny - 1);
		}
	}
	else {
		grd.x_inc = x[1] - x[0];
		grd.y_inc = y[1] - y[0];
		for (i = 2; !error && i < grd.nx; i++) if ((x[i] - x[i-1]) != grd.x_inc) error = 1;
		for (j = 2; !error && j < grd.ny; j++) if ((y[j] - y[j-1]) != grd.y_inc) error = 1;
		if (error) return (3);
		grd.x_min = (pix) ? x[0] - 0.5 * grd.x_inc : x[0];
		grd.x_max = (pix) ? x[nx-1] + 0.5 * grd.x_inc : x[nx-1];
		grd.y_min = (pix) ? y[0] - 0.5 * grd.y_inc : y[0];
		grd.y_max = (pix) ? y[ny-1] + 0.5 * grd.y_inc : y[ny-1];
		grd.node_offset = pix;
	}
		
	sprintf (grd.remark, "File written from %s with grdwrite", MEX_PROG);
	
	/*  Allocate memory */

	if ((z_4 = (float *) malloc (sizeof (float) * nx * ny)) == (float *)NULL) return (1); 

	/* Transpose from Matlab orientation to grd orientation */

	for (i = 0, i2 = ny - 1; i < ny; i++, i2--) for (j = 0; j < nx; j++) z_4[i2*nx+j] = z_8[j*ny+i];
     
	/* Update the header using values passed */

	strncpy (grd.title, title, 80);
 
	/*  Write the file */

	if (GMT_write_grd (fileout, &grd, z_4, 0.0, 0.0, 0.0, 0.0, pad, 0)) {
		free ((void *)z_4);
		return (2);
	}
	
	/*  Free memory */

	free ((void *)z_4);
	
	return (0);
}

 /* Gateway routine */

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	double *z_8, *info = (double *)NULL, *x = NULL, *y = NULL;
	char *fileout, title[80], *argv = "grdwrite-mex";
	int error, ns, ssz, nx, ny, k, pix = 0;	/* If no info we assume gridline reg */

	if (nrhs < 3 || nrhs > 6) {
		mexPrintf ("usage: grdwrite(Z, D, 'filename'[, 'title']);\n");
		mexPrintf ("       grdwrite(X, Y, Z, 'filename', 'title', [1]);\n");
		return;
	}

	GMT_begin (0, &argv);

	GMT_grdio_init ();
 
	/* Load the file name into a char string */

	k = (nrhs >= 5) ? 3 : 2;
	ns = mxGetN (prhs[k]) + 1;
	ssz = ns * sizeof (mxChar);

	if (ssz > BUFSIZ)
		mexErrMsgTxt ("grdread: filename too long\n");

	fileout = mxMalloc (ssz);

	if (mxGetString (prhs[k], fileout, ns + 1) ) {
		mexPrintf ("%s\n", fileout);
		mexErrMsgTxt ("grdwrite: failure to decode string\n");
	}
	
	title[0] = 0;
	if (nrhs >= 4) { /* Load the title into a char string */

		k = (nrhs >= 5) ? 4 : 3;
		ns = mxGetN (prhs[k]) + 1;

		if (mxGetString (prhs[k], title, ns + 1) ) {
			mexPrintf ("%s\n", title);
			mexErrMsgTxt (" *** grdwrite  failure to decode string \n");
		}
	}

	/*  get the data and info pointers */
    
	if (nrhs >= 5) {
		x   = mxGetPr (prhs[0]);
		y   = mxGetPr (prhs[1]);
		z_8 = mxGetPr (prhs[2]);
		nx  = mxGetN (prhs[2]);
		ny  = mxGetM (prhs[2]);
	}
	else {
		z_8 = mxGetPr (prhs[0]);
		nx = mxGetN (prhs[0]);
		ny = mxGetM (prhs[0]);
		info = mxGetPr (prhs[1]);
	}

	/* Do the actual computations in a subroutine */
 
	if ((error = grdwrite (z_8, x, y, info, fileout, title, nx, ny, pix))) {
		if (error == 1)
			mexErrMsgTxt ("grdwrite: failure to allocate memory\n");
		else if (error == 2)
			mexErrMsgTxt ("grdwrite: failure to write file\n");
		else
			mexErrMsgTxt ("grdwrite: x and/or y not equidistant\n");
	}
	mxFree (fileout);
	return;
}
