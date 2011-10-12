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
/* Program:	MATLAB/OCTAVE interface to psxy
 */
 
#include "gmt_mex.h"

/* Matlab Gateway routine for psxy */

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	GMT_LONG status;
	struct	GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
	struct	GMT_VECTOR *Vi = NULL;
	char	*input = NULL, *options = NULL, *cmd = NULL; 
	int	n_cols;

	/* Make sure in/out arguments match expectation, or give usage message */
	if (!(nrhs >= 2 && nrhs <= 5) || nlhs > 0) {	/* Not what we expected, or nothing */
		GMT5MEX_banner;
		mexPrintf ("usage: psxy ('filename', 'options');\n");
		mexPrintf ("	   psxy (xi, yi[, zi[, size]], 'options');\n");
		return;
	}
	if (!mxIsChar(prhs[nrhs-1])) mexErrMsgTxt ("Last input must contain the options string\n");

	/* Initializing new GMT session */
	if (GMT_Create_Session (&API, "MEX", GMTAPI_GMTPSL)) mexErrMsgTxt ("Failure to create GMT/PSL Session\n");

	/* Make sure options are given, and get them */
	options = GMTMEX_options_init (API, prhs, nrhs);

	/* Set up input file (actual or via Matlab vectors) */
	n_cols = (nrhs == 2) ? 0 : nrhs - 1;
	input = GMTMEX_src_vector_init (API, prhs, n_cols, 0, &Vi);

	/* Build module command from input and option strings */
	cmd = GMTMEX_build_cmd (API, input, options, NULL, GMT_IS_PS);
	
	/* Run psxy module, or give usage message if errors arise during parsing */
	if ((status = GMT_psxy (API, 0, (void *)cmd))) mexErrMsgTxt ("Run-time error\n");
		
	/* Free temporary local variables  */
	GMTMEX_free (input, NULL, options, cmd);
	GMT_free_vector (API->GMT, &Vi, FALSE);	/* FALSE since vectors came from Matlab */
	
	/* Destroy GMT API session */
	if (GMT_Destroy_Session (&API)) mexErrMsgTxt ("Failure to destroy GMT Session\n");

	return;
}
