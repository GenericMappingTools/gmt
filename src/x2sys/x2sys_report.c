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
 *      Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/* x2sys_report will read the crossover data base and report on the statistics
 * of the crossovers for each track and overall.
 *
 * Author:	Paul Wessel
 * Date:	20-SEPT-2008
 * Version:	1.0, based on the spirit of the old x_system code x_report
 *		but completely rewritten from the ground up.
 *
 */

#define THIS_MODULE_NAME	"x2sys_report"
#define THIS_MODULE_LIB		"x2sys"
#define THIS_MODULE_PURPOSE	"Report statistics from crossover data base"
#define THIS_MODULE_KEYS	"LTi,ITi,>TO"

#include "x2sys.h"

#define GMT_PROG_OPTIONS "->RV"

struct X2SYS_REPORT_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct A {	/* -A */
		bool active;
	} A;
	struct C {	/* -C */
		bool active;
		char *col;
	} C;
	struct I {	/* -I */
		bool active;
		char *file;
	} I;
	struct L {	/* -L */
		bool active;
		char *file;
	} L;
	struct N {	/* -N */
		bool active;
		uint64_t min;
	} N;
	struct Q {	/* -Q */
		bool active;
		int mode;
	} Q;
	struct S {	/* -S */
		bool active;
		char *file;
	} S;
	struct T {	/* -T */
		bool active;
		char *TAG;
	} T;
};

struct COE_REPORT {	/* Holds summary info for each track */
	uint64_t nx;	/* Total number of COE for this track */
	double mean, stdev, rms;
	double sum, sum2, W;
	double d_max;	/* Length of track in distance units */
};

struct COE_ADJUST {	/* Holds adjustment spline knots */
	double d;	/* Distance along track */
	double c;	/* value to subtract from observation at crossover */
};

struct COE_ADJLIST {	/* Array with the growing arrays of COE_ADJUST per track */
	struct COE_ADJUST *K;
	unsigned int n;
	size_t n_alloc;
};

void *New_x2sys_report_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_REPORT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_REPORT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

void Free_x2sys_report_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_REPORT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);
	if (C->C.col) free (C->C.col);
	if (C->I.file) free (C->I.file);
	if (C->L.file) free (C->L.file);
	if (C->S.file) free (C->S.file);
	if (C->T.TAG) free (C->T.TAG);
	GMT_free (GMT, C);
}

int GMT_x2sys_report_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: x2sys_report -C<column> -T<TAG> [<COEdbase>] [-A] [-I<ignorelist>] [-L[<corrtable.txt>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-N<nx_min>] [-Qe|i] [-S<track>] [%s] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-C <column> is the name of the data column whose crossovers we want.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T <TAG> is the system tag for the data set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<COEdbase> File with crossover error data base [stdin].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Create adjustment splines per track to redistribute COEs between tracks\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   according to their relative weight.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I List of tracks to ignore [Use all tracks].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Subtract systematic corrections from the data. If no correction file is given,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the default file <TAG>_corrections.txt in $X2SYS_HOME/<TAG> is assumed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Output results for tracks with more than <nx_min> crossovers only [0, i.e., report all tracks].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Append e or i for external or internal crossovers [Default is external].\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default region is the entire data domain].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Return only crossovers involving this track [Use all tracks].\n");
	GMT_Option (API, "V,.");
	
	return (EXIT_FAILURE);
}

