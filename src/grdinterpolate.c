/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * Brief synopsis: grdinterpolate reads a 3d netcdf spatial data cube with
 * the 3rd dimension either depth/height or time.  It then interpolates
 * the cube at arbitrary z (or time) values and writes either a single
 * slice 2-D grid or another 3-D data cube (via ncecat).
 *
 * Author:	Paul Wessel
 * Date:	1-AUG-2019
 * Version:	6 API
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"grdinterpolate"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Interpolate a 3-D data cube"
#define THIS_MODULE_KEYS	"<G{,GG}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"->RV"

struct GRDINTERPOLATE_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct C {	/* -C<cpt> or -C<color1>,<color2>[,<color3>,...] */
		bool active;
		char *file;
	} C;
	struct F {	/* -Fl|a|c[1|2] */
		bool active;
		unsigned int mode;
		unsigned int type;
	} F;
	struct G {	/* -G<output_grdfile>  */
		bool active;
		char *file;
	} G;
	struct T {	/* -T<start>/<stop>/<inc> or -T<value> */
		bool active;
		struct GMT_ARRAY T;
	} T;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDINTERPOLATE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDINTERPOLATE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDINTERPOLATE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	char type[3] = {'l', 'a', 'c'};
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <3Dgrid> -G<outgrid> -T[<min>/<max>/]<inc>[<unit>][+n]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Fl|a|c|n][+1|2]  [%s] [%s]\n\n", GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<3Dgrid> is name of a 3D netCDF data cube.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify output grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Interpolate the 3-D grid at given knots in the 3rd dimension\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Make evenly spaced output time steps from <min> to <max> by <inc>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n to indicate <inc> is the number of knot-values to produce over the range instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, give a file with output knots in the first column, or a comma-separated list.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Set the interpolation mode.  Choose from:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   l Linear interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a Akima spline interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c Cubic spline interpolation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   n No interpolation (nearest point).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append +1 for 1st derivative or +2 for 2nd derivative.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is -F%c].\n", type[API->GMT->current.setting.interpolant]);
	GMT_Option (API, "V,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDINTERPOLATE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files[2] = {0, 0};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files[GMT_IN]++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'F':
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'l':
						Ctrl->F.mode = GMT_SPLINE_LINEAR;
						break;
					case 'a':
						Ctrl->F.mode = GMT_SPLINE_AKIMA;
						break;
					case 'c':
						Ctrl->F.mode = GMT_SPLINE_CUBIC;
						break;
					case 'n':
						Ctrl->F.mode = GMT_SPLINE_NN;
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: Bad spline selector %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				if (opt->arg[1] == '+') Ctrl->F.type = (opt->arg[2] - '0');	/* Want first or second derivatives */
				break;
			case 'G':	/* Output file */
				if (n_files[GMT_OUT]++ > 0) break;
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'T':
				Ctrl->T.active = true;
				n_errors += gmt_parse_array (GMT, 'T', opt->arg, &(Ctrl->T.T), GMT_ARRAY_TIME | GMT_ARRAY_NOMINMAX, GMT_Z);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files[GMT_IN] < 1, "Error: No 3D grid cube specified.\n");
	n_errors += gmt_M_check_condition (GMT, n_files[GMT_IN] != 1, "Syntax error: Must specify one input 3D grid cube file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.type > 2, "Syntax error -F option: Only 1st or 2nd derivatives may be requested\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output grid file\n");
	n_errors += gmt_M_check_condition (GMT, n_files[GMT_OUT] != 1, "Syntax error: Must specify only one output grid file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdinterpolate (void *V_API, int mode, void *args) {
	int error = 0;
	uint64_t nz = 0;
	double *zarray = NULL;
	struct GMT_GRID *G = NULL;
	struct GRDINTERPOLATE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdinterpolate main code ----------------------------*/

	if ((error = gmt_examine_nc_cube (GMT, Ctrl->In.file, &nz, &zarray))) Return (error);

	Return ((error) ? error : GMT_NOERROR);
}
