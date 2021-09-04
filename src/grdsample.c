/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * API functions to support the grdsample application.
 *
 * Brief synopsis: grdsample reads a grid file and evaluates the grid at new grid
 * positions specified by new dx/dy values using a 2-D Taylor expansion of order 3.
 * In order to evaluate derivatives along the edges of the surface, I assume
 * natural bicubic spline conditions, i.e. both the second and third normal
 * derivatives are zero, and that the dxdy derivative in the corners are zero, too.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdsample"
#define THIS_MODULE_MODERN_NAME	"grdsample"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Resample a grid onto a new lattice"
#define THIS_MODULE_KEYS	"<G{,GG}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-RVfnr" GMT_ADD_x_OPT GMT_OPT("FQ")

struct GRDSAMPLE_CTRL {
	struct GRDSAMPLE_In {
		bool active;
		char *file;
	} In;
	struct GRDSAMPLE_G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct GRDSAMPLE_I {	/* -I (for checking only) */
		bool active;
	} I;
	struct GRDSAMPLE_T {	/* -T */
		bool active;
	} T;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDSAMPLE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDSAMPLE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDSAMPLE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s %s -G%s [%s] [%s] [-T] [%s] [%s] [%s] [%s]%s[%s]\n",
		name, GMT_INGRID, GMT_OUTGRID, GMT_I_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_n_OPT, GMT_r_OPT, GMT_x_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Name of input grid to be resampled");
	gmt_outgrid_syntax (API, 'G', "Set the name of the interpolated output grid");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Option (API, "I");
	if (gmt_M_showusage (API)) GMT_Usage (API, -2, "If -I is not given then grid spacing is copied from the input grid.");
	GMT_Usage (API, 1, "\n%s", GMT_Rgeo_OPT);
	GMT_Usage (API, -2, "Specify a subregion [Default is input grid region].");
	GMT_Usage (API, 1, "\n-T Translate between grid registration and pixel registration. "
		"Implies resampling halfway between nodes (a destructive change to the grid).");
	GMT_Option (API, "V,f,n,r,x,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDSAMPLE_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdsample and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int n_errors = 0, n_files = 0, ii = 0, jj = 0;
	char format[GMT_BUFSIZ];
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) {n_errors++; continue; }
				Ctrl->In.active = true;
				if (opt->arg[0]) Ctrl->In.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file))) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'G':	/* Output file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file))) n_errors++;
				break;
			case 'I':	/* Grid spacings */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'L':	/* BCs */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -L is deprecated; -n+b%s was set instead, use this in the future.\n", opt->arg);
					/* coverity[buffer_size_warning] */	/* Do not remove this comment */
					gmt_strncpy (GMT->common.n.BC, opt->arg, 4U);
					/* We turn on geographic coordinates if -Lg is given by faking -fg */
					/* But since GMT_parse_f_option is private to gmt_init and all it does */
					/* in this case are 2 lines below we code it here */
					if (!strcmp (GMT->common.n.BC, "g")) {
						gmt_set_geographic (GMT, GMT_IN);
						gmt_set_geographic (GMT, GMT_OUT);
					}
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'N':	/* Backwards compatible.  n_columns/n_rows can now be set with -I */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -N<n_columns>/<n_rows> is deprecated; use -I<n_columns>+/<n_rows>+ instead.\n");
					sscanf (opt->arg, "%d/%d", &ii, &jj);
					if (jj == 0) jj = ii;
					sprintf (format, "%d+/%d+", ii, jj);
					n_errors += gmt_parse_inc_option (GMT, 'I', format);
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'T':	/* Convert from pixel file <-> gridfile */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				if (GMT->common.R.active[FSET]) GMT->common.R.active[GSET] = false;	/* Override any implicit -r via -Rgridfile */
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, (n_files != 1), "Must specify a single input grid file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Option -G: Must specify output file\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[GSET] && Ctrl->T.active && !GMT->common.R.active[FSET],
	                                   "Only one of -r, -T may be specified\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[ISET] && (GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0),
	                                   "Option -I: Must specify positive increments\n");
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grdsample (void *V_API, int mode, void *args) {

	int error = 0, row, col;
	unsigned int registration;

	uint64_t ij;

	char format[GMT_BUFSIZ];

	double *lon = NULL, lat, wesn_i[4], wesn_o[4], inc[2];

	struct GRDSAMPLE_CTRL *Ctrl = NULL;
	struct GMT_GRID *Gin = NULL, *Gout = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
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

	/*---------------------------- This is the grdsample main code ----------------------------*/

	gmt_grd_set_datapadding (GMT, true);	/* Turn on gridpadding when reading a subset */

	gmt_enable_threads (GMT);	/* Set number of active threads, if supported */
	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input grid\n");
	gmt_set_pad (GMT, 2U);	/* Ensure space for BCs in case an API passed pad == 0 */
	if ((Gin = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	/* There are two separate regions that we need to worry about and ensure they are consistent when a user
	 * selects a subset. This is because the grid region and the desired region may not only be different, but
	 * may be phase-shifted. That means a single w/e/s/n cannot be used both to specify a subset for READING and
	 * also as an OUTPUT region.  Thus, below we create two regions:
	 * wesn_i: This is initially the input grid's domain.  If no -R is given then that is what we will read.
	 * wesn_o: This is the region given by -R.  If no -R then it is the same as wesn_i.
	 * If no -R is given then wesn_i == wesn_o and (presumably) there is only differences in inc and registration.
	 * If there is a -R, then we adjust wesn_i and wesn_o thus:
	 *   wesn_i: We move the boundaries inwards by the grid's own increments until the bounds are equal to or inside the -R.
	 *			 Then, if we are inside and we are able to move one step outwards we do so to ensure we cover the output range.
	 *   wesn_o: We move the boundaries inwards by the -Iinc spacing as long as we are outside the input grid.
	 */

	gmt_M_memcpy (wesn_i, Gin->header->wesn, 4, double);	/* wesn_i is eventually the subset we will read from this grid */
	gmt_M_memcpy (wesn_o, (GMT->common.R.active[RSET] ? GMT->common.R.wesn : Gin->header->wesn), 4, double);	/* wesn_o is the region we want for the output [Same as input] */
	gmt_M_memcpy (inc, (GMT->common.R.active[ISET] ? GMT->common.R.inc : Gin->header->inc), 2, double);	/* Either a new increment is given or we use the one from the input grid */

	/* Set the registration for the output via -R, -r or default input grid */
	if (Ctrl->T.active)
		registration = !Gin->header->registration;
	else if (GMT->common.R.active[GSET])
		registration = GMT->common.R.registration;
	else
		registration = Gin->header->registration;

	gmt_increment_adjust (GMT, wesn_o, inc, registration);	/* In case user specified incs using distance units we must call this here before adjusting wesn_o */

	if (GMT->common.R.active[RSET]) {		/* Gave -R */
		bool wrap_360_i = (gmt_M_x_is_lon (GMT, GMT_IN) && gmt_M_360_range (Gin->header->wesn[XLO], Gin->header->wesn[XHI]));
		bool wrap_360_o = (gmt_M_x_is_lon (GMT, GMT_OUT) && gmt_M_360_range (wesn_o[XLO], wesn_o[XHI]));
		unsigned int k;

		/* Adjust wesn used to READ subset unless 360 geographic */
		if (!wrap_360_i) {
			k = 0;
			while (wesn_i[YLO] < wesn_o[YLO]) wesn_i[YLO] += Gin->header->inc[GMT_Y], k++;	/* Now on or inside grid boundary */
			if (wesn_i[YLO] > Gin->header->wesn[YLO] && k) wesn_i[YLO] -= Gin->header->inc[GMT_Y];	/* Now exactly on boundary or just outside but still inside input grid south boundary */
			k = 0;
			while (wesn_i[YHI] > wesn_o[YHI]) wesn_i[YHI] -= Gin->header->inc[GMT_Y], k++;	/* Now on or inside boundary */
			if (wesn_i[YHI] < Gin->header->wesn[YHI] && k) wesn_i[YHI] += Gin->header->inc[GMT_Y];	/* Now exactly on boundary or just outside but still inside input grid north boundary */
		}
		if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* Must carefully check the longitude overlap between grid and -R */
			int shift = 0;
			if (Gin->header->wesn[XHI] < wesn_i[XLO]) shift += 360;
			else if (Gin->header->wesn[XLO] > wesn_i[XHI]) shift -= 360;
			else if (wrap_360_i && wesn_i[XHI] > Gin->header->wesn[XHI]) shift += 360;
			else if (wrap_360_i && wesn_i[XLO] < Gin->header->wesn[XLO]) shift -= 360;
			if (shift) {	/* Must modify header to match -R */
				Gin->header->wesn[XLO] += shift, Gin->header->wesn[XHI] += shift;
				wesn_o[XLO] += shift, wesn_o[XHI] += shift;
				GMT_Report (API, GMT_MSG_INFORMATION, "File %s region needed a %d longitude adjustment to fit final grid region\n", Ctrl->In.file, shift);
			}
		}
		if (!wrap_360_o && !wrap_360_i) {	/* Can only shrink wesn_i if there is no 360-wrapping going on */
			k = 0;
			while (wesn_i[XLO] < wesn_o[XLO]) wesn_i[XLO] += Gin->header->inc[GMT_X], k++;	/* Now on or inside boundary */
			if (wesn_i[XLO] > Gin->header->wesn[XLO] && k) wesn_i[XLO] -= Gin->header->inc[GMT_X];	/* Now exactly on boundary or just outside but still inside input grid south boundary */
			k = 0;
			while (wesn_i[XHI] > wesn_o[XHI]) wesn_i[XHI] -= Gin->header->inc[GMT_X], k++;	/* Now on or inside boundary */
			if (wesn_i[XHI] < Gin->header->wesn[XHI] && k) wesn_i[XHI] += Gin->header->inc[GMT_X];	/* Now exactly on boundary or just outside but still inside input grid south boundary */
		}

		/* Adjust wesn_o used to CREATE the output grid: It cannot exceed the input grid bounds */
		while (wesn_o[YLO] < Gin->header->wesn[YLO]) wesn_o[YLO] += inc[GMT_Y];	/* Now on or inside boundary */
		while (wesn_o[YHI] > Gin->header->wesn[YHI]) wesn_o[YHI] -= inc[GMT_Y];	/* Now on or inside boundary */
		if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* Must carefully check the longitude overlap */
			int shift = 0;
			if (Gin->header->wesn[XHI] < wesn_o[XLO]) shift += 360;
			else if (Gin->header->wesn[XLO] > wesn_o[XHI]) shift -= 360;
			else if (wrap_360_i && wesn_o[XHI] > Gin->header->wesn[XHI]) shift += 360;
			else if (wrap_360_i && wesn_o[XLO] < Gin->header->wesn[XLO]) shift -= 360;
			if (shift) {	/* Must modify header */
				Gin->header->wesn[XLO] += shift, Gin->header->wesn[XHI] += shift;
				GMT_Report (API, GMT_MSG_INFORMATION, "File %s region needed a %d longitude adjustment to fit final grid region\n", Ctrl->In.file, shift);
			}
		}
		if (!wrap_360_o && !wrap_360_i) {	/* Can only shrink wesn_o if there is no 360-wrapping going on */
			while (wesn_o[XLO] < Gin->header->wesn[XLO]) wesn_o[XLO] += inc[GMT_X];	/* Now on or inside boundary */
			while (wesn_o[XHI] > Gin->header->wesn[XHI]) wesn_o[XHI] -= inc[GMT_X];	/* Now on or inside boundary */
		}
	}

	/* Here, wesn_i is compatible with the INPUT grid so we can read the subset from which we will resample.
	 * Here, wesn_o is the region we wish to use when creating the output grid */

	if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, wesn_o, inc, \
		registration, GMT_NOTSET, NULL)) == NULL) Return (API->error);

	sprintf (format, "Input  grid (%s/%s/%s/%s) n_columns = %%d n_rows = %%d dx = %s dy = %s registration = %%d\n",
		GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);

	GMT_Report (API, GMT_MSG_INFORMATION, format, Gin->header->wesn[XLO], Gin->header->wesn[XHI],
		Gin->header->wesn[YLO], Gin->header->wesn[YHI], Gin->header->n_columns, Gin->header->n_rows,
		Gin->header->inc[GMT_X], Gin->header->inc[GMT_Y], Gin->header->registration);

	memcpy (&format, "Output", 6);

	GMT_Report (API, GMT_MSG_INFORMATION, format, Gout->header->wesn[XLO], Gout->header->wesn[XHI],
		Gout->header->wesn[YLO], Gout->header->wesn[YHI], Gout->header->n_columns, Gout->header->n_rows,
		Gout->header->inc[GMT_X], Gout->header->inc[GMT_Y], Gout->header->registration);

	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn_i, Ctrl->In.file, Gin) == NULL) {	/* Get subset */
		Return (API->error);
	}

	if (Gout->header->inc[GMT_X] > Gin->header->inc[GMT_X])
		GMT_Report (API, GMT_MSG_WARNING, "Output sampling interval in x exceeds input interval and may lead to aliasing.\n");
	if (Gout->header->inc[GMT_Y] > Gin->header->inc[GMT_Y])
		GMT_Report (API, GMT_MSG_WARNING, "Output sampling interval in y exceeds input interval and may lead to aliasing.\n");

	/* Precalculate longitudes from the output grid layout */

	HH = gmt_get_H_hidden (Gin->header);
	lon = gmt_M_memory (GMT, NULL, Gout->header->n_columns, double);
	for (col = 0; col < (int)Gout->header->n_columns; col++) {
		lon[col] = gmt_M_grd_col_to_x (GMT, col, Gout->header);
		if (!HH->nxp)
			/* Nothing */;
		else if (lon[col] > Gin->header->wesn[XHI])
			lon[col] -= Gin->header->inc[GMT_X] * HH->nxp;
		else if (lon[col] < Gin->header->wesn[XLO])
			lon[col] += Gin->header->inc[GMT_X] * HH->nxp;
	}

	/* Loop over input point and estimate output values */

	Gout->header->z_min = FLT_MAX; Gout->header->z_max = -FLT_MAX;	/* Min/max for out */

