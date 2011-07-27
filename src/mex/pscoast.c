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
/* Program:	MATLAB/OCTAVE interface to pscoast
 */
 
#include "gmt_mex.h"

/* Matlab Gateway routine for pscoast */

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	GMT_LONG status;
	struct	GMTAPI_CTRL *API = NULL;		/* GMT API control structure */
	char	*options = NULL, *cmd = NULL; 

	/* Make sure in/out arguments match expectation, or give usage message */
	if (nrhs != 1 || nlhs > 0) {	/* Not what we expected, or nothing */
		GMT5MEX_banner;
		mexPrintf ("usage: pscoast ('options');\n");
		return;
	}
	if (!mxIsChar(prhs[nrhs-1])) mexErrMsgTxt ("Last input must contain the options string\n");

	/* Initializing new GMT session */
	if (GMT_Create_Session (&API, "MEX", GMTAPI_GMTPSL)) mexErrMsgTxt ("Failure to create GMT/PSL Session\n");

	/* Make sure options are given, and get them */
	options = GMTMEX_options_init (API, prhs, nrhs);

	/* Build module command from input and option strings */
	cmd = GMTMEX_build_cmd (API, NULL, options, NULL, GMT_IS_PS);
	
	/* Run pscoast module, or give usage message if errors arise during parsing */
	if ((status = GMT_pscoast_cmd (API, 0, (void *)cmd))) mexErrMsgTxt ("Run-time error\n");
		
	/* Free temporary local variables  */
	mxFree (cmd);
	
	/* Destroy GMT API session */
	if (GMT_Destroy_Session (&API)) mexErrMsgTxt ("Failure to destroy GMT Session\n");

	return;
}
