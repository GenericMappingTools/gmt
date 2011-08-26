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
 *      Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/* x2sys_list will read the crossover data base and output a subset of
 * the crossovers in a format determined by the options.
 *
 * Author:	Paul Wessel
 * Date:	20-SEPT-2008
 * Version:	1.0, based on the spirit of the old x_system code x_list
 *		but completely rewritten from the ground up.
 *
 */

#include "x2sys.h"

#define GMT_T	3	/* Just used to indicate abs time formatting */
#define LETTERS "acdhiInNtTvwxyz"

struct X2SYS_LIST_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct A {	/* -A */
		GMT_LONG active;
		double value;
	} A;
	struct C {	/* -C */
		GMT_LONG active;
		char *col;
	} C;
	struct F {	/* -F */
		GMT_LONG active;
		char *flags;
	} F;
	struct I {	/* -I */
		GMT_LONG active;
		char *file;
	} I;
	struct L {	/* -L */
		GMT_LONG active;
		char *file;
	} L;
	struct N {	/* -N */
		GMT_LONG active;
		GMT_LONG min;
	} N;
	struct Q {	/* -Q */
		GMT_LONG active;
		GMT_LONG mode;
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
		char *file;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
		char *TAG;
	} T;
	struct W {	/* -W */
		GMT_LONG active;
		char *file;
	} W;
};

void *New_x2sys_list_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_LIST_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_LIST_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->A.value = 1.0;
	return ((void *)C);
}