#ifdef _OPENMP
#pragma omp parallel for private(row,col,lat,ij) shared(GMT,Gin,Gout,lon)
#endif
	for (row = 0; row < (int)Gout->header->n_rows; row++) {
		lat = gmt_M_grd_row_to_y (GMT, row, Gout->header);
		if (!HH->nyp)
			/* Nothing */;
		else if (lat > Gin->header->wesn[YHI])
			lat -= Gin->header->inc[GMT_Y] * HH->nyp;
		else if (lat < Gin->header->wesn[YLO])
			lat += Gin->header->inc[GMT_Y] * HH->nyp;
		for (col = 0; col < (int)Gout->header->n_columns; col++) {
			ij = gmt_M_ijp (Gout->header, row, col);
			Gout->data[ij] = (gmt_grdfloat)gmt_bcr_get_z (GMT, Gin, lon[col], lat);
			if (Gout->data[ij] < Gout->header->z_min) Gout->header->z_min = Gout->data[ij];
			if (Gout->data[ij] > Gout->header->z_max) Gout->header->z_max = Gout->data[ij];
		}
	}
	gmt_M_free (GMT, lon);

	if (!GMT->common.n.truncate && (Gout->header->z_min < Gin->header->z_min || Gout->header->z_max > Gin->header->z_max)) {	/* Report and possibly truncate output to input extrama */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Output grid extrema [%g/%g] exceeds extrema of input grid [%g/%g]; to clip output use -n...+c""\n",
			Gout->header->z_min, Gout->header->z_max, Gin->header->z_min, Gin->header->z_max);
	}

	gmt_set_pad (GMT, API->pad);	/* Reset to session default pad before output */

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Gout)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Gout) != GMT_NOERROR) {
		Return (API->error);
	}

	Return (GMT_NOERROR);
}
