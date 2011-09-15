/*-----------------------------------------------------------------
 *	$Id$
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
/* x2sys_datalist will read one or several data files and dump their
 * contents to stdout in ascii or binary (double precision) mode.
 * Input data file formats are determined by the definition file
 * given by the -D option.
 *
 * Author:	Paul Wessel
 * Date:	15-JUN-2004
 * Version:	1.1, based on the spirit of the old xsystem code
 *
 */

#include "x2sys.h"

struct X2SYS_DATALIST_CTRL {
	struct A {	/* -A */
		GMT_LONG active;
	} A;
	struct F {	/* -F */
		GMT_LONG active;
		char *flags;
	} F;
	struct L {	/* -L */
		GMT_LONG active;
		char *file;
	} L;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
		char *TAG;
	} T;
};

struct X2SYS_ADJUST {
	GMT_LONG n;
	double *d, *c;
};

void *New_x2sys_datalist_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_DATALIST_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_DATALIST_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return ((void *)C);
}

void Free_x2sys_datalist_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_DATALIST_CTRL *C) {	/* Deallocate control structure */
	if (C->F.flags) free ((void *)C->F.flags);
	if (C->L.file) free ((void *)C->L.file);
	if (C->T.TAG) free ((void *)C->T.TAG);
	GMT_free (GMT, C);
}

