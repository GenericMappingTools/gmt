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
 * sph2grd evaluates a grid using a spherical harmonics model
 *
 * Author:	Paul Wessel
 * Date:	1-JUN-2013
 */
 
#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"sph2grd"
#define THIS_MODULE_MODERN_NAME	"sph2grd"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Compute grid from spherical harmonic coefficients"
#define THIS_MODULE_KEYS	"<D{,GG}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "->RVbdhirs" GMT_ADD_x_OPT

#ifndef M_LN2
#define M_LN2 0.69314718055994530942  /* log_e 2 */
#endif

enum Sph2grd_fmode {
	SPH2GRD_BANDPASS = 1,
	SPH2GRD_GAUSSIAN = 2,
};

struct SPH2GRD_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct D {	/* -D */
		bool active;
		char mode;
	} D;
	struct E {	/* -E */
		bool active;
	} E;
	struct G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct F {	/* -F[k]<lc>/<lp>/<hp>/<hc> or -F[k]<lo>/<hi> */
		bool active;
		bool km;	/* True if filter was specified in km instead of harmonic degree */
		int mode;
		double lc, lp, hp, hc;
	} F;
	struct N {	/* -Ng|m|s */
		bool active;
		char mode;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPH2GRD_CTRL *C = NULL;
	
	C = gmt_M_memory (GMT, NULL, 1, struct SPH2GRD_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->F.hc = C->F.hp = DBL_MAX;	/* No high-cutting is the default */
	C->F.lc = C->F.lp = 0.0;	/* No low-cutting is the default */
	
	C->N.mode = 'm';
		
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SPH2GRD_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [coeff_file] -G<grdfile> %s\n", name, GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-Dg|n] [-E] [-F[k]<filter>] [-N<norm>] [-Q]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s]%s [%s]\n\n",
		GMT_Rgeo_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_s_OPT, GMT_x_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-G filename for output grid file.\n");
	GMT_Option (API, "I,Rg");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "	The input is expected to contain records of degree, order, cos, sin.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Will evaluate a derived field from a geopotential model.  Choose between\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Dg will compute the gravitational field [Add -E for anomalies on ellipsoid]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Dn will compute the geoid [Add -E for anomalies on ellipsoid]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E to evaluate expansion on the current ellipsoid [Default is sphere]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Filter coefficients according to one of two kinds of filter specifications:.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Fk if values are given in km [Default is coefficient degree L]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a) Cosine band-pass: Append four wavelengths <lc>/<lp>/<hp>/<hc>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      coefficients outside <lc>/<hc> are cut; inside <lp>/<hp> are passed, rest are tapered.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Replace wavelength by - to skip, e.g., -F-/-/50/75 is a low-pass filter.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   b) Gaussian band-pass: Append two wavelengths <lo>/<hi> where filter amplitudes = 0.5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Replace wavelength by - to skip, e.g., -F70/- is a high-pass Gaussian filter.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Normalization used for coefficients.  Choose among\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m: Mathematical normalization - inner products summed over surface equal 1 [Default]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g: Geodesy normalization - inner products summed over surface equal 4pi\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s: Schmidt normalization - as used in geomagnetism\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Coefficients have phase convention from physics, i.e., the (-1)^m factor\n");
	GMT_Option (API, "V,bi4,e,h,i,r,s,x,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SPH2GRD_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to sph2grd and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n;
	char A[GMT_LEN32] = {""}, B[GMT_LEN32] = {""}, D[GMT_LEN32] = {""}, E[GMT_LEN32] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Evaluate derivative solutions */
				Ctrl->D.active = true;
				Ctrl->D.mode = opt->arg[0];
				GMT_Report (API, GMT_MSG_NORMAL, "Comment -D: Not implemented yet.\n");
				break;
			case 'E':	/* Evaluate on ellipsoid */
				Ctrl->E.active = true;
				GMT_Report (API, GMT_MSG_NORMAL, "Comment -E: Not implemented yet.\n");
				break;
			case 'F':	/* Bandpass or Gaussian filter -F[k]<lc>/<lp>/<hp>/<hc> or -F[k]<lo>/<hi>*/
				Ctrl->F.active = true;
				k = 0;
				if (opt->arg[0] == 'k') Ctrl->F.km = true, k = 1;	/* Convert later when GMT is initialized */
				n = sscanf (&opt->arg[k], "%[^/]/%[^/]/%[^/]/%s", A, B, D, E);
				if (n == 4) {	/* Bandpass */
					if (A[0] != '-') Ctrl->F.lc = atof (A);
					if (B[0] != '-') Ctrl->F.lp = atof (B);
					if (D[0] != '-') Ctrl->F.hp = atof (D);
					if (E[0] != '-') Ctrl->F.hc = atof (E);
					Ctrl->F.mode = SPH2GRD_BANDPASS;
				}
				else if (n == 2) {	/* Gaussian filter */
					if (A[0] != '-') Ctrl->F.lc = atof (A);
					if (B[0] != '-') Ctrl->F.hp = atof (B);
					Ctrl->F.mode = SPH2GRD_GAUSSIAN;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F: Cannot find 2 or 4 tokens separated by slashes.\n");
					n_errors++;
				}
				break;
			case 'G':
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.mode = opt->arg[0];
				break;
			case 'Q':
				Ctrl->Q.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Syntax error: Must specify output grid file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && !(Ctrl->D.mode == 'g' || Ctrl->D.mode == 'n'), "Syntax error -D option: Must append g or n\n");
	n_errors += gmt_M_check_condition (GMT, strchr ("mgs", Ctrl->N.mode) == NULL, "Syntax error: -N Normalization must be one of m, g, or s\\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define LM_index(L,M) ((L)*((L)+1)/2+(M))	/* Index into the packed P_LM array given L, M */

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_sph2grd (void *V_API, int mode, void *args) {
	bool ortho = false, duplicate_col;
	int row, col, n_columns, error, L_sign = 1, L, L_min = 0, L_max = 0, M, M_max = 0, kk = 0;
	unsigned int n_PLM, n_CS, n_CS_nx, next_10_percent = 10;
	uint64_t tbl, seg, drow, node, k;
	char text[GMT_LEN32] = {""};
	double lon, lat, sum, lo, hi, filter, percent_inc, percent = 0;
	struct GMT_GRID *Grid = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *T = NULL;
	double **C = NULL, **S = NULL, **Cosm = NULL, **Sinm = NULL;
	double *Cosmx = NULL, *Sinmx = NULL, *P_lm = NULL;
	struct SPH2GRD_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);
	
	/*---------------------------- This is the sph2grd main code ----------------------------*/

	gmt_enable_threads (GMT);	/* Set number of active threads, if supported */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Process input coefficients\n");
	for (col = 0; col < 4; col++) gmt_set_column (GMT, GMT_IN, col, GMT_IS_FLOAT);	/* Not reading lon,lat in this program */
	
	if ((error = GMT_Set_Columns (API, GMT_IN, 4, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (D->n_columns < 4) {
		GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 4 are needed\n", (int)D->n_columns);
		Return (GMT_DIM_TOO_SMALL);
	}
	gmt_set_geographic (GMT, GMT_IN);	/* But x and y are really lon,lat in the rest of the program */
	
	if (Ctrl->F.active && Ctrl->F.km) {	/* Convert cutoffs to harmonic degrees */
		double scale = 360.0 * GMT->current.proj.DIST_KM_PR_DEG;
		Ctrl->F.lc = scale / Ctrl->F.lc;
		Ctrl->F.lp = scale / Ctrl->F.lp;
		Ctrl->F.hp = scale / Ctrl->F.hp;
		Ctrl->F.hc = scale / Ctrl->F.hc;
	}
	
	/* First loop over all records and determine the highest L, M */
	
	for (tbl = 0; tbl < D->n_tables; tbl++) for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
		T = D->table[tbl]->segment[seg];
		for (drow = 0; drow < T->n_rows; drow++) {
			L = irint (T->data[0][drow]);
			M = irint (T->data[1][drow]);
			if (L > L_max) L_max = L;
			if (M > M_max) M_max = M;
		}
	}

	if (M_max > L_max) {
		GMT_Report (API, GMT_MSG_VERBOSE, "M_max = %d exceeds L_max = %d, wrong column order?\n", M_max, L_max);
		Return (GMT_RUNTIME_ERROR);
	}
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Coefficient file has L_max = %d and M_max = %d\n", L_max, M_max);
	
	if (Ctrl->F.active && Ctrl->F.mode == SPH2GRD_BANDPASS) {	/* See if we can save work by ignoring low or high terms */
		L = (Ctrl->F.hc < DBL_MAX) ? irint (Ctrl->F.hc) : INT_MAX;	/* Get the highest L needed given the high-cut filter */
		if (L_max > L) {
			L_max = L;
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Chosen high-cut bandpass filter sets effective L_max = %d and M_max = %d\n", L_max, MIN(L_max,M_max));
		}
		L = irint (Ctrl->F.lc);	/* Get the lowest L needed given the low-cut filter */
		if (L > L_min) {
			L_min = L;
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Chosen low-cut bandpass filter sets effective L_min = %d\n", L_min);
		}
	}
	
	/* Allocate C[L][M] and S[L][M] arrays to simplify accessing the coefficients in the big loop */
	gmt_M_malloc2 (GMT, C, S, L_max + 1, NULL, double *);
	for (L = 0; L <= L_max; L++) gmt_M_malloc2 (GMT, C[L], S[L], L_max + 1, NULL, double);

	/* Place the coefficients into the C and S arrays and apply filtering, if selected */

	for (tbl = 0; tbl < D->n_tables; tbl++) {
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
			T = D->table[tbl]->segment[seg];	/* Short-hand notation for current segment */
			for (drow = 0; drow < T->n_rows; drow++) {
				L = irint (T->data[0][drow]);
				if (L > L_max) continue;	/* Skip stuff beyond the high cut-off filter */
				if (L < L_min) continue;	/* Skip stuff beyond the low cut-off filter */
				M = irint (T->data[1][drow]);
				C[L][M]  = T->data[2][drow];
				S[L][M]  = T->data[3][drow];
				if (!Ctrl->F.active) continue;	/* No filtering selected */
				if (Ctrl->F.mode == SPH2GRD_BANDPASS) {	/* Note: L_min/L_max have already taken care of the low-cut and high-cut */
					if (L < Ctrl->F.lp)	/* Taper the low order components */
						filter = 0.5 * (1.0 + cos (M_PI * (Ctrl->F.lp - L) / (Ctrl->F.lp - Ctrl->F.lc)));
					else if (L > Ctrl->F.hp)	/* Taper the high order components */
						filter = 0.5 * (1.0 + cos (M_PI * (L - Ctrl->F.hp) / (Ctrl->F.hc - Ctrl->F.hp)));
					else	/* We are inside the band where filter == 1.0 */
						continue;	/* No need to scale by 1 */
				}
				else {	/* Gaussian filter(s) */
					lo = (Ctrl->F.lc > 0.0) ? exp (-M_LN2 * pow (L / Ctrl->F.lc, 2.0)) : 0.0;	/* Low-pass part */
					hi = (Ctrl->F.hp < DBL_MAX)  ? exp (-M_LN2 * pow (L / Ctrl->F.hp, 2.0)) : 1.0;	/* Hi-pass given by its complementary low-pass */
					filter = hi - lo;	/* Combined filter */
				}
				C[L][M] *= filter;
				S[L][M] *= filter;
			}
		}
	}

	/* We are done with the input table; remove it to free up memory */
	if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
		Return (API->error);
	}
	
	if (Ctrl->Q.active) {	/* Set Condon-Shortley phase flag */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Legendre polynomials will include the Condon-Shortley phase\n");
		L_sign = -1;
	}
	if (Ctrl->N.mode == 'm') {		/* Set ortho flag */
		ortho = true;
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Mathematical normalization - inner products summed over surface equal 1\n");
	}
	if (Ctrl->N.mode == 'g') GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Geodesy normalization - inner products summed over surface equal 4pi\n");
	if (Ctrl->N.mode == 's') GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Schmidt normalization - as used in geomagnetism\n");
	
	/* Allocate output grid */
	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, "Grid evaluated from spherical harmonic coefficients", Grid)) Return (API->error);
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);

	n_PLM = LM_index (L_max + 1, L_max + 1);	/* Number of P_lm terms needed */
	n_CS = L_max + 1;				/* Number of Cos,Sin terms needed for each longitude */
	n_CS_nx = n_CS * Grid->header->n_columns;		/* Number of Cos,Sin terms needed for all longitudes */
	P_lm  = gmt_M_memory (GMT, NULL, n_PLM, double);
	gmt_M_malloc2 (GMT, Cosmx, Sinmx, n_CS_nx, NULL, double);
	gmt_M_malloc2 (GMT, Cosm,  Sinm,  Grid->header->n_columns, NULL, double *);
	
	/* Evaluate longitude terms once and for all to avoid doing it repeatedly in the big loop.
	 * We compute a matrix with rows representing order M and columns representing longitude.
	 * Then, we allocate a set of pointers assigned to each row to enable 2-D indexing.*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Evaluate exp (i*m*lon) for all M [%d] and all lon [%u]\n", M_max+1, Grid->header->n_columns);
	k = 0;
	gmt_M_col_loop2 (GMT, Grid, col) {	/* Evaluate all sin, cos terms */
		lon = gmt_M_grd_col_to_x (GMT, col, Grid->header);	/* Current longitude */
		for (M = 0; M <= L_max; M++, k++) sincosd (lon * M, &Sinmx[k], &Cosmx[k]);
	}
	GMT_Report (API, GMT_MSG_DEBUG, "Array sizes: n_PLM = %u, n_CS = %u n_CS_nx = %u\n", n_PLM, n_CS, n_CS_nx);
	gmt_M_col_loop2 (GMT, Grid, col) {	/* Assign pointers for 2-D indexing */
		k = col * (L_max + 1);	/* Start 1-D array index in Cosmx/Sinmx for this longitude */
		Cosm[col] = &Cosmx[k];	/* Cosm[col][M] has cos(M*lon) terms for all lon[col], fixed M <= L_max */
		Sinm[col] = &Sinmx[k];	/* Sinm[col][M] has sin(M*lon) terms for all lon[col], fixed M <= L_max */
	}
	if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) {	/* Memory reporting */
		unsigned int kind = 0;
		size_t n_bytes = sizeof (struct GMT_GRID);
		double mem;
		char *unit = "KMG";	/* Kilo-, Mega-, Giga- */
		n_bytes += Grid->header->size * sizeof (gmt_grdfloat);			/* Grid */
		n_bytes += 2 * (L_max + 1) * sizeof (double *);			/* C[] and S[] pointers */
		n_bytes += 2 * (L_max + 1) * (L_max + 1) * sizeof (double);	/* C[] and S[] contents */
		n_bytes += n_PLM * sizeof (double);				/* P_lm */
		n_bytes += 2 * n_CS_nx * sizeof (double);			/* Sinmx and Cosmn */
		n_bytes += 2 * Grid->header->n_columns * sizeof (double *);		/* Sinm and Cosm pointers */
		
		mem = n_bytes / 1024.0;	/* Report kbytes unless it is too much */
		while (mem > 1024.0 && kind < 2) { mem /= 1024.0;	kind++; }	/* Goto next higher unit */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Using a total of %.3g %cb for grid and all arrays.\n", mem, unit[kind]);
	}
	percent_inc = 100.0 / Grid->header->n_rows;	/* Percentage of whole grid represented by one row */
	duplicate_col = (gmt_M_360_range (Grid->header->wesn[XLO], Grid->header->wesn[XHI]) && Grid->header->registration == GMT_GRID_NODE_REG);	/* E.g., lon = 0 column should match lon = 360 column */
	n_columns = (duplicate_col) ? Grid->header->n_columns - 1 : Grid->header->n_columns;
	
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Start evaluating the spherical harmonic series\n");
	
	/* Below section will become parallellized via OpenMP soon.
	 * Shared:  Grid, L_sign, L_max, L_min, ortho, Cosm, Sinm, Cosmx, Sinmx.
	 * Private: P_lm, row, lat, col, node, sum, k, L, M.
	 * Probably redo the verbosity to deal with multiple threads: Split 100% over
	 * the n_threads, split 10/n_treads into nearest integer.
	 */
	
	gmt_M_row_loop (GMT, Grid, row) {					/* For each output latitude */
		lat = gmt_M_grd_row_to_y (GMT, row, Grid->header);	/* Current latitude */
		/* Compute all P_lm needed for this latitude at once via gmt_plm_bar_all */
		gmt_plm_bar_all (GMT, L_sign * L_max, sind (lat), ortho, P_lm);	/* sind(lat) = cosine of colatitude */
		if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) {	/* Give user feedback on progress every 10 percent */
			percent += percent_inc;
			if (percent > (double)next_10_percent) {
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Finished %3.3d %% of evaluation\n", next_10_percent);
				next_10_percent = urint (ceil (percent / 10.0)) * 10;
			}
			gmt_ascii_format_col (GMT, text, lat, GMT_OUT, GMT_Y);
			GMT_Report (API, GMT_MSG_DEBUG, "Working on latitude: %s\n", text);
		}
