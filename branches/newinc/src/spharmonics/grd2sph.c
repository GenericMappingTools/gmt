/*--------------------------------------------------------------------
 *    $Id$
 *
 *	Copyright (c) 1991-2013 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * sph2grd evalutes a grid using a spherical harmonics model
 *
 * Author:	Paul Wessel
 * Date:	1-JUN-2006
 */
 
#include "gmt_dev.h"

#define THIS_MODULE_NAME	"grd2sph"
#define THIS_MODULE_PURPOSE	"Compute spherical harmonic coefficients from grid"

#define GMT_PROG_OPTIONS "->RVbhirs"

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
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct L {	/* -L<lc>/<lp>/<hp>/<hc> or -L<lo>/<hi> */
		bool active;
		int mode;
		double lc, lp, hp, hc;
	} L;
	struct N {	/* -Ng|m|s */
		bool active;
		char mode;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
};

void *New_sph2grd_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPH2GRD_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct SPH2GRD_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->N.mode = 'm';
		
	return (C);
}

void Free_sph2grd_Ctrl (struct GMT_CTRL *GMT, struct SPH2GRD_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free (C->G.file);	
	GMT_free (GMT, C);	
}

int GMT_sph2grd_usage (struct GMTAPI_CTRL *API, int level)
{
	char type[3] = {'l', 'a', 'c'};

	GMT_Message (API, GMT_TIME_NONE, "sph2grd - Evaluate spherical harmonic models on a grid\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: sph2grd [coeff_file] %s %s [-Dg|n]\n", GMT_I_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-E] [-F] [-G<grdfile>] [-L[d]<filter>] [-N<norm>] [-Q] [%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_bi_OPT, GMT_h_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_explain_options ("I,Rg");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "	The input is expected to contain records of degree, order, cos, sin.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Will evaluate a derived field from a geopotential model.  Choose between\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Dg will compute the gravitational field [Add -E for anomalies on ellipsoid]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Dn will compute the geoid [Add -E for anomalies on ellipsoid]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E to evaluate expansion on the current ellipsoid [Default is sphere]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Force pixel registration [Default is gridline registration].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G filename for output grid file\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Filter coefficients according to one of two kinds of filter specifications:.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Ld if values are given in terms of coefficient degrees [Default is km]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a) Cosine band-pass: Append four wavelengths <lc>/<lp>/<hp>/<hc>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      coefficients outside <lc>/<hc> are cut; inside <lp>/<hp> are passed, rest are tapered.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Replace wavelength by - to skip, e.g., -L-/-/500/100 is a low-pass filter.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   b) Gaussian band-pass: Append two wavelengths <lo>/<hi> where filter amplitudes = 0.5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Replace wavelength by - to skip, e.g., -L300/- is a high-pass Gaussian filter.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Normalization used for coefficients.  Choose among\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m: Mathematical normalization - inner products summed over surface equal 1 [Default]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g: Geodesy normalization - inner products summed over surface equal 4pi\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s: Schmidt normalization - as used in geomagnetism\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Coefficients have phase convention from physics, i.e., the (-1)^m factor\n");
	GMT_explain_options ("V,bi4,h,i,.");
	
	return (EXIT_FAILURE);
}

