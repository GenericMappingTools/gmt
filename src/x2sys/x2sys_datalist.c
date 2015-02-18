/*-----------------------------------------------------------------
 *	$Id$
 *
 *      Copyright (c) 1999-2015 by P. Wessel
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
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

#define THIS_MODULE_NAME	"x2sys_datalist"
#define THIS_MODULE_LIB		"x2sys"
#define THIS_MODULE_PURPOSE	"Extract content of track data files"
#define THIS_MODULE_KEYS	"LTi,ITi,>DO"

#include "x2sys.h"

#define GMT_PROG_OPTIONS "->RVbd"

struct X2SYS_DATALIST_CTRL {
	struct A {	/* -A */
		bool active;
	} A;
	struct E {	/* -E */
		bool active;
	} E;
	struct F {	/* -F */
		bool active;
		char *flags;
	} F;
	struct I {	/* -I */
		bool active;
		char *file;
	} I;
	struct L {	/* -L */
		bool active;
		char *file;
	} L;
	struct S {	/* -S */
		bool active;
	} S;
	struct T {	/* -T */
		bool active;
		char *TAG;
	} T;
};

struct X2SYS_ADJUST {
	uint64_t n;
	double *d, *c;
};

void *New_x2sys_datalist_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_DATALIST_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_DATALIST_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

void Free_x2sys_datalist_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_DATALIST_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->F.flags) free (C->F.flags);
	if (C->I.file) free (C->I.file);
	if (C->L.file) free (C->L.file);
	if (C->T.TAG) free (C->T.TAG);
	GMT_free (GMT, C);
}

int GMT_x2sys_datalist_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: x2sys_datalist <files> -T<TAG> [-A] [-E] [-F<fields>] [-L[<corrtable.txt>]] [-I<ignorelist>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-S] [%s] [%s] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_bo_OPT, GMT_do_OPT);
	
	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);
	
	GMT_Message (API, GMT_TIME_NONE, "\t<files> is one or more datafiles, or give =<files.lis> for a file with a list of datafiles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T <TAG> is the system tag for the data set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Use any adjustment splines per track to redistribute COEs between tracks\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   according to their relative weight [no adjustments].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Add segment headers with track names between separate file output [no added segment headers].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Comma-separated list of column names to output [Default are all fields].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I List of tracks to ignore [Use all tracks].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Subtract systematic corrections from the data. If no correction file is given,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the default file <TAG>_corrections.txt in $X2SYS_HOME/<TAG> is assumed.\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Suppress output records where all data columns are NaN [Output all records].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (Note: data columns exclude navigation (lon|x|lat|y|time) columns.)\n");
	GMT_Option (API, "V,bo,do,.");
	
	return (EXIT_FAILURE);
}

