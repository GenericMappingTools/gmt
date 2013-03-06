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
 
#define THIS_MODULE k_mod_sph2grd /* I am sph2grd */

#include "gmt_dev.h"

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

int GMT_sph2grd_usage (struct GMTAPI_CTRL *C, int level)
{
	struct GMT_CTRL *GMT = C->GMT;

	gmt_module_show_name_and_purpose (THIS_MODULE);
	GMT_message (GMT, "usage: sph2grd [coeff_file] %s %s [-Dg|n]\n", GMT_I_OPT, GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[-E] [-F] [-G<grdfile>] [-L[d]<filter>] [-N<norm>] [-Q] [%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_bi_OPT, GMT_h_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-G filename for output grid file\n");
	GMT_explain_options (GMT, "R");
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "	The input is expected to contain records of degree, order, cos, sin.\n");
	GMT_message (GMT, "\t-D Will evaluate a derived field from a geopotential model.  Choose between\n");
	GMT_message (GMT, "\t   -Dg will compute the gravitational field [Add -E for anomalies on ellipsoid]\n");
	GMT_message (GMT, "\t   -Dn will compute the geoid [Add -E for anomalies on ellipsoid]\n");
	GMT_message (GMT, "\t-E to evaluate expansion on the current ellipsoid [Default is sphere]\n");
	GMT_message (GMT, "\t-L Filter coefficients according to one of two kinds of filter specifications:.\n");
	GMT_message (GMT, "\t   Use -Ld if values are given in terms of coefficient degrees [Default is km]\n");
	GMT_message (GMT, "\t   a) Cosine band-pass: Append four wavelengths <lc>/<lp>/<hp>/<hc>.\n");
	GMT_message (GMT, "\t      coefficients outside <lc>/<hc> are cut; inside <lp>/<hp> are passed, rest are tapered.\n");
	GMT_message (GMT, "\t      Replace wavelength by - to skip, e.g., -L-/-/500/100 is a low-pass filter.\n");
	GMT_message (GMT, "\t   b) Gaussian band-pass: Append two wavelengths <lo>/<hi> where filter amplitudes = 0.5.\n");
	GMT_message (GMT, "\t      Replace wavelength by - to skip, e.g., -L300/- is a high-pass Gaussian filter.\n");
	GMT_message (GMT, "\t-N Normalization used for coefficients.  Choose among\n");
	GMT_message (GMT, "\t   m: Mathematical normalization - inner products summed over surface equal 1 [Default]\n");
	GMT_message (GMT, "\t   g: Geodesy normalization - inner products summed over surface equal 4pi\n");
	GMT_message (GMT, "\t   s: Schmidt normalization - as used in geomagnetism\n");
	GMT_message (GMT, "\t-Q Coefficients have phase convention from physics, i.e., the (-1)^m factor\n");
	GMT_explain_options (GMT, "VC4hiF.");
	
	return (EXIT_FAILURE);
}

int GMT_sph2grd_parse (struct GMTAPI_CTRL *C, struct SPH2GRD_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to sph2grd and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'D':	/* Evaluate derivative solutions */
				Ctrl->D.active = true;
				Ctrl->D.mode = opt->arg[0];
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
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
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

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error: Must specify output grid file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && !(Ctrl->D.mode == 'g' || Ctrl->D.mode == 'n'), "Syntax error -D option: Must append g or n\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->N.mode == 'm' || Ctrl->N.mode == 'g' || Ctrl->N.mode == 's'), "Syntax error: -N Normalization must be one of m, g, or s\\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define LM_index(L,M) ((L)*((L)+1)/2+(M))

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_sph2grd_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_sph2grd (void *V_API, int mode, void *args)
{
	bool ortho, first = true;
	int error, L_sign, L, L_max = 0, M, M_max = 0;
	unsigned int tbl, row, col, n_PLM, n_CS, n_CSM;
	size_t n_alloc = 0;
	uint64_t seg, drow, node, k;
	char text[GMT_TEXT_LEN32];
	double **C = NULL, **S = NULL, **Cosm = NULL, **Sinm = NULL;
	double *Cosmx = NULL, *Sinmx = NULL, *P_lm = NULL;
	double lon, lat, sum;
	struct GMT_GRID *Grid = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *T = NULL;
	struct SPH2GRD_CTRL *Ctrl;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_sph2grd_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_sph2grd_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_gmt_module (API, THIS_MODULE, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, "-VRfb", "hirs>" GMT_OPT("HMm"), options)) Return (API->error);
	Ctrl = New_sph2grd_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_sph2grd_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the sph2grd main code ----------------------------*/

	GMT_report (GMT, GMT_MSG_VERBOSE, "Process input coefficients\n");
	if ((error = GMT_set_cols (GMT, GMT_IN, 4)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_ANY, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	/* Loop over all records and determine the highest L, M */
	
	for (tbl = 0; tbl < D->n_tables; tbl++) for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
		T = D->table[tbl]->segment[seg];
		for (drow = 0; drow < T->n_rows; drow++) {
			L = lrint (T->coord[0][drow]);
			M = lrint (T->coord[1][drow]);
			if (L > L_max) L_max = L;
			if (M > M_max) M_max = M;
		}
	}

	GMT_report (GMT, GMT_MSG_VERBOSE, "Found L_max = %d and M_max = %d\n", L_max, M_max);
	/* Allocate C[L][M] and S[L][M] arrays to simplify accessing the coefficients */
	GMT_malloc2 (GMT, C, S, L_max + 1, &n_alloc, double *);
	for (L = 0; L <= L_max; L++) {
		n_alloc = 0;
		GMT_malloc2 (GMT, C[L], S[L], L_max + 1, &n_alloc, double);
	}

	/* Place the coefficients into the C and S arrays */
	for (tbl = 0; tbl < D->n_tables; tbl++) for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
		T = D->table[tbl]->segment[seg];
		for (drow = 0; drow < T->n_rows; drow++) {
			L = lrint (T->coord[0][drow]);
			M = lrint (T->coord[1][drow]);
			C[L][M]  = T->coord[2][drow];
			S[L][M]  = T->coord[3][drow];
		}
	}
	/* We are done with the input table; remove it */
	if (GMT_Destroy_Data (API, GMT_ALLOCATED, &D) != GMT_OK) {
		Return (API->error);
	}
	
	ortho = (Ctrl->N.mode == 'm');		/* Set ortho flag */
	L_sign = (Ctrl->Q.active) ? -1 : +1;	/* Set Condon-Shortley phase if requested */
	
	/* If filtering requested, apply filter, update highest L, M that are nonzero */

	/* <filtering goes here> */
	
	/* Allocate output grid */
	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_GRID_ALL, NULL, GMT->common.R.wesn, Ctrl->I.inc, \
		GMT->common.r.registration, GMTAPI_NOTSET, NULL)) == NULL) Return (API->error);

	n_PLM = LM_index (L_max + 1, L_max + 1);/* Number of P_lm terms needed */
	n_CS = L_max + 1;			/* Number of Cos,Sin terms needed per longitude */
	n_CSM = n_CS * Grid->header->nx;	/* Number of Cos,Sin terms needed for all longitudes */
	P_lm  = GMT_memory (GMT, NULL, n_PLM, double);
	n_alloc = 0;	GMT_malloc2 (GMT, Cosmx, Sinmx, n_CSM, &n_alloc, double);
	n_alloc = 0;	GMT_malloc2 (GMT, Cosm,  Sinm,  Grid->header->nx,  &n_alloc, double *);
	
	/* Evaluate longitude terms once and for all to avoid doing it repeatedly in the big loop.
	 * We compute a matrix with rows representing order M and columns representing longitude.
	 * Then, we allocate a set of pointers assigned to each row to enable 2-D indexing.*/

	GMT_report (GMT, GMT_MSG_VERBOSE, "Evaluate exp (i*m*lon) for all m [%d] and all lon [%u]\n", M_max+1, Grid->header->nx);
	k = 0;
	GMT_col_loop2 (GMT, Grid, col) {	/* Evaluate all sin, cos terms */
		lon = GMT_grd_col_to_x (GMT, col, Grid->header);	/* Current longitude */
		for (M = 0; M <= L_max; M++, k++) sincosd (lon * M, &Sinmx[k], &Cosmx[k]);
	}
	GMT_col_loop2 (GMT, Grid, col) {	/* Evaluate all sin, cos terms */
		k = col * (L_max + 1);
		Cosm[col] = &Cosmx[k];	/* Cosm[k][M] has cos(mx) terms for all x[k], fixed M <= L_max*/
		Sinm[col] = &Sinmx[k];	/* Sinm[k][M] has sin(mx) terms for all x[k], fixed M <= L_max*/
	}
	
	GMT_report (GMT, GMT_MSG_VERBOSE, "Start evaluate the spherical harmonic sums\n");
	GMT_row_loop (GMT, Grid, row) {					/* For each output latitude */
		lat = GMT_grd_row_to_y (GMT, row, Grid->header);	/* Current latitude */
		GMT_ascii_format_col (GMT, text, lat, GMT_Y);
		GMT_report (GMT, GMT_MSG_VERBOSE, "Working on latitude: %s\n", text);
		/* Compute all P_lm needed with GMT_plm_bar_all for this latitude */
		GMT_plm_bar_all (GMT, L_sign * L_max, sind (lat), ortho, P_lm);	/* I.e., cosine of colatitude */
		GMT_col_loop (GMT, Grid, row, col, node) {	/* For each longitude along this parallel */
			sum = 0.0;	/* Initialize sum to zero for new output node */
			for (L = k = 0; L <= L_max; L++) {	/* For all degrees */
				for (M = 0; M <= L; M++, k++) {	/* For all orders <= L */
					sum += P_lm[k] * (C[L][M]*Cosm[col][M] + S[L][M]*Sinm[col][M]);
				}
			}
			Grid->data[node] = (float)sum;	/* Assign total to the grid, cast to float */
		}
	}
	
	GMT_report (GMT, GMT_MSG_VERBOSE, "Write grid to file\n");
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Grid) != GMT_OK) {
		Return (API->error);
	}
	
	GMT_free (GMT, P_lm);
	GMT_free (GMT, Cosm);
	GMT_free (GMT, Sinm);
	GMT_free (GMT, Cosmx);
	GMT_free (GMT, Sinmx);
	for (L = 0; L <= L_max; L++) {
		GMT_free (GMT, C[L]);
		GMT_free (GMT, S[L]);
		
	}
	GMT_free (GMT, C);
	GMT_free (GMT, S);
	
	GMT_report (GMT, GMT_MSG_VERBOSE, "Completed\n");
	Return (EXIT_SUCCESS);
}
