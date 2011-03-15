/*
 *	$Id: mapproject.c,v 1.2 2011-03-15 02:06:37 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 of the License.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      Contact info: www.soest.hawaii.edu/pwessel
 *--------------------------------------------------------------------*/
/* Program:	MATLAB/OCTAVE interface to mapproject
 */
 
#include "gmt_mex.h"

/* Matlab Gateway routine for psxy */

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	GMT_LONG status;
	struct	GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
	struct	GMT_VECTOR *Vi = NULL;
	struct	GMT_VECTOR *Vo = NULL;
	char	*input = NULL, *output = NULL, *options = NULL, *cmd = NULL; 
	int	n_cols;

	/* Make sure in/out arguments match expectation, or give usage message */
	if (!(nrhs >= 2) || !(nlhs >= 0 || nlhs <= 4 )) {
		GMT5MEX_banner;
		mexPrintf ("usage: [x y [z]] = mapproject ('filename', 'options');\n");
		mexPrintf ("		 out = mapproject (xi, yi [,zi], 'options');\n");
		mexPrintf ("		       mapproject (xi, yi [,zi], 'options');\n");
		return;
	}
	if (!mxIsChar(prhs[nrhs-1])) mexErrMsgTxt ("Last input must contain the options string\n");

	/* Initializing new GMT session */
	if (GMT_Create_Session (&API, "MEX", GMTAPI_GMTPSL)) mexErrMsgTxt ("Failure to create GMT/PSL Session\n");

	/* Make sure options are given, and get them */
	options = GMTMEX_options_init (API, prhs, nrhs);

	/* Set this so that a test in GMT_mapproject does not go in a un-wanted direction */
	if (nrhs > 2) API->GMT->common.b.ncol[GMT_IN] = 2;

	/* Set up input file (actual or via Matlab vectors) */
	n_cols = (nrhs == 2) ? 0 : nrhs - 1;
	input = GMTMEX_src_vector_init (API, prhs, n_cols, 0, &Vi);

	/* Register output vectors Vo to be the destination, allocated and written to by the module */
	output = GMTMEX_dest_vector_init (API, (GMT_LONG)nlhs, &Vo, nlhs, options);

	/* Build module command from input and option strings */
	cmd = GMTMEX_build_cmd (API, input, options, output, GMT_IS_DATASET);
	
	/* Run module, or give usage message if errors arise during parsing */
	if ((status = GMT_mapproject_cmd (API, 0, (void *)cmd))) mexErrMsgTxt ("Run-time error\n");
	
	/* Pass output arguments to Matlab column vectors. */
	if (nlhs) GMTMEX_prep_mextbl (API, plhs, nlhs, Vo);

	/* Destroy the columns returned from the module  */
	GMT_free_vector (API->GMT, &Vo, TRUE);	/* TRUE since vectors are being duplicated for Matlab */
		
	/* Free temporary local variables  */
	GMTMEX_free (input, NULL, options, cmd);
	GMT_free_vector (API->GMT, &Vi, FALSE);	/* FALSE since vectors came from Matlab */
	
	/* Destroy GMT API session */
	if (GMT_Destroy_Session (&API)) mexErrMsgTxt ("Failure to destroy GMT Session\n");

	return;
}