int GMT_x2sys_datalist_parse (struct GMT_CTRL *GMT, struct X2SYS_DATALIST_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */
			
			case 'A':
				Ctrl->A.active = true;
				break;
			case 'E':
				Ctrl->E.active = true;
				break;
			case 'F':
				Ctrl->F.active = true;
				Ctrl->F.flags = strdup (opt->arg);
				break;
			case 'I':
				if ((Ctrl->I.active = GMT_check_filearg (GMT, 'I', opt->arg, GMT_IN, GMT_IS_TEXTSET)))
					Ctrl->I.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'L':	/* Crossover correction table */
				if ((Ctrl->L.active = GMT_check_filearg (GMT, 'L', opt->arg, GMT_IN, GMT_IS_TEXTSET)))
					Ctrl->L.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'S':
				Ctrl->S.active = true;
				break;
			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.TAG = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.active && !Ctrl->F.flags, "Syntax error: -F must be given a comma-separated list of columns.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->I.file, "Syntax error: -I must be given a filename.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

bool x2sys_load_adjustments (struct GMT_CTRL *GMT, char *DIR, char *TAG, char *track, char *column, struct X2SYS_ADJUST **A)
{
	unsigned int k, type[2] = {GMT_IS_FLOAT, GMT_IS_FLOAT};
	int n_fields;
	uint64_t n = 0, n_expected_fields = 2;
	size_t n_alloc = GMT_CHUNK;
	double *in = NULL;
	char file[GMT_BUFSIZ] = {""};
	FILE *fp = NULL;
	struct X2SYS_ADJUST *adj = NULL;
	
	sprintf (file, "%s/%s/%s.%s.adj", DIR, TAG, track, column);
	if ((fp = GMT_fopen (GMT, file, "r")) == NULL) return false;	/* Nuthin' to read */
	
	adj = GMT_memory (GMT, NULL, 1, struct X2SYS_ADJUST);
	adj->d = GMT_memory (GMT, NULL, n_alloc, double);
	adj->c = GMT_memory (GMT, NULL, n_alloc, double);
	for (k = 0; k < 2; k++) uint_swap (type[k], GMT->current.io.col_type[GMT_IN][k]);	/* Save original input type setting */
	while ((in = GMT->current.io.input (GMT, fp, &n_expected_fields, &n_fields)) != NULL && !(GMT->current.io.status & GMT_IO_EOF)) {	/* Not yet EOF */
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
	for (k = 0; k < 2; k++) uint_swap (GMT->current.io.col_type[GMT_IN][k], type[k]);	/* Restore original input type setting */
	return (true);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_x2sys_datalist_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_x2sys_datalist (void *V_API, int mode, void *args)
{
	char **trk_name = NULL, **ignore = NULL;

	int error = 0, this_col, xpos = -1, ypos = -1, tpos = -1;
	bool cmdline_files, gmt_formatting = false, skip, *adj_col = NULL;
	unsigned int ocol, last_col, bad, trk_no, n_tracks, n_data_col_out = 0, k, n_ignore = 0;
	uint64_t row;

	double **data = NULL, *out = NULL, correction = 0.0, aux_dvalue[N_GENERIC_AUX];
	double ds = 0.0, cumulative_dist, dist_scale = 1.0, dt, vel_scale = 1.0, adj_amount;
	double t_scale;				/* Scale to give time in seconds */

	struct X2SYS_INFO *s = NULL;
	struct X2SYS_FILE_INFO p;		/* File information */
	struct X2SYS_BIX B;
	struct MGD77_CORRTABLE **CORR = NULL;
	struct MGD77_AUX_INFO aux[N_MGD77_AUX];
	struct MGD77_AUXLIST auxlist[N_GENERIC_AUX] = {
		{ "dist",    MGD77_AUX_DS, false, false, "d(km)"},
		{ "azim",    MGD77_AUX_AZ, false, false, "azimuth"},
		{ "vel",     MGD77_AUX_SP, false, false, "v(m/s)"}
	};
	struct X2SYS_ADJUST **A = NULL;
	struct X2SYS_DATALIST_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_x2sys_datalist_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_x2sys_datalist_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_x2sys_datalist_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_x2sys_datalist_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_datalist_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the x2sys_datalist main code ----------------------------*/

	if ((n_tracks = x2sys_get_tracknames (GMT, options, &trk_name, &cmdline_files)) == 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "No datafiles given!\n");
		Return (EXIT_FAILURE);		
	}

	if (Ctrl->I.active && (k = x2sys_read_list (GMT, Ctrl->I.file, &ignore, &n_ignore)) != X2SYS_NOERROR) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: Ignore file %s cannot be read - aborting\n", Ctrl->I.file);
		exit (EXIT_FAILURE);
	}

	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);

	if (Ctrl->F.flags) x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->F.flags, s), "-F");	/* Determine output order of selected columns */

	s->ascii_out = !GMT->common.b.active[1];

	if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, B.wesn, 4, double);

	out = GMT_memory (GMT, NULL, s->n_fields, double);

	for (ocol = 0; ocol < s->n_out_columns; ocol++) {	/* Set output formats for each output column */
		if ((int)s->out_order[ocol] == s->t_col) {
			GMT->current.io.col_type[GMT_OUT][ocol] = GMT_IS_ABSTIME;
			tpos = ocol;	/* This is the output column with time */
		}
		else if ((int)s->out_order[ocol] == s->x_col) {
			GMT->current.io.col_type[GMT_OUT][ocol] = (s->geographic) ? GMT_IS_LON : GMT_IS_FLOAT;
			xpos = ocol;	/* This is the output column with x */
		}
		else if ((int)s->out_order[ocol] == s->y_col) {
			GMT->current.io.col_type[GMT_OUT][ocol] = (s->geographic) ? GMT_IS_LAT : GMT_IS_FLOAT;
			ypos = ocol;	/* This is the output column with y */
		}
		else
			GMT->current.io.col_type[GMT_OUT][ocol] = GMT_IS_FLOAT;

		if (s->info[s->out_order[ocol]].format[0] != '-') gmt_formatting = true;
	}
	if (GMT->common.b.active[GMT_OUT]) gmt_formatting = false;

	if (GMT->common.R.active) {	/* Restrict output to given domain */
		if (xpos == -1 || ypos == -1) {
			GMT_Report (API, GMT_MSG_NORMAL, "The -R option was selected but lon,lat not included in -F\n");
			x2sys_end (GMT, s);
			Return (EXIT_FAILURE);		
		}
		/* Supply dummy linear proj */
		GMT->current.proj.projection = GMT->current.proj.xyz_projection[0] = GMT->current.proj.xyz_projection[1] = GMT_LINEAR;
		GMT->current.proj.pars[0] = GMT->current.proj.pars[1] = 1.0;
		GMT->common.J.active = true;
		if (GMT->common.R.wesn[XLO] < 0.0 && GMT->common.R.wesn[XHI] < 0.0) {
			GMT->common.R.wesn[XLO] += 360.0;
			GMT->common.R.wesn[XHI] += 360.0;
		}
		GMT_err_fail (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "");
	}

	if (Ctrl->S.active) {	/* Must count output data columns (except t, x, y) */
		for (ocol = n_data_col_out = 0; ocol < s->n_out_columns; ocol++) {
			this_col = s->out_order[ocol];
			if (this_col == s->t_col) continue;
			if (this_col == s->x_col) continue;
			if (this_col == s->y_col) continue;
			n_data_col_out++;
		}
	}

	MGD77_Set_Unit (GMT, s->unit[X2SYS_DIST_SELECTION],  &dist_scale, -1);	/* Gets scale which multiplies meters to chosen distance unit */
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
			vel_scale /= (METERS_IN_A_FOOT * dist_scale);	/* Must counteract any distance scaling to get feet. dt is in sec so we get ft/s */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(ft/s)");
			break;
		case 'k':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get km. dt is in sec so 3600 gives km/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(km/hr)");
			break;
		case 'm':
			if (GMT_compat_check (GMT, 4)) /* Warn and fall through */
				GMT_Report (API, GMT_MSG_COMPAT, "Warning: Unit m for miles is deprecated; use unit M instead\n");
			else {
				GMT_Report (API, GMT_MSG_NORMAL, "Error: Unit m for miles is not recognized\n");
				exit (EXIT_FAILURE);
				break;
			}
		case 'M':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get miles. dt is in sec so 3600 gives miles/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(mi/hr)");
			break;
		case 'n':
			vel_scale *= (3600.0 / dist_scale);		/* Must counteract any distance scaling to get miles. dt is in sec so 3600 gives miles/hr */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(kts)");
			break;
		case 'u':
			vel_scale /= (METERS_IN_A_SURVEY_FOOT * dist_scale);	/* Must counteract any distance scaling to get survey feet. dt is in sec so we get ft/s */
			strcpy (auxlist[MGD77_AUX_SP].header, "v(sft/s)");
			break;
	}
	switch (s->unit[X2SYS_DIST_SELECTION][0]) {
		case 'c':
			strcpy (auxlist[MGD77_AUX_DS].header, "d(user)");
			break;
		case 'e':
			strcpy (auxlist[MGD77_AUX_DS].header, "d(m)");
			break;
		case 'f':
			strcpy (auxlist[MGD77_AUX_DS].header, "d(feet)");
			break;
		case 'k':
			strcpy (auxlist[MGD77_AUX_DS].header, "d(km)");
			break;
		case 'm':
			if (GMT_compat_check (GMT, 4)) /* Warn and fall through */
				GMT_Report (API, GMT_MSG_COMPAT, "Warning: Unit m for miles is deprecated; use unit M instead\n");
			else {
				GMT_Report (API, GMT_MSG_NORMAL, "Error: Unit m for miles is not recognized\n");
				exit (EXIT_FAILURE);
				break;
			}
		case 'M':
			strcpy (auxlist[MGD77_AUX_DS].header, "d(miles)");
			break;
		case 'n':
			strcpy (auxlist[MGD77_AUX_DS].header, "d(nm)");
			break;
		case 'u':
			strcpy (auxlist[MGD77_AUX_DS].header, "d(sfeet)");
			break;
	}
	t_scale = GMT->current.setting.time_system.scale;	/* Convert user's TIME_UNIT to seconds */

	GMT_init_distaz (GMT, s->dist_flag ? GMT_MAP_DIST_UNIT : 'X', s->dist_flag, GMT_MAP_DIST);
	
	if (Ctrl->L.active) {	/* Load an ephemeral correction table */
		x2sys_get_corrtable (GMT, s, Ctrl->L.file, n_tracks, trk_name, NULL, aux, auxlist, &CORR);
		if (auxlist[MGD77_AUX_SP].requested && s->t_col == -1) {
			GMT_Report (API, GMT_MSG_NORMAL, "Selected correction table requires velocity which implies time (not selected)\n");
			MGD77_Free_Correction (GMT, CORR, n_tracks);
			x2sys_free_list (GMT, trk_name, n_tracks);
			exit (EXIT_FAILURE);
		}
	}

	if (Ctrl->A.active) {	/* Allocate an along-track adjustment table */
		A = GMT_memory (GMT, NULL, s->n_out_columns, struct X2SYS_ADJUST *);
		adj_col = GMT_memory (GMT, NULL, s->n_out_columns, bool);
	}
	if (Ctrl->E.active) GMT_set_segmentheader (GMT, GMT_OUT, true);	/* Enable segment headers */
	
	last_col = s->n_out_columns - 1;	/* column number of last output column */
	
	for (trk_no = 0; trk_no < n_tracks; trk_no++) {	/* Process each track */

		if (Ctrl->I.active && n_ignore) {	/* First see if this track is in the ignore list */
			for (k = 0, skip = false; !skip && k < n_ignore; k++)
				if (!strcmp (trk_name[trk_no], ignore[k])) skip = true;
			if (skip) continue;	/* Found it, so skip */
		}

		GMT_Report (API, GMT_MSG_VERBOSE, "Reading track %s\n", trk_name[trk_no]);

		x2sys_err_fail (GMT, (s->read_file) (GMT, trk_name[trk_no], &data, s, &p, &GMT->current.io, &row), trk_name[trk_no]);

		if (Ctrl->L.active && s->t_col >= 0) MGD77_Init_Correction (GMT, CORR[trk_no], data);	/* Initialize origins if needed */

		if (Ctrl->A.active) {	/* Load along-track adjustments */
			for (k = 0; k < s->n_out_columns; k++) adj_col[k] = x2sys_load_adjustments (GMT, X2SYS_HOME, Ctrl->T.TAG, trk_name[trk_no], s->info[s->out_order[k]].name, &A[k]);
		}

		if (Ctrl->E.active) {	/* Insert a segment header between files */
			sprintf (GMT->current.io.segment_header, "%s\n", trk_name[trk_no]);
			GMT_write_segmentheader (GMT, GMT->session.std[GMT_OUT], s->n_fields);
		}

		cumulative_dist = 0.0;
		for (row = 0; row < p.n_rows; row++) {	/* Process all records in this file */
			if (GMT->common.R.active && GMT_map_outside (GMT, data[xpos][row], data[ypos][row])) continue;	/* Point is outside region */
			if (Ctrl->S.active) {	/* Skip record if all data columns are NaN (not considering lon,lat,time) */
				for (ocol = bad = 0; ocol < s->n_out_columns; ocol++) {
					this_col = s->out_order[ocol];
					if (this_col == s->t_col) continue;
					if (this_col == s->x_col) continue;
					if (this_col == s->y_col) continue;
					if (GMT_is_dnan (data[ocol][row])) bad++;
				}
				if (bad == n_data_col_out) continue;	/* Yep, just NaNs here */
			}
			if (auxlist[MGD77_AUX_AZ].requested) {	/* Need azimuths to be computed from track coordinates */
				if (row == 0)	/* Look forward from first to second point to get an azimuth at the first point */
					aux_dvalue[MGD77_AUX_AZ] = GMT_az_backaz (GMT, data[xpos][1], data[ypos][1], data[xpos][0], data[ypos][0], false);
				else		/* else go from previous to current point */
					aux_dvalue[MGD77_AUX_AZ] = GMT_az_backaz (GMT, data[xpos][row], data[ypos][row], data[xpos][row-1], data[ypos][row-1], false);
			}
			if (auxlist[MGD77_AUX_DS].requested) {	/* Need distances to be computed from track coordinates */
				ds = (row == 0) ? 0.0 : dist_scale * GMT_distance (GMT, data[xpos][row], data[ypos][row], data[xpos][row-1], data[ypos][row-1]);
				cumulative_dist += ds;
				aux_dvalue[MGD77_AUX_DS] = cumulative_dist;
			}
			if (auxlist[MGD77_AUX_SP].requested) {	/* Need speed to be computed from track coordinates and time */
				dt = (row == 0) ? data[tpos][1] - data[tpos][0] : data[tpos][row] - data[tpos][row-1];
				aux_dvalue[MGD77_AUX_SP] = (GMT_is_dnan (dt) || dt == 0.0) ? GMT->session.d_NaN : vel_scale * ds / (dt * t_scale);
			}
			for (ocol = 0; ocol < s->n_out_columns; ocol++) {	/* Load output record one column at the time */
				correction = (Ctrl->L.active) ? MGD77_Correction (GMT, CORR[trk_no][ocol].term, data, aux_dvalue, row) : 0.0;
				if (Ctrl->A.active && adj_col[ocol]) {	/* Determine along-track adjustment */
					if (GMT_intpol (GMT, A[ocol]->d, A[ocol]->c, A[ocol]->n, 1, &aux_dvalue[MGD77_AUX_DS], &adj_amount, GMT->current.setting.interpolant)) {
						GMT_Report (API, GMT_MSG_NORMAL, "Error interpolating adjustment for %s near row %" PRIu64 " - no adjustment made!\n", s->info[s->out_order[ocol]].name, row);
						adj_amount = 0.0;
					}
					correction -= adj_amount;
				}
				out[ocol] = data[ocol][row] - correction;	/* Save final [possibly corrected and adjusted] value */
			}
			if (gmt_formatting)  {	/* Must use the specified formats in the definition file for one or more columns */
				for (ocol = 0; ocol < s->n_out_columns; ocol++) {
					if (s->info[s->out_order[ocol]].format[0] == '-')
						GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], out[ocol], ocol);
					else {
						if (GMT_is_dnan (out[ocol]))
							fprintf (GMT->session.std[GMT_OUT], "NaN");
						else
							fprintf (GMT->session.std[GMT_OUT], s->info[s->out_order[ocol]].format, out[ocol]);
					}
					(ocol == last_col) ? fprintf (GMT->session.std[GMT_OUT], "\n") : fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
				}
			}
			else {
				GMT->current.io.output (GMT, GMT->session.std[GMT_OUT], s->n_out_columns, out);
			}
		}

		/* Free memory allocated for the current data set */
		x2sys_free_data (GMT, data, s->n_out_columns, &p);
		for (ocol = 0; ocol < s->n_out_columns; ocol++) if (Ctrl->A.active && adj_col[ocol]) {
			GMT_free (GMT, A[ocol]->d);
			GMT_free (GMT, A[ocol]->c);
			GMT_free (GMT, A[ocol]);
		}
	}

	/* Clean up before quitting */
	
	if (Ctrl->L.active) MGD77_Free_Correction (GMT, CORR, n_tracks);

	x2sys_end (GMT, s);
	GMT_free (GMT, out);
	if (Ctrl->A.active) {
		GMT_free (GMT, A);
		GMT_free (GMT, adj_col);
	}
	x2sys_free_list (GMT, trk_name, n_tracks);
	if (Ctrl->I.active) x2sys_free_list (GMT, ignore, n_ignore);
	
	Return (GMT_OK);
}