void Free_x2sys_list_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_LIST_CTRL *C) {	/* Deallocate control structure */
	if (C->In.file) free ((void *)C->In.file);
	if (C->C.col) free ((void *)C->C.col);
	if (C->F.flags) free ((void *)C->F.flags);
	if (C->I.file) free ((void *)C->I.file);
	if (C->L.file) free ((void *)C->L.file);
	if (C->S.file) free ((void *)C->S.file);
	if (C->T.TAG) free ((void *)C->T.TAG);
	if (C->W.file) free ((void *)C->W.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_x2sys_list_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "x2sys_list %s - Extract subset from crossover data base\n\n", X2SYS_VERSION);
	GMT_message (GMT, "usage: x2sys_list -C<column> -T<TAG> [<COEdbase>] [-A<asymm_max] [-F<flags>] [-I<ignorelist>] [-L[<corrtable.txt>]]\n");
	GMT_message (GMT, "\t [-N<nx_min>] [-Qe|i] [-S<track>] [%s] [%s] [-W<weight>] [-m]\n\n", GMT_Rgeo_OPT, GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-C <column> is the name of the data column whose crossovers we want.\n");
	GMT_message (GMT, "\t-T <TAG> is the system tag for the data set.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t<COEdbase> File with crossover error data base [stdin].\n");
	GMT_message (GMT, "\t-A Return only crossovers whose distribution in time [or dist if no time]\n");
	GMT_message (GMT, "\t   are fairly symmetric about the mid-point. Specify max abs value for\n");
	GMT_message (GMT, "\t   asymmetry = (n_right - n_left)/(nright + n_left) [1, i.e., use all tracks].\n");
	GMT_message (GMT, "\t-F Specify any combination of %s in the order of desired output:\n", LETTERS);
	GMT_message (GMT, "\t   a Angle (<= 90) between the two tracks at the crossover.\n");
	GMT_message (GMT, "\t   c Crossover error in chosen observable (see -C).\n");
	GMT_message (GMT, "\t   d Distance along tracks at the crossover.\n");
	GMT_message (GMT, "\t   h Heading along tracks at the crossover.\n");
	GMT_message (GMT, "\t   i Signed time interval between the two tracks at the crossover.\n");
	GMT_message (GMT, "\t   I Unsigned time interval between the two tracks at the crossover.\n");
	GMT_message (GMT, "\t   n Names of the two tracks.\n");
	GMT_message (GMT, "\t   N Id numbers of the two tracks.\n");
	GMT_message (GMT, "\t   t Absolute time along tracks at the crossover.\n");
	GMT_message (GMT, "\t   T Time since start of track along tracks at the crossover.\n");
	GMT_message (GMT, "\t   v Speed along tracks at the crossover.\n");
	GMT_message (GMT, "\t   w weight assigned to the crossover.\n");
	GMT_message (GMT, "\t   x x-coordinate of the crossover.\n");
	GMT_message (GMT, "\t   y y-coordinate of the crossover.\n");
	GMT_message (GMT, "\t   z Observed values (see -C) along tracks at the crossover.\n");
	GMT_message (GMT, "\t   Unless -S is specified, d,h,n,t,T,v,z will yield two columns.\n");
	GMT_message (GMT, "\t-I List of tracks to ignore [Use all tracks].\n");
	GMT_message (GMT, "\t-L Subtract systematic corrections from the data. If no correction file is given,\n");
	GMT_message (GMT, "\t   the default file <TAG>_corrections.txt in $X2SYS_HOME/<TAG> is assumed.\n");
	GMT_message (GMT, "\t-N Output results for tracks with more than <nx_min> crossovers only [Use all tracks].\n");
	GMT_message (GMT, "\t-Q Append e or i for external or internal crossovers [Default is both].\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t   [Default region is the entire data domain].\n");
	GMT_message (GMT, "\t-S Return only crossovers involving this track [Use all tracks].\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W If argument can be opened as a file then we expect a List of tracks and their\n");
	GMT_message (GMT, "\t   relative weights; otherwise the argument is the constant weight for all tracks [1].\n");
	GMT_explain_options (GMT, "Dm");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_x2sys_list_parse (struct GMTAPI_CTRL *C, struct X2SYS_LIST_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, mixed = FALSE, i, n_items, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Skip input files */
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */
			
			case 'A':
				Ctrl->A.active = TRUE;
				Ctrl->A.value = atof (opt->arg);
				break;
			case 'C':
				Ctrl->C.active = TRUE;
				Ctrl->C.col = strdup (opt->arg);
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				Ctrl->F.flags = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				Ctrl->I.file = strdup (opt->arg);
				break;
			case 'L':	/* Crossover correction table */
				Ctrl->L.active = TRUE;
				Ctrl->L.file = strdup (opt->arg);
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				Ctrl->N.min = atoi (opt->arg);
				break;
			case 'Q':	/* Specify internal or external only */
				Ctrl->Q.active = TRUE;
				if (opt->arg[0] == 'e') Ctrl->Q.mode = 1;
				else if (opt->arg[0] == 'i') Ctrl->Q.mode = 2;
				else Ctrl->Q.mode = 3;
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				Ctrl->S.file = strdup (opt->arg);
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				Ctrl->T.TAG = strdup (opt->arg);
				break;
			case 'W':
				Ctrl->W.active = TRUE;
				Ctrl->W.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one COEdatabase can be given (or stdin)\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.mode == 3, "Syntax error: Only one of -Qe -Qi can be specified!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && (Ctrl->A.value <= 0.0 || Ctrl->A.value > 1.0), "Syntax error option -A: Asymmetry must be in the range 0-1\n");
	n_errors += GMT_check_condition (GMT, GMT->current.io.multi_segments[GMT_OUT] && GMT->common.b.active[GMT_OUT], "Syntax error: Must use -F to specify output items.\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->F.flags, "Syntax error: Cannot use -M with binary output\n");
	n_items = strlen (Ctrl->F.flags);
	for (i = 0; i < (GMT_LONG)strlen (Ctrl->F.flags); i++) {
		if (!strchr (LETTERS, (int)Ctrl->F.flags[i])) {
			GMT_report (GMT, GMT_MSG_FATAL, "ERROR -F: Unknown item %c.\n", Ctrl->F.flags[i]);
			n_errors++;			
		}
		if (Ctrl->F.flags[i] == 'n') mixed = TRUE;		/* Both numbers and text - cannot use binary output */
	}
	n_errors += GMT_check_condition (GMT, mixed && GMT->common.b.active[GMT_OUT], "Syntax error: Cannot use -Fn with binary output\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

void dump_ascii_cols (struct GMT_CTRL *GMT, double *val, GMT_LONG col, GMT_LONG n, GMT_LONG first)
{	/* Short-hand to dump n = 1 or 2 numerical values in chosen format.
	 * col is used to set the format, and first is TRUE for first item per record.
	 */
	GMT_LONG i;
	for (i = 0; i < n; i++) {
		if (!first) fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], val[i], col);
		first = FALSE;
	}
}

#define Return(code) {Free_x2sys_list_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_x2sys_list (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	char **trk_name = NULL, **weight_name = NULL, *tofrom[2] = {"stdin", "stdout"}, *from = NULL;
	struct X2SYS_INFO *s = NULL;
	struct X2SYS_BIX B;
	struct X2SYS_COE_PAIR *P = NULL;
	GMT_LONG error = FALSE, mixed = FALSE, check_for_NaN = FALSE, both, first, z_is_requested = FALSE;
	GMT_LONG internal = TRUE;	/* FALSE if only external xovers are needed */
	GMT_LONG external = TRUE;	/* FALSE if only internal xovers are needed */
	GMT_LONG i, j, k, coe_kind, one, two, n_items, n_out, n_tracks, n_weights = 0, *trk_nx = NULL;
	GMT_LONG p, np_use = 0, nx_use = 0, np, m, nx, id;
	double *wesn = NULL, val[2], out[128], corr[2] = {0.0, 0.0}, sec_2_unit = 1.0, w_k, w;
	double fixed_weight = 1.0, *weights = NULL, *trk_symm = NULL;
	struct MGD77_CORRTABLE **CORR = NULL;
	struct X2SYS_LIST_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_x2sys_list_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_x2sys_list_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_x2sys_list", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRb", ">", options))) Return (error);
	Ctrl = (struct X2SYS_LIST_CTRL *)New_x2sys_list_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_list_parse (API, Ctrl, options))) Return (error);
	
 	/*---------------------------- This is the x2sys_list main code ----------------------------*/

	for (i = 0; i < (GMT_LONG)strlen (Ctrl->F.flags); i++) {
		if (Ctrl->F.flags[i] == 'c' || Ctrl->F.flags[i] == 'z') check_for_NaN = TRUE; /* Do not output records where the crossover or values are NaN */
		if (Ctrl->F.flags[i] == 'n') mixed = TRUE;		/* Both numbers and text */
	}
	if (Ctrl->Q.active) {
		if (Ctrl->Q.mode == 1) internal = FALSE;
		if (Ctrl->Q.mode == 2) external = FALSE;
	}
	
	sec_2_unit = GMT->current.setting.time_system.i_scale;	/* Save conversion from secs to TIME_UNIT before MGD77_Init switches to UNIX time system (seconds) */
	
	/* Initialize system via the tag */
	
	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);

	/* Verify that the chosen column is known to the system */
	
	if (Ctrl->C.col) x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->C.col, s), "-C");
	if (s->n_out_columns != 1) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: -C must specify a single column name\n");
		Return (EXIT_FAILURE);
	}
	
	/* Select internal, external, or both */
	
	coe_kind = 0;
	if (internal) coe_kind |= 1;
	if (external) coe_kind |= 2;
	if (coe_kind == 0) coe_kind = 3;	/* Both */
	
	/* Read the entire data base; note the -I, R and -S options are applied during reading */
	
	from = (Ctrl->In.file) ? Ctrl->In.file : tofrom[GMT_IN];
	if (GMT->common.R.active) wesn = GMT->common.R.wesn;	/* Passed a sub region request */
	GMT_report (GMT, GMT_MSG_NORMAL, "Read crossover database from %s...\n", from);
	np = x2sys_read_coe_dbase (GMT, s, Ctrl->In.file, Ctrl->I.file, wesn, Ctrl->C.col, coe_kind, Ctrl->S.file, &P, &nx, &n_tracks);
	GMT_report (GMT, GMT_MSG_NORMAL, "Found %ld pairs and a total of %ld crossover records.\n", np, nx);

	if (np == 0 && nx == 0) {	/* End here since nothing was allocated */
		x2sys_end (GMT, s);
		Return (GMT_OK);
	}
	
	if (Ctrl->W.file) {	/* Got file with weights for each track, OR a fixed weight [1] */
		if (x2sys_read_weights (GMT, Ctrl->W.file, &weight_name, &weights, &n_weights) != X2SYS_NOERROR) fixed_weight = atof (Ctrl->W.file);
	}
	
	/* Must count to see total number of COE per track */
	
	trk_nx = GMT_memory (GMT, NULL, n_tracks, GMT_LONG);
	trk_name = GMT_memory (GMT, NULL, n_tracks, char *);
	for (p = 0; p < np; p++) {	/* For each pair of tracks that generated crossovers */
		for (k = 0; k < 2; k++) {
			trk_nx[P[p].id[k]] += P[p].nx;
			trk_name[P[p].id[k]] = P[p].trk[k];
		}
	}
	
	/* Initialize column output types */
	
	one = 0;	two = 1;	/* Normal track order */
	both = !Ctrl->S.active;			/* Two columns for many output choices */
	n_out = 1 + both;		/* Number of column output for some cols if single is not specified */

	GMT->current.io.col_type[GMT_OUT][GMT_X] = (s->geographic) ? GMT_IS_LON : GMT_IS_FLOAT;
	GMT->current.io.col_type[GMT_OUT][GMT_Y] = (s->geographic) ? GMT_IS_LAT : GMT_IS_FLOAT;
	GMT->current.io.col_type[GMT_OUT][GMT_T] = GMT_IS_ABSTIME;

	n_items = strlen (Ctrl->F.flags); 
	for (i = j = 0; !mixed && i < n_items; i++, j++) {	/* Overwrite the above settings */
		switch (Ctrl->F.flags[i]) {	/* acdhintTvxyz */
			case 'a':	/* Angle between tracks */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				break;
			case 'c':	/* Crossover value */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				z_is_requested = TRUE;
				break;
			case 'd':	/* Distance along track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				break;
			case 'h':	/* Heading along track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				break;
			case 'I':	/* Time interval (unsigned) */
			case 'i':	/* Time interval (signed) */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				break;
			case 'n':	/* Names of the track(s) [need this case to fall through] */
			case 'N':	/* ID numbers of tracks */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				break;
			case 't':	/* Time along track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_ABSTIME;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_ABSTIME;
				break;
			case 'T':	/* Time along track since beginning of the first year of the track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				break;
			case 'v':	/* Speed along track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				break;
			case 'w':	/* Crossover composite weight */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				break;
			case 'x':	/* x coordinate of crossover */
				GMT->current.io.col_type[GMT_OUT][j] = (s->geographic) ? GMT_IS_LON : GMT_IS_FLOAT;
				break;
			case 'y':	/* y coordinate of crossover */
				GMT->current.io.col_type[GMT_OUT][j] = (s->geographic) ? GMT_IS_LAT : GMT_IS_FLOAT;
				break;
			case 'z':	/* Observed value along track */
				GMT->current.io.col_type[GMT_OUT][j] = GMT_IS_FLOAT;
				if (both) GMT->current.io.col_type[GMT_OUT][++j] = GMT_IS_FLOAT;
				z_is_requested = TRUE;
				break;
		}
	}

	if (Ctrl->L.active && !check_for_NaN) {	/* Correction table would not be needed for output */
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Correction table not needed for chosen output (corrections ignored).\n");
		Ctrl->L.active = FALSE;
	}

	if (Ctrl->L.active) {	/* Load an ephemeral correction table */
		x2sys_get_corrtable (GMT, s, Ctrl->L.file, n_tracks, trk_name, Ctrl->C.col, NULL, NULL, &CORR);
	}
	
	if (Ctrl->A.active) {
		GMT_LONG *x_side[2] = {NULL, NULL}, half;
		double mid[2];
		for (j = 0; j < 2; j++) x_side[j] = GMT_memory (GMT, NULL, n_tracks, GMT_LONG);
		trk_symm = GMT_memory (GMT, NULL, n_tracks, double);
		for (p = 0; p < np; p++) {	/* For each pair of tracks that generated crossovers */
			for (j = 0; j < 2; j++) {	/* Set mid-point for these two tracks */
				if (GMT_is_dnan (P[p].start[j]))
					mid[j] = P[p].dist[j];
				else
					mid[j] = 0.5 * (P[p].stop[j] + P[p].start[j]);
			}
			for (k = 0; k < P[p].nx; k++) {
				for (j = 0; j < 2; j++) {
					if (GMT_is_dnan (P[p].COE[k].data[j][COE_T]))
						half = (P[p].COE[k].data[j][COE_D] > mid[j]);
					else
						half = (P[p].COE[k].data[j][COE_T] > mid[j]);
					x_side[half][P[p].id[j]]++;
				}
			}
		}
		/* Compute symmetry */
		for (k = 0; k < n_tracks; k++) trk_symm[k] = (double)( (x_side[1][k] - x_side[0][k]) / (x_side[1][k] + x_side[0][k]) );
		for (j = 0; j < 2; j++) GMT_free (GMT, x_side[j]);
	}
	/* Time to issue output */
	
	if (!GMT->common.b.active[GMT_OUT]) {	/* Write 3 header records */
		char *cmd = NULL;
		fprintf (GMT->session.std[GMT_OUT], "# Tag: %s %s\n", Ctrl->T.TAG, Ctrl->C.col);
		GMT_Create_Cmd (API, &cmd, options);
		fprintf (GMT->session.std[GMT_OUT], "# Command: %s %s\n", GMT->init.progname, cmd);	/* Build command line argument string */
		GMT_free (GMT, cmd);
		
		fprintf (GMT->session.std[GMT_OUT], "#");
		for (i = j = 0; i < n_items; i++, j++) {	/* Overwrite the above settings */
			if (i > 0) fprintf (GMT->session.std[GMT_OUT], "\t");
			switch (Ctrl->F.flags[i]) {	/* acdhintTvxyz */
				case 'a':	/* Angle between tracks */
					fprintf (GMT->session.std[GMT_OUT], "angle");
					break;
				case 'c':	/* Crossover value */
					fprintf (GMT->session.std[GMT_OUT], "%s_x", Ctrl->C.col);
					break;
				case 'd':	/* Distance along track */
					(both) ? fprintf (GMT->session.std[GMT_OUT], "dist_1\tdist_2") : fprintf (GMT->session.std[GMT_OUT], "dist");
					break;
				case 'h':	/* Heading along track */
					(both) ? fprintf (GMT->session.std[GMT_OUT], "head_1\thead_2") : fprintf (GMT->session.std[GMT_OUT], "head");
					break;
				case 'I':	/* Time interval (unsigned) */
					fprintf (GMT->session.std[GMT_OUT], "u_tint");
					break;
				case 'i':	/* Time interval (signed) */
					fprintf (GMT->session.std[GMT_OUT], "s_tint");
					break;
				case 'n':	/* Names of the track(s) [need this case to fall through] */
					(both) ? fprintf (GMT->session.std[GMT_OUT], "track_1\ttrack_2") : fprintf (GMT->session.std[GMT_OUT], "track");
					break;
				case 'N':	/* ID numbers of tracks */
					(both) ? fprintf (GMT->session.std[GMT_OUT], "ID_1\tID_2") : fprintf (GMT->session.std[GMT_OUT], "ID");
					break;
				case 't':	/* Time along track */
					(both) ? fprintf (GMT->session.std[GMT_OUT], "t_1\tt_2") : fprintf (GMT->session.std[GMT_OUT], "t");
					break;
				case 'T':	/* Time along track since beginning of the first year of the track */
					(both) ? fprintf (GMT->session.std[GMT_OUT], "T_1\tT_2\n") : fprintf (GMT->session.std[GMT_OUT], "T");
					break;
				case 'v':	/* Speed along track */
					(both) ? fprintf (GMT->session.std[GMT_OUT], "vel_1\tvel_2") : fprintf (GMT->session.std[GMT_OUT], "vel");
					break;
				case 'w':	/* Composite weight of crossover */
					fprintf (GMT->session.std[GMT_OUT], "weight");
					break;
				case 'x':	/* x coordinate of crossover */
					(s->geographic) ? fprintf (GMT->session.std[GMT_OUT], "lon") : fprintf (GMT->session.std[GMT_OUT], "x");
					break;
				case 'y':	/* y coordinate of crossover */
					(s->geographic) ? fprintf (GMT->session.std[GMT_OUT], "lat") : fprintf (GMT->session.std[GMT_OUT], "y");
					break;
				case 'z':	/* Observed value along track */
					(both) ? fprintf (GMT->session.std[GMT_OUT], "%s_1\t%s_2", Ctrl->C.col, Ctrl->C.col) : fprintf (GMT->session.std[GMT_OUT], "%s", Ctrl->C.col);
					break;
			}
		}
		fprintf (GMT->session.std[GMT_OUT], "\n");
	}
	
	for (p = 0; p < np; p++) {	/* For each pair of tracks that generated crossovers */
		if (Ctrl->N.active && (trk_nx[P[p].id[0]] < Ctrl->N.min || trk_nx[P[p].id[1]] < Ctrl->N.min)) continue;			/* Not enough COEs */
		if (Ctrl->A.active && (fabs (trk_symm[P[p].id[0]]) > Ctrl->A.value || fabs (trk_symm[P[p].id[1]]) > Ctrl->A.value)) continue;	/* COEs not distributed symmatrically */
		np_use++;
		nx_use += P[p].nx;
		if (GMT->current.io.multi_segments[GMT_OUT]) fprintf (GMT->session.std[GMT_OUT], "%c %s - %s nx = %ld\n", GMT->current.setting.io_seg_marker[GMT_OUT], P[p].trk[0], P[p].trk[1], P[p].nx);
		GMT_report (GMT, GMT_MSG_NORMAL, "Crossovers from %s minus %s [%ld].\n", P[p].trk[0], P[p].trk[1], P[p].nx);
		if (Ctrl->S.active) {	/* May have to flip which is track one and two */
			two = !strcmp (Ctrl->S.file, P[p].trk[0]);
			one = 1 - two;
		}
		
		for (k = 0; k < P[p].nx; k++) {	/* For each crossover */
			/* First check if this record has a non-NaN COE */
			if (check_for_NaN && (GMT_is_dnan (P[p].COE[k].data[one][COE_Z]) || GMT_is_dnan (P[p].COE[k].data[two][COE_Z]))) continue;
			
			for (i = j = 0, first = TRUE; i < n_items; i++, j++) {
				switch (Ctrl->F.flags[i]) {	/* acdhintTvwxyz */
					case 'a':	/* Angle between tracks */
						val[0] = fabs (P[p].COE[k].data[0][COE_H] - P[p].COE[k].data[1][COE_H]);
						while (val[0] >= 180.0) val[0] -= 180.0;
						out[j] = (val[0] > 90.0) ? 180.0 - val[0] : val[0];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, 1, first);
						break;
					case 'c':	/* Crossover value */
					 	if (Ctrl->L.active) {
							corr[one] = MGD77_Correction_Rec (GMT, CORR[P[p].id[one]][COE_Z].term, P[p].COE[k].data[one], NULL);
							corr[two] = MGD77_Correction_Rec (GMT, CORR[P[p].id[two]][COE_Z].term, P[p].COE[k].data[two], NULL);
						}
						out[j] = val[0] = (P[p].COE[k].data[one][COE_Z] - corr[one]) - (P[p].COE[k].data[two][COE_Z] - corr[two]);
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, 1, first);
						break;
					case 'd':	/* Distance along track */
						out[j] = val[0] = P[p].COE[k].data[one][COE_D];
						if (both) out[++j] = val[1] = P[p].COE[k].data[two][COE_D];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first);
						break;
					case 'h':	/* Heading along track */
						out[j] = val[0] = P[p].COE[k].data[one][COE_H];
						if (both) out[++j] = val[1] = P[p].COE[k].data[two][COE_H];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first);
						break;
					case 'i':	/* Time interval in current TIME_UNIT */
						out[j] = val[0] = sec_2_unit * (P[p].COE[k].data[one][COE_T] - P[p].COE[k].data[two][COE_T]);
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, 1, first);
						break;
					case 'I':	/* Time interval in current TIME_UNIT */
						out[j] = val[0] = sec_2_unit * fabs (P[p].COE[k].data[one][COE_T] - P[p].COE[k].data[two][COE_T]);
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, 1, first);
						break;
					case 'n':	/* Names of the track(s) */
						if (both) {
							if (!first) fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
							fprintf (GMT->session.std[GMT_OUT], "%s", P[p].trk[0]);
							fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
							fprintf (GMT->session.std[GMT_OUT], "%s", P[p].trk[1]);
						}
						else {
							if (!first) fprintf (GMT->session.std[GMT_OUT], "%s", GMT->current.setting.io_col_separator);
							fprintf (GMT->session.std[GMT_OUT], "%s", P[p].trk[0]);
						}
						break;
					case 'N':	/* ID numbers of tracks */
						out[j] = val[0] = (double)P[p].id[one];
						if (both) out[++j] = val[1] = (double)P[p].id[two];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first);
						break;
					case 't':	/* Time along track */
						out[j] = val[0] = P[p].COE[k].data[one][COE_T];
						if (both) out[++j] = val[1] = P[p].COE[k].data[two][COE_T];
						if (mixed) dump_ascii_cols (GMT, val, GMT_T, n_out, first);
						break;
					case 'T':	/* Time along track since beginning of the track */
						out[j] = val[0] = sec_2_unit * (P[p].COE[k].data[one][COE_T] - P[p].start[one]);
						if (both) out[++j] = val[1] = sec_2_unit * (P[p].COE[k].data[two][COE_T] - P[p].start[two]);
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first);
						break;
					case 'v':	/* Speed along track */
						out[j] = val[0] = P[p].COE[k].data[one][COE_V];
						if (both) out[++j] = val[1] = P[p].COE[k].data[two][COE_V];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first);
						break;
					case 'w':	/* Weight for this crossover */
						if (weights) {	/* Weightfile was given; compute composite weight for this COE */
							for (m = 0, w_k = 0.0; m < 2; m++) {
								if ((id = x2sys_find_track (GMT, P[p].trk[m], weight_name, n_weights)) == -1) {
									GMT_report (GMT, GMT_MSG_NORMAL, "No weights found for track %s - using weight = 1.\n", P[p].trk[m]);
									w = 1.0;
								}
								else
									w = weights[id];
								w_k += 1.0 / (w*w);
							}
							out[j] = sqrt (1.0/w_k);
						}
						else
							out[j] = fixed_weight;
						if (mixed) {val[0] = out[j];	dump_ascii_cols (GMT, val, GMT_Z, 1, first);}
						break;
					case 'x':	/* x coordinate of crossover */
						out[j] = val[0] = P[p].COE[k].data[0][COE_X];
						if (mixed) dump_ascii_cols (GMT, val, GMT_X, 1, first);
						break;
					case 'y':	/* y coordinate of crossover */
						out[j] = val[0] = P[p].COE[k].data[0][COE_Y];
						if (mixed) dump_ascii_cols (GMT, val, GMT_Y, 1, first);
						break;
					case 'z':	/* Observed value along track */
						if (Ctrl->L.active) corr[one] = MGD77_Correction_Rec (GMT, CORR[P[p].id[one]][COE_Z].term, P[p].COE[k].data[one], NULL);
						out[j] = val[0] = P[p].COE[k].data[one][COE_Z] - corr[one];
						if (both) {
							if (Ctrl->L.active) corr[two] = MGD77_Correction_Rec (GMT, CORR[P[p].id[two]][COE_Z].term, P[p].COE[k].data[two], NULL);
							out[++j] = val[1] = P[p].COE[k].data[two][COE_Z] - corr[two];
						}
						if (mixed) dump_ascii_cols (GMT, val, GMT_Z, n_out, first);
						break;
				}
				first = FALSE;
			}
			if (mixed)
				GMT_fputs ("\n", GMT->session.std[GMT_OUT]);
			else
				GMT->current.io.output (GMT, GMT->session.std[GMT_OUT], j, out);
		}
	}
	GMT_report (GMT, GMT_MSG_NORMAL, "Output %ld pairs and a total of %ld crossover records.\n", np_use, nx_use);
	
	/* Done, free up data base array */
	
	x2sys_free_coe_dbase (GMT, P, np);
	GMT_free (GMT, trk_nx);
	if (Ctrl->A.active) GMT_free (GMT,  trk_symm);

	if (Ctrl->L.active) MGD77_Free_Correction (GMT, CORR, n_tracks);
	GMT_free (GMT, trk_name);
	x2sys_end (GMT, s);

	Return (GMT_OK);
}
