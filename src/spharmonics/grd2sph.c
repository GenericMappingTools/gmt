/*--------------------------------------------------------------------
 *    $Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * grd2sph evalutes a grid using a spherical harmonics model
 *
 * Author:	Paul Wessel
 * Date:	10-FEB-2015
 */
 
#include "gmt_dev.h"

#define THIS_MODULE_NAME	"grd2sph"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Compute spherical harmonic coefficients from global grid"
#define THIS_MODULE_KEYS	"<GI,>DO"

#define GMT_PROG_OPTIONS "->Vbho"

struct GRD2SPH_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct In {	/* <grdfile> */
		bool active;
		char *file;
	} In;
	struct Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct N {	/* -Ng|m|s */
		bool active;
		char mode;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
};

void *New_grd2sph_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRD2SPH_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct GRD2SPH_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->N.mode = 'm';
		
	return (C);
}

void Free_grd2sph_Ctrl (struct GMT_CTRL *GMT, struct GRD2SPH_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->Out.file) free (C->Out.file);
	GMT_free (GMT, C);	
}

int GMT_grd2sph_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_Message (API, GMT_TIME_NONE, "grd2sph - Create spherical harmonic model from a global grid\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: grd2sph <grid> [-N<norm>] [-Q] [%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_bo_OPT, GMT_h_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<grid> must be a global grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Normalization used for coefficients.  Choose among\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m: Mathematical normalization - inner products summed over surface equal 1 [Default]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g: Geodesy normalization - inner products summed over surface equal 4pi\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s: Schmidt normalization - as used in geomagnetism\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Coefficients have phase convention from physics, i.e., the (-1)^m factor\n");
	GMT_explain_options ("V,bo4,h,o,.");
	
	return (EXIT_FAILURE);
}

int GMT_grd2sph_parse (struct GMT_CTRL *GMT, struct GRD2SPH_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grd2sph and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files[] = {0, 0};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Got input files */
				if (n_files[GMT_IN]++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;
			case '>':	/* Got named output file */
				if (n_files[GMT_OUT]++ == 0 && GMT_check_filearg (GMT, '>', opt->arg, GMT_OUT, GMT_IS_DATASET))
					Ctrl->Out.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.mode = opt->arg[0];
				break;
			case 'Q':
				Ctrl->Q.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files[GMT_IN] > 1, "Syntax error: Can only handle one input grid file\n");
	n_errors += GMT_check_condition (GMT, n_files[GMT_OUT] > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->N.mode == 'm' || Ctrl->N.mode == 'g' || Ctrl->N.mode == 's'), "Syntax error: -N Normalization must be one of m, g, or s\\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grd2sph_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grd2sph (void *V_API, int mode, void *args)
{
	int error;
	struct GMT_GRID *G = NULL;
	struct GMT_DATASET *D = NULL;
	struct GRD2SPH_CTRL *Ctrl;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	void *New_grd2sph_Ctrl (), Free_grd2sph_Ctrl (struct GRD2SPH_CTRL *C);

	Ctrl = (struct GRD2SPH_CTRL *) New_grd2sph_Ctrl ();		/* Allocate and initialize defaults in a new control structure */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grd2sph_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grd2sph_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, NULL, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grd2sph_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grd2sph_parse (GMT, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the grd2sph main code ----------------------------*/

	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->In.file, NULL)) == NULL) {
		Return (API->error);
	}
	
	/* Do all the hard work! */
	
	/* Write out the coefficients in D */
	
	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_SET, NULL, Ctrl->Out.file, D) != GMT_OK) {
		Return (API->error);
	}
		
	Return (EXIT_SUCCESS);
}
