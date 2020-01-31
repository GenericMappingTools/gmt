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
 * Brief synopsis: grdconvert.c reads a grid file in one format and outputs it in another
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdconvert"
#define THIS_MODULE_MODERN_NAME	"grdconvert"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Convert between different grid formats"
#define THIS_MODULE_KEYS	"<G{,>G}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-RVf"

struct GRDCONVERT_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct G {	/* -G<outgrid> */
		bool active;
		char *file;
	} G;
	struct N {	/* -N */
		bool active;
	} N;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDCONVERT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDCONVERT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDCONVERT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	int i;
	char **grdformats = gmt_grdformats_sorted (API->GMT);

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <ingrid>[=<id>][+s<scale>][+o<offset>][+n<nan>]\n\t-G<outgrid>[=<id>][+s<scale>][+o<offset>][+n<nan>][:<driver>[/<dataType>]] [-N]\n\t[%s] [%s] [%s] [%s]\n\n",
		name, GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is the grid file to convert.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G <outgrid> is the new converted grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   scale and offset, if given, will multiply data by scale and add offset.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, +n designates an invalid grid value to act as NaN.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do NOT write the header (for native grids only - ignored otherwise).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Useful when creating files to be used by external programs.\n");
	GMT_Option (API, "R,V,f,.");

	GMT_Message (API, GMT_TIME_NONE, "\nThe following grid file formats are supported:\n");
	for (i = 1; i < GMT_N_GRD_FORMATS; ++i) {
		if (!strstr (grdformats[i], "not supported"))
			GMT_Message (API, GMT_TIME_NONE, "\t%s\n", grdformats[i]);
	}
#ifdef HAVE_GDAL
	GMT_Message (API, GMT_TIME_NONE, "\n\tWhen <id>=gd on output, the grid will be saved using the GDAL library.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tSpecify <driver> and optionally <dataType>. Driver names are as in GDAL\n\t(e.g., netCDF, GTiFF, etc.)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<dataType> is u8|u16|i16|u32|i32|float32; i|u denote signed|unsigned\n\tinteger.  Default type is float32.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tBoth driver names and data types are case insensitive.\n");
#endif
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDCONVERT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdconvert and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_in = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input and Output files */
				/* Since grdconvert allowed output grid to be given without -G we must actually
				 * check for two input files and assign the 2nd as the actual output file */
				if (n_in == 0 && gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)) {
					Ctrl->In.file= strdup (opt->arg);
					n_in++;
				}
				else if (n_in == 1 && gmt_check_filearg (GMT, '>', opt->arg, GMT_OUT, GMT_IS_GRID)) {
					Ctrl->G.active = true;
					Ctrl->G.file= strdup (opt->arg);
					n_in++;
				}
				else {
					n_in++;
					GMT_Report (API, GMT_MSG_ERROR, "Specify only one input file\n");
					n_errors++;
				}
				break;
			case '>':	/* Output file may be set this way from the external API */
				Ctrl->G.active = true;
				if (gmt_check_filearg (GMT, '>', opt->arg, GMT_OUT, GMT_IS_GRID))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				n_in++;
				break;

			/* Processes program-specific parameters */

			case 'G':
				Ctrl->G.active = true;
				if (Ctrl->G.file) {
					GMT_Report (API, GMT_MSG_ERROR, "Specify only one output file\n");
					n_errors++;
				}
				else
					Ctrl->G.file = strdup (opt->arg);
				break;

			case 'N':
				Ctrl->N.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !(Ctrl->In.file && Ctrl->G.file), "Must specify both input and output file names\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC void gmtlib_grd_set_units (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);