#ifdef _OPENMP
#pragma omp parallel for private(col,node,sum,kk,L,M) shared(Grid,row,n_columns,L_min,L_max,P_lm,C,Cosm,S,Sinm)
#endif
		for (col = 0; col < n_columns; col++) {	/* For each longitude along this parallel */
			sum = 0.0;	/* Initialize sum to zero for new output node */
			kk = (L_min) ? LM_index (L_min, 0) : 0;	/* Set start index for P_lm packed array */
			for (L = L_min; L <= L_max; L++) {	/* For all degrees */
				for (M = 0; M <= L; M++, kk++) {	/* For all orders <= L */
					sum += P_lm[kk] * (C[L][M] * Cosm[col][M] + S[L][M] * Sinm[col][M]);
				}
			}
			node = gmt_M_ijp (Grid->header, row, col);
			Grid->data[node] = (gmt_grdfloat)sum;	/* Assign total to the grid, cast as gmt_grdfloat */
		}
	}
	if (duplicate_col) {	/* Just copy over what we found on the western boundary to the repeated eastern boundary */
		uint64_t node_L, node_R;
		gmt_M_row_loop (GMT, Grid, row) {	/* For each output latitude */
			node_L = gmt_M_ijp (Grid->header, row, 0);	/* West */
			node_R = node_L + Grid->header->n_columns - 1;		/* East */
			Grid->data[node_R] = Grid->data[node_L];
		}
	}
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Finished 100 %% of evaluation\n");
	
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Write grid to file\n");
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR)
		error = API->error;
	else
		error = 0;
	
	/* Clear up by freeing memory */
	
	gmt_M_free (GMT, P_lm);
	gmt_M_free (GMT, Cosm);
	gmt_M_free (GMT, Sinm);
	gmt_M_free (GMT, Cosmx);
	gmt_M_free (GMT, Sinmx);
	for (L = 0; L <= L_max; L++) {
		gmt_M_free (GMT, C[L]);
		gmt_M_free (GMT, S[L]);
	}
	gmt_M_free (GMT, C);
	gmt_M_free (GMT, S);
	
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Completed\n");
	Return ((error) ? error : EXIT_SUCCESS);
}
