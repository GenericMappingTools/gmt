/*--------------------------------------------------------------------
 *	$Id: grdgradient_func.c,v 1.11 2011-04-26 17:52:49 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Brief synopsis: grdgradient read a grid file and compute gradient in azim direction:
 * azim = azimuth clockwise from north in degrees.
 * gradient = -[(dz/dx)sin(azim) + (dz/dy)cos(azim)].
 * the expression in [] is the correct gradient.  We take
 * -[]  in order that data which goes DOWNHILL in the
 * azim direction will give a positive value; this is
 * for image shading purposes.
 *
 * Author:	W.H.F. Smith
 * Date: 	1-JAN-2010
 * Version:	5 API
 */
 
#include "gmt.h"

struct GRDGRADIENT_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct A {	/* -A<azim>[/<azim2>] */
		GMT_LONG active;
		GMT_LONG two;
		double azimuth[2];
	} A;
	struct D {	/* -D[a][o][n] */
		GMT_LONG active;
		GMT_LONG mode;
	} D;
	struct E {	/* -E[s|p]<azim>/<elev[ambient/diffuse/specular/shine]> */
		GMT_LONG active;
		double azimuth, elevation;
		double ambient, diffuse, specular, shine;
		GMT_LONG mode;
	} E;
	struct G {	/* -G<file> */
		GMT_LONG active;
		char *file;
	} G;
	struct N {	/* -N[t_or_e][<amp>[/<sigma>[/<offset>]]] */
		GMT_LONG active;
		GMT_LONG mode;	/* 1 = atan, 2 = exp */
		double norm, sigma, offset;
	} N;
	struct S {	/* -S<slopefile> */
		GMT_LONG active;
		char *file;
	} S;
};

void *New_grdgradient_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDGRADIENT_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDGRADIENT_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->E.ambient = 0.55;
	C->E.diffuse = 0.6;
	C->E.specular = 0.4;
	C->E.shine = 10;
	C->N.norm = 1.0;		
	return ((void *)C);
}

void Free_grdgradient_Ctrl (struct GMT_CTRL *GMT, struct GRDGRADIENT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->G.file) free ((void *)C->G.file);	
	if (C->S.file) free ((void *)C->S.file);	
	GMT_free (GMT, C);	
}

double specular (double nx, double ny, double nz, double *s) {
	/* SPECULAR Specular reflectance.
	   R = SPECULAR(Nx,Ny,Nz,S,V) returns the reflectance of a surface with
	   normal vector components [Nx,Ny,Nz].  S and V specify the direction
	   to the light source and to the viewer, respectively. 
	   For the time beeing I'm using V = [azim elev] = [0 90] so the following

	   V[0] =  sind(V[0])*cosd(V[1]);
	   V[1] = -cosd(V[0])*cosd(V[1]);
	   V[2] =  sind(V[1]);

	   Reduces to V[0] = 0;		V[1] = 0;	V[2] = 1 */

	/*r = MAX(0,2*(s[0]*nx+s[1]*ny+s[2]*nz).*(v[0]*nx+v[1]*ny+v[2]*nz) - (v'*s)*ones(m,n)); */

	return (MAX(0, 2 * (s[0]*nx + s[1]*ny + s[2]*nz) * nz - s[2]));
}