int GMT_grdconvert (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int hmode, type[2] = {0, 0};
	char fname[2][GMT_BUFSIZ];
	char command[GMT_GRID_COMMAND_LEN320] = {""};
	struct GMT_GRID *Grid = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GRDCONVERT_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the grdconvert main code ----------------------------*/

	if ((Grid = gmt_create_grid (API->GMT)) == NULL) Return (API->error);	/* Tmp grid only, no i/o is used */
	gmt_grd_init (GMT, Grid->header, options, false);
	HH = gmt_get_H_hidden (Grid->header);
	hmode = (Ctrl->N.active) ? GMT_GRID_NO_HEADER : 0;
	gmt_M_err_fail (GMT, gmt_grd_get_format (GMT, Ctrl->In.file, Grid->header, true), Ctrl->In.file);
	type[GMT_IN] = Grid->header->type;
	strncpy (fname[GMT_IN], HH->name, GMT_BUFSIZ);
	gmt_M_err_fail (GMT, gmt_grd_get_format (GMT, Ctrl->G.file, Grid->header, false), Ctrl->G.file);
	type[GMT_OUT] = Grid->header->type;
	strncpy (fname[GMT_OUT], HH->name, GMT_BUFSIZ);
	gmt_free_grid (GMT, &Grid, true);	/* Free temp grid, Grid is now NULL */

	if (type[GMT_OUT] == GMT_GRID_IS_SD) {
		/* Golden Surfer format 7 is read-only */
		GMT_Report (API, GMT_MSG_ERROR, "Writing unsupported: %s\n", GMT->session.grdformat[GMT_GRID_IS_SD]);
		Return (GMT_RUNTIME_ERROR);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_WARNING)) {
		if (Ctrl->In.file[0] == '=') strcpy (fname[GMT_IN], "<stdin>");
		if (Ctrl->G.file[0] == '=') strcpy (fname[GMT_OUT], "<stdout>");
		GMT_Report (API, GMT_MSG_INFORMATION, "Translating file %s (format %s) to file %s (format %s)\n",
		            fname[GMT_IN], GMT->session.grdformat[type[GMT_IN]], fname[GMT_OUT], GMT->session.grdformat[type[GMT_OUT]]);
		if (hmode && GMT->session.grdformat[type[GMT_OUT]][0] != 'c' && GMT->session.grdformat[type[GMT_OUT]][0] != 'n')
			GMT_Report (API, GMT_MSG_WARNING, "No grd header will be written\n");
	}

	if ((Grid = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}
	HH = gmt_get_H_hidden (Grid->header);

	if (GMT->common.R.active[RSET]) {	/* Specified a subset */
		bool global = false;
		double noise[2];
		noise[GMT_X] = GMT_CONV4_LIMIT * Grid->header->inc[GMT_X];	/* Tolerate a bit of slop */
		noise[GMT_Y] = GMT_CONV4_LIMIT * Grid->header->inc[GMT_Y];
		global = gmt_grd_is_global (GMT, Grid->header);
		if (!global && (GMT->common.R.wesn[XLO] < (Grid->header->wesn[XLO]-noise[GMT_X]) || GMT->common.R.wesn[XHI] > (Grid->header->wesn[XHI]+noise[GMT_X]))) error++;
		if (GMT->common.R.wesn[YLO] < (Grid->header->wesn[YLO]-noise[GMT_Y]) || GMT->common.R.wesn[YHI] > (Grid->header->wesn[YHI]+noise[GMT_Y])) error++;
		if (error) {
			GMT_Report (API, GMT_MSG_ERROR, "Subset exceeds data domain!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, GMT->common.R.wesn, Ctrl->In.file, Grid) == NULL) {
			Return (API->error);	/* Get subset */
		}
	}
	else if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, Ctrl->In.file, Grid) == NULL) {
		Return (API->error);	/* Get all */
	}

	if (gmt_M_is_cartesian (GMT, GMT_IN)) {	/* Check in case grid really is geographic */
		if (gmt_M_360_range (Grid->header->wesn[XLO], Grid->header->wesn[XHI]) && gmt_M_180_range (Grid->header->wesn[YLO], Grid->header->wesn[YHI])) {
			GMT_Report (API, GMT_MSG_WARNING, "Input grid says it is Cartesian but has exactly 360 by 180 degree range.\n");
			GMT_Report (API, GMT_MSG_WARNING, "Use -fg to ensure the output grid will be identified as geographic.\n");
		}
	}
	Grid->header->type = type[GMT_OUT];

	/* When converting from netcdf to netcdf, we will keep the old command, so we need to make a copy of it now */
	command[0] = '\n';	command[1] = '\t';
	strcat(command, "(old cmd) ");
	strncat(command, Grid->header->command, GMT_GRID_COMMAND_LEN320-13);

	gmt_grd_init (GMT, Grid->header, options, true);

	if (!GMT->common.R.active[RSET] && ((type[GMT_IN]  >= GMT_GRID_IS_CB && type[GMT_IN]  <= GMT_GRID_IS_CD)  ||	/* That is, from netCDF to netCDF */
	                              (type[GMT_IN]  >= GMT_GRID_IS_NB && type[GMT_IN]  <= GMT_GRID_IS_ND)) &&
	                             ((type[GMT_OUT] >= GMT_GRID_IS_CB && type[GMT_OUT] <= GMT_GRID_IS_CD)  ||
	                              (type[GMT_OUT] >= GMT_GRID_IS_NB && type[GMT_OUT] <= GMT_GRID_IS_ND)) ) {
		/* Do nothing, which means the new grid will keep the command string of the old grid */
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_COMMAND, command, Grid))
			Return (API->error);
	}
	else if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid))
		Return (API->error);

	if (gmt_M_is_geographic (GMT, GMT_IN) && gmt_M_is_cartesian (GMT, GMT_OUT)) {	/* Force a switch from geographic to Cartesian */
		HH->grdtype = GMT_GRID_CARTESIAN;
		strcpy (Grid->header->x_units, "x_units");
		strcpy (Grid->header->y_units, "y_units");
	}
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, hmode, NULL, Ctrl->G.file, Grid) != GMT_NOERROR)
		Return (API->error);

	Return (GMT_NOERROR);
}