int GMT_x2sys_report_parse (struct GMT_CTRL *GMT, struct X2SYS_REPORT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input files */
				Ctrl->In.active = true;
				if (n_files == 0) Ctrl->In.file = strdup (opt->arg);
				n_files++;
				break;

			/* Processes program-specific parameters */
			
			case 'A':
				Ctrl->A.active = true;
				break;
			case 'C':
				Ctrl->C.active = true;
				Ctrl->C.col = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = true;
				Ctrl->I.file = strdup (opt->arg);
				break;
			case 'L':	/* Crossover correction table */
				Ctrl->L.active = true;
				Ctrl->L.file = strdup (opt->arg);
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.min = atoi (opt->arg);
				break;
			case 'Q':	/* Specify internal or external only */
				Ctrl->Q.active = true;
				if (opt->arg[0] == 'e') Ctrl->Q.mode = 1;
				else if (opt->arg[0] == 'i') Ctrl->Q.mode = 2;
				else Ctrl->Q.mode = 3;
				break;
			case 'S':
				Ctrl->S.file = strdup (opt->arg);
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

	n_errors += GMT_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->C.active || !Ctrl->C.col, "Syntax error: Must use -C to specify observation of interest\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.mode == 3, "Syntax error: Only one of -Qe -Qi can be specified!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int comp_structs (const void *point_1, const void *point_2) { /* Sort ADJ structure on distance */
        if ( ((struct COE_ADJUST *)point_1)->d < ((struct COE_ADJUST *)point_2)->d)
                return(-1);
        else if ( ((struct COE_ADJUST *)point_1)->d > ((struct COE_ADJUST *)point_2)->d)
                return(+1);
        else
                return(0);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_x2sys_report_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_x2sys_report (void *V_API, int mode, void *args)
{
	char **trk_name = NULL, *c = NULL, fmt[GMT_BUFSIZ] = {""}, record[GMT_BUFSIZ] = {""}, word[GMT_BUFSIZ] = {""};
	struct X2SYS_INFO *s = NULL;
	struct X2SYS_BIX B;
	struct X2SYS_COE_PAIR *P = NULL;
	struct COE_REPORT *R = NULL;
	struct MGD77_CORRTABLE **CORR = NULL;
	int error = 0;
	bool internal = false;	/* false if only external xovers are needed */
	bool external = true;	/* false if only internal xovers are needed */
	uint64_t i, k, n, n_use, n_tracks;
	uint64_t p, np, nx, Tnx = 0;
	unsigned int coe_kind;
	double sum, sum2, sum_w, Tsum, Tsum2, COE, sign, scale, corr[2] = {0.0, 0.0};
	double Tmean, Tstdev, Trms;
	struct GMT_OPTION *opt = NULL;
	struct X2SYS_REPORT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_x2sys_report_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_x2sys_report_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_x2sys_report_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_x2sys_report_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_report_parse (GMT, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the x2sys_report main code ----------------------------*/

	c = GMT->current.setting.io_col_separator;
	
	/* Initialize system via the tag */
	
	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &s, &B, &GMT->current.io), Ctrl->T.TAG);

	/* Verify that the chosen column is known to the system */
	
	if (Ctrl->C.col) x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->C.col, s), "-C");
	if (s->n_out_columns != 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: -C must specify a single column name\n");
		Return (EXIT_FAILURE);
	}
	
	/* Select internal, external, or both */
	
	if (Ctrl->Q.active) {
		if (Ctrl->Q.mode == 1) internal = false;
		if (Ctrl->Q.mode == 2) external = false;
	}
	coe_kind = 0;
	if (internal) coe_kind |= 1;
	if (external) coe_kind |= 2;
	if (coe_kind == 0) coe_kind = 2;	/* External */
	
	/* Read the entire data base; note the -I, R and -S options are applied during reading */
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Read crossover database %s...\n", Ctrl->In.file);
	np = x2sys_read_coe_dbase (GMT, s, Ctrl->In.file, Ctrl->I.file, GMT->common.R.wesn, Ctrl->C.col, coe_kind, Ctrl->S.file, &P, &nx, &n_tracks);
	GMT_Report (API, GMT_MSG_VERBOSE, "Found %" PRIu64 " pairs and a total of %" PRIu64 " crossover records.\n", np, nx);

	if (np == 0 && nx == 0) {	/* End here since nothing was allocated */
		x2sys_end (GMT, s);
		Return (GMT_OK);
	}
	
	R = GMT_memory (GMT, NULL, n_tracks, struct COE_REPORT);
	trk_name = GMT_memory (GMT, NULL, n_tracks, char *);
	
	for (p = 0; p < np; p++) {	/* Get track name list */
		for (k = 0; k < 2; k++) trk_name[P[p].id[k]] = P[p].trk[k];
	}
	
	if (Ctrl->L.active) {	/* Load an ephemeral correction table */
		x2sys_get_corrtable (GMT, s, Ctrl->L.file, n_tracks, trk_name, Ctrl->C.col, NULL, NULL, &CORR);
	}

	Tsum = Tsum2 = 0.0;
	for (p = 0; p < np; p++) {	/* For each pair of tracks that generated crossovers */
		for (i = n = 0, sum = sum2 = 0.0; i < P[p].nx; i++) {
			if (Ctrl->L.active) {
				corr[0] = MGD77_Correction_Rec (GMT, CORR[P[p].id[0]][COE_Z].term, P[p].COE[i].data[0], NULL);
				corr[1] = MGD77_Correction_Rec (GMT, CORR[P[p].id[1]][COE_Z].term, P[p].COE[i].data[1], NULL);
			}
			COE = (P[p].COE[i].data[0][COE_Z] - corr[0]) - (P[p].COE[i].data[1][COE_Z] - corr[1]);
			if (GMT_is_dnan(COE)) continue;
			sum += COE;	sum2 += COE * COE;	n++;
			Tsum += COE;	Tsum2 += COE * COE;	Tnx++;
		}
		for (k = 0, sign = 1.0; n && k < 2; k++) {
			R[P[p].id[k]].nx += n;
			R[P[p].id[k]].sum += sign * sum;
			R[P[p].id[k]].sum2 += sum2;
			R[P[p].id[k]].d_max = P[p].dist[k];	/* Just copy over max distance for later */
			sign = -1.0;
		}
	}
	
	for (k = n_use = 0, sum_w = 0.0; k < n_tracks; k++) {	/* Calculate normalized weights */
		if (R[k].nx <= Ctrl->N.min) continue;	/* Not enough COEs */
		if (R[k].nx && R[k].sum2 > 0.0) {
			R[k].W = R[k].nx / R[k].sum2;
			sum_w += R[k].W;
			n_use++;
		}
		else
			R[k].W = GMT->session.d_NaN;
	}
	scale = (n_use && sum_w > 0.0) ? n_use / sum_w : 1.0;

	/* Time to issue output */

	if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {
		Return (API->error);	/* Enables data output and sets access mode */
	}
	GMT_set_tableheader (GMT, GMT_OUT, true);	/* Turn on -ho explicitly */
	
	sprintf (record, " Tag: %s %s", Ctrl->T.TAG, Ctrl->C.col);
	GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, record);
	sprintf (record, " Command: %s", THIS_MODULE_NAME);
	if (!Ctrl->In.file) strcat (record, " [stdin]");
	for (opt = options; opt; opt = opt->next) {
		strcat (record, " ");
		if (opt->option == GMT_OPT_INFILE) 
			strcat (record, opt->arg);
		else {
			sprintf (word, "-%c%s", opt->option, opt->arg);
			strcat (record, word);
		}
	}
	GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, record);
	sprintf (record, "track%sN%smean%sstdev%srms%sweight[%" PRIu64 "]", c, c, c, c, c, n_use);
	GMT_Put_Record (API, GMT_WRITE_TABLE_HEADER, record);
	Tmean = (Tnx) ? Tsum / Tnx : GMT->session.d_NaN;
	Tstdev = (Tnx > 1) ? sqrt ((Tnx * Tsum2 - Tsum * Tsum) / (Tnx * (Tnx - 1.0))) : GMT->session.d_NaN;
	Trms = (Tnx) ? sqrt (Tsum2 / Tnx) : GMT->session.d_NaN;
	sprintf (fmt, "TOTAL%%s%%" PRIu64 "%%s%s%%s%s%%s%s%%s1", GMT->current.setting.format_float_out,GMT->current.setting.format_float_out,
		GMT->current.setting.format_float_out);
	sprintf (record, fmt, c, Tnx, c, Tmean, c, Tstdev, c, Trms, c);
	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
	sprintf (fmt, "%%s%%s%%" PRIu64 "%%s%s%%s%s%%s%s%%s%s\n", GMT->current.setting.format_float_out,GMT->current.setting.format_float_out,
		GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	for (k = 0; k < n_tracks; k++) {	/* For each track that generated crossovers */
		if (R[k].nx <= Ctrl->N.min) continue;			/* Not enough COEs */
		if (!GMT_is_dnan (R[k].W)) R[k].W *= scale;
		R[k].mean = (R[k].nx) ? R[k].sum / R[k].nx : GMT->session.d_NaN;
		R[k].stdev = (R[k].nx > 1) ? sqrt ((R[k].nx * R[k].sum2 - R[k].sum * R[k].sum) / (R[k].nx * (R[k].nx - 1.0))) : GMT->session.d_NaN;
		R[k].rms = (R[k].nx) ? sqrt (R[k].sum2 / R[k].nx) : GMT->session.d_NaN;
		sprintf (record, fmt, trk_name[k], c, R[k].nx, c, R[k].mean, c, R[k].stdev, c, R[k].rms, c, R[k].W);
		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}
	
	if (Ctrl->A.active) {	/* Create track adjustment spline files for each track */
		unsigned int n_out, n1;
		char file[GMT_BUFSIZ] = {""};
		double out[2], z[2], z_ij;
		FILE *fp = NULL;
		struct COE_ADJLIST *adj = NULL;

		GMT_set_cartesian (GMT, GMT_OUT);	/* Since we will write (dist, COE) pairs */
		
		adj = GMT_memory (GMT, NULL, n_tracks, struct COE_ADJLIST);
		for (p = 0; p < np; p++) {	/* For each pair of tracks that generated crossovers */
			for (i = n = 0; i < P[p].nx; i++) {	/* For each COE between this pair */
				for (k = 0; k < 2; k++) {
					z[k] = P[p].COE[i].data[k][COE_Z];
					if (Ctrl->L.active) z[k] -= MGD77_Correction_Rec (GMT, CORR[P[p].id[k]][COE_Z].term, P[p].COE[i].data[k], NULL);
				}
				z_ij = (z[0] * R[P[p].id[0]].W + z[1] * R[P[p].id[1]].W) / (R[P[p].id[0]].W + R[P[p].id[1]].W);	/* Desired z-value at crossover */
				if (GMT_is_dnan(z_ij)) continue;
				for (k = 0; k < 2; k++) {
					if (R[P[p].id[k]].nx <= Ctrl->N.min) continue;	/* This track will not have enough total COE to be used in the end */
					if (adj[P[p].id[k]].n >= adj[P[p].id[k]].n_alloc) {	/* So first time both are zero and we allocate first */
						if (adj[P[p].id[k]].n_alloc) adj[P[p].id[k]].n_alloc <<= 1; else adj[P[p].id[k]].n_alloc = GMT_SMALL_CHUNK;
						adj[P[p].id[k]].K = GMT_memory (GMT, adj[P[p].id[k]].K, adj[P[p].id[k]].n_alloc, struct COE_ADJUST);
					}
					adj[P[p].id[k]].K[adj[P[p].id[k]].n].d = P[p].COE[i].data[k][COE_D];	/* Distance along current track at COE */
					adj[P[p].id[k]].K[adj[P[p].id[k]].n].c = z_ij - z[k];			/* Observed difference at COE to be adjusted */
					adj[P[p].id[k]].n++;
				}
			}
		}
		/* Time to create the track adjustment spline files */
		for (k = 0; k < n_tracks; k++) {	/* For each track that generated crossovers */
			if (adj[k].n <= Ctrl->N.min) continue;			/* Not enough COEs */
			adj[k].K[adj[k].n].d = adj[k].K[adj[k].n].c = 0.0;	/* Add in anchor point (0,0) */
			adj[k].n++;
			adj[k].K[adj[k].n].d = R[k].d_max;			/* Add in anchor point (d_max,0) */
			adj[k].K[adj[k].n].c = 0.0;
			adj[k].n++;
			
			qsort(adj[k].K, adj[k].n, sizeof(struct COE_ADJUST), comp_structs);
			sprintf (file, "%s/%s/%s.%s.adj", X2SYS_HOME, Ctrl->T.TAG, trk_name[k], Ctrl->C.col);
			if ((fp = GMT_fopen (GMT, file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to create file %s!\n", file);
				Return (EXIT_FAILURE);
			}
			n1 = adj[k].n - 1;
			out[1] = 0.0;
			for (i = n_out = 0; i <= n1; i++) {
				out[0] = adj[k].K[i].d;
				out[1] += adj[k].K[i].c;
				n_out++;
				if (i == n1 || out[0] < adj[k].K[i+1].d) {	/* Time to output */
					out[1] /= n_out;
					GMT->current.io.output (GMT, fp, 2, out);
					out[1] = 0.0;
					n_out = 0;
				}
			}
			GMT_fclose (GMT, fp);
			GMT_free (GMT, adj[k].K);
		}
		GMT_free (GMT, adj);
	}
	
	/* Done, free up data base array, etc */
	
	x2sys_free_coe_dbase (GMT, P, np);
	GMT_free (GMT, trk_name);
	GMT_free (GMT, R);

	if (Ctrl->L.active) MGD77_Free_Correction (GMT, CORR, (unsigned int)n_tracks);
	x2sys_end (GMT, s);

	Return (GMT_OK);
}
