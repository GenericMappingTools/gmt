/*
 *	$Id: surface.c,v 1.3 2011-04-23 02:14:13 guru Exp $
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
/* Program:	MATLAB/OCTAVE interface to GMT_surface
 */
 
#include "gmt_mex.h"

/* Matlab Gateway routine for surface */

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	GMT_LONG status;
	struct	GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
	struct	GMT_GRID *G = NULL;
	struct	GMT_VECTOR *V = NULL;
	char	*input = NULL, *output = NULL, *options = NULL, *cmd = NULL; 
	int	n_cols;

	/* Make sure in/out arguments match expectation, or give usage message */
	if (!(nrhs == 2 || nrhs == 4) || nlhs > 4) {	/* Not what we expected, or nothing */
		GMT5MEX_banner;
		mexPrintf ("usage: Z = surface ('filename', 'options');\n");
		mexPrintf ("	Z = surface (xi, yi, zi, 'options');\n");
		mexPrintf ("	[Z info] = surface ('filename', 'options');\n");
		mexPrintf ("	[Z info] = surface (xi, yi, zi, 'options');\n");
		mexPrintf ("	[x y Z] = surface ('filename', 'options');\n");
		mexPrintf ("	[x y Z] = surface (xi, yi, zi, 'options');\n");
		mexPrintf ("	[x y Z info] = surface ('filename', 'options');\n");
		mexPrintf ("	[x y Z info] = surface (xi, yi, zi, 'options');\n");
		return;
	}
	if (!mxIsChar(prhs[nrhs-1])) mexErrMsgTxt ("Last input must contain the options string\n");

	/* Initializing new GMT session */
	if (GMT_Create_Session (&API, "MEX", GMTAPI_GMT)) mexErrMsgTxt ("Failure to create GMT Session\n");

	/* Make sure options are given, and get them */
	options = GMTMEX_options_init (API, prhs, nrhs);

	/* Set up input file (actual or via Matlab vectors) */
	n_cols = (nrhs == 2) ? 0 : nrhs - 1;
	input = GMTMEX_src_vector_init (API, prhs, n_cols, 0, &V);

	/* Register a grid struct G to be the destination, allocated and written to by the module */
	output = GMTMEX_dest_grid_init (API, &G, nlhs, options);

	/* Build module command from input, ouptput, and option strings */
	cmd = GMTMEX_build_cmd (API, input, options, output, GMT_IS_GRID);

	/* Run GMT_surface module, or give usage message if errors arise during parsing */
	if ((status = GMT_surface_cmd (API, 0, (void *)cmd))) mexErrMsgTxt ("Run-time error\n");
	
	/* Pass output arguments to Matlab vectors Z, with optional (x, y) or hdr. */
	if (nlhs) GMTMEX_prep_mexgrd (API, plhs, nlhs, G);

	/* Destroy the main grid returned from the module  */
	GMT_Destroy_Data (API, GMT_REFERENCE, (void **)&G);
	
	/* Free temporary local variables  */
	GMTMEX_free (input, output, options, cmd);
	GMT_free_vector (API->GMT, &V, FALSE);	/* FALSE since vectors came from Matlab */
	
	/* Destroy GMT API session */
	if (GMT_Destroy_Session (&API)) mexErrMsgTxt ("Failure to destroy GMT Session\n");
	
	return;
}