GMT_LONG GMT_x2sys_datalist_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "x2sys_datalist %s - Extract content of track data files\n\n", X2SYS_VERSION);
	GMT_message (GMT, "usage: x2sys_datalist <files> -T<TAG> [-A] [-F<fields>] [-L[<corrtable.txt>]]\n");
	GMT_message (GMT, "\t[%s] [-S] [%s] [%s] [-m]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_bo_OPT);
	
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);
	
	GMT_message (GMT, "\t<files> is one or more datafiles, or give =<files.lis> for a file with a list of datafiles.\n");
	GMT_message (GMT, "\t-T <TAG> is the system tag for the data set.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Use any adjustment splines per track to redistribute COEs between tracks\n");
	GMT_message (GMT, "\t   according to their relative weight [no adjustments].\n");
	GMT_message (GMT, "\t-F Comma-separated list of column names to output [Default are all fields].\n");
	GMT_message (GMT, "\t-L Subtract systematic corrections from the data. If no correction file is given,\n");
	GMT_message (GMT, "\t   the default file <TAG>_corrections.txt in $X2SYS_HOME/<TAG> is assumed.\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-S Suppress output records where all data columns are NaN [Output all records].\n");
	GMT_explain_options (GMT, "VD");
	GMT_message (GMT, "\t-m Write a multi-segment header between the output from each file.\n");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_x2sys_datalist_parse (struct GMTAPI_CTRL *C, struct X2SYS_DATALIST_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */
			
			case 'A':
				Ctrl->A.active = TRUE;
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				Ctrl->F.flags = strdup (opt->arg);
				break;
			case 'L':	/* Crossover correction table */
				Ctrl->L.active = TRUE;
				Ctrl->L.file = strdup (opt->arg);
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				Ctrl->T.TAG = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LONG x2sys_load_adjustments (struct GMT_CTRL *GMT, char *DIR, char *TAG, char *track, char *column, struct X2SYS_ADJUST **A)
{
	GMT_LONG n_fields, n_expected_fields = 2, n = 0, k, type[2] = {GMT_IS_FLOAT, GMT_IS_FLOAT}, n_alloc = GMT_CHUNK;
	double *in = NULL;
	char file[GMT_BUFSIZ];
	FILE *fp = NULL;
	struct X2SYS_ADJUST *adj = NULL;
	
	sprintf (file, "%s/%s/%s.%s.adj", DIR, TAG, track, column);
	if ((fp = GMT_fopen (GMT, file, "r")) == NULL) return FALSE;	/* Nuthin' to read */
	
	adj = GMT_memory (GMT, NULL, 1, struct X2SYS_ADJUST);
	adj->d = GMT_memory (GMT, NULL, n_alloc, double);
	adj->c = GMT_memory (GMT, NULL, n_alloc, double);
	for (k = 0; k < 2; k++) l_swap (type[k], GMT->current.io.col_type[GMT_IN][k]);	/* Save original input type setting */
	while ((n_fields = GMT->current.io.input (GMT, fp, &n_expected_fields, &in)) >= 0 && !(GMT->current.io.status & GMT_IO_EOF)) {	/* Not yet EOF */
		adj->d[n] = in[0];
		adj->c[n] = in[1];
		n++;
		if (n == n_alloc) {
			n_alloc <<= 1;
			adj->d = GMT_memory (GMT, adj->d, n_alloc, double);
			adj->c = GMT_memory (GMT, adj->c, n_alloc, double);
		}
	}
	GMT_fclose (GMT, fp);
	adj->d = GMT_memory (GMT, adj->d, n, double);
	adj->c = GMT_memory (GMT, adj->c, n, double);
	adj->n = n;
	*A = adj;
	for (k = 0; k < 2; k++) l_swap (GMT->current.io.col_type[GMT_IN][k], type[k]);	/* Restore original input type setting */
	return (TRUE);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_x2sys_datalist_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_x2sys_datalist (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	char *sfile, *def = "x2sys", **trk_name = NULL;

	GMT_LONG i, j, k, bad, trk_no, n_tracks, n_data_col_out = 0;
	GMT_LONG error = FALSE,  cmdline_files, special_formatting = FALSE, *adj_col = NULL;

	double **data = NULL, *out = NULL, correction = 0.0, aux_dvalue[N_GENERIC_AUX];
	double ds = 0.0, cumulative_dist, dist_scale = 1.0, dt, vel_scale = 1.0, adj_amount;

	struct X2SYS_INFO *s = NULL;
	struct X2SYS_FILE_INFO p;		/* File information */
	struct X2SYS_BIX B;
	struct MGD77_CORRTABLE **CORR = NULL;
	struct MGD77_AUX_INFO aux[N_MGD77_AUX];
	struct MGD77_AUXLIST auxlist[N_GENERIC_AUX] = {
		{ "dist",    MGD77_AUX_DS, 0, 0, "d(km)"},
		{ "azim",    MGD77_AUX_AZ, 0, 0, "azimuth"},
		{ "vel",     MGD77_AUX_SP, 0, 0, "v(m/s)"}
	};
	struct X2SYS_ADJUST **A = NULL;
	struct X2SYS_DATALIST_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_x2sys_datalist_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_x2sys_datalist_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_x2sys_datalist", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VfRb", ">", options))) Return (error);
	Ctrl = (struct X2SYS_DATALIST_CTRL *)New_x2sys_datalist_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_datalist_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the x2sys_datalist main code ----------------------------*/

	if ((n_tracks = x2sys_get_tracknames (GMT, options, &trk_name, &cmdline_files)) == 0) {
		GMT_report (GMT, GMT_MSG_FATAL, "No datafiles given!\n");
		Return (EXIT_FAILURE);		
	}

	sfile = def;

	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);

	if (Ctrl->F.flags) x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->F.flags, s), "-F");

	s->ascii_out = !GMT->common.b.active[1];

	if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, B.wesn, 4, double);

	if (GMT->common.R.active) {
		/* Supply dummy linear proj */
		GMT->current.proj.projection = GMT->current.proj.xyz_projection[0] = GMT->current.proj.xyz_projection[1] = GMT_LINEAR;
		GMT->current.proj.pars[0] = GMT->current.proj.pars[1] = 1.0;
		GMT->common.J.active = TRUE;
		if (GMT->common.R.wesn[XLO] < 0.0 && GMT->common.R.wesn[XHI] < 0.0) {
			GMT->common.R.wesn[XLO] += 360.0;
			GMT->common.R.wesn[XHI] += 360.0;
		}
		GMT_err_fail (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "");
	}

	out = GMT_memory (GMT, NULL, s->n_fields, double);

	for (i = 0; i < s->n_out_columns; i++) {	/* Set output formats */
		if (i == s->t_col)
			GMT->current.io.col_type[GMT_OUT][i] = GMT_IS_ABSTIME;
		else if (i == s->x_col)
			GMT->current.io.col_type[GMT_OUT][i] = (!strcmp (s->info[s->out_order[i]].name, "lon")) ? GMT_IS_LON : GMT_IS_FLOAT;
		else if (i == s->y_col)
			GMT->current.io.col_type[GMT_OUT][i] = (!strcmp (s->info[s->out_order[i]].name, "lat")) ? GMT_IS_LAT : GMT_IS_FLOAT;
		else
			GMT->current.io.col_type[GMT_OUT][i] = GMT_IS_FLOAT;

		if (s->info[s->out_order[i]].format[0] != '-') special_formatting = TRUE;
	}
	if (GMT->common.b.active[GMT_OUT]) special_formatting = FALSE;

	if (Ctrl->S.active) {	/* Must count output data columns (except t, x, y) */
		for (i = n_data_col_out = 0; i < s->n_out_columns; i++) {
			if (s->out_order[i] == s->t_col) continue;
			if (s->out_order[i] == s->x_col) continue;
			if (s->out_order[i] == s->y_col) continue;
			n_data_col_out++;
		}
	}

	MGD77_Set_Unit (GMT, s->unit[X2SYS_DIST_SELECTION], &dist_scale, -1);	/* Gets scale which multiplies meters to chosen distance unit */
	MGD77_Set_Unit (GMT, s->unit[X2SYS_SPEED_SELECTION], &vel_scale,  -1);	/* Sets output scale for distances using in velocities */
	switch (s->unit[X2SYS_SPEED_SELECTION][0]) {
		case 'c':
			vel_scale = 1.0;
			break;
		case 'e':
			vel_scale /= dist_scale;			/* Must counteract any distance scaling to get meters. dt is in sec so we get m/s */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(m/s)");
			break;
		case 'f':
			vel_scale /= (METERS_IN_A_FOOT * dist_scale);		/* Must counteract any distance scaling to get feet. dt is in sec so we get ft/s */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(ft/s)");
			break;
		case 'k':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get km. dt is in sec so 3600 gives km/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(km/hr)");
			break;
#ifdef GMT_COMPAT
		case 'm':
			GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Unit m for miles is deprecated; use unit M instead\n");
#endif
		case 'M':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get miles. dt is in sec so 3600 gives miles/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(mi/hr)");
			break;
		case 'n':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get miles. dt is in sec so 3600 gives miles/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(kts)");
			break;
	}
	switch (s->unit[X2SYS_DIST_SELECTION][0]) {
		case 'c':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(user)");
			break;
		case 'e':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(m)");
			break;
		case 'f':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(feet)");
			break;
		case 'k':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(km)");
			break;
#ifdef GMT_COMPAT
		case 'm':
			GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Unit m for miles is deprecated; use unit M instead\n");
#endif
		case 'M':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(miles)");
			break;
		case 'n':
			strcpy (auxlist[MGD77_AUX_SP].header, "d(nm)");
			break;
	}

	GMT_init_distaz (GMT, s->dist_flag ? GMT_MAP_DIST_UNIT : 'X', s->dist_flag, GMT_MAP_DIST);
	
	if (Ctrl->L.active) {	/* Load an ephemeral correction table */
		x2sys_get_corrtable (GMT, s, Ctrl->L.file, n_tracks, trk_name, NULL, aux, auxlist, &CORR);
		if (auxlist[MGD77_AUX_SP].requested && s->t_col == -1) {
			GMT_report (GMT, GMT_MSG_FATAL, "Selected correction table requires velocity which implies time (not selected)\n");
			MGD77_Free_Correction (GMT, CORR, n_tracks);
			x2sys_free_list (GMT, trk_name, n_tracks);
			exit (EXIT_FAILURE);
		}
	}

	if (Ctrl->A.active) {
		A = GMT_memory (GMT, NULL, s->n_out_columns, struct X2SYS_ADJUST *);
		adj_col = GMT_memory (GMT, NULL, s->n_out_columns, GMT_LONG);
	}
	
	for (trk_no = 0; trk_no < n_tracks; trk_no++) {

		GMT_report (GMT, GMT_MSG_NORMAL, "Reading track %s\n", trk_name[trk_no]);

		x2sys_err_fail (GMT, (s->read_file) (GMT, trk_name[trk_no], &data, s, &p, &GMT->current.io, &k), trk_name[trk_no]);

		if (Ctrl->L.active && s->t_col >= 0) MGD77_Init_Correction (GMT, CORR[trk_no], data);	/* Initialize origins if needed */

		if (Ctrl->A.active) {
			for (k = 0; k < s->n_out_columns; k++) adj_col[k] = x2sys_load_adjustments (GMT, X2SYS_HOME, Ctrl->T.TAG, trk_name[trk_no], s->info[s->out_order[k]].name, &A[k]);
		}

		if (GMT->current.io.multi_segments[GMT_OUT]) GMT_write_segmentheader (GMT, GMT->session.std[GMT_OUT], s->n_fields);

		cumulative_dist = 0.0;
		for (j = 0; j < p.n_rows; j++) {
			if (GMT->common.R.active && GMT_map_outside (GMT, data[s->x_col][j], data[s->y_col][j])) continue;
			if (Ctrl->S.active) {
				for (k = bad = 0; k < s->n_out_columns; k++) {
					if (s->out_order[k] == s->t_col) continue;
					if (s->out_order[k] == s->x_col) continue;
					if (s->out_order[k] == s->y_col) continue;
					if (GMT_is_dnan (data[s->out_order[k]][j])) bad++;
				}
				if (bad == n_data_col_out) continue;
			}
			if (auxlist[MGD77_AUX_AZ].requested) {
				if (j == 0)	/* Look forward at first point to get an azimuth */
					aux_dvalue[MGD77_AUX_AZ] = GMT_az_backaz (GMT, data[s->x_col][1], data[s->y_col][1], data[s->x_col][0], data[s->y_col][0], FALSE);
				else		/* else go from previous to this point */
					aux_dvalue[MGD77_AUX_AZ] = GMT_az_backaz (GMT, data[s->x_col][j], data[s->y_col][j], data[s->x_col][j-1], data[s->y_col][j-1], FALSE);
			}
			if (auxlist[MGD77_AUX_DS].requested) {
				ds = (j == 0) ? 0.0 : dist_scale * GMT_distance (GMT, data[s->x_col][j], data[s->y_col][j], data[s->x_col][j-1], data[s->y_col][j-1]);
				cumulative_dist += ds;
				aux_dvalue[MGD77_AUX_DS] = cumulative_dist;
			}
			if (auxlist[MGD77_AUX_SP].requested) {
				dt =  (j == 0) ? data[s->t_col][1] - data[s->t_col][0] : data[s->t_col][j] - data[s->t_col][j-1];
				aux_dvalue[MGD77_AUX_SP] = (GMT_is_dnan (dt) || dt == 0.0) ? GMT->session.d_NaN : vel_scale * ds / dt;
			}
			for (k = 0; k < s->n_out_columns; k++) {	/* Load output record */
				correction = (Ctrl->L.active) ? MGD77_Correction (GMT, CORR[trk_no][k].term, data, aux_dvalue, j) : 0.0;
				if (Ctrl->A.active && adj_col[k]) {
					if (GMT_intpol (GMT, A[k]->d, A[k]->c, A[k]->n, 1, &aux_dvalue[MGD77_AUX_DS], &adj_amount, GMT->current.setting.interpolant)) {
						GMT_report (GMT, GMT_MSG_FATAL, "Error interpolating adjustment for %s near row %ld - no adjustment made!\n", s->info[s->out_order[k]].name, j);
						adj_amount = 0.0;
					}
					correction -= adj_amount;
				}
				out[k] = data[s->out_order[k]][j] - correction;	/* This loads out in the correct output order */
			}
			if (special_formatting)  {	/* use the specified formats */
				for (k = 0; k < s->n_out_columns; k++) {
					if (s->info[s->out_order[k]].format[0] == '-')
						GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], out[k], k);
					else {
						if (!GMT_is_dnan (out[k]))
							GMT_fprintf (GMT->session.std[GMT_OUT], s->info[s->out_order[k]].format, out[k]);
						else
							GMT_fprintf (GMT->session.std[GMT_OUT], "NaN");
					}
					(k == (s->n_out_columns - 1)) ? GMT_fprintf (GMT->session.std[GMT_OUT], "\n") : GMT_fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
				}
			}
			else {
				GMT->current.io.output (GMT, GMT->session.std[GMT_OUT], s->n_out_columns, out);
			}
		}

		x2sys_free_data (GMT, data, s->n_out_columns, &p);
		for (k = 0; k < s->n_out_columns; k++) if (Ctrl->A.active && adj_col[k]) {
			GMT_free (GMT, A[k]->d);
			GMT_free (GMT, A[k]->c);
			GMT_free (GMT, A[k]);
		}
	}

	if (Ctrl->L.active) MGD77_Free_Correction (GMT, CORR, n_tracks);

	x2sys_end (GMT, s);
	GMT_free (GMT, out);
	if (Ctrl->A.active) {
		GMT_free (GMT, A);
		GMT_free (GMT, adj_col);
	}
	x2sys_free_list (GMT, trk_name, n_tracks);
	
	Return (GMT_OK);
}
