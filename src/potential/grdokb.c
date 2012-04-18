/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 *--------------------------------------------------------------------
 *
 *
 * API functions to support the grdokb application.
 *
 * Brief synopsis: Compute the gravity anomaly of the volume contained between
 *                 a surface provided by one grid and a plane, or between a top
 *                 and a bottom surface provided by two grids.
 *
 * Author:	Joaquim Luis
 * Date:	1-Apr-2012
 *
 */

#include "gmt_potential.h"
#include "okbfuns.h"

struct GRDOKB_CTRL {

	struct GRDOKB_In {
		int active;
		char *file[3];
	} In;

	struct GRDOKB_C {	/* -C */
		int active;
		double rho;
	} C;
	struct GRDOKB_D {	/* -D */
		float z_dir;
	} D;
	struct GRDOKB_I {	/* -Idx[/dy] */
		int active;
		double inc[2];
	} I;
	struct GRDOKB_F {	/* -F<grdfile> */
		int active;
		char *file;
	} F;
	struct GRDOKB_G {	/* -G<grdfile> */
		int active;
		char *file;
	} G;
	struct GRDOKB_H {	/* -H */
		int active;
		double	t_dec, t_dip, m_int, m_dec, m_dip;
	} H;
	struct GRDOKB_L {	/* -L */
		double zobs;
	} L;
	struct GRDOKB_S {	/* -S */
		int active;
		double radius;
	} S;
	struct GRDOKB_Z {	/* -Z */
		double z0;
	} Z;
	struct GRDOKB_Q {	/* -Q */
		int active;
		int n_pad;
		char region[GMT_BUFSIZ];	/* gmt_parse_R_option has this!!!! */
		double pad_dist;
	} Q;
	struct GRDOKB_box {	/* No option, just a container */
		int is_geog;
		double	d_to_m, *mag_int, lon_0, lat_0;
	} box;
};

struct DATA {
	double  x, y;
} *data;

GMT_LONG read_poly__ (struct GMT_CTRL *GMT, char *fname, int switch_xy);
void set_center (int n_triang);
int grdokb_body_set(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
	struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
	double *cos_vec, int j, int i, int inc_j, int inc_i);
int grdokb_body_desc(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc,
	struct BODY_VERTS **body_verts, int face);
void grdokb_calc_top_surf (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
	struct GMT_GRID *Gout, double *g, int n_pts, double *x_grd, double *y_grd, double *x_obs,
	double *y_obs, double *cos_vec, struct MAG_VAR *mag_var, struct LOC_OR *loc_or,
	struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts);

void *New_grdokb_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDOKB_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRDOKB_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->L.zobs = 0;
	C->D.z_dir = -1;		/* -1 because Z was positive down for Okabe */
	C->S.radius = 30000;
	return (C);
}