int GMT_sph2grd_parse (struct GMT_CTRL *GMT, struct SPH2GRD_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to sph2grd and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int col;
	char A[GMT_LEN64] = {""}, B[GMT_LEN64] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Evaluate derivative solutions */
				Ctrl->D.active = true;
				Ctrl->D.mode = argv[i][2];
				break;
			case 'E':	/* Evaluate on ellipsoid */
				Ctrl->E.active = true;
				break;
			case 'G':
				Ctrl->G.active = true;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = true;
				if (GMT_getinc (opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax ('I', 1);
					error++;
				}
				break;
			case 'L':	/* Bandpass or Gaussian filter */
				Ctrl->L.active = true;
				sscanf (opt->arg, "%lg/%lg/%lg/%lg", &Ctrl->L.lc, &Ctrl->L.lp, &Ctrl->L.hp, &Ctrl->L.hc);
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.mode = opt->arg[0];
				break;
			case 'Q':
				Ctrl->Q.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Can only handle one input coefficient file\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error: Must specify output grid file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && !(Ctrl->D.mode == 'g' || Ctrl->D.mode == 'n'), "Syntax error -D option: Must append g or n\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->N.mode == 'm' || Ctrl->N.mode == 'g' || Ctrl->N.mode == 's'), "Syntax error: -N Normalization must be one of m, g, or s\\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_sph2grd_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_sph2grd (void *V_API, int mode, void *args)
{
	struct SPH2GRD_CTRL *Ctrl;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	void *New_sph2grd_Ctrl (), Free_sph2grd_Ctrl (struct SPH2GRD_CTRL *C);

	Ctrl = (struct SPH2GRD_CTRL *) New_sph2grd_Ctrl ();		/* Allocate and initialize defaults in a new control structure */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_sph2grd_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_sph2grd_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, NULL, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_sph2grd_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_sph2grd_parse (GMT, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the sph2grd main code ----------------------------*/

	if ((error = GMT_set_cols (GMT, GMT_IN, 4)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	
	/* Loop over records and determine highest L, M */
	
	for (seg = 0; seg < D->tbl[0]->n_segments; seg++) {
		S = D->tbl[0]->segment[seg];
		for (row = 0; row < S->n_rows; row++) {
			L = irint (S->coord[0][row]);
			M = irint (S->coord[1][row]);
			if (L > L_max) L_max = L;
			if (M > M_max) M_max = M;
		}
	}
	/* Allocate two-D C[L][M] and S[L][M] arrays */
	
	C = GMT_memory (GMT, NULL, L_max+1, double *);
	S = GMT_memory (GMT, NULL, L_max+1, double *);
	for (L = 0; L <= L_max; L++) {
		C[L] = GMT_memory (GMT, NULL, M_max+1, double);
		S[L] = GMT_memory (GMT, NULL, M_max+1, double);
	}
	for (seg = 0; seg < D->tbl[0]->n_segments; seg++) {
		S = D->tbl[0]->segment[seg];
		for (row = 0; row < S->n_rows; row++) {
			L = irint (S->coord[0][row]);
			M = irint (S->coord[1][row]);
			C[L][M]  = S->coord[2][row]
			S[L][M]  = S->coord[3][row]
		}
	}
	if (GMT_Destroy_Data (API, &D) != GMT_OK) {
		Return (API->error);
	}
	
	/* If filtering requested, apply filter, update highest L, M that are nonzero */

	/* Set up and allocate output grid */
	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_GRID_ALL, NULL, GMT->common.R.wesn, Ctrl->I.inc, \
		GMT->common.r.registration, GMT_NOTSET, NULL)) == NULL) Return (API->error);

	n_PLM = L_max * (L_max + 1);	/* Number of P_lm terms needed */
	m_CS = L_max + 1;		/* Number of Cos,Sin terms needed per longitude */
	P_lm = GMT_memory (GMT, NULL, n_PLM, double);
	Cosm = GMT_memory (GMT, NULL, m_CS * Grid->header->nx, double);
	Sinm = GMT_memory (GMT, NULL, m_CS * Grid->header->nx, double);
	/* Evaluate longitude terms */
	m_ij = 0;
	GMT_col_loop2 (GMT, Grid, col) {	/* Evaluate all sin, cos terms */
		lon = GMT_grd_col_to_x (col, Grid->header);	/* Current longitude */
		for (M = 0; M < L; M++, m_ij++) sincosd (lon * M, &Sinm[m_ij], &Cosm[m_ij]);
	}
	
	GMT_row_loop (GMT, Grid, row) {	/* For each output latitude */
		lat = GMT_grd_row_to_y (GMT, row, Grid->header);	/* Current latitude */
		GMT_ascii_format_col (GMT, text, lat, GMT_OUT, GMT_Y);
		GMT_Report (API, GMT_MSG_VERBOSE, "Working on latitude: %s", text);
		/* Compute all P_lm needed with GMT_set_Plm */
		GMT_col_loop (GMT, Grid, row, col, node) {	/* For each longitude along this parallel */
			for (L = LL_start; L <= L_stop; L++) {
				m_ij = 0;
				for (M = 0; M <= L_stop; M++, m_ij++) {
					sum += P[L][M] * (C[L][M]*Cosm[m_ij] + S[L][M]*Sinm[m_ij]);
				}
			}
			Grid->data[node] = sum;
		}
	}
	
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Grid) != GMT_OK) {
		Return (API->error);
	}
	
	GMT_free (P_lm);
	GMT_free (Cosm);
	GMT_free (Sinm);
	for (L = 0; L <= L_max; L++) {
		GMT_free (GMT, C[L]);
		GMT_free (GMT, S[L]);
	}
	GMT_free (GMT, C);
	GMT_free (GMT, S);
	
	Return (EXIT_SUCCESS);
}
