/*
 * Copyright (c) 2015-2023 by P. Wessel
 * See LICENSE.TXT file for copying and redistribution conditions.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3 or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * Contact info: http://www.soest.hawaii.edu/PT/GSFML
 *--------------------------------------------------------------------
 *
 * fzblender reads a FZ analysis file and produces a smooth, blended trace.
 *
 * Author:	Paul Wessel
 * Date:	01-DEC-2023 (Requires GMT >= 6)
 */

#define THIS_MODULE_NAME	"fzblender"
#define THIS_MODULE_LIB		"gsfml"
#define THIS_MODULE_PURPOSE	"Produce a smooth blended FZ trace"
#define THIS_MODULE_LIB_PURPOSE	"GMT supplemental modules for GSFML"
#define THIS_MODULE_KEYS	"<DI,>DO"
#define THIS_MODULE_NEEDS       ""
#define THIS_MODULE_OPTIONS	"-Vh>"

#include "gmt_dev.h"
#include "fz_analysis.h"

#define DEF_Q_MIN		0.0	/* Minimum quality index  */
#define DEF_Q_MAX		4.0	/* Maximum quality index */
#define DEF_Z_AMP_CUT		25.0	/* Amplitude cutoff for VGG */
#define DEF_Z_VAR_CUT		50.0	/* Variance reduction cutoff */
#define DEF_Z_F_CUT		50.0	/* F statistic cutoff */
#define DEF_Z_W_CUT		15.0	/* Width cutoff */

#define N_BLENDS	5	/* Total number of traces available for blending */
#define B_MODEL		0
#define D_MODEL		1
#define E_MODEL		2
#define T_MODEL		3
#define U_MODEL		4

#define N_LONG_COL	13	/* Number of input columns with longitudes */
#define N_BLEND_COLS	10	/* Number of output columns */
#define OUT_LON0	0
#define OUT_LAT0	1
#define OUT_DIST	2
#define OUT_SHFT	3
#define OUT_WDTH	4
#define OUT_QWHT	5
#define OUT_LONL	6
#define OUT_LATL	7
#define OUT_LONR	8
#define OUT_LATR	9

struct FZBLENDER_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct I {	/* -I<profile_ID> */
		bool active;
		int profile;
	} I;
	struct D {	/* -D */
		bool active;
		char *file;
	} D;
	struct E {	/* -E<type><width>[<mode>] sEcondary filter */
		bool active;
		char *args;	/* Full filter args for filter1d */
	} E;
	struct F {	/* -F<type><width>[<mode>] Primary filter */
		bool active;
		char *args;	/* Full filter args for filter1d */
	} F;
	struct Q {	/* -Q<min_q>/<max_q> */
		bool active;
		double min, max;
	} Q;
	struct S {	/* -S[b][d][t][u][<weight>] */
		bool active;
		int mode[N_BLENDS];
		int n_blend;
		double weight[N_BLENDS];
	} S;
	struct T {	/* -T<tag> */
		bool active;
		char *prefix;
		char *file;
	} T;
	struct Z {	/* -Z<acut>/<vcut>/<fcut>] */
		bool active;
		double amp_cut, var_cut, f_cut, w_cut;
	} Z;
};

