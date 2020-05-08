/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: grdmix.c reads two grid files and writes a new file with
 * the first two pasted together along their common edge.
 *
 * Author:	Walter Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdmix"
#define THIS_MODULE_MODERN_NAME	"grdmix"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Transparency-blending of grids or images"
#define THIS_MODULE_KEYS	"<G{2,GG}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-Vf"

struct GRDMIX_CTRL {
	struct GRDMIX_In {
		bool active;
		unsigned int n_in;
		char *file[2];
	} In;
	struct GRDMIX_A {
		bool active;
		char *file;
	} A;
	struct GRDMIX_G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDMIX_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDMIX_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDMIX_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->In.file[GMT_IN]);
	gmt_M_str_free (C->In.file[GMT_OUT]);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <grid1> <grid2> -G<outgrid> [%s] [%s] [%s]\n\n", name, GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\twhere grids <grid1> and <grid2> are to be combined into <outgrid>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<grid1> and <grid2> must have same dx,dy and one edge in common.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tIf in doubt, run grdinfo first and check your files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tUse grdmix and/or grdsample to adjust files as necessary.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tIf grids are geographic and adds to full 360-degree range then grid1\n");
	GMT_Message (API, GMT_TIME_NONE, "\tdetermines west.  Use grdedit -S to rotate grid to another -Rw/e/s/n.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output grid file.\n");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "V,f,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDMIX_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdmix and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_in = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (Ctrl->In.n_in == 0 && gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					Ctrl->Ctrl->In.n_in.file[Ctrl->In.n_in++] = strdup (opt->arg);
				else if (n_in == 1 && gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID))
					Ctrl->In.file[Ctrl->In.n_in++] = strdup (opt->arg);
				else {
					n_errors++;
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Only two files may be pasted\n");
				}
				break;

			/* Processes program-specific parameters */

 			case 'A':
				if ((Ctrl->A.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->A.file = strdup (opt->arg);
				else
					n_errors++;
				break;


 			case 'G':
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file[0] || !Ctrl->In.file[1], "Must specify two input files\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Option -G: Must specify output file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

/* True if grid is a COARDS/CF netCDF file */
GMT_LOCAL inline bool grdmix_is_nc_grid (struct GMT_GRID *grid) {
	return
		grid->header->type == GMT_GRID_IS_NB ||
		grid->header->type == GMT_GRID_IS_NS ||
		grid->header->type == GMT_GRID_IS_NI ||
		grid->header->type == GMT_GRID_IS_NF ||
		grid->header->type == GMT_GRID_IS_ND;
}

EXTERN_MSC int GMT_grdmix (void *V_API, int mode, void *args) {
	int error = 0, way = 0;
	unsigned int one_or_zero;
	bool common_y = false;

	char format[GMT_BUFSIZ];

	double x_noise, y_noise;

	struct GMT_GRID *A = NULL, *B = NULL, *C = NULL;
	struct GMT_GRID_HEADER_HIDDEN *AH = NULL, *BH = NULL;
	struct GRDMIX_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdmix main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input grids\n");
	gmt_set_pad (GMT, 0); /* No padding */

	for (k = 0; k < Ctrl->In.n_in; k++) {	/* Determine if a grid or image */
		Ctrl->In.type[k] = gmt_raster_type (GMT, Ctrl->In.file[k]);
	}

	if (Ctrl->In.n_in == 1 && Ctrl->In.type[0] == GMT_NOTSET) {
		GMT_Report (API, GMT_MSG_ERROR, "For a single input raster it must be an image\n");
		Return (GMT_RUNTIME_ERROR);		
	}

	for (k = 0; k < Ctrl->In.n_in; k++) {	/* Read headers */
		if (Ctrl->In.type[k] == GMT_NOTSET) {
			if ((Ag = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			h[k] = Ag->header;
		}
		else {
			if ((Ai = GMT_Read_Data (API, GMT_IS_IMAGE GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file[k], NULL)) == NULL) {	/* Get header only */
				Return (API->error);
			}
			h[k] = Ai->header;
		}
	}
	if (Ctrl->In.n_in == 2) {
		if (h[0]->registration != h[1]->registration || (h[0]->n_rows != h[1]->n_rows) || (h[0]->n_columns != h[1]->n_columns)) {
			GMT_Report (API, GMT_MSG_ERROR, "Dimension/registrations are not compatible!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (Ctrl->In.type[0] != Ctrl->In.type[1]) {
			GMT_Report (API, GMT_MSG_ERROR, "Both inputs must either be images or grids, not a mix\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	Return (GMT_NOERROR);
}
