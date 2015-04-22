/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * API functions to support the grdgravmag3d application.
 *
 * Brief synopsis: Compute the gravity anomaly of the volume contained between
 *                 a surface provided by one grid and a plane, or between a top
 *                 and a bottom surface provided by two grids.
 *
 * Author:	Joaquim Luis
 * Date:	1-Apr-2012
 *
 *          Removed #if 0 code that dealt with side faces in rev r12620 
 *
 */

#define THIS_MODULE_NAME	"grdgravmag3d"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Computes the gravity effect of one (or two) grids by the method of Okabe"

#include "gmt_dev.h"
#include "okbfuns.h"

#define GMT_PROG_OPTIONS "-:RVf"

struct GRDOKB_CTRL {

	struct GRDOKB_In {
		bool active;
		char *file[3];
	} In;

	struct GRDOKB_C {	/* -C */
		bool active;
		double rho;
	} C;
	struct GRDOKB_D {	/* -D */
		bool active;
		float z_dir;
	} D;
	struct GRDOKB_I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct GRDOKB_F {	/* -F<grdfile> */
		bool active;
		char *file;
	} F;
	struct GRDOKB_G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct GRDOKB_H {	/* -H */
		bool active;
		double	t_dec, t_dip, m_int, m_dec, m_dip;
	} H;
	struct GRDOKB_L {	/* -L */
		double zobs;
	} L;
	struct GRDOKB_S {	/* -S */
		bool active;
		double radius;
	} S;
	struct GRDOKB_Z {	/* -Z */
		double z0;
	} Z;
	struct GRDOKB_Q {	/* -Q */
		bool active;
		unsigned int n_pad;
		char region[GMT_BUFSIZ];	/* gmt_parse_R_option has this!!!! */
		double pad_dist;
	} Q;
	struct GRDOKB_box {	/* No option, just a container */
		bool is_geog;
		double	d_to_m, *mag_int, lon_0, lat_0;
	} box;
};

struct DATA {
	double  x, y;
} *data;

int read_poly__ (struct GMT_CTRL *GMT, char *fname, bool switch_xy);
void set_center (unsigned int n_triang);
int grdgravmag3d_body_set (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
	struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
	double *cos_vec, unsigned int j, unsigned int i, unsigned int inc_j, unsigned int inc_i);
int grdgravmag3d_body_desc(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc,
	struct BODY_VERTS **body_verts, unsigned int face);
void grdgravmag3d_calc_top_surf (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
	struct GMT_GRID *Gout, double *g, unsigned int n_pts, double *x_grd, double *y_grd, double *x_obs,
	double *y_obs, double *cos_vec, struct MAG_VAR *mag_var, struct LOC_OR *loc_or,
	struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts);

void *New_grdgravmag3d_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDOKB_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRDOKB_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->L.zobs = 0;
	C->D.z_dir = -1;		/* -1 because Z was positive down for Okabe */
	C->S.radius = 30000;
	return (C);
}

void Free_grdgravmag3d_Ctrl (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file[0]) free (C->In.file[0]);
	if (C->In.file[1]) free (C->In.file[1]);
	if (C->F.file) free (C->F.file);
	if (C->G.file) free (C->G.file);
	GMT_free (GMT, C);
}

int GMT_grdgravmag3d_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdgravmag3d grdfile_up [grdfile_low] [-C<density>] [-D] [-F<xy_file>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-G<outfile>] [%s] [-L<z_obs>]\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Q[n<n_pad>]|[pad_dist]|[<w/e/s/n>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-Z<level>] [-fg]\n", GMT_Rgeo_OPT, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\tgrdfile_up is the grdfile whose gravity efect is to be computed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If two grids are provided then the gravity/magnetic efect of the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   volume between them is computed\n\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C sets body density in SI\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F pass file with locations where anomaly is going to be computed\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G name of the output grdfile.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D if z is positive down [Default positive up]\n");
	/*GMT_Message (API, GMT_TIME_NONE, "\t-H sets parameters for computation of magnetic anomaly\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f_dec/f_dip -> geomagnetic declination/inclination\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m_int/m_dec/m_dip -> body magnetic intensity/declination/inclination\n");*/
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t   The new xinc and yinc should be divisible by the old ones (new lattice is subset of old).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L sets level of observation [Default = 0]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Map units true; x,y in degrees, dist units in m [Default dist unit = x,y unit].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Extend the domain of computation with respect to output -R region.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qn<N> artifficially extends the width of the outer rim of cells to have a fake\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   width of N * dx[/dy].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Q<pad_dist> extend the region by west-pad, east+pad, etc\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Q<region> Same sintax as -R.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-R For new Range of output grid; enter <WESN> (xmin, xmax, ymin, ymax) separated by slashes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses the same region as the input grid].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z z level of reference plane [Default = 0]\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Convert geographic grids to meters using a \"Flat Earth\" approximation.\n");
	GMT_Option (API, ":,.");

	return (EXIT_FAILURE);
}