GMT_LONG GMT_grdgradient_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdgradient %s [API] - Compute directional gradients from grid files\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdgradient <infile> -G<outfile> [-A<azim>[/<azim2>]] [-D[a][o][n]]\n");
	GMT_message (GMT, "[-E[s|p]<azim>/<elev[ambient/diffuse/specular/shine]>]\n");
	GMT_message (GMT, "[-N[t_or_e][<amp>[/<sigma>[/<offset>]]]] [%s] [-S<slopefile>] [%s] [%s] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_n_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<infile> is name of input grid file\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Sets azimuth (0-360 CW from North (+y)) for directional derivatives.\n");
	GMT_message (GMT, "\t  -A<azim>/<azim2> will compute two directions and save the one larger in magnitude.\n");
	GMT_message (GMT, "\t-D Finds the direction of grad z.\n");
	GMT_message (GMT, "\t   Append c to get cartesian angle (0-360 CCW from East (+x)) [Default: azimuth].\n");
	GMT_message (GMT, "\t   Append o to get bidirectional orientations [0-180] rather than directions [0-360].\n");
	GMT_message (GMT, "\t   Append n to add 90 degrees to the values from c or o.\n");
	GMT_message (GMT, "\t-E Compute Lambertian radiance appropriate to use with grdimage/grdview.\n");
	GMT_message (GMT, "\t   -E<azim/elev> sets azimuth and elevation of light vector.\n");
	GMT_message (GMT, "\t   -E<azim/elev/ambient/diffuse/specular/shine> sets azim, elev and\n");
	GMT_message (GMT, "\t    other parameters that control the reflectance properties of the surface.\n");
	GMT_message (GMT, "\t    Default values are: 0.55/0.6/0.4/10.\n");
	GMT_message (GMT, "\t    Specify '=' to get the default value (e.g. -E60/30/=/0.5).\n");
	GMT_message (GMT, "\t   Append s to use a simpler Lambertian algorithm (note that with this form\n");
	GMT_message (GMT, "\t   you only have to provide the azimuth and elevation parameters).\n");
	GMT_message (GMT, "\t   Append p to use the Peucker piecewise linear approximation (simpler but faster algorithm).\n");
	GMT_message (GMT, "\t   Note that in this case the azimuth and elevation are hardwired to 315 and 45 degrees.\n");
	GMT_message (GMT, "\t   This means that even if you provide other values they will be ignored.\n");
	GMT_message (GMT, "\t-G Output file for results from -A or -D.\n");
	GMT_message (GMT, "\t-N Will normalize gradients so that max |grad| = <amp> [1.0].\n");
	GMT_message (GMT, "\t  -Nt will make atan transform, then scale to <amp> [1.0].\n");
	GMT_message (GMT, "\t  -Ne will make exp  transform, then scale to <amp> [1.0].\n");
	GMT_message (GMT, "\t  -Nt<amp>/<sigma>[/<offset>] or -Ne<amp>/<sigma>[/<offset>] sets sigma\n");
	GMT_message (GMT, "\t     (and offset) for transform. [sigma, offset estimated from data].\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-S Output file for |grad z|; requires -D.\n");
	GMT_explain_options (GMT, "Vfn.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdgradient_parse (struct GMTAPI_CTRL *C, struct GRDGRADIENT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdgradient and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0, j, entry, pos, n_opt_args = 0;
	char ptr[BUFSIZ];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Set azimuth */
				Ctrl->A.active = TRUE;
				j = sscanf(opt->arg, "%lf/%lf", &Ctrl->A.azimuth[0], &Ctrl->A.azimuth[1]);
				Ctrl->A.two = (j == 2);
				break;
			case 'D':	/* Find direction of grad|z| */
				Ctrl->D.active = TRUE;
				j = 0;
				while (opt->arg[j]) {
					switch (opt->arg[j]) {
						case 'C': case 'c': Ctrl->D.mode |= 1; break;
						case 'O': case 'o': Ctrl->D.mode |= 2; break;
						case 'N': case 'n': Ctrl->D.mode |= 4; break;
						default:
							GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -D option: Unrecognized modifier\n");
							n_errors++;
							break;
					}
					j++;
				}
				break;
			case 'E':	/* Lambertian family radiance */
				Ctrl->E.active = TRUE;
				switch (opt->arg[0]) {
					case 'p':	/* Peucker */
						Ctrl->E.mode = 1;
						break;
					case 's':	/* "simple" Lambertian case */
						Ctrl->E.mode = 2;
						n_errors += GMT_check_condition (GMT, sscanf(&opt->arg[1], "%lf/%lf", &Ctrl->E.azimuth, &Ctrl->E.elevation) != 2, "Syntax error -Es option: Must append azimuth/elevation\n");
						break;
					default:
						Ctrl->E.mode = 3;	/* "full" Lambertian case */
						n_errors += GMT_check_condition (GMT, sscanf(opt->arg, "%lf/%lf", &Ctrl->E.azimuth, &Ctrl->E.elevation) < 2, "Syntax error -E option: Must give at least azimuth and elevation\n");
						entry = pos = 0;
						while (entry < 6 && (GMT_strtok (opt->arg, "/", &pos, ptr))) {
							switch (entry) {
								case 2:
									if (ptr[0] != '=') Ctrl->E.ambient = atof (ptr);
									break;
								case 3:
									if (ptr[0] != '=') Ctrl->E.diffuse = atof (ptr);
									break;
								case 4:
									if (ptr[0] != '=') Ctrl->E.specular = atof (ptr);
									break;
								case 5:
									if (ptr[0] != '=') Ctrl->E.shine = atof (ptr);
									break;
							}
							entry++;
						}
						break;
				}
				break;
			case 'G':	/* Output grid */
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
#ifdef GMT_COMPAT
			case 'L':	/* BCs */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -L is deprecated; -n+b%s was set instead, use this in the future.\n", opt->arg);
				strncpy (GMT->common.n.BC, opt->arg, (size_t)4);
				/* We turn on geographic coordinates if -Lg is given by faking -fg */
				/* But since GMT_parse_f_option is private to gmt_init and all it does */
				/* in this case are 2 lines bellow we code it here */
				if (!strcmp (GMT->common.n.BC, "g")) {
					GMT->current.io.col_type[GMT_IN][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
					GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
				}
				break;
#endif

#ifdef GMT_COMPAT
			case 'M':	/* Geographic data */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -M is deprecated; -fg was set instead, use this in the future.\n");
				if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
#endif
				break;
			case 'N':	/* Normalization */
				Ctrl->N.active = TRUE;
				j = 0;
				if (opt->arg[j]) {
					if (opt->arg[j] == 't' || opt->arg[j] == 'T') {
						Ctrl->N.mode = 1;
						j++;
					}
					else if (opt->arg[j] == 'e' || opt->arg[j] == 'E') {
						Ctrl->N.mode = 2;
						j++;
					}
					n_opt_args = sscanf(&opt->arg[j], "%lf/%lf/%lf", &Ctrl->N.norm, &Ctrl->N.sigma, &Ctrl->N.offset);
				}
				break;
			case 'S':	/* Slope grid */
				Ctrl->S.active = TRUE;
				Ctrl->S.file = strdup (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !(Ctrl->A.active || Ctrl->D.active || Ctrl->E.active), "Syntax error: Must specify -A, -D, or -E\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && !Ctrl->S.file, "Syntax error -S option: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file && !Ctrl->S.active, "Syntax error -G option: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && (n_opt_args && Ctrl->N.norm <= 0.0), "Syntax error -N option: Normalization amplitude must be > 0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && (n_opt_args > 1 && Ctrl->N.sigma <= 0.0) , "Syntax error -N option: Sigma must be > 0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && Ctrl->E.mode > 1 && (Ctrl->E.elevation < 0.0 || Ctrl->E.elevation > 90.0), "Syntax error -E option: Use 0-90 degree range for elevation\n");
	if (Ctrl->E.active && (Ctrl->A.active || Ctrl->D.active || Ctrl->S.active)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: -E option overrides -A, -D or -S\n");
		Ctrl->A.active = Ctrl->D.active = Ctrl->S.active = FALSE;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdgradient_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdgradient (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, sigma_set = FALSE, offset_set = FALSE, bad;
	GMT_LONG p[4], row, col, ij, ij0, index, n, n_used = 0, new_grid = FALSE;
	
	char format[BUFSIZ];
	
	double dx_grid, dy_grid, x_factor, y_factor, dzdx, dzdy, ave_gradient, wesn[4];
	double azim, denom, max_gradient = 0.0, min_gradient = 0.0, rpi, lat, output;
	double x_factor2 = 0.0, y_factor2 = 0.0, dzdx2 = 0.0, dzdy2 = 0.0, dzds1, dzds2;
	double p0 = 0.0, q0 = 0.0, p0q0_cte = 1.0, norm_z, mag, s[3], lim_x, lim_y, lim_z;
	double k_ads = 0.0, diffuse, spec, r_min = DBL_MAX, r_max = -DBL_MAX, scale;
	
	struct GMT_GRID *Surf = NULL, *Slope = NULL, *Out = NULL;
	struct GRDGRADIENT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdgradient_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdgradient_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdgradient", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRf:", "", options))) Return (error);
	Ctrl = (struct GRDGRADIENT_CTRL *) New_grdgradient_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdgradient_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdgradient main code ----------------------------*/

	GMT_memset (s, 3, double);

	if (Ctrl->N.active && Ctrl->N.sigma  != 0.0) sigma_set  = TRUE;
	if (Ctrl->N.active && Ctrl->N.offset != 0.0) offset_set = TRUE;
	if (Ctrl->A.active) {	/* Get azimuth in 0-360 range */
		while (Ctrl->A.azimuth[0] < 0.0) Ctrl->A.azimuth[0] += 360.0;
		while (Ctrl->A.azimuth[0] > 360.0) Ctrl->A.azimuth[0] -= 360.0;
		if (Ctrl->A.two) {
			while (Ctrl->A.azimuth[1] < 0.0) Ctrl->A.azimuth[1] += 360.0;
			while (Ctrl->A.azimuth[1] > 360.0) Ctrl->A.azimuth[1] -= 360.0;
		}
	}
	if (Ctrl->E.active) {	/* Get azimuth in 0-360 range */
		while (Ctrl->E.azimuth < 0.0) Ctrl->E.azimuth += 360.0;
		while (Ctrl->E.azimuth > 360.0) Ctrl->E.azimuth -= 360.0;
	}
	if (Ctrl->E.mode == 2) {	/* Precalculate constants */
		p0 = cosd (90.0 - Ctrl->E.azimuth) * tand (90.0 - Ctrl->E.elevation);
		q0 = sind (90.0 - Ctrl->E.azimuth) * tand (90.0 - Ctrl->E.elevation);
		p0q0_cte = sqrt (1.0 + p0*p0 + q0*q0);
	}
	if (Ctrl->E.mode == 3) {	/* Precalculate constants */
		Ctrl->E.elevation = 90 - Ctrl->E.elevation;
		s[0] = sind (Ctrl->E.azimuth) * cosd (Ctrl->E.elevation);
		s[1] = cosd (Ctrl->E.azimuth) * cosd (Ctrl->E.elevation);
		s[2] = sind (Ctrl->E.elevation);
		k_ads = Ctrl->E.ambient + Ctrl->E.diffuse + Ctrl->E.specular;
	}
	
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */

	GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */

	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file), (void **)&Surf)) Return (GMT_DATA_READ_ERROR);
	if (GMT_is_subset (Surf->header, wesn)) GMT_err_fail (GMT, GMT_adjust_loose_wesn (GMT, wesn, Surf->header), "");	/* Subset requested; make sure wesn matches header spacing */
	GMT_grd_init (GMT, Surf->header, options, TRUE);

	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->In.file), (void **)&Surf)) Return (GMT_DATA_READ_ERROR);	/* Get subset */

	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (Ctrl->S.active) {	/* Want slope grid */
		Slope = GMT_create_grid (GMT);
		GMT_memcpy (Slope->header, Surf->header, 1, struct GRD_HEADER);
		Slope->data = GMT_memory (GMT, NULL, Surf->header->size, float);
	}
	new_grid = GMT_set_outgrid (GMT, Surf, &Out);	/* TRUE if input is a read-only array */
	
	if (GMT_is_geographic (GMT, GMT_IN)) {
		dx_grid = GMT->current.proj.DIST_M_PR_DEG * Surf->header->inc[GMT_X] * cosd ((Surf->header->wesn[YHI] + Surf->header->wesn[YLO]) / 2.0);
		dy_grid = GMT->current.proj.DIST_M_PR_DEG * Surf->header->inc[GMT_Y];
	}
	else {
		dx_grid = Surf->header->inc[GMT_X];
		dy_grid = Surf->header->inc[GMT_Y];
	}
	x_factor = -1.0 / (2.0 * dx_grid);
	y_factor = -1.0 / (2.0 * dy_grid);
	if (Ctrl->A.active) {	/* Convert azimuths to radians to save multiplication later */
		if (Ctrl->A.two) {
			Ctrl->A.azimuth[1] *= D2R;
			x_factor2 = x_factor * sin (Ctrl->A.azimuth[1]);
			y_factor2 = y_factor * cos (Ctrl->A.azimuth[1]);
		}
		Ctrl->A.azimuth[0] *= D2R;
		x_factor *= sin (Ctrl->A.azimuth[0]);
		y_factor *= cos (Ctrl->A.azimuth[0]);
	}

	/* Index offset of 4-star points relative to current node */
	p[0] = 1;	p[1] = -1;	p[2] = Surf->header->mx;	p[3] = -Surf->header->mx;

	min_gradient = DBL_MAX;	max_gradient = -DBL_MAX;	ave_gradient = 0.0;
	if (Ctrl->E.mode == 3) {
		lim_x = Surf->header->wesn[XHI] - Surf->header->wesn[XLO];
		lim_y = Surf->header->wesn[YHI] - Surf->header->wesn[YLO];
		lim_z = Surf->header->z_max - Surf->header->z_min;
		scale = MAX (lim_z, MAX (lim_x, lim_y));
		lim_x /= scale;	lim_y /= scale;		lim_z /= scale;
		dx_grid /= lim_x;	dy_grid /= lim_y;
		x_factor = -dy_grid / (2.0 * lim_z);	y_factor = -dx_grid / (2.0 * lim_z);
	}
	for (row = ij0 = 0; row < Surf->header->ny; row++) {	/* ij0 is the index in a non-padded grid */
		if (GMT_is_geographic (GMT, GMT_IN)) {	/* Evaluate latitude-dependent factors */
			lat = GMT_grd_row_to_y (row, Surf->header);
			dx_grid = GMT->current.proj.DIST_M_PR_DEG * Surf->header->inc[GMT_X] * cosd (lat);
			if (dx_grid > 0.0) x_factor = -1.0 / (2.0 * dx_grid);	/* Use previous value at the poles */
			if (Ctrl->A.active) {
				if (Ctrl->A.two) x_factor2 = x_factor * sin (Ctrl->A.azimuth[1]);
				x_factor *= sin (Ctrl->A.azimuth[0]);
			}
		}
		for (col = 0; col < Surf->header->nx; col++, ij0++) {
			ij = GMT_IJP (Surf->header, row, col);	/* Index into padded grid */
			for (n = 0, bad = FALSE; !bad && n < 4; n++) if (GMT_is_fnan (Surf->data[ij+p[n]])) bad = TRUE;
			if (bad) {	/* One of star corners = NaN; assign NaN answers and skip to next node */
				index = (new_grid) ? ij : ij0;
				Out->data[index] = GMT->session.f_NaN;
				if (Ctrl->S.active) Slope->data[ij] = GMT->session.f_NaN;
				continue;
			}

			/* We can now evalute the central finite differences */
			dzdx = (Surf->data[ij+1] - Surf->data[ij-1]) * x_factor;
			dzdy = (Surf->data[ij-Surf->header->mx] - Surf->data[ij+Surf->header->mx]) * y_factor;
			if (Ctrl->A.two) {
				dzdx2 = (Surf->data[ij+1] - Surf->data[ij-1]) * x_factor2;
				dzdy2 = (Surf->data[ij-Surf->header->mx] - Surf->data[ij+Surf->header->mx]) * y_factor2;
			}

			/* Write output to unused NW corner */

			if (Ctrl->A.active) {	/* Directional derivatives requested */
				if (Ctrl->A.two) {
					dzds1 = dzdx + dzdy;
					dzds2 = dzdx2 + dzdy2;
					output = (fabs (dzds1) > fabs (dzds2)) ? dzds1 : dzds2;
				}
				else
					output = dzdx + dzdy;
				ave_gradient += output;
				min_gradient = MIN (min_gradient, output);
				max_gradient = MAX (max_gradient, output);
			}
			else if (Ctrl->D.active) {
				azim = (Ctrl->D.mode & 1) ? atan2d (-dzdy, -dzdx) : 90.0 - atan2d (-dzdy, -dzdx);
				if (Ctrl->D.mode & 4) azim += 90.0;
				if (azim < 0.0) azim += 360.0;
				if (azim >= 360.0) azim -= 360.0;
				if (Ctrl->D.mode & 2 && azim >= 180) azim -= 180.0;
				output = azim;
				if (Ctrl->S.active) Slope->data[ij] = (float)hypot (dzdx, dzdy);
			}
			else {	/* Ctrl->E.active */
				if (Ctrl->E.mode == 3) {
					norm_z = dx_grid * dy_grid;
					mag = d_sqrt (dzdx * dzdx + dzdy * dzdy + norm_z * norm_z);
					dzdx /= mag;	dzdy /= mag;	norm_z /= mag;
					diffuse = MAX (0, s[0] * dzdx + s[1] * dzdy + s[2] * norm_z); 
					spec = specular (dzdx, dzdy, norm_z, s);
					spec = pow (spec, Ctrl->E.shine);
					output = (Ctrl->E.ambient + Ctrl->E.diffuse * diffuse + Ctrl->E.specular * spec) / k_ads;
				}
				else if (Ctrl->E.mode == 2)
					output = (1.0 + p0 * dzdx + q0 * dzdy) / (sqrt (1.0 + dzdx * dzdx + dzdy * dzdy) * p0q0_cte);
				else	/* Peucker method */
					output = -0.4285 * (dzdx - dzdy) - 0.0844 * fabs (dzdx  + dzdy) + 0.6599;
				r_min = MIN (r_min, output);
				r_max = MAX (r_max, output);
			}
			index = (new_grid) ? ij : ij0;
			Out->data[index] = (float)output;
			n_used++;
		}
	}

	if (!new_grid)	{	/* We got away with using the input grid by ignoring the pad.  Now we must put the pad back in */
		GMT_memset (Out->header->pad, 4, GMT_LONG);	/* Must set pad to zero first otherwise we cannot add the pad in */
		Out->header->mx = Out->header->nx;	Out->header->my = Out->header->ny;	/* Since there is no pad */
		GMT_grd_pad_on (GMT, Out, GMT->current.io.pad);	/* Now reinstate the pad */
	}

	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Data is geographic */
		double sum;
		/* If the N or S poles are included then we only want a single estimate at these repeating points */
		if (Out->header->wesn[YLO] == -90.0 && Out->header->registration == GMT_GRIDLINE_REG) {	/* Average all the multiple N pole estimates */
			for (col = 0, ij = GMT_IJP (Out->header, 0, 0), sum = 0.0; col < Out->header->nx; col++, ij++) sum += Out->data[ij];
			sum /= Out->header->nx;	/* Average gradient */
			for (col = 0, ij = GMT_IJP (Out->header, 0, 0); col < Out->header->nx; col++, ij++) Out->data[ij] = (float)sum;
		}
		if (Out->header->wesn[YLO] == -90.0 && Out->header->registration == GMT_GRIDLINE_REG) {	/* Average all the multiple S pole estimates */
			for (col = 0, ij = GMT_IJP (Out->header, Out->header->ny - 1, 0), sum = 0.0; col < Out->header->nx; col++, ij++) sum += Out->data[ij];
			sum /= Out->header->nx;	/* Average gradient */
			for (col = 0, ij = GMT_IJP (Out->header, Out->header->ny - 1, 0); col < Out->header->nx; col++, ij++) Out->data[ij] = (float)sum;
		}
	}
	
	if (Ctrl->E.active) {	/* data must be scaled to the [-1,1] interval, but we'll do it into [-.95, .95] to not get too bright */
		scale = 1.0 / (r_max - r_min);
		GMT_grd_loop (Out, row, col, ij) {
			if (GMT_is_fnan (Out->data[ij])) continue;
			Out->data[ij] = (float)((-1.0 + 2.0 * ((Out->data[ij] - r_min) * scale)) * 0.95);
		}
	}

	if (offset_set)
		ave_gradient = Ctrl->N.offset;
	else
		ave_gradient /= n_used;

	if (Ctrl->A.active) {	/* Report some statistics */

		if (Ctrl->N.active) {
			if (Ctrl->N.mode == 1) {
				if (sigma_set)
					denom = 1.0 / Ctrl->N.sigma;
				else {
					denom = 0.0;
					GMT_grd_loop (Out, row, col, ij) {
						if (!GMT_is_fnan (Out->data[ij])) denom += pow (Out->data[ij] - ave_gradient, 2.0);
					}
					denom = sqrt ((n_used - 1) / denom);
					Ctrl->N.sigma = 1.0 / denom;
				}
				rpi = 2.0 * Ctrl->N.norm / M_PI;
				GMT_grd_loop (Out, row, col, ij) {
					if (!GMT_is_fnan (Out->data[ij])) Out->data[ij] = (float)(rpi * atan ((Out->data[ij] - ave_gradient) * denom));
				}
				Out->header->z_max = rpi * atan ((max_gradient - ave_gradient) * denom);
				Out->header->z_min = rpi * atan ((min_gradient - ave_gradient) * denom);
			}
			else if (Ctrl->N.mode == 2) {
				if (!sigma_set) {
					Ctrl->N.sigma = 0.0;
					GMT_grd_loop (Out, row, col, ij) {
						if (!GMT_is_fnan (Out->data[ij])) Ctrl->N.sigma += fabs((double)Out->data[ij]);
					}
					Ctrl->N.sigma = M_SQRT2 * Ctrl->N.sigma / n_used;
				}
				denom = M_SQRT2 / Ctrl->N.sigma;
				GMT_grd_loop (Out, row, col, ij) {
					if (GMT_is_fnan (Out->data[ij])) continue;
					if (Out->data[ij] < ave_gradient) {
						Out->data[ij] = (float)(-Ctrl->N.norm * (1.0 - exp ( (Out->data[ij] - ave_gradient) * denom)));
					}
					else {
						Out->data[ij] = (float)( Ctrl->N.norm * (1.0 - exp (-(Out->data[ij] - ave_gradient) * denom)));
					}
				}
				Out->header->z_max =  Ctrl->N.norm * (1.0 - exp (-(max_gradient - ave_gradient) * denom));
				Out->header->z_min = -Ctrl->N.norm * (1.0 - exp ( (min_gradient - ave_gradient) * denom));
			}
                	else {
				if ((max_gradient - ave_gradient) > (ave_gradient - min_gradient))
					denom = Ctrl->N.norm / (max_gradient - ave_gradient);
				else
					denom = Ctrl->N.norm / (ave_gradient - min_gradient);
				GMT_grd_loop (Out, row, col, ij) {
					if (!GMT_is_fnan (Out->data[ij])) Out->data[ij] = (float)((Out->data[ij] - ave_gradient) * denom);
				}
				Out->header->z_max = (max_gradient - ave_gradient) * denom;
				Out->header->z_min = (min_gradient - ave_gradient) * denom;
			}
		}
	}

	/* Now we write out: */

	if (Ctrl->A.active) {
		if (Ctrl->N.active)
			strcpy (Out->header->title, "Normalized directional derivative(s)");
		else
			strcpy (Out->header->title, "Directional derivative(s)");
		sprintf (format, "\t%s\t%s\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, " Min Mean Max sigma intensities:");
		GMT_report (GMT, GMT_MSG_NORMAL, format, min_gradient, ave_gradient, max_gradient, Ctrl->N.sigma);
	}
	else {
		if (Ctrl->E.mode > 1)
			strcpy (Out->header->title, "Lambertian radiance");
		else if (Ctrl->E.mode == 1)
			strcpy (Out->header->title, "Peucker piecewise linear radiance");
		else
			strcpy (Out->header->title, "Directions of maximum slopes");
	}

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);		/* Enables data output and sets access mode */
	if (Ctrl->G.active) GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->G.file, (void *)Out);

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Surf);
	if (new_grid) GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Out);

	if (Ctrl->S.active) {
		strcpy (Slope->header->title, "Magnitude of maximum slopes");
		GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->S.file, (void *)Slope);
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Slope);
	}
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */

	Return (EXIT_SUCCESS);
}