EXTERN_MSC int gmtlib_detrend (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double increment, double *intercept, double *slope, int mode);
EXTERN_MSC int gmtlib_append_ogr_item (struct GMT_CTRL *GMT, char *name, unsigned int type, struct GMT_OGR *S);
EXTERN_MSC void gmtlib_write_ogr_header (FILE *fp, struct GMT_OGR *G);

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct FZBLENDER_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct FZBLENDER_CTRL);
	
	/* Initialize values whose defaults are not 0/NULL */
	C->I.profile = -1;			/* Default is to use all profiles */
	C->Q.min = DEF_Q_MIN;			/* Min blend = Atlantic signal */
	C->Q.max = DEF_Q_MAX;			/* Max blend = Pacitif signal */
	C->S.weight[B_MODEL] = C->S.weight[D_MODEL] = C->S.weight[E_MODEL] = C->S.weight[T_MODEL] = C->S.weight[U_MODEL] = 1.0;	
	C->T.prefix = strdup ("fztrack");	/* Default file prefix */
	C->Z.amp_cut = DEF_Z_AMP_CUT;		/* Minimum significant amplitude */
	C->Z.var_cut = DEF_Z_VAR_CUT;		/* Minimum significant variance reduction */
	C->Z.f_cut = DEF_Z_F_CUT;		/* Minimum significant F value */
	C->Z.w_cut = DEF_Z_W_CUT;		/* Minimum significant width */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct FZBLENDER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->D.file) free (C->D.file);	
	if (C->E.args) free (C->E.args);	
	if (C->F.args) free (C->F.args);	
	if (C->T.prefix) free (C->T.prefix);	
	if (C->T.file) free (C->T.file);	
	gmt_M_free (GMT, C);	
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [-F<primaryfilter>] [-D] [-E<sEcondaryfilter>] [-I<FZid>] "
		"[-Q<qmin>/<qmax>] [-Sbdetu[<weight>]] [-T<prefix>] [%s] [-Z<amp/var/F/width>]\n\n", GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  OPTIONAL ARGUMENTS:\n");

	GMT_Usage (API, 1, "\n-D Save filtered data to <prefix>_filtered_{P|S}.txt. [Delete filtered files].");
	GMT_Usage (API, 1, "\n-E<sEcondaryfilter>");
	GMT_Usage (API, -2, "Sets sEcondary filter.  See -F for filter selections. [No secondary filtering].");
	GMT_Usage (API, 1, "\n-F<primaryfilter>");
	GMT_Usage (API, -2, "Sets primary filter.  Choose from convolution and non-convolution filters "
		"and append full filter <width> (6-sigma width) in km.");
	GMT_Usage (API, -2, "Convolution filters:");
	GMT_Usage (API, 3, "b: Boxcar : Weights are equal.");
	GMT_Usage (API, 3, "c: Cosine arch : Weights given by cosine arch.");
	GMT_Usage (API, 3, "g: Gaussian : Weights given by Gaussian function.");
	GMT_Usage (API, 3, "f: Custom : Weights given in one-column file <name>.");
	GMT_Usage (API, -2, "Non-convolution filters:");
	GMT_Usage (API, 3, "m: Median : Return the median value.");
	GMT_Usage (API, 3, "p: Maximum likelihood probability (mode) estimator : Return the mode:");
	GMT_Usage (API, 4, "+l Return the lowest mode if multiple modes are found [return average mode].");
	GMT_Usage (API, 4, "+u Return the uppermost mode if multiple modes are found [return average mode].");
	GMT_Usage (API, 3, "l: Lower : Return minimum of all points.");
	GMT_Usage (API, 3, "L: Lower+ : Return minimum of all positive points.");
	GMT_Usage (API, 3, "u: Upper : Return maximum of all points.n");
	GMT_Usage (API, 3, "U: Upper- : Return maximum of all negative points.");
	GMT_Usage (API, -2, "Upper case type B, C, G, M, P, F will use robust filter versions, "
		"i.e., replace outliers (2.5 L1 scale (MAD) of median) with median during filtering.");
	GMT_Usage (API, 1, "\n-I<FZid>");
	GMT_Usage (API, -2, "Specify a particular <FZid> id (first FZ is 0) to model "
		"[Default models all FZ traces].");
	GMT_Usage (API, 1, "\n-Q<qmin>/<qmax>");
	GMT_Usage (API, -2, "Specifies how FZ blend modeling is to be done:");
	GMT_Usage (API, 3, "%s <qmin>: Minimum blend value [%g].", GMT_LINE_BULLET, DEF_Q_MIN);
	GMT_Usage (API, 3, "%s <qmax>: Maximum blend value [%g].", GMT_LINE_BULLET, DEF_Q_MAX);
	GMT_Usage (API, -2, "Points whose quality index is below <qmin> is given zero weight, "
		"while points whose quality index is above <qmax> is given unity weight.");
	GMT_Usage (API, 1, "\n-Sbdetu[<weight>]");
	GMT_Usage (API, -2, "Select the FZ estimates to blend by appending the desired code; "
		"Repeatable; optionally, append a custom weight [1]:");
	GMT_Usage (API, 3, "b: Selects the optional trough/edge blend minimum.");
	GMT_Usage (API, 3, "d: Selects the empirical data minimum.");
	GMT_Usage (API, 3, "e: Selects the maximum slope blend model location.");
	GMT_Usage (API, 3, "t: Selects the optional trough model mimimum.");
	GMT_Usage (API, 3, "u: Selects the user's digitized original locations.");
	GMT_Usage (API, 1, "\n--T<prefix>");
	GMT_Usage (API, -2, "Set file prefix for all input/output files [fztrack]. "
		"Note: no files are give on the command line.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-Z<amp/var/F/width>");
	GMT_Usage (API, -2, "Specify four threshold values used to determine quality of fit:");
	GMT_Usage (API, 3, "%s <amp> is the minimum amplitude value [%g].", GMT_LINE_BULLET, DEF_Z_AMP_CUT);
	GMT_Usage (API, 3, "%s <var> is the minimum variance reduction value (in %%).", GMT_LINE_BULLET, DEF_Z_VAR_CUT);
	GMT_Usage (API, 3, "%s <F> is the minimum F value [%g].", GMT_LINE_BULLET, DEF_Z_F_CUT);
	GMT_Usage (API, 3, "%s <width> is the typical FZ width (in km) [%g].", GMT_LINE_BULLET, DEF_Z_AMP_CUT);
	GMT_Option (API, ".");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMTAPI_CTRL *API, struct FZBLENDER_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdsample and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int j, n_files = 0, n_errors = 0;
	char *choice = "bdetu";
	char ta[GMT_LEN64], tb[GMT_LEN64], tc[GMT_LEN64], td[GMT_LEN64];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = API->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Save intermediate filtered results */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				break;
			case 'E':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				Ctrl->E.args = strdup (opt->arg);
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.args = strdup (opt->arg);
				break;
			case 'I':	/* Just pick a single profile for analysis */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.profile = atoi (opt->arg);
				break;
			case 'Q':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				j = sscanf (opt->arg, "%[^/]/%s", ta, tb); 
				Ctrl->Q.min = atof (ta); 
				Ctrl->Q.max = atof (tb); 
				break;
			case 'S':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				switch (opt->arg[0]) {
					case 'b': j = B_MODEL; break;
					case 'd': j = D_MODEL; break;
					case 'e': j = E_MODEL; break;
					case 't': j = T_MODEL; break;
					case 'u': j = U_MODEL; break;
					default:
						GMT_Message (API, GMT_TIME_NONE, "Error -S: Not a valid model type [%c].\n", (int)opt->arg[0]);
						n_errors++;
						j = -1;
						break;
				}
				if (j != -1) {
					if (Ctrl->S.mode[j]) GMT_Message (API, GMT_TIME_NONE, "Option -S%c already selected once!\n", choice[j]);
					Ctrl->S.mode[j] = 1;
					if (opt->arg[1]) Ctrl->S.weight[j] = atof (&opt->arg[1]);
				}
				break;
			case 'T':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				free (Ctrl->T.prefix);
				Ctrl->T.prefix = strdup (opt->arg);
				break;
			case 'Z':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%s", ta, tb, tc, td); 
				Ctrl->Z.amp_cut = atof (ta); 
				Ctrl->Z.var_cut = atof (tb); 
				Ctrl->Z.f_cut   = atof (tc); 
				Ctrl->Z.w_cut   = atof (td); 

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	for (j = 0; j < N_BLENDS; j++) if (Ctrl->S.mode[j]) Ctrl->S.n_blend++;
	
	n_errors += gmt_M_check_condition (GMT, n_files > 0, "GMT SYNTAX ERROR:  No input files should be given on command line (see -T).\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && (Ctrl->Q.min < 0.0 || Ctrl->Q.max > 4.0), "GMT SYNTAX ERROR -Q:  Values must be 0 <= w <= 4.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && !Ctrl->E.args, "GMT SYNTAX ERROR -E:  No secondary along-track filter selected.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && !Ctrl->F.active, "GMT SYNTAX ERROR -E:  Cannot have secondary without primary filter.\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->S.active || Ctrl->S.n_blend == 0, "GMT SYNTAX ERROR -S:  No traces have been selected for blending.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void FZ_fit_quality (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, int r, double a, double v, double f, double w, double *Q)
{	/* Return Q[B_MODEL]=Q[E_MODEL] for blend, Q[T_MODEL] for trough model, Q[E_MODEL] and Q[FZ_EMP] for empirical trough model for this segment's row r */
	if (S->data[POS_AB][r] > a && S->data[POS_VB][r] > v && S->data[POS_FB][r] > f)
		Q[B_MODEL] = 4.0;
	else if (S->data[POS_AB][r] > a && S->data[POS_VB][r] > v)
		Q[B_MODEL] = 3.0;
	else if (S->data[POS_VB][r] > v)
		Q[B_MODEL] = 2.0;
	else if (S->data[POS_AB][r] > a)
		Q[B_MODEL] = 1.0;
	else
		Q[B_MODEL] = 0.0;
	if (gmt_M_is_dnan (S->data[POS_AB][r])) Q[B_MODEL] = GMT->session.d_NaN;	/* Flag as undetermined */
	Q[E_MODEL] = Q[B_MODEL];
	if (S->data[T_MODEL][r] > a && S->data[POS_VT][r] > v && S->data[POS_FT][r] > f)
		Q[T_MODEL] = 4.0;
	else if (S->data[POS_AT][r] > a && S->data[POS_VT][r] > v)
		Q[T_MODEL] = 3.0;
	else if (S->data[POS_VT][r] > v)
		Q[T_MODEL] = 2.0;
	else if (S->data[POS_AT][r] > a)
		Q[T_MODEL] = 1.0;
	else
		Q[T_MODEL] = 0.0;
	if (gmt_M_is_dnan (S->data[POS_AT][r])) Q[T_MODEL] = GMT->session.d_NaN;	/* Flag as undetermined */
	/* For Empirical quality, we only have width and amplitude. Quality should increase with amp and decrease with width.
	 * We form the ratio (amplitude/amp_cut) over (width/width_cut), take atan and scale the result from 0-4, truncated to int */
	Q[D_MODEL] = irint (8.0 * atan ((S->data[POS_AD][r] / a) / (S->data[POS_WD][r]/ w)) / M_PI);
	if (gmt_M_is_dnan (S->data[POS_AD][r]) || gmt_M_is_dnan (S->data[POS_WD][r])) Q[D_MODEL] = GMT->session.d_NaN;	/* Flag as undetermined */
	Q[U_MODEL] = 0.0;	/* This will depend on the others selected */
}

GMT_LOCAL void Ensure_Continuous_Longitudes (struct GMTAPI_CTRL *API, struct GMT_DATASET *D)
{
	struct GMT_DATATABLE *T = D->table[0];	/* Since there is only input one table */
	struct GMT_QUAD *Q = gmt_quad_init (API->GMT, 1);
	unsigned int way, k, loncol[N_LONG_COL] = {POS_XR, POS_XDL, POS_XD0, POS_XDR, POS_XTL, POS_XT0, POS_XTR, POS_XBL, POS_XB0, POS_XBR, POS_XEL, POS_XE0, POS_XER};
	uint64_t fz, row, col;
	char *txt[2] = {"-180 to +180", "0 to 360"};
	for (fz = 0; fz < T->n_segments; fz++) {	/* For each FZ to determine */
		for (row = 0; row < T->segment[fz]->n_rows; row++)
			gmt_quad_add (API->GMT, Q, T->segment[fz]->data[GMT_X][row]);	/* Consider this longitude, the one along the fZ */
	}
	way = gmt_quad_finalize (API->GMT, Q);
	GMT_Report (API, GMT_MSG_VERBOSE, "Range finder %g to %g, selecting longitude formatting for range %s\n", Q[0].min[way], Q[0].max[way], txt[way]);
	API->GMT->current.io.geo.range = Q[0].range[way];	/* Set longitude format based on input range to ensure no jumps in output */
	
	for (fz = 0; fz < T->n_segments; fz++) {	/* For each FZ to determine */
		for (row = 0; row < T->segment[fz]->n_rows; row++) {
			for (k = 0; k < N_LONG_COL; k++) {
				col = loncol[k];
				gmt_lon_range_adjust (Q->range[way], &T->segment[fz]->data[col][row]);
			}
		}
	}
	gmt_M_free (API->GMT, Q);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout(code);}

struct TREND {	/* Holds slope and intercept for each segment column we need to detrend */
	double slp[N_BLEND_COLS];
	double icp[N_BLEND_COLS];
};

#define N_TCOLS	10	/* Number of columns to detrend before filtering */

EXTERN_MSC int GMT_fzblender (void *V_API, int mode, void *args) {
	unsigned int fz, row;
	int error = 0, n_d, n_g, k, n, item, status, ndig;
	int col[N_BLEND_COLS][N_BLENDS] =	/* Columns in the analyzis file for b,d,e,t,u trace parameters */
	{
		{POS_XB0, POS_XD0, POS_XE0, POS_XT0, POS_XR},	/* FZ longitudes */
		{POS_YB0, POS_YD0, POS_YE0, POS_YT0, POS_YR},	/* FZ latitudes */
		{-1,  	      -1,  	-1, 	-1,      -1},	/* Skip distances */
		{POS_SB,  POS_SD,  POS_SE,  POS_ST,  POS_SD},	/* FZ model offsets */
		{POS_WB,  POS_WD,  POS_WB,  POS_WT,  POS_WD},	/* FZ model widths */
		{-1,  	      -1,  	-1, 	-1,      -1},	/* Put quality index here */
		{POS_XBL, POS_XDL, POS_XEL, POS_XTL, POS_XDL},	/* FZ left lon */
		{POS_YBL, POS_YDL, POS_YEL, POS_YTL, POS_YDL},	/* FZ left lat */
		{POS_XBR, POS_XDR, POS_XER, POS_XTR, POS_XDR},	/* FZ right lon */
		{POS_YBR, POS_YDR, POS_YER, POS_YTR, POS_YDR}	/* FZ right lat */
	};
	int tcols[N_TCOLS] = {POS_XR, POS_YR, POS_XB0, POS_YB0, POS_SB, POS_WB, POS_XBL, POS_YBL, POS_XBR, POS_YBR};
	
	char buffer[BUFSIZ], run_cmd[BUFSIZ], *cmd = NULL;
	char source[GMT_VF_LEN] = {""}, destination[GMT_VF_LEN] = {""};
	
	double Q[N_BLENDS], P[N_BLENDS], q_weight[N_BLENDS], sum_P, sum_w, sum_q, i_q_range, q_model;
	
	struct GMT_OPTION *options = NULL;
	struct FZBLENDER_CTRL *Ctrl = NULL;
	struct TREND *trend = NULL;
	struct GMT_DATASET *Fin = NULL, *Fout = NULL;
	struct GMT_DATATABLE *Tin = NULL, *Tout = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the program-specific arguments */

	GMT = gmt_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the fzblender main code ----------------------------*/
	
	sprintf (buffer, "%s_analysis.txt", Ctrl->T.prefix);
	Ctrl->In.file = strdup (buffer);
	GMT->current.setting.io_header[GMT_OUT] = 1;	/* To allow writing of headers */
	
	GMT_Report (API, GMT_MSG_NORMAL, "Read FZ analysis file %s\n", Ctrl->In.file);
	if ((Fin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, Ctrl->In.file, NULL)) == NULL) Return ((error = GMT_DATA_READ_ERROR));
	
	Ensure_Continuous_Longitudes (API, Fin);	/* Set longitude to 0-360 or -180/180 so there are no jumps */
	
	/* Set up the primary GMT_filter1d cmd call */
	
	if (Ctrl->F.active) {	/* Wants to filter before blending */
		/* Must remove linear trends from the lon/lat columns before filtering since median fails on sloping lines */
		Tin = Fin->table[0];	/* Since there is only input one table */
		trend =  gmt_M_memory (GMT, NULL, Tin->n_segments, struct TREND);
		for (fz = 0; fz < Tin->n_segments; fz++) {	/* For each FZ to determine */
			GMT_Report (API, GMT_MSG_VERBOSE, "Detrend FZ %s [segment %ld]\n", Tin->segment[fz]->label, fz);
			if (Ctrl->I.profile >= 0 || fz == (unsigned int)Ctrl->I.profile) continue;	/* Skip all but selected profile */
			for (k = 0; k < N_TCOLS; k++) {	/* Detrend key columns */
				gmtlib_detrend (GMT, Tin->segment[fz]->data[POS_DR], Tin->segment[fz]->data[tcols[k]], Tin->segment[fz]->n_rows, 0.0, &trend[fz].icp[k], &trend[fz].slp[k], -1);
			}
		}
		if (Ctrl->D.active) {
			/* Save the detrended temporary file */
			sprintf (buffer, "%s_detrended.txt", Ctrl->T.prefix);
			GMT_Report (API, GMT_MSG_VERBOSE, "Save detrended data to temporary file %s\n", buffer);
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, 0, NULL, buffer, Fin) != GMT_NOERROR) Return ((error = GMT_DATA_WRITE_ERROR));
		}
		
		/* Register output for filter1d results */
		/* Create virtual files for using the data in filter1d and another for holding the result */
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN|GMT_IS_REFERENCE, Fin, source) != GMT_NOERROR) {
			Return (API->error);
		}
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT|GMT_IS_REFERENCE, NULL, destination) == GMT_NOTSET) {
			Return (API->error);
		}
		
		GMT_Report (API, GMT_MSG_NORMAL, "Perform primary filtering\n");
		//sprintf (buffer, "-F%s -E -N%d %s ->%s", Ctrl->F.args, POS_DR, p_in_string, p_out_string);
		sprintf (buffer, "-F%s -N%d %s ->%s", Ctrl->F.args, POS_DR, source, destination);
		GMT_Report (API, GMT_MSG_DEBUG, "Args to primary filter1d: %s\n", buffer);
		if ((status = GMT_Call_Module (API, "filter1d", GMT_MODULE_CMD, buffer))) {
			GMT_Report (API, GMT_MSG_NORMAL, "GMT SYNTAX ERROR:  Primary filtering failed with exit code %d!\n", status);
			Return (status);
		}
		GMT_Report (API, GMT_MSG_DEBUG, "filter1d completed\n");
		/* Close the source virtual file and destroy the data set */
		if (GMT_Close_VirtualFile (GMT->parent, source) != GMT_NOERROR) {
			Return (API->error);
		}
		GMT_Destroy_Data (API, &Fin);
		
		if (Ctrl->E.active) {	/* Now apply the secondary filter */
			struct GMT_DATASET *D = NULL;
			char s_in_string[GMT_LEN256], s_out_string[GMT_LEN256];
			/* Retrieve the primary filtering results */
			if ((D = GMT_Read_VirtualFile (API, destination)) == NULL) {
				Return (API->error);
			}
			/* Close the destination virtual file */
			if (GMT_Close_VirtualFile (GMT->parent, destination) != GMT_NOERROR) {
				Return (API->error);
			}
			if (Ctrl->D.active) {
				/* Save the primary filtered file */
				sprintf (buffer, "%s_primary_filter_P.txt", Ctrl->T.prefix);
				GMT_Report (API, GMT_MSG_VERBOSE, "Save primary filtered detrended data to temporary file %s\n", buffer);
				if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, 0, NULL, buffer, D) != GMT_NOERROR) Return ((error = GMT_DATA_WRITE_ERROR));
			}
			/* Setup in/out for secondary filter */
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN|GMT_IS_REFERENCE, D, source) != GMT_NOERROR) {
				Return (API->error);
			}
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT|GMT_IS_REFERENCE, NULL, destination) == GMT_NOTSET) {
				Return (API->error);
			}
			//sprintf (buffer, "-F%s -E -N%d %s ->%s", Ctrl->E.args, POS_DR, s_in_string, s_out_string);
			sprintf (buffer, "-F%s -N%d %s ->%s", Ctrl->E.args, POS_DR, source, destination);
			GMT_Report (API, GMT_MSG_DEBUG, "Args to secondary filter1d: %s\n", buffer);
			if ((status = GMT_Call_Module (API, "filter1d", GMT_MODULE_CMD, buffer))) {
				GMT_Report (API, GMT_MSG_NORMAL, "GMT SYNTAX ERROR:  Secondary filtering failed with exit code %d!\n", status);
				Return (status);
			}
			if (GMT_Close_VirtualFile (GMT->parent, source) != GMT_NOERROR) {
				Return (API->error);
			}
			GMT_Destroy_Data (API, &D);
		}
		/* Retrieve the final filtering results */
		if ((Fin = GMT_Read_VirtualFile (API, destination)) == NULL) {
			Return (API->error);
		}
		/* Close the destination virtual file */
		if (GMT_Close_VirtualFile (GMT->parent, destination) != GMT_NOERROR) {
			Return (API->error);
		}
		if (Ctrl->D.active && Ctrl->E.active) {
			/* Save the primary or secondary filtered file */
			char F = (Ctrl->E.active) ? 'S' : 'P';
			char *str = (Ctrl->E.active) ? "secondary" : "primary";
			sprintf (buffer, "%s_primary_filter_%c.txt", Ctrl->T.prefix, F);
			GMT_Report (API, GMT_MSG_VERBOSE, "Save %s filtered detrended data to temporary file %s\n", str, buffer);
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, 0, NULL, buffer, Fin) != GMT_NOERROR) Return ((error = GMT_DATA_WRITE_ERROR));
		}
			
		/* Now restore original trends */
		GMT_Report (API, GMT_MSG_DEBUG, "Restore original trends\n");
		Tin = Fin->table[0];	/* Since there is only input one table */
		for (fz = 0; fz < Tin->n_segments; fz++) {	/* For each FZ to determine */
			GMT_Report (API, GMT_MSG_VERBOSE, "Restore trends for FZ %s [segment %ld]\n", Tin->segment[fz]->label, fz);
			if (Ctrl->I.profile >= 0 || fz == (unsigned int)Ctrl->I.profile) continue;	/* Skip all but selected profile */
			for (k = 0; k < N_TCOLS; k++) {	/* Restore trend to key columns */
				gmtlib_detrend (GMT, Tin->segment[fz]->data[POS_DR], Tin->segment[fz]->data[tcols[k]], Tin->segment[fz]->n_rows, 0.0, &trend[fz].icp[k], &trend[fz].slp[k], +1);
			}
		}
		gmt_M_free (GMT, trend);
	}
	
	/* OK, all input/filtering is completed */
	
	Tin = Fin->table[0];	/* Since there is only input one table */
	if (Tin->segment[0]->n_columns < N_FZ_ANALYSIS_COLS) {	/* Trouble */
		GMT_Message (API, GMT_TIME_NONE, "GMT SYNTAX ERROR:  %s does not have %d columns\n", Ctrl->In.file, N_FZ_ANALYSIS_COLS);
		Return (EXIT_FAILURE);
	}
	
	if (Ctrl->D.active && Ctrl->F.active) {
		sprintf (buffer, "%s_filtered.txt", Ctrl->T.prefix);
		Ctrl->D.file = strdup (buffer);
		GMT_Report (API, GMT_MSG_NORMAL, "Filtered analysis results saved to %s\n", Ctrl->D.file);
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, 0, NULL, Ctrl->D.file, Fin) != GMT_NOERROR) Return ((error = GMT_DATA_WRITE_ERROR));
	}
	
	/* Specify geographic columns where needed for blend output */
	
	GMT->current.io.col_type[GMT_OUT][OUT_LON0] = GMT_IS_LON;	GMT->current.io.col_type[GMT_OUT][OUT_LAT0] = GMT_IS_LAT;
	GMT->current.io.col_type[GMT_OUT][OUT_LONL] = GMT_IS_LON;	GMT->current.io.col_type[GMT_OUT][OUT_LATL] = GMT_IS_LAT;
	GMT->current.io.col_type[GMT_OUT][OUT_LONR] = GMT_IS_LON;	GMT->current.io.col_type[GMT_OUT][OUT_LATR] = GMT_IS_LAT;

	GMT_Report (API, GMT_MSG_DEBUG, "Perform blending\n");
	cmd = GMT_Create_Cmd (API, options);
	sprintf (run_cmd, "# %s %s", GMT->init.module_name, cmd);	/* Build command line argument string */
	gmt_M_free (GMT, cmd);

	Fout  = gmt_alloc_dataset (GMT, Fin, 0, N_BLEND_COLS, GMT_ALLOC_NORMAL);	/* Same table length as FZ, but with N_BLEND_COLS columns */
	Tout = Fout->table[0];
	Tout->n_headers = 3;
	Tout->header = gmt_M_memory (GMT, NULL, Tout->n_headers, char *);
	Tout->header[0] = strdup ("# Blended FZ traces");
	Tout->header[1] = strdup (run_cmd);
	Tout->header[2] = strdup ("# lon\tlat\tdist\tshift\twidth\tqweight\tlon_l\tlat_l\tlon_r\tlat_r");
	
	i_q_range = 1.0 / (Ctrl->Q.max - Ctrl->Q.min);
	gmt_M_memset (q_weight, N_BLENDS, double);	/* So those that are not set below remain 0 */
	
	for (fz = 0; fz < Tin->n_segments; fz++) {	/* For each FZ to determine */

		GMT_Report (API, GMT_MSG_NORMAL, "Process FZ %s [segment %ld]\n", Tin->segment[fz]->label, fz);
		
		if (Ctrl->I.profile >= 0 || fz == (unsigned int)Ctrl->I.profile) {	/* Skip all but selected profile */
			struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (Tout->segment[fz]);
			SH->mode = GMT_WRITE_SKIP;	/* Ignore on output */
			continue;
		}
	
		ndig = irint (floor (log10 ((double)Tin->segment[fz]->n_rows))) + 1;	/* Determine how many decimals are needed for largest FZ id */
		
		for (row = 0; row < Tin->segment[fz]->n_rows; row++) {	/* Process each point along digitized FZ trace */
			
			FZ_fit_quality (GMT, Tin->segment[fz], row, Ctrl->Z.amp_cut, Ctrl->Z.var_cut, Ctrl->Z.f_cut, Ctrl->Z.w_cut, Q);	/* Compute quality indeces for blend, trough, empirical models */
			for (k = n = 0, sum_q = 0.0; k < N_BLENDS-1; k++) {	/* Compute quality weights for each model trace */
				if (!Ctrl->S.mode[k]) continue;
				q_weight[k] = (Q[k] - Ctrl->Q.min) * i_q_range;
				sum_q += q_weight[k];
				n++;	/* Count the candidates selected */
			}
			q_model = (n) ? sum_q / n : 0.0;
			q_weight[N_BLENDS-1] = 1.0 - q_model;	/* Estimate remaining q_weight for user profile */
			
			/* Now compute the blended output */
			
			for (item = 0; item < N_BLEND_COLS; item++) {
				if (item == OUT_DIST) {	/* No blending, just pass on along-track distance */
					Tout->segment[fz]->data[item][row] = Tin->segment[fz]->data[POS_DR][row];
					continue;
				}
				if (item == OUT_QWHT) {	/* Store the quality weight for model */
					Tout->segment[fz]->data[item][row] = q_model;
					continue;
				}
				/* Get the 4 candidates for blending */
				for (k = 0; k < N_BLENDS; k++) P[k] = Tin->segment[fz]->data[col[item][k]][row];
				if (item == OUT_SHFT) P[N_BLENDS-1] = 0.0;	/* Digitized trace defines origin so offset == 0 */
				if (item == OUT_LON0 || item == OUT_LONL || item == OUT_LONR) {	/* Must be careful with longitudes */
					for (k = n_g = n_d = 0; k < N_BLENDS; k++) {	/* Determine if we have longs across Greenwich */
						if (P[k] > 350.0) n_d++;
						if (P[k] <  10.0) n_g++;
					}
					if (n_d && n_g)	/* Some longitudes crossing 0/360 line, switch to -180/180 mode */
						for (k = 0; k < N_BLENDS; k++) if (P[k] > 180.0) P[k] -= 360.0;
				}
				for (k = 0, sum_P = sum_w = 0.0; k < N_BLENDS; k++) {	/* Those with q_weight == 0 or not requested won't contribute */
					if (!Ctrl->S.mode[k]) continue;
					sum_P += P[k] * q_weight[k] * Ctrl->S.weight[k];
					sum_w += q_weight[k] * Ctrl->S.weight[k];
				}
				Tout->segment[fz]->data[item][row] = sum_P / sum_w;
			}
		}
	
		/* Update the header */

		sprintf (buffer, "Blend result for trace FZ -L\"%s\" [segment %d]", Tin->segment[fz]->label, fz);
		Tout->segment[fz]->header = strdup (buffer);
	}

	/* Store final blended profiles */
	
	sprintf (buffer, "%s_blend.txt", Ctrl->T.prefix);
	Ctrl->T.file = strdup (buffer);
	GMT_Report (API, GMT_MSG_NORMAL, "Save FZ blend results to %s\n", Ctrl->T.file);
	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, 0, NULL, Ctrl->T.file, Fout) != GMT_NOERROR) Return ((error = GMT_DATA_WRITE_ERROR));

	Return (GMT_NOERROR);
}