int GMT_grdgravmag3d_parse (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdgravmag3d and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct	GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	int i = 0;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file */
				Ctrl->In.active = true;
				if (n_files == 0)
					Ctrl->In.file[n_files++] = strdup (opt->arg);
				else if (n_files == 1)
					Ctrl->In.file[n_files++] = strdup (opt->arg);
				else {
					n_errors++;
					GMT_Report (API, GMT_MSG_NORMAL, "Error: A maximum of two input grids may be processed\n");
				}
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.rho = atof (opt->arg) * 6.674e-6;
				Ctrl->C.active = true;
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.z_dir = 1;
				break;
			case 'F':
				Ctrl->F.active = true;
				Ctrl->F.file = strdup (opt->arg);
				break;
 			case 'G':
				Ctrl->G.active = true;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'H':
				if ((sscanf(opt->arg, "%lf/%lf/%lf/%lf/%lf",
					    &Ctrl->H.t_dec, &Ctrl->H.t_dip, &Ctrl->H.m_int, &Ctrl->H.m_dec, &Ctrl->H.m_dip)) != 5) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -H option: Can't dechiper values\n");
					n_errors++;
				}
				Ctrl->H.active = true;
				Ctrl->C.active = false;
				break;
			case 'I':
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
	 		case 'L':
				Ctrl->L.zobs = atof (opt->arg);
				break;
			case 'M':
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -M is deprecated; -fg was set instead, use this in the future.\n");
					if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				if (opt->arg[0] == 'n') {
					Ctrl->Q.n_pad = atoi (&opt->arg[1]);
				}
				else {
					int n = 0;
					char *pch = strchr(opt->arg, '/');
					while (pch != NULL) {
						n++;
						pch = strchr(pch+1,'/');
					}
					if (n == 0)
						Ctrl->Q.pad_dist = atof (opt->arg);	/* Pad given as a scalar distance */
					else if (n == 3)
						strncpy(Ctrl->Q.region, opt->arg, GMT_BUFSIZ);	/* Pad given as a -R region */
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option. Either -Q<pad> or -Q<region>\n");
						n_errors++;
					}
				}
				break;
	 		case 'S':
				Ctrl->S.radius = atof (opt->arg) * 1000;
				Ctrl->S.active = true;
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
	i += GMT_check_condition (GMT, Ctrl->G.active && Ctrl->F.active, "Warning: -F overrides -G\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdgravmag3d_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdgravmag3d (void *V_API, int mode, void *args) {

	unsigned int nx_p, ny_p, i, j, k, ndata = 0, clockwise_type[] = {0, 5};
	bool two_grids = false, switch_xy = false;
	unsigned int km = 0;		/* index of current body facet (for mag only) */
	int error = 0, retval;
	unsigned int n_vert_max;
	double	a, d, x_o, y_o;
	double	*x_obs = NULL, *y_obs = NULL, *x_grd = NULL, *y_grd = NULL, *cos_vec = NULL;
	double	*g = NULL, *x_grd2 = NULL, *y_grd2 = NULL, *cos_vec2 = NULL;
	double	cc_t, cs_t, s_t, wesn_new[4], wesn_padded[4];

	struct	GMT_GRID *GridA = NULL, *GridB = NULL;
	struct	LOC_OR *loc_or = NULL;
	struct	BODY_VERTS *body_verts = NULL;
	struct	BODY_DESC body_desc;
	struct	GRDOKB_CTRL *Ctrl = NULL;
	struct	GMT_GRID *Gout = NULL;
	struct	GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct	GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	mag_var = NULL, mag_param = NULL, data = NULL;
	body_desc.n_v = NULL, body_desc.ind = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdgravmag3d_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE)
		bailout (GMT_grdgravmag3d_usage (API, GMT_USAGE));		/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS)
		bailout (GMT_grdgravmag3d_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdgravmag3d_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdgravmag3d_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdgravmag3d main code ---------------------*/

	if (GMT_is_geographic (GMT, GMT_IN)) Ctrl->box.is_geog = true;

	if (!Ctrl->box.is_geog)
		Ctrl->box.d_to_m = 1.;
	else
		Ctrl->box.d_to_m = 2.0 * M_PI * 6371008.7714 / 360.0;
		/*Ctrl->box.d_to_m = 2.0 * M_PI * gmtdefs.ellipse[N_ELLIPSOIDS-1].eq_radius / 360.0;*/

	if (Ctrl->F.active) { 		/* Read xy file where anomaly is to be computed */
		if ( (retval = read_poly__ (GMT, Ctrl->F.file, switch_xy)) < 0 ) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open file %s\n", Ctrl->F.file);
			return (EXIT_FAILURE);
		}
		ndata = retval;
	}

	/* ---------------------------------------------------------------------------- */

	if ((GridA = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL,
				Ctrl->In.file[0], NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	if (Ctrl->G.active) {
		double wesn[4], inc[2];
		/* Use the -R region for output if set; otherwise match grid domain */
		GMT_memcpy (wesn, (GMT->common.R.active ? GMT->common.R.wesn : GridA->header->wesn), 4, double);
		GMT_memcpy (inc, (Ctrl->I.active ? Ctrl->I.inc : GridA->header->inc), 2, double);
		if (wesn[XLO] < GridA->header->wesn[XLO]) error = true;
		if (wesn[XHI] > GridA->header->wesn[XHI]) error = true;

		if (wesn[YLO] < GridA->header->wesn[YLO]) error = true;
		if (wesn[YHI] > GridA->header->wesn[YHI]) error = true;

		if (error) {
			GMT_Report (API, GMT_MSG_NORMAL, "New WESN incompatible with old.\n");
			Return (EXIT_FAILURE);
		}

		if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, wesn, inc, \
			GridA->header->registration, GMT_NOTSET, Ctrl->G.file)) == NULL) Return (API->error);

		GMT_Report (API, GMT_MSG_VERBOSE, "Grid dimensions are nx = %d, ny = %d\n",
					Gout->header->nx, Gout->header->ny);
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "Allocates memory and read data file\n");

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

		GMT->common.R.active = false;
		GMT_parse_common_options (GMT, "R", 'R', Ctrl->Q.region);	/* Use the -R parsing machinery to handle this */
		GMT_memcpy (wesn_padded, GMT->common.R.wesn, 4, double);
		GMT_memcpy (GMT->common.R.wesn, wesn_new, 4, double);		/* Reset previous WESN */
		GMT->common.R.active = true;

		if (wesn_padded[XLO] < GridA->header->wesn[XLO]) {
			GMT_Report (API, GMT_MSG_NORMAL, "Request padding at the West border exceed grid limit, trimming it\n");
			wesn_padded[XLO] = GridA->header->wesn[XLO];
		}
		if (wesn_padded[XHI] > GridA->header->wesn[XHI]) {
			GMT_Report (API, GMT_MSG_NORMAL, "Request padding at the East border exceed grid limit, trimming it\n");
			wesn_padded[XHI] = GridA->header->wesn[XHI];
		}
		if (wesn_padded[YLO] < GridA->header->wesn[YLO]) {
			GMT_Report (API, GMT_MSG_NORMAL, "Request padding at the South border exceed grid limit, trimming it\n");
			wesn_padded[YLO] = GridA->header->wesn[YLO];
		}
		if (wesn_padded[YHI] > GridA->header->wesn[YHI]) {
			GMT_Report (API, GMT_MSG_NORMAL, "Request padding at the North border exceed grid limit, trimming it\n");
			wesn_padded[YHI] = GridA->header->wesn[YHI];
		}
	}
	else
		GMT_memcpy (wesn_padded, GridA->header->wesn, 4, double);

	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn_padded, Ctrl->In.file[0], GridA) == NULL) {	/* Get subset, or all */
		Return (API->error);
	}

	/* Check that Inner region request does not exceeds input grid limits */
	if (GMT->common.R.active && Ctrl->G.active) {
		if (Gout->header->wesn[XLO] < GridA->header->wesn[XLO] ||
				Gout->header->wesn[XHI] > GridA->header->wesn[XHI]) {
			GMT_Report (API, GMT_MSG_NORMAL, " Selected region exceeds the X-boundaries of the grid file!\n");
			return (EXIT_FAILURE);
		}
		else if (Gout->header->wesn[YLO] < GridA->header->wesn[YLO] ||
				Gout->header->wesn[YHI] > GridA->header->wesn[YHI]) {
			GMT_Report (API, GMT_MSG_NORMAL, " Selected region exceeds the Y-boundaries of the grid file!\n");
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
		if ((GridB = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file[1], NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}

		if(GridA->header->registration != GridB->header->registration) {
			GMT_Report (API, GMT_MSG_NORMAL, "Up and bottom grids have different registrations!\n");
			Return (EXIT_FAILURE);
		}
		if ((GridA->header->z_scale_factor != GridB->header->z_scale_factor) ||
				(GridA->header->z_add_offset != GridB->header->z_add_offset)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Scale/offset not compatible!\n");
			Return (EXIT_FAILURE);
		}

		if (fabs (GridA->header->inc[GMT_X] - GridB->header->inc[GMT_X]) > 1.0e-6 ||
				fabs (GridA->header->inc[GMT_Y] - GridB->header->inc[GMT_Y]) > 1.0e-6) {
			GMT_Report (API, GMT_MSG_NORMAL, "Up and bottom grid increments do not match!\n");
			Return (EXIT_FAILURE);
		}
		/* ALMOST FOR SURE THERE ARE MEMORY CLEAN UPS TO DO HERE BEFORE RETURNING */

		if (Ctrl->D.z_dir != -1) {
			for (j = 0; j < GridB->header->ny; j++)
				for (i = 0; i < GridB->header->nx; i++)
					GridB->data[GMT_IJP(GridB->header,j,i)] *= Ctrl->D.z_dir;
		}
		two_grids = true;
	}

	nx_p = !Ctrl->F.active ? Gout->header->nx : ndata;
	ny_p = !Ctrl->F.active ? Gout->header->ny : ndata;
	x_grd = GMT_memory (GMT, NULL, GridA->header->nx, double);
	y_grd = GMT_memory (GMT, NULL, GridA->header->ny, double);
	x_obs = GMT_memory (GMT, NULL, nx_p, double);
	y_obs = GMT_memory (GMT, NULL, ny_p, double);
	if (Ctrl->F.active) g = GMT_memory (GMT, NULL, ndata, double);

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
		x_grd2 = GMT_memory (GMT, NULL, GridB->header->nx, double);
		y_grd2 = GMT_memory (GMT, NULL, GridB->header->ny, double);
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
		grdgravmag3d_body_desc(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[0]);		/* Set CW or CCW of top triangs */

	/* Allocate a structure that will be used inside okabe().
	   We do it here to avoid thousands of alloc/free that would result if done in okabe() */
	n_vert_max = body_desc.n_v[0];
	for (i = 1; i < body_desc.n_f; i++)
		n_vert_max = MAX(body_desc.n_v[i], n_vert_max);

	loc_or = GMT_memory (GMT, NULL, (n_vert_max+1), struct LOC_OR);

/* ---------------> Now start computing <------------------------------------- */

	if (Ctrl->G.active) { /* grid output */
		grdgravmag3d_calc_top_surf (GMT, Ctrl, GridA, Gout, NULL, 0, x_grd, y_grd, x_obs, y_obs, cos_vec,
					mag_var, loc_or, &body_desc, body_verts);

		if (!two_grids) {		/* That is, one grid and a flat base Do the BASE now */
			grdgravmag3d_body_desc(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of BOT triangs */
			grdgravmag3d_body_set(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 0, 0,
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
			grdgravmag3d_body_desc(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of top triangs */
			grdgravmag3d_calc_top_surf (GMT, Ctrl, GridB, Gout, NULL, 0, x_grd2, y_grd2, x_obs, y_obs,
					cos_vec2, mag_var, loc_or, &body_desc, body_verts);
		}
	}
	else {		/* polygon output */
		grdgravmag3d_calc_top_surf (GMT, Ctrl, GridA, NULL, g, ndata, x_grd, y_grd, x_obs, y_obs, cos_vec,
				mag_var, loc_or, &body_desc, body_verts);

		if (!two_grids) {		/* That is, one grid and a flat base. Do the BASE now */
			grdgravmag3d_body_desc(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of BOT triangs */
			grdgravmag3d_body_set(GMT, Ctrl, GridA, &body_desc, body_verts, x_grd, y_grd, cos_vec, 0, 0,
					GridA->header->ny-1, GridA->header->nx-1);
			for (k = 0; k < ndata; k++)
				g[k] += okabe (GMT, x_obs[k], y_obs[k], Ctrl->L.zobs, Ctrl->C.rho, Ctrl->C.active, body_desc, body_verts, km, 0, loc_or);
		}
		else {		/* "two_grids". One at the top and the other at the base */
			grdgravmag3d_body_desc(GMT, Ctrl, &body_desc, &body_verts, clockwise_type[1]);		/* Set CW or CCW of top triangs */
			grdgravmag3d_calc_top_surf (GMT, Ctrl, GridB, NULL, g, ndata, x_grd2, y_grd2, x_obs, y_obs,
					cos_vec2, mag_var, loc_or, &body_desc, body_verts);
		}
	}

	/*---------------------------------------------------------------------------------------------*/

	if (Ctrl->G.active) {
		if (Ctrl->C.active) {
			strcpy (Gout->header->title, "Gravity field");
			strcpy (Gout->header->z_units, "mGal");
		}
		else {
			strcpy (Gout->header->title, "Magnetic field");
			strcpy (Gout->header->z_units, "nT");
		}

		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Gout)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Gout) != GMT_OK) {
			Return (API->error);
		}
	}
	else {
		double out[3];
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data output */
			Return (API->error);
		}
		if ((error = GMT_set_cols (GMT, GMT_OUT, 3)) != GMT_OK) {
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		for (k = 0; k < ndata; k++) {
			out[GMT_X] = data[k].x;
			out[GMT_Y] = data[k].y;
			out[GMT_Z] = g[k];
			GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
		}
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
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
int read_poly__ (struct GMT_CTRL *GMT, char *fname, bool switch_xy) {
	/* Read file with xy points where anomaly is going to be computed
	   This is a temporary function while we not use the API to do this job. */
	unsigned int ndata, ix = 0, iy = 1;
	size_t n_alloc;
	double in[2];
	char line[GMT_LEN256] = {""};
	FILE *fp = NULL;

	if ((fp = fopen (fname, "r")) == NULL) return (-1);

	n_alloc = GMT_CHUNK;
	ndata = 0;
	if (switch_xy) {iy = 0; ix = 1;}

	data = GMT_memory (GMT, NULL, n_alloc, struct DATA);

	while (fgets (line, GMT_LEN256, fp)) {
		if (sscanf (line, "%lg %lg", &in[0], &in[1]) !=2)
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "ERROR deciphering line %d of polygon file\n", ndata+1);
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
int grdgravmag3d_body_desc(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct BODY_DESC *body_desc, struct BODY_VERTS **body_verts, unsigned int face) {
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
	GMT_UNUSED(Ctrl);
	if (face == 0) {			/* Decompose the TOP square surface in 2 triangles using CW order */
		body_desc->n_f = 2;
		if (body_desc->n_v == NULL)
			body_desc->n_v = GMT_memory (GMT, NULL, body_desc->n_f, unsigned int);
		body_desc->n_v[0] = body_desc->n_v[1] = 3;
		if (body_desc->ind == NULL)
			body_desc->ind = GMT_memory (GMT, NULL, body_desc->n_v[0] + body_desc->n_v[1], unsigned int);
		body_desc->ind[0] = 0;	body_desc->ind[1] = 1; 	body_desc->ind[2] = 2;	/* 1st top triang (0 1 3)*/
		body_desc->ind[3] = 0;	body_desc->ind[4] = 2; 	body_desc->ind[5] = 3;	/* 2nd top triang (1 2 3) */
		if (*body_verts == NULL) *body_verts = GMT_memory (GMT, NULL, 4, struct BODY_VERTS);
		return(0);
	}
	else if (face == 5) {			/* Decompose the BOT square surface in 2 triangles using CCW order */
		body_desc->n_f = 2;
		if (body_desc->n_v == NULL)
			body_desc->n_v = GMT_memory (GMT, NULL, body_desc->n_f, unsigned int);
		body_desc->n_v[0] = body_desc->n_v[1] = 3;
		if (body_desc->ind == NULL)
			body_desc->ind = GMT_memory (GMT, NULL, body_desc->n_v[0] + body_desc->n_v[1], unsigned int);
		body_desc->ind[0] = 0;	body_desc->ind[1] = 2; 	body_desc->ind[2] = 1;	/* 1st bot triang */
		body_desc->ind[3] = 0;	body_desc->ind[4] = 3; 	body_desc->ind[5] = 2;	/* 2nd bot triang */
		if (*body_verts == NULL) *body_verts = GMT_memory (GMT, NULL, 4, struct BODY_VERTS);
		return(0);
	}
	/* Other face cases will go here */
	return(0);
}

int grdgravmag3d_body_set(struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
		struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts, double *x, double *y,
		double *cos_vec, unsigned int j, unsigned int i, unsigned int inc_j, unsigned int inc_i) {
	/* Allocate and fille the body_desc structure with the description on how to
	   connect the vertex of the polygonal planar surface */
	unsigned int i1, j1;
	bool is_geog = Ctrl->box.is_geog;
	float *z = Grid->data;
	double cosj, cosj1;
	struct GMT_GRID_HEADER *h = Grid->header;
	GMT_UNUSED(GMT); GMT_UNUSED(body_desc);

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
		int ij;
		ij = (int)GMT_IJP(h,j,i);
		body_verts[0].z = z[ij];
		body_verts[1].z = z[ij + 1];  /* z[GMT_IJP(h,j,i1)];  */
		ij = (int)GMT_IJP(h,j1,i1);
		body_verts[2].z = z[ij];      /* z[GMT_IJP(h,j1,i1)]; */
		body_verts[3].z = z[ij - 1];  /* z[GMT_IJP(h,j1,i)];  */
	}
	else {		/* Base surface */
		body_verts[0].z = body_verts[1].z = body_verts[2].z = body_verts[3].z = Ctrl->Z.z0;
	}

	return(0);
}

void grdgravmag3d_calc_top_surf (struct GMT_CTRL *GMT, struct GRDOKB_CTRL *Ctrl, struct GMT_GRID *Grid,
		struct GMT_GRID *Gout, double *g, unsigned int n_pts, double *x_grd, double *y_grd, double *x_obs,
		double *y_obs, double *cos_vec, struct MAG_VAR *mag_var, struct LOC_OR *loc_or,
		struct BODY_DESC *body_desc, struct BODY_VERTS *body_verts) {

	/* Send g = NULL for grid computations (e.g. -G) or Gout = NULL otherwise (-F).
	   In case of polyline output (-F) n_pts is the number of output locations (irrelevant otherwise) */
	unsigned int row, col, k, i, km;
	double x_o, y_o, tmp = 1, a;

/*#ifdef _OPENMP
#pragma omp parallel for private(row, col, k, i, y_o, x_o)
#endif */
	for (row = 0; row < Grid->header->ny - 1; row++) {		/* Loop over input grid rows */

		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE))
			GMT_Message (GMT->parent, GMT_TIME_NONE, "Line = %d\t of = %.3d\r", row, Grid->header->ny);

		for (col = 0; col < Grid->header->nx - 1; col++) {	/* Loop over input grid cols */
			km = (Ctrl->H.active) ? row * (Grid->header->nx - 1) + col : 0;
			/* Don't waste time with zero mag triangles */
			if (Ctrl->H.active && mag_var[km].rk[0] == 0 && mag_var[km].rk[1] == 0 && mag_var[km].rk[2] == 0)
				continue;

			grdgravmag3d_body_set(GMT, Ctrl, Grid, body_desc, body_verts, x_grd, y_grd, cos_vec, row, col, 1, 1);

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