void Free_grdokb_Ctrl (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free (C->G.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_grdokb_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {

	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdokb %s - Computes the gravity effect of one (or two) grids by the method of Okabe\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdokb grdfile_up [grdfile_low] [-C<density>] [-D] [-F<xy_file>] [-fg] \n");
	GMT_message (GMT, "\t[-G<outfile>] [%s] [-L<z_obs>]\n", GMT_I_OPT);
	GMT_message (GMT, "\t[-Q[n<n_pad>]|[pad_dist]|[<w/e/s/n>]]\n");
	GMT_message (GMT, "\t[%s] [%s] [-Z<level>]\n", GMT_Rgeo_OPT, GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\tgrdfile_up is the grdfile whose gravity efect is to be computed.\n");
	GMT_message (GMT, "\t   If two grids are provided then the gravity/magnetic efect of the\n");
	GMT_message (GMT, "\t   volume between them is computed\n\n");
	GMT_message (GMT, "\t-C sets body density in SI\n");
	GMT_message (GMT, "\t-F pass file with locations where anomaly is going to be computed\n");
	GMT_message (GMT, "\t-G name of the output grdfile.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-D if z is positive down [Default positive up]\n");
	/*GMT_message (GMT, "\t-H sets parameters for computation of magnetic anomaly\n");
	GMT_message (GMT, "\t   f_dec/f_dip -> geomagnetic declination/inclination\n");
	GMT_message (GMT, "\t   m_int/m_dec/m_dip -> body magnetic intensity/declination/inclination\n");*/
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_message (GMT, "\t   The new xinc and yinc should be divisible by the old ones (new lattice is subset of old).\n");
	GMT_message (GMT, "\t-L sets level of observation [Default = 0]\n");
	GMT_message (GMT, "\t-fg Map units TRUE; x,y in degrees, dist units in m [Default dist unit = x,y unit].\n");
	GMT_message (GMT, "\t-Q Extend the domain of computation with respect to output -R region.\n");
	GMT_message (GMT, "\t   -Qn<N> artifficially extends the width of the outer rim of cells to have a fake\n");
	GMT_message (GMT, "\t   width of N * dx[/dy].\n");
	GMT_message (GMT, "\t   -Q<pad_dist> extend the region by west-pad, east+pad, etc\n");
	GMT_message (GMT, "\t   -Q<region> Same sintax as -R.\n");
	GMT_message (GMT, "\t-R For new Range of output grid; enter <WESN> (xmin, xmax, ymin, ymax) separated by slashes.\n");
	GMT_message (GMT, "\t   [Default uses the same region as the input grid].\n");
	GMT_message (GMT, "\t-Z z level of reference plane [Default = 0]\n");
	GMT_explain_options (GMT, "V:.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdokb_parse (struct GMTAPI_CTRL *C, struct GRDOKB_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdokb and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct	GMT_OPTION *opt = NULL;
	struct	GMT_CTRL *GMT = C->GMT;
	int n_files = 0;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file */
				Ctrl->In.active = TRUE;
				if (n_files == 0)
					Ctrl->In.file[n_files++] = strdup (opt->arg);
				else if (n_files == 1)
					Ctrl->In.file[n_files++] = strdup (opt->arg);
				else {
					n_errors++;
					GMT_report (GMT, GMT_MSG_FATAL, "Error: A maximum of two input grids may be processed\n");
				}
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.rho = atof (opt->arg) * 6.674e-6;
				Ctrl->C.active = TRUE;
				break;
			case 'D':
				Ctrl->D.z_dir = 1;
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				Ctrl->F.file = strdup (opt->arg);
				break;
 			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'H':
				if ((sscanf(opt->arg, "%lf/%lf/%lf/%lf/%lf",
					    &Ctrl->H.t_dec, &Ctrl->H.t_dip, &Ctrl->H.m_int, &Ctrl->H.m_dec, &Ctrl->H.m_dip)) != 5) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -H option: Can't dechiper values\n");
					n_errors++;
				}
				Ctrl->H.active = TRUE;
				Ctrl->C.active = FALSE;
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
	 		case 'L':
				Ctrl->L.zobs = atof (opt->arg);
				break;
#ifdef GMT_COMPAT
			case 'M':	/* Geographic data */
				if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
				break;
#endif
			case 'Q':
				Ctrl->Q.active = TRUE;
				if (opt->arg[0] == 'n') {
					Ctrl->Q.n_pad = atoi (&opt->arg[1]);
				}
				else {
					int n = 0;
					char *pch;
					pch = strchr(opt->arg, '/');
					while (pch != NULL) {
						n++;
						pch = strchr(pch+1,'/');
					}
					if (n == 0)
						Ctrl->Q.pad_dist = atof (opt->arg);	/* Pad given as a scalar distance */
					else if (n == 3)
						strcpy(Ctrl->Q.region, opt->arg);	/* Pad given as a -R region */
					else {
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -Q option. Either -Q<pad> or -Q<region>\n");
						n_errors++;
					}
				}
				break;
	 		case 'S':
				Ctrl->S.radius = atof (opt->arg) * 1000;
				Ctrl->S.active = TRUE;
				break;
			case 'Z':
				Ctrl->Z.z0 = atof(opt->arg);
				break;
			default:
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->In.file[0], "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && (Ctrl->S.radius <= 0.0 || GMT_is_dnan (Ctrl->S.radius)),
				"Syntax error: Radius is NaN or negative\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.active && !Ctrl->F.active,
				"Error: Must specify either -G or -F options\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active && Ctrl->Q.active && !Ctrl->Q.n_pad,
				"Error: Cannot specify -Q<pad>|<region> without -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.rho == 0.0 && !Ctrl->H.active,
				"Error: Must specify either -Cdensity or -H<stuff>\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.active && !Ctrl->G.file,
				"Syntax error -G option: Must specify output file\n");
	GMT_check_condition (GMT, Ctrl->G.active && Ctrl->F.active, "Warning: -F overrides -G\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdokb_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_grdokb (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args) {

	int nx_p, ny_p, i, j, k, ndata = 0;
	int two_grids = FALSE, switch_xy = FALSE, clockwise_type[] = {0, 5};
	int km = 0;		/* index of current body facet (for mag only) */
	GMT_LONG error = FALSE;
	GMT_LONG n_vert_max;
	double	a, d, x_o, y_o;
	double	*x_obs = NULL, *y_obs = NULL, *x_grd = NULL, *y_grd = NULL, *cos_vec = NULL;
	double	*g = NULL, *x_grd2 = NULL, *y_grd2 = NULL, *cos_vec2 = NULL;
	double	cc_t, cs_t, s_t, wesn_new[4], wesn_padded[4];

	struct	GMT_GRID *GridA = NULL, *GridB = NULL;
	struct	LOC_OR *loc_or;
	struct	BODY_VERTS *body_verts = NULL;
	struct	BODY_DESC body_desc;
	struct	GRDOKB_CTRL *Ctrl = NULL;
	struct	GMT_GRID *Gout = NULL;
	struct	GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct	GMT_OPTION *options = NULL;

	mag_var = NULL, mag_param = NULL, data = NULL;
	body_desc.n_v = NULL, body_desc.ind = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE)
		bailout (GMT_grdokb_usage (API, GMTAPI_USAGE));		/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS)
		bailout (GMT_grdokb_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdokb", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-VRf:", "", options)) Return (API->error);
	Ctrl = New_grdokb_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdokb_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdokb main code ----------------------------*/

	if (GMT_is_geographic (GMT, GMT_IN)) Ctrl->box.is_geog = TRUE;

	if (!Ctrl->box.is_geog)
		Ctrl->box.d_to_m = 1.;
	else
		Ctrl->box.d_to_m = 2.0 * M_PI * 6371008.7714 / 360.0;
		/*Ctrl->box.d_to_m = 2.0 * M_PI * gmtdefs.ellipse[N_ELLIPSOIDS-1].eq_radius / 360.0;*/

	if (Ctrl->F.active) { 		/* Read xy file where anomaly is to be computed */
		if ( (ndata = (int)read_poly__ (GMT, Ctrl->F.file, switch_xy)) < 0 ) {
			GMT_report (GMT, GMT_MSG_FATAL, "Cannot open file %s\n", Ctrl->F.file);
			return (EXIT_FAILURE);
		}
	}

	/* ---------------------------------------------------------------------------- */

	if ((GridA = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER,
				Ctrl->In.file[0], NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	if (Ctrl->G.active) {
		if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, NULL)) == NULL) Return (API->error);
		GMT_grd_init (GMT, Gout->header, options, FALSE);
		/* Use the -R region for output if set; otherwise match grid domain */
		GMT_memcpy (Gout->header->wesn, (GMT->common.R.active ? GMT->common.R.wesn :
			GridA->header->wesn), 4,double);
		GMT_memcpy (Gout->header->inc, (Ctrl->I.active ? Ctrl->I.inc :
			GridA->header->inc), 2, double);
		if (Gout->header->wesn[XLO] < GridA->header->wesn[XLO]) error = TRUE;
		if (Gout->header->wesn[XHI] > GridA->header->wesn[XHI]) error = TRUE;

		if (Gout->header->wesn[YLO] < GridA->header->wesn[YLO]) error = TRUE;
		if (Gout->header->wesn[YHI] > GridA->header->wesn[YHI]) error = TRUE;

		if (error) {
			GMT_report (GMT, GMT_MSG_FATAL, "New WESN incompatible with old.\n");
			Return (EXIT_FAILURE);
		}

		/* Completely determine the header for the new grid; croak if there are issues.
		   No memory is allocated here. */
		GMT_err_fail (GMT, GMT_init_newgrid (GMT, Gout, Gout->header->wesn,
					Gout->header->inc, GridA->header->registration), Ctrl->G.file);

		GMT_report (GMT, GMT_MSG_NORMAL, "Grid dimensions are nx = %d, ny = %d\n",
					Gout->header->nx, Gout->header->ny);
		Gout->data = GMT_memory (GMT, NULL, Gout->header->size, float);
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "Allocates memory and read data file\n");

	if (!GMT->common.R.active)
		GMT_memcpy (wesn_new, GridA->header->wesn, 4, double);
	else
		GMT_memcpy (wesn_new, GMT->common.R.wesn, 4, double);

	/* Process Padding request */
	if (Ctrl->Q.active && !Ctrl->Q.n_pad) {
		if (Ctrl->Q.pad_dist)		/* Convert the scalar pad distance into a -R string type */
			sprintf (Ctrl->Q.region, "%f/%f/%f/%f", wesn_new[XLO] - Ctrl->Q.pad_dist,
					wesn_new[XHI] + Ctrl->Q.pad_dist, wesn_new[YLO] - Ctrl->Q.pad_dist,
					wesn_new[YHI] + Ctrl->Q.pad_dist);

		GMT->common.R.active = FALSE;
		GMT_parse_common_options (GMT, "R", 'R', Ctrl->Q.region);	/* Use the -R parsing machinery to handle this */
		GMT_memcpy (wesn_padded, GMT->common.R.wesn, 4, double);
		GMT_memcpy (GMT->common.R.wesn, wesn_new, 4, double);		/* Reset previous WESN */
		GMT->common.R.active = TRUE;

		if (wesn_padded[XLO] < GridA->header->wesn[XLO]) {
			GMT_report (GMT, GMT_MSG_FATAL, "Request padding at the West border exceed grid limit, trimming it\n");
			wesn_padded[XLO] = GridA->header->wesn[XLO];
		}
		if (wesn_padded[XHI] > GridA->header->wesn[XHI]) {
			GMT_report (GMT, GMT_MSG_FATAL, "Request padding at the East border exceed grid limit, trimming it\n");
			wesn_padded[XHI] = GridA->header->wesn[XHI];
		}
		if (wesn_padded[YLO] < GridA->header->wesn[YLO]) {
			GMT_report (GMT, GMT_MSG_FATAL, "Request padding at the South border exceed grid limit, trimming it\n");
			wesn_padded[YLO] = GridA->header->wesn[YLO];
		}
		if (wesn_padded[YHI] > GridA->header->wesn[YHI]) {
			GMT_report (GMT, GMT_MSG_FATAL, "Request padding at the North border exceed grid limit, trimming it\n");
			wesn_padded[YHI] = GridA->header->wesn[YHI];
		}
	}
	else
		GMT_memcpy (wesn_padded, GridA->header->wesn, 4, double);

	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn_padded, GMT_GRID_DATA, Ctrl->In.file[0], GridA) == NULL) {	/* Get subset, or all */
		Return (API->error);
	}

	/* Check that Inner region request does not exceeds input grid limits */
	if (GMT->common.R.active && Ctrl->G.active) {
		if (Gout->header->wesn[XLO] < GridA->header->wesn[XLO] ||
				Gout->header->wesn[XHI] > GridA->header->wesn[XHI]) {
			GMT_report (GMT, GMT_MSG_FATAL, " Selected region exceeds the X-boundaries of the grid file!\n");
			return (EXIT_FAILURE);
		}
		else if (Gout->header->wesn[YLO] < GridA->header->wesn[YLO] ||
				Gout->header->wesn[YHI] > GridA->header->wesn[YHI]) {
			GMT_report (GMT, GMT_MSG_FATAL, " Selected region exceeds the Y-boundaries of the grid file!\n");
			return (EXIT_FAILURE);
		}
		GMT_RI_prepare (GMT, Gout->header);	/* Ensure -R -I consistency and set nx, ny */
	}

	Ctrl->box.lon_0 = (GridA->header->wesn[XLO] + GridA->header->wesn[XHI]) / 2;
	Ctrl->box.lat_0  = (GridA->header->wesn[YLO] + GridA->header->wesn[YHI]) / 2;

	if (Ctrl->Z.z0 > GridA->header->z_max) {
		/* Typical when computing the effect of whater shell for Buguer reduction of marine data */
		clockwise_type[0] = 5;		/* Means Top triangs will have a CCW description and CW for bot */
		clockwise_type[1] = 0;
	}

	if (Ctrl->D.z_dir != 1) {
		Ctrl->Z.z0 *= Ctrl->D.z_dir;
		for (j = 0; j < GridA->header->ny; j++)
			for (i = 0; i < GridA->header->nx; i++)
				GridA->data[GMT_IJP(GridA->header,j,i)] *= Ctrl->D.z_dir;
	}

	/* In case we have one second grid, for bottom surface */
	if (Ctrl->In.file[1]) {
		if ((GridB = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, Ctrl->In.file[1], NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}

		if(GridA->header->registration != GridB->header->registration) {
			GMT_report (GMT, GMT_MSG_FATAL, "Up and bottom grids have different registrations!\n");
			Return (EXIT_FAILURE);
		}
		if ((GridA->header->z_scale_factor != GridB->header->z_scale_factor) ||
				(GridA->header->z_add_offset != GridB->header->z_add_offset)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Scale/offset not compatible!\n");
			Return (EXIT_FAILURE);
		}

		if (fabs (GridA->header->inc[GMT_X] - GridB->header->inc[GMT_X]) > 1.0e-6 ||
				fabs (GridA->header->inc[GMT_Y] - GridB->header->inc[GMT_Y]) > 1.0e-6) {
			GMT_report (GMT, GMT_MSG_FATAL, "Up and bottom grid increments do not match!\n");
			Return (EXIT_FAILURE);
		}
		/* ALMOST FOR SURE THERE ARE MEMORY CLEAN UPS TO DO HERE BEFORE RETURNING */

		if (Ctrl->D.z_dir != -1) {
			for (j = 0; j < GridB->header->ny; j++)
				for (i = 0; i < GridB->header->nx; i++)
					GridB->data[GMT_IJP(GridB->header,j,i)] *= Ctrl->D.z_dir;
		}
		two_grids = TRUE;
	}

	nx_p = !Ctrl->F.active ? Gout->header->nx : ndata;
	ny_p = !Ctrl->F.active ? Gout->header->ny : ndata;
	x_grd = GMT_memory (GMT, NULL, GridA->header->nx, double);
	y_grd = GMT_memory (GMT, NULL, GridA->header->ny, double);
	x_obs = GMT_memory (GMT, NULL, nx_p, double);
	y_obs = GMT_memory (GMT, NULL, ny_p, double);
	if (Ctrl->F.active) g = GMT_memory (GMT, NULL, (size_t) ndata, double);

	d = GridA->header->xy_off;		/*  0.5 : 0.0 */

	/* ----------------------- Build observation point vectors ---------------------------------- */
	for (i = 0; i < GridA->header->nx; i++)
		x_grd[i] = (i == (GridA->header->nx-1)) ? GridA->header->wesn[XHI] - d*GridA->header->inc[GMT_X] :
			GridA->header->wesn[XLO] + (i + d) * GridA->header->inc[GMT_X];
	for (j = 0; j < GridA->header->ny; j++)
		y_grd[j] = (j == (GridA->header->ny-1)) ? -(GridA->header->wesn[YLO] + d*GridA->header->inc[GMT_Y]) :
			-(GridA->header->wesn[YHI] - (j + d) * GridA->header->inc[GMT_Y]);

	if (Ctrl->G.active) {
		for (i = 0; i < Gout->header->nx; i++)
			x_obs[i] = (i == (Gout->header->nx-1)) ? Gout->header->wesn[XHI] - d * Gout->header->inc[GMT_X] :
				Gout->header->wesn[XLO] + (i + d) * Gout->header->inc[GMT_X];
		for (j = 0; j < Gout->header->ny; j++)
			y_obs[j] = (j == (Gout->header->ny-1)) ? -(Gout->header->wesn[YLO] + d * Gout->header->inc[GMT_Y]) :
				-(Gout->header->wesn[YHI] - (j + d) * Gout->header->inc[GMT_Y]);
	}

	if (two_grids) {
		d = GridB->header->xy_off;		/*  0.5 : 0.0 */
		x_grd2 = GMT_memory (GMT, NULL, (size_t) GridB->header->nx, double);
		y_grd2 = GMT_memory (GMT, NULL, (size_t) GridB->header->ny, double);
		for (i = 0; i < GridB->header->nx; i++)
			x_grd2[i] = (i == (GridB->header->nx-1)) ? GridB->header->wesn[XHI] - d*GridB->header->inc[GMT_X] :
				GridB->header->wesn[XLO] + (i + d) * GridB->header->inc[GMT_X];
		for (j = 0; j < GridB->header->ny; j++)
			y_grd2[j] = (j == (GridB->header->ny-1)) ? -(GridB->header->wesn[YLO] + d*GridB->header->inc[GMT_Y]) :
				-(GridB->header->wesn[YHI] - (j + d) * GridB->header->inc[GMT_Y]);
	}
	/* --------------------------------------------------------------------------------------------- */

	/* ---------- Pre-compute a vector of cos(lat) to use in the Geog cases ------------------------ */
	cos_vec = GMT_memory (GMT, NULL, GridA->header->ny, double);
	for (j = 0; j < GridA->header->ny; j++)
		cos_vec[j] = (Ctrl->box.is_geog) ? cos(y_grd[j]*D2R): 1;
	if (two_grids) {
		cos_vec2 = GMT_memory (GMT, NULL, GridB->header->ny, double);
		for (j = 0; j < GridB->header->ny; j++)
			cos_vec2[j] = (Ctrl->box.is_geog) ? cos(y_grd2[j]*D2R): 1;
	}
	/* --------------------------------------------------------------------------------------------- */

	/* ------ Convert distances to arc distances at Earth surface *** NEEDS CONFIRMATION *** ------- */
	if (Ctrl->box.is_geog) {
		for (i = 0; i < GridA->header->nx; i++)
			x_grd[i] = (x_grd[i] - Ctrl->box.lon_0) * Ctrl->box.d_to_m;
		for (j = 0; j < GridA->header->ny; j++)
			y_grd[j] = (y_grd[j] + Ctrl->box.lat_0)  * Ctrl->box.d_to_m;

		if (two_grids) {
			for (i = 0; i < GridB->header->nx; i++)
				x_grd2[i] = (x_grd[i] - Ctrl->box.lon_0) * Ctrl->box.d_to_m;
			for (j = 0; j < GridB->header->ny; j++)
				y_grd2[j] = (y_grd[j] + Ctrl->box.lat_0)  * Ctrl->box.d_to_m;
		}
	}
	/* --------------------------------------------------------------------------------------------- */

	if (Ctrl->Q.n_pad) {
		x_grd[0] = x_grd[1] - Ctrl->Q.n_pad * (x_grd[2] - x_grd[1]);
		y_grd[0] = y_grd[1] - Ctrl->Q.n_pad * fabs(y_grd[2] - y_grd[1]);
		x_grd[GridA->header->nx-1] = x_grd[GridA->header->nx-1] + Ctrl->Q.n_pad * (x_grd[2] - x_grd[1]);
		y_grd[GridA->header->ny-1] = y_grd[GridA->header->ny-1] + Ctrl->Q.n_pad * fabs(y_grd[2] - y_grd[1]);
	}

	if (Ctrl->F.active) { /* Need to compute observation coords only once */
		for (i = 0; i < ndata; i++) {
			x_obs[i] = (Ctrl->box.is_geog) ?  (data[i].x - Ctrl->box.lon_0)*Ctrl->box.d_to_m*cos(data[i].y*D2R) : data[i].x;
			y_obs[i] = (Ctrl->box.is_geog) ? -(data[i].y - Ctrl->box.lat_0)*Ctrl->box.d_to_m : -data[i].y; /* - because y positive 'south' */
		}
	}

	if (Ctrl->H.active) { /* 1e2 is a factor to obtain nT from magnetization in A/m */
		mag_param = GMT_memory (GMT, NULL, 1, struct MAG_PARAM);
		mag_param[0].rim[0] = 1e2*cos(Ctrl->H.t_dip*D2R) * cos((Ctrl->H.t_dec - 90.)*D2R);
		mag_param[0].rim[1] = 1e2*cos(Ctrl->H.t_dip*D2R) * sin((Ctrl->H.t_dec - 90.)*D2R);
		mag_param[0].rim[2] = 1e2*sin(Ctrl->H.t_dip*D2R);
		cc_t = cos(Ctrl->H.m_dip*D2R)*cos((Ctrl->H.m_dec - 90.)*D2R);
		cs_t = cos(Ctrl->H.m_dip*D2R)*sin((Ctrl->H.m_dec - 90.)*D2R);
		s_t  = sin(Ctrl->H.m_dip*D2R);
		 /* Case of constant magnetization */
			mag_var = GMT_memory (GMT, NULL, 1, struct MAG_VAR);
			mag_var[0].rk[0] = Ctrl->H.m_int * cc_t;
			mag_var[0].rk[1] = Ctrl->H.m_int * cs_t;
			mag_var[0].rk[2] = Ctrl->H.m_int * s_t;
	}

	/* Decompose a paralelogram into two triangular prisms */
	if (!Ctrl->In.file[1] && !Ctrl->In.file[2])		/* One grid only - EXPLICAR */
		grdokb_body_desc(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[0]);		/* Set CW or CCW of top triangs */

	/* Allocate a structure that will be used inside okabe().
	   We do it here to avoid thousands of alloc/free that would result if done in okabe() */
	n_vert_max = body_desc.n_v[0];
	for (i = 1; i < body_desc.n_f; i++)
		n_vert_max = MAX(body_desc.n_v[i], n_vert_max);

	loc_or = GMT_memory (GMT, NULL, (n_vert_max+1), struct LOC_OR);

/* ---------------> Now start computing <------------------------------------- */

	if (Ctrl->G.active) { /* grid output */
		grdokb_calc_top_surf (GMT, Ctrl, GridA, Gout, NULL, 0, x_grd, y_grd, x_obs, y_obs, cos_vec,
					mag_var, loc_or, &body_desc, body_verts);

		if (!two_grids) {		/* That is, one grid and a flat base Do the BASE now */
			grdokb_body_desc(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of BOT triangs */
			grdokb_body_set(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 0, 0,
					GridA->header->ny-1, GridA->header->nx-1);
			for (k = 0; k < Gout->header->ny; k++) {
				y_o = (Ctrl->box.is_geog) ? (y_obs[k] + Ctrl->box.lat_0) * Ctrl->box.d_to_m : y_obs[k];
				for (i = 0; i < Gout->header->nx; i++) {
					x_o = (Ctrl->box.is_geog) ? (x_obs[i] - Ctrl->box.lon_0) *
							Ctrl->box.d_to_m * cos(y_obs[k]*D2R) : x_obs[i];
					a = okabe (GMT, x_o, y_o, Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, body_desc, body_verts, km, 0, loc_or);
					Gout->data[GMT_IJP(Gout->header, k, i)] += (float)a;
				}
			}
		}
		else {		/* "two_grids". One at the top and the other at the base */
			grdokb_body_desc(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of top triangs */
			grdokb_calc_top_surf (GMT, Ctrl, GridB, Gout, NULL, 0, x_grd2, y_grd2, x_obs, y_obs,
					cos_vec2, mag_var, loc_or, &body_desc, body_verts);
		}
	}
	else {		/* polygon output */
		grdokb_calc_top_surf (GMT, Ctrl, GridA, NULL, g, ndata, x_grd, y_grd, x_obs, y_obs, cos_vec,
				mag_var, loc_or, &body_desc, body_verts);

		if (!two_grids) {		/* That is, one grid and a flat base. Do the BASE now */
			grdokb_body_desc(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of BOT triangs */
			grdokb_body_set(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 0, 0,
					GridA->header->ny-1, GridA->header->nx-1);
			for (k = 0; k < ndata; k++)
				g[k] += okabe (GMT, x_obs[k], y_obs[k], Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, body_desc, body_verts, km, 0, loc_or);
		}
		else {		/* "two_grids". One at the top and the other at the base */
			grdokb_body_desc(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of top triangs */
			grdokb_calc_top_surf (GMT, Ctrl, GridB, NULL, g, ndata, x_grd2, y_grd2, x_obs, y_obs,
					cos_vec2, mag_var, loc_or, &body_desc, body_verts);
		}
	}
#if 0
		k = grdokb_body_set(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 2);
		loc_or = GMT_memory (GMT, loc_or, k, struct LOC_OR);
		for (k = 0; k < Gout->header->ny; k++) {
			y_o = (Ctrl->box.is_geog) ? (y_obs[k] + Ctrl->box.lat_0) * Ctrl->box.d_to_m : y_obs[k];
			for (i = 0; i < np; i++) {
				x_o = (Ctrl->box.is_geog) ? (x_obs[i] - Ctrl->box.lon_0) *
						Ctrl->box.d_to_m * cos(y_obs[k]*D2R) : x_obs[i];
				a = okabe (GMT, x_o, y_o, body_desc, km, 0, loc_or);
				Gout->data[GMT_IJP(Gout->header, k, i)] += (float)a;
			}
		}

		k = grdokb_body_set(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 4);
		for (k = 0; k < Gout->header->ny; k++) {
			y_o = (Ctrl->box.is_geog) ? (y_obs[k] + Ctrl->box.lat_0) * Ctrl->box.d_to_m : y_obs[k];
			for (i = 0; i < np; i++) {
				x_o = (Ctrl->box.is_geog) ? (x_obs[i] - Ctrl->box.lon_0) *
						Ctrl->box.d_to_m * cos(y_obs[k]*D2R) : x_obs[i];
				a = okabe (GMT, x_o, y_o, body_desc, km, 0, loc_or);
				Gout->data[GMT_IJP(Gout->header, k, i)] += (float)a;
			}
		}
#endif
	/*---------------------------------------------------------------------------------------------*/
	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) fprintf (stderr, "\n");

	if (Ctrl->G.active) {
		if (Ctrl->C.active) {
			strcpy (Gout->header->title, "Gravity field");
			strcpy (Gout->header->z_units, "mGal");
		}
		else {
			strcpy (Gout->header->title, "Magnetic field");
			strcpy (Gout->header->z_units, "nT");
		}

		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, Ctrl->G.file, Gout) != GMT_OK) {
			Return (API->error);
		}
	}
	else {
		for (k = 0; k < ndata; k++)
			fprintf (stdout, "%lg %lg %lg\n", data[k].x, data[k].y, g[k]);
	}

	GMT_free (GMT, x_grd);
	GMT_free (GMT, y_grd);
	if (x_grd2) GMT_free (GMT, x_grd2);
	if (y_grd2) GMT_free (GMT, y_grd2);
	if (cos_vec2) GMT_free (GMT, cos_vec2);
	if (g) GMT_free (GMT, g);
	if (data) GMT_free (GMT, data);
	GMT_free (GMT, x_obs);
	GMT_free (GMT, y_obs);
	GMT_free (GMT, cos_vec);
	GMT_free (GMT, loc_or);
	GMT_free (GMT, body_desc.n_v);
	GMT_free (GMT, body_desc.ind);
	GMT_free (GMT, body_verts);
	if (mag_param) GMT_free (GMT, mag_param);

	Return (GMT_OK);
}

/* -----------------------------------------------------------------*/
GMT_LONG read_poly__ (struct GMT_CTRL *GMT, char *fname, int switch_xy) {
	/* Read file with xy points where anomaly is going to be computed
	   This is a temporary function while we not use the API to do this job. */
	int n_alloc, ndata, ix = 0, iy = 1;
	double in[2];
	char line[GMT_TEXT_LEN256];
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata = 0;
	if (switch_xy) {iy = 0; ix = 1;}

	data = GMT_memory (GMT, NULL, n_alloc, struct DATA);

	while (fgets (line, GMT_TEXT_LEN256, fp)) {
		if (sscanf (line, "%lg %lg", &in[0], &in[1]) !=2)
			GMT_report (GMT, GMT_MSG_FATAL, "ERROR deciphering line %ld of polygon file\n", ndata+1);
		if (ndata == n_alloc) {
			n_alloc <<= 1;
			data = GMT_memory (GMT, data, n_alloc, struct DATA);
		}
		data[ndata].x = in[ix];
		data[ndata].y = in[iy];
		ndata++;
	}
	fclose(fp);
	return (ndata);
}

/* -----------------------------------------------------------------*/
int grdokb_body_desc(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc, struct BODY_VERTS **body_verts, int face) {
/*
		__________________________________________
		|                                        |
		|               _________________        |
		|             /|         X (Nord)        |
		|            / |                         |
		|           /  |     0 ________ 1        |
		|          /   |      /       /|         |
		|       Y /    |     /       / |         |
		|       (Est)  |  3 /_______/2 |         |
		|              |    |       |  |         |
		|              |    |       | / 5        |
		|            Z |    |       |/           |
		|              V    |_______/            |
		|                  7         6           |
		|________________________________________|

*/
	if (face == 0) {			/* Decompose the TOP square surface in 2 triangles using CW order */
		body_desc->n_f = 2;
		if (body_desc->n_v == NULL)
			body_desc->n_v = GMT_memory (GMT, NULL, body_desc->n_f, GMT_LONG);
		body_desc->n_v[0] = body_desc->n_v[1] = 3;
		if (body_desc->ind == NULL)
			body_desc->ind = GMT_memory (GMT, NULL, body_desc->n_v[0] + body_desc->n_v[1], GMT_LONG);
		body_desc->ind[0] = 0;	body_desc->ind[1] = 1; 	body_desc->ind[2] = 2;	/* 1st top triang (0 1 3)*/
		body_desc->ind[3] = 0;	body_desc->ind[4] = 2; 	body_desc->ind[5] = 3;	/* 2nd top triang (1 2 3) */
		if (*body_verts == NULL) *body_verts = GMT_memory (GMT, NULL, 4, struct BODY_VERTS);
		return(0);
	}
	else if (face == 5) {			/* Decompose the BOT square surface in 2 triangles using CCW order */
		body_desc->n_f = 2;
		if (body_desc->n_v == NULL)
			body_desc->n_v = GMT_memory (GMT, NULL, body_desc->n_f, GMT_LONG);
		body_desc->n_v[0] = body_desc->n_v[1] = 3;
		if (body_desc->ind == NULL)
			body_desc->ind = GMT_memory (GMT, NULL, body_desc->n_v[0] + body_desc->n_v[1], GMT_LONG);
		body_desc->ind[0] = 0;	body_desc->ind[1] = 2; 	body_desc->ind[2] = 1;	/* 1st bot triang */
		body_desc->ind[3] = 0;	body_desc->ind[4] = 3; 	body_desc->ind[5] = 2;	/* 2nd bot triang */
		if (*body_verts == NULL) *body_verts = GMT_memory (GMT, NULL, 4, struct BODY_VERTS);
		return(0);
	}
	/* Other face cases will go here */
	return(0);
}

int grdokb_body_set(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
		struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
		double *cos_vec, int j, int i, int inc_j, int inc_i) {
	/* Allocate and fille the body_desc structure with the description on how to
	   connect the vertex of the polygonal planar surface */
	int i1, j1, is_geog = Ctrl->box.is_geog;
	float *z = Grid->data;
	double cosj, cosj1;
	struct GRD_HEADER *h = Grid->header;

		j1 = j + inc_j;		i1 = i + inc_i;
		cosj = cos_vec[j];		cosj1 = cos_vec[j1];

		body_verts[0].x = (is_geog) ? x[i]  * cosj  : x[i];
		body_verts[1].x = (is_geog) ? x[i1] * cosj  : x[i1];
		body_verts[2].x = (is_geog) ? x[i1] * cosj1 : x[i1];
		body_verts[3].x = (is_geog) ? x[i]  * cosj1 : x[i];
		body_verts[0].y = y[j];
		body_verts[1].y = body_verts[0].y;
		body_verts[2].y = y[j1];
		body_verts[3].y = body_verts[2].y;
		if (inc_i == 1) {
			GMT_LONG ij;
			ij = GMT_IJP(h,j,i);
			body_verts[0].z = z[ij];
			body_verts[1].z = z[ij + 1];  /* z[GMT_IJP(h,j,i1)];  */
			ij = GMT_IJP(h,j1,i1);
			body_verts[2].z = z[ij];      /* z[GMT_IJP(h,j1,i1)]; */
			body_verts[3].z = z[ij - 1];  /* z[GMT_IJP(h,j1,i)];  */
		}
		else {		/* Base surface */
			body_verts[0].z = body_verts[1].z = body_verts[2].z = body_verts[3].z = Ctrl->Z.z0;
		}
#if 0
	else {
		int k;

		n_pts = Grid->header->nx;
		if ((face % 2) == 0) n_pts = Grid->header->ny;		/* Even faces are at const X */

		body_desc->n_f = 1;
		body_desc->n_v = GMT_memory (GMT, body_verts->n_v, body_verts->n_f, int);
		body_desc->n_v[0] = n_pts + 2;
		body_desc->ind = GMT_memory (GMT, body_verts->ind, body_verts->n_v[0], int);
		body_verts.x = GMT_memory (GMT, body_verts.x, body_desc->n_v[0], double);
		body_verts.y = GMT_memory (GMT, body_verts.y, body_desc->n_v[0], double);
		body_verts.z = GMT_memory (GMT, body_verts.z, body_desc->n_v[0], double);

		if (face == 1) {		/* South face. Build facet in CW direction when viewed from outside */
			k = Grid->header->ny - 1;
			if (is_geog) cosLat = cos(y[k]*D2R);
			for (i = 0; i < n_pts; i++) {
				body_desc->ind[i] = i;
				body_verts[i].x = (is_geog) ? (x[i]  - Ctrl->box.lon_0) * Ctrl->box.d_to_m * cosLat : x[i];
				body_verts[i].y = (is_geog) ? (y[k]  + Ctrl->box.lat_0)  * Ctrl->box.d_to_m : y[k];
				body_verts[i].z = z[GMT_IJP(h,0,i)];
			}
		}
		else if (face == 2) {
			k = Grid->header->nx - 1;
			for (i = 0; i < n_pts; i++) {
				if (is_geog) cosLat = cos(y[i]*D2R);
				body_desc->ind[i] = n_pts - 1 - i;
				body_verts[i].x = (is_geog) ? (x[k]  - Ctrl->box.lon_0) * Ctrl->box.d_to_m * cosLat : x[k];
				body_verts[i].y = (is_geog) ? (y[i]  + Ctrl->box.lat_0)  * Ctrl->box.d_to_m : y[i];
				body_verts[i].z = z[GMT_IJP(h,i,k)];
			}
		}
		else if (face == 3) {
			k = 0;
			if (is_geog) cosLat = cos(y[k]*D2R);
			for (i = 0; i < n_pts; i++) {
				i = n_pts - 1 - i;
				body_desc->ind[i] = i;
				body_verts[i].x = (is_geog) ? (x[i] - Ctrl->box.lon_0) * Ctrl->box.d_to_m * cosLat : x[i];
				body_verts[i].y = (is_geog) ? (y[k]  + Ctrl->box.lat_0)  * Ctrl->box.d_to_m : y[k];
				body_verts[i].z = z[GMT_IJP(h,h->ny-1,i)];
			}
		}
		else if (face == 4) {
			k = 0;
			for (i = 0; i < n_pts; i++) {
				if (is_geog) cosLat = cos(y[i]*D2R);
				body_desc->ind[i] = i;
				body_verts[i].x = (is_geog) ? (x[k]  - Ctrl->box.lon_0) * Ctrl->box.d_to_m * cosLat : x[k];
				body_verts[i].y = (is_geog) ? (y[i]  + Ctrl->box.lat_0)  * Ctrl->box.d_to_m : y[i];
				body_verts[i].z = z[GMT_IJP(h,i,k)];
			}
		}
		/* Now this is common to all 4 cases */
		body_desc->ind[n_pts]  = n_pts;
		body_desc->ind[n_pts+1]= n_pts + 1;
		body_verts[n_pts].x   = body_verts[n_pts-1].x;
		body_verts[n_pts+1].x = body_verts[0].x;
		body_verts[n_pts].y   = body_verts[n_pts-1].y;
		body_verts[n_pts+1].y = body_verts[0].y;
		body_verts[n_pts].z   = body_verts[n_pts+1].z = Ctrl->Z.z0;
	}
	return(n_pts+3);
#endif
	return(0);
}

void grdokb_calc_top_surf (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
		struct GMT_GRID *Gout, double *g, int n_pts, double *x_grd, double *y_grd, double *x_obs,
		double *y_obs, double *cos_vec, struct MAG_VAR *mag_var, struct LOC_OR *loc_or,
		struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts) {

	/* Send g = NULL for grid computations (e.g. -G) or Gout = NULL otherwise (-F).
	   In case of polyline output (-F) n_pts is the number of output locations (irrelevant otherwise) */
	int row, col, k, i, km;
	double x_o, y_o, tmp, a;

/*#ifdef _OPENMP
#pragma omp parallel for private(row, col, k, i, y_o, x_o)
#endif */
	for (row = 0; row < Grid->header->ny - 1; row++) {		/* Loop over input grid rows */

		if (GMT_is_verbose (GMT, GMT_MSG_NORMAL))
			GMT_message (GMT, "Line = %d\t of = %.3d\r", row, Grid->header->ny);

		for (col = 0; col < Grid->header->nx - 1; col++) {	/* Loop over input grid cols */
			km = (Ctrl->H.active) ? row * (Grid->header->nx - 1) + col : 0;
			/* Don't waste time with zero mag triangles */
			if (Ctrl->H.active && mag_var[km].rk[0] == 0 && mag_var[km].rk[1] == 0 && mag_var[km].rk[2] == 0)
				continue;

			grdokb_body_set(GMT, Ctrl, Grid, body_desc, body_verts, x_grd, y_grd, cos_vec, row, col, 1, 1);

			if (Ctrl->G.active) {
				for (k = 0; k < Gout->header->ny; k++) {
					y_o = (Ctrl->box.is_geog) ? (y_obs[k] + Ctrl->box.lat_0) * Ctrl->box.d_to_m : y_obs[k]; /* + lat_0 because y was already *= -1 */

					if (Ctrl->box.is_geog) tmp = Ctrl->box.d_to_m * cos(y_obs[k]*D2R);
					for (i = 0; i < Gout->header->nx; i++) {
						x_o = (Ctrl->box.is_geog) ? (x_obs[i] - Ctrl->box.lon_0) * tmp : x_obs[i];
						a = okabe (GMT, x_o, y_o, Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, *body_desc, body_verts, km, 0, loc_or);
						Gout->data[GMT_IJP(Gout->header, k, i)] += (float)a;
					}
				}
			}
			else {
				for (k = 0; k < n_pts; k++)
					g[k] += okabe (GMT, x_obs[k], y_obs[k], Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, *body_desc, body_verts, km, 0, loc_or);
			}
		}
	}
}
