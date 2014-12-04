/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: grdgradient read a grid file and compute gradient in azim direction:
 * azim = azimuth clockwise from north in degrees.
 * gradient = -[(dz/dx)sin(azim) + (dz/dy)cos(azim)].
 * the expression in [] is the correct gradient.  We take
 * -[] in order that data which goes DOWNHILL in the
 * azim direction will give a positive value; this is
 * for image shading purposes.  Note for -D we instead use
 * the common convention of reporting the UPHILL direction.
 *
 * Author:	W.H.F. Smith
 * Date: 	1-JAN-2010
 * Version:	5 API
 */
 
#define THIS_MODULE_NAME	"grdgradient"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Compute directional gradients from a grid"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-RVfn"

struct GRDGRADIENT_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct A {	/* -A<azim>[/<azim2>] */
		bool active;
		bool two;
		double azimuth[2];
	} A;
	struct D {	/* -D[a][c][o][n] */
		bool active;
		unsigned int mode;
	} D;
	struct E {	/* -E[s|p]<azim>/<elev[ambient/diffuse/specular/shine]> */
		bool active;
		unsigned int mode;
		double azimuth, elevation;
		double ambient, diffuse, specular, shine;
	} E;
	struct G {	/* -G<file> */
		bool active;
		char *file;
	} G;
	struct N {	/* -N[t_or_e][<amp>[/<sigma>[/<offset>]]] */
		bool active;
		unsigned int mode;	/* 1 = atan, 2 = exp */
		double norm, sigma, offset;
	} N;
	struct S {	/* -S<slopefile> */
		bool active;
		char *file;
	} S;
};

void *New_grdgradient_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDGRADIENT_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDGRADIENT_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->E.ambient = 0.55;
	C->E.diffuse = 0.6;
	C->E.specular = 0.4;
	C->E.shine = 10;
	C->N.norm = 1.0;		
	return (C);
}

void Free_grdgradient_Ctrl (struct GMT_CTRL *GMT, struct GRDGRADIENT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->G.file) free (C->G.file);	
	if (C->S.file) free (C->S.file);	
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

int GMT_grdgradient_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdgradient <ingrid> -G<outgrid> [-A<azim>[/<azim2>]] [-D[a][c][o][n]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-E[s|p|m]<azim>/<elev>[/<ambient>/<diffuse>/<specular>/<shine>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-N[t|e][<amp>[/<sigma>[/<offset>]]]] [%s]\n\t[-S<slopegrid>] [%s] [-fg] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_n_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is name of input grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Set azimuth (0-360 CW from North (+y)) for directional derivatives.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  -A<azim>/<azim2> will compute two directions and save the one larger in magnitude.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Find the direction of the vector grad z (up-slope direction).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append a to get the aspect instead (down-slope direction).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append c to report Cartesian angle (0-360 CCW from East (+x-axis)) [Default: azimuth].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append o to get bidirectional orientations [0-180] rather than directions [0-360].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append n to add 90 degrees to the values from c or o.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Compute Lambertian radiance appropriate to use with grdimage/grdview.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -E<azim/elev> sets azimuth and elevation of light vector.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -E<azim>/<elev>/<ambient>/<diffuse>/<specular>/<shine> sets azim, elev and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    other parameters that control the reflectance properties of the surface\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    [Default values are: 0.55/0.6/0.4/10].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Specify '=' to get the default value (e.g., -E60/30/=/0.5).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append s to use a simpler Lambertian algorithm (note that with this form\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   you only have to provide the azimuth and elevation parameters).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append p to use the Peucker piecewise linear approximation (simpler but faster algorithm).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append m to use another algorithm that gives results close to ESRI's 'hillshade' but faster\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note that in this case the azimuth and elevation are hardwired to 315 and 45 degrees.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This means that even if you provide other values they will be ignored.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Output file for results from -A or -D.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Normalize gradients so that max |grad| = <amp> [1.0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  -Nt will make atan transform, then scale to <amp> [1.0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  -Ne will make exp  transform, then scale to <amp> [1.0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  -Nt<amp>/<sigma>[/<offset>] or -Ne<amp>/<sigma>[/<offset>] sets sigma\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     (and offset) for transform. [sigma, offset estimated from data].\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Output file for |grad z|; requires -D.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Convert geographic grids to meters using a \"Flat Earth\" approximation.\n");
	GMT_Option (API, "n,.");
	
	return (EXIT_FAILURE);
}

int GMT_grdgradient_parse (struct GMT_CTRL *GMT, struct GRDGRADIENT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdgradient and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, j, entry, pos;
	int n_opt_args = 0;
	char ptr[GMT_BUFSIZ];
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Set azimuth */
				Ctrl->A.active = true;
				j = sscanf(opt->arg, "%lf/%lf", &Ctrl->A.azimuth[0], &Ctrl->A.azimuth[1]);
				Ctrl->A.two = (j == 2);
				break;
			case 'D':	/* Find direction of grad|z| */
				Ctrl->D.active = true;
				j = 0;
				while (opt->arg[j]) {
					switch (opt->arg[j]) {
						case 'c': Ctrl->D.mode |= 1; break;
						case 'o': Ctrl->D.mode |= 2; break;
						case 'n': Ctrl->D.mode |= 4; break;
						case 'a': Ctrl->D.mode |= 8; break;
						default:
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -D option: Unrecognized modifier\n");
							n_errors++;
							break;
					}
					j++;
				}
				break;
			case 'E':	/* Lambertian family radiance */
				Ctrl->E.active = true;
				switch (opt->arg[0]) {
					case 'p':	/* Peucker */
						Ctrl->E.mode = 1;
						break;
					case 's':	/* "simple" Lambertian case */
						Ctrl->E.mode = 2;						
						n_errors += GMT_check_condition (GMT, sscanf(&opt->arg[1], "%lf/%lf", &Ctrl->E.azimuth, &Ctrl->E.elevation) != 2, "Syntax error -Es option: Must append azimuth/elevation\n");
						break;
					case 'm':	/* Nice algorithm from an old program called manipRaster by Tierry Souriot */
						Ctrl->E.mode = 4;
						j = sscanf(&opt->arg[1], "%lf/%lf", &Ctrl->E.azimuth, &Ctrl->E.elevation);
						if (j == 0) {				/* Use default values */
							Ctrl->E.azimuth = 360 - 45;
							Ctrl->E.elevation = 45;
						}
						else if (j == 1)
							Ctrl->E.elevation = 45;
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
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'L':	/* GMT4 BCs */
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -L is deprecated; -n+b%s was set instead, use this in the future.\n", opt->arg);
					strncpy (GMT->common.n.BC, opt->arg, 4U);
					/* We turn on geographic coordinates if -Lg is given by faking -fg */
					/* But since GMT_parse_f_option is private to gmt_init and all it does */
					/* in this case are 2 lines below we code it here */
					if (!strcmp (GMT->common.n.BC, "g")) {
						GMT_set_geographic (GMT, GMT_IN);
						GMT_set_geographic (GMT, GMT_OUT);
					}
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;

			case 'M':	/* Geographic data */
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -M is deprecated; -fg was set instead, use this in the future.\n");
					if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'N':	/* Normalization */
				Ctrl->N.active = true;
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
				if ((Ctrl->S.active = GMT_check_filearg (GMT, 'S', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->S.file = strdup (opt->arg);
				else
					n_errors++;
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
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: -E option overrides -A, -D or -S\n");
		Ctrl->A.active = Ctrl->D.active = Ctrl->S.active = false;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdgradient_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdgradient (void *V_API, int mode, void *args)
{
	bool sigma_set = false, offset_set = false, bad, new_grid = false;
	int p[4], mx, error = 0;
	unsigned int row, col, n;
	uint64_t ij, ij0, index, n_used = 0;
	
	char format[GMT_BUFSIZ] = {""}, buffer[GMT_GRID_REMARK_LEN160] = {""};
	
	double dx_grid, dy_grid, x_factor, y_factor, dzdx, dzdy, ave_gradient, wesn[4];
	double azim, denom, max_gradient = 0.0, min_gradient = 0.0, rpi, lat, output, one;
	double x_factor2 = 0.0, y_factor2 = 0.0, dzdx2 = 0.0, dzdy2 = 0.0, dzds1, dzds2;
	double p0 = 0.0, q0 = 0.0, p0q0_cte = 1.0, norm_z, mag, s[3], lim_x, lim_y, lim_z;
	double k_ads = 0.0, diffuse, spec, r_min = DBL_MAX, r_max = -DBL_MAX, scale;
	
	struct GMT_GRID *Surf = NULL, *Slope = NULL, *Out = NULL;
	struct GRDGRADIENT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdgradient_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdgradient_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdgradient_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdgradient_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdgradient_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdgradient main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grid\n");
	GMT_memset (s, 3, double);
	GMT_set_pad (GMT, 2U);	/* Ensure space for BCs in case an API passed pad == 0 */
	
	if (Ctrl->N.active && Ctrl->N.sigma  != 0.0) sigma_set  = true;
	if (Ctrl->N.active && Ctrl->N.offset != 0.0) offset_set = true;
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
	else if (Ctrl->E.mode == 3 || Ctrl->E.mode == 4) {	/* Precalculate constants */
		if (Ctrl->E.mode == 3) {
			Ctrl->E.elevation = 90 - Ctrl->E.elevation;
			k_ads = Ctrl->E.ambient + Ctrl->E.diffuse + Ctrl->E.specular;
		}
		s[0] = sind (Ctrl->E.azimuth) * cosd (Ctrl->E.elevation);
		s[1] = cosd (Ctrl->E.azimuth) * cosd (Ctrl->E.elevation);
		s[2] = sind (Ctrl->E.elevation);
	}

	GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */

	if ((Surf = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (GMT_is_subset (GMT, Surf->header, wesn)) GMT_err_fail (GMT, GMT_adjust_loose_wesn (GMT, wesn, Surf->header), "");	/* Subset requested; make sure wesn matches header spacing */
	GMT_grd_init (GMT, Surf->header, options, true);

	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, Ctrl->In.file, Surf) == NULL) {	/* Get subset */
		Return (API->error);
	}

	if (Ctrl->S.active) {	/* Want slope grid */
		if ((Slope = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, Surf)) == NULL) Return (API->error);
	}
	new_grid = GMT_set_outgrid (GMT, Ctrl->In.file, Surf, &Out);	/* true if input is a read-only array */
	
	if (GMT_is_geographic (GMT, GMT_IN) && !Ctrl->E.active) {	/* Flat-Earth approximation */
		dx_grid = GMT->current.proj.DIST_M_PR_DEG * Surf->header->inc[GMT_X] * cosd ((Surf->header->wesn[YHI] + Surf->header->wesn[YLO]) / 2.0);
		dy_grid = GMT->current.proj.DIST_M_PR_DEG * Surf->header->inc[GMT_Y];
	}
	else {	/* Cartesian */
		dx_grid = Surf->header->inc[GMT_X];
		dy_grid = Surf->header->inc[GMT_Y];
	}
	one = (Ctrl->D.active) ? +1.0 : -1.0;	/* With -D we want positive grad direction, not negative as for shading (-A, -E) */
	x_factor = one / (2.0 * dx_grid);
	y_factor = one / (2.0 * dy_grid);
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
	mx = Surf->header->mx;	/* Need a signed mx for p[3] in line below */
	p[0] = 1;	p[1] = -1;	p[2] = mx;	p[3] = -mx;

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
	for (row = 0, ij0 = 0ULL; row < Surf->header->ny; row++) {	/* ij0 is the index in a non-padded grid */
		if (GMT_is_geographic (GMT, GMT_IN) && !Ctrl->E.active) {	/* Evaluate latitude-dependent factors */
			lat = GMT_grd_row_to_y (GMT, row, Surf->header);
			dx_grid = GMT->current.proj.DIST_M_PR_DEG * Surf->header->inc[GMT_X] * cosd (lat);
			if (dx_grid > 0.0) x_factor = one / (2.0 * dx_grid);	/* Use previous value at the poles */
			if (Ctrl->A.active) {
				if (Ctrl->A.two) x_factor2 = x_factor * sin (Ctrl->A.azimuth[1]);
				x_factor *= sin (Ctrl->A.azimuth[0]);
			}
		}
		for (col = 0; col < Surf->header->nx; col++, ij0++) {
			ij = GMT_IJP (Surf->header, row, col);	/* Index into padded grid */
			for (n = 0, bad = false; !bad && n < 4; n++) if (GMT_is_fnan (Surf->data[ij+p[n]])) bad = true;
			if (bad) {	/* One of star corners = NaN; assign NaN answers and skip to next node */
				index = (new_grid) ? ij : ij0;
				Out->data[index] = GMT->session.f_NaN;
				if (Ctrl->S.active) Slope->data[ij] = GMT->session.f_NaN;
				continue;
			}

			/* We can now evaluate the central finite differences */
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
				if (dzdx == 0.0 && dzdy == 0.0)	/* Flat, so no preferred direction */
					azim = GMT->session.d_NaN;
				else {
					azim = (Ctrl->D.mode & 1) ? atan2d (dzdy, dzdx) : 90.0 - atan2d (dzdy, dzdx);
					if (Ctrl->D.mode & 4) azim += 90.0;
					if (Ctrl->D.mode & 8) azim += 180.0;
					if (azim < 0.0) azim += 360.0;
					if (azim >= 360.0) azim -= 360.0;
					if (Ctrl->D.mode & 2 && azim >= 180) azim -= 180.0;
				}
				output = azim;
				if (Ctrl->S.active) Slope->data[ij] = (float)hypot (dzdx, dzdy);
			}
			else {	/* Ctrl->E.active */
				if (Ctrl->E.mode == 2)
					output = (1.0 + p0 * dzdx + q0 * dzdy) / (sqrt (1.0 + dzdx * dzdx + dzdy * dzdy) * p0q0_cte);
				else if (Ctrl->E.mode == 3) {
					norm_z = dx_grid * dy_grid;
					mag = d_sqrt (dzdx * dzdx + dzdy * dzdy + norm_z * norm_z);
					dzdx /= mag;	dzdy /= mag;	norm_z /= mag;
					diffuse = MAX (0, s[0] * dzdx + s[1] * dzdy + s[2] * norm_z); 
					spec = specular (dzdx, dzdy, norm_z, s);
					spec = pow (spec, Ctrl->E.shine);
					output = (Ctrl->E.ambient + Ctrl->E.diffuse * diffuse + Ctrl->E.specular * spec) / k_ads;
				}
				else if (Ctrl->E.mode == 4)
					output = (dzdy*s[0] + dzdx*s[1] + 2*s[2]) / (sqrt(dzdy * dzdy + dzdx * dzdx + 4));
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
		GMT_memset (Out->header->pad, 4, int);	/* Must set pad to zero first otherwise we cannot add the pad in */
		Out->header->mx = Out->header->nx;	Out->header->my = Out->header->ny;	/* Since there is no pad */
		GMT_grd_pad_on (GMT, Out, GMT->current.io.pad);	/* Now reinstate the pad */
	}

	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Data is geographic */
		double sum;
		/* If the N or S poles are included then we only want a single estimate at these repeating points */
		if (Out->header->wesn[YLO] == -90.0 && Out->header->registration == GMT_GRID_NODE_REG) {	/* Average all the multiple N pole estimates */
			for (col = 0, ij = GMT_IJP (Out->header, 0, 0), sum = 0.0; col < Out->header->nx; col++, ij++) sum += Out->data[ij];
			sum /= Out->header->nx;	/* Average gradient */
			for (col = 0, ij = GMT_IJP (Out->header, 0, 0); col < Out->header->nx; col++, ij++) Out->data[ij] = (float)sum;
		}
		if (Out->header->wesn[YLO] == -90.0 && Out->header->registration == GMT_GRID_NODE_REG) {	/* Average all the multiple S pole estimates */
			for (col = 0, ij = GMT_IJP (Out->header, Out->header->ny - 1, 0), sum = 0.0; col < Out->header->nx; col++, ij++) sum += Out->data[ij];
			sum /= Out->header->nx;	/* Average gradient */
			for (col = 0, ij = GMT_IJP (Out->header, Out->header->ny - 1, 0); col < Out->header->nx; col++, ij++) Out->data[ij] = (float)sum;
		}
	}
	
	if (Ctrl->E.active) {	/* data must be scaled to the [-1,1] interval, but we'll do it into [-.95, .95] to not get too bright */
		scale = 1.0 / (r_max - r_min);
		GMT_grd_loop (GMT, Out, row, col, ij) {
			if (GMT_is_fnan (Out->data[ij])) continue;
			Out->data[ij] = (float)((-1.0 + 2.0 * ((Out->data[ij] - r_min) * scale)) * 0.95);
		}
	}

	if (offset_set)
		ave_gradient = Ctrl->N.offset;
	else
		ave_gradient /= n_used;

	if (Ctrl->A.active) {	/* Report some statistics */

		if (Ctrl->N.active) {	/* Chose normalization */
			if (Ctrl->N.mode == 1) {	/* atan transformation */
				if (sigma_set)
					denom = 1.0 / Ctrl->N.sigma;
				else {
					denom = 0.0;
					GMT_grd_loop (GMT, Out, row, col, ij) {
						if (!GMT_is_fnan (Out->data[ij])) denom += pow (Out->data[ij] - ave_gradient, 2.0);
					}
					denom = sqrt ((n_used - 1) / denom);
					Ctrl->N.sigma = 1.0 / denom;
				}
				rpi = 2.0 * Ctrl->N.norm / M_PI;
				GMT_grd_loop (GMT, Out, row, col, ij) {
					if (!GMT_is_fnan (Out->data[ij])) Out->data[ij] = (float)(rpi * atan ((Out->data[ij] - ave_gradient) * denom));
				}
				Out->header->z_max = rpi * atan ((max_gradient - ave_gradient) * denom);
				Out->header->z_min = rpi * atan ((min_gradient - ave_gradient) * denom);
			}
			else if (Ctrl->N.mode == 2) {	/* Exp transformation */
				if (!sigma_set) {
					Ctrl->N.sigma = 0.0;
					GMT_grd_loop (GMT, Out, row, col, ij) {
						if (!GMT_is_fnan (Out->data[ij])) Ctrl->N.sigma += fabsf (Out->data[ij]);
					}
					Ctrl->N.sigma = M_SQRT2 * Ctrl->N.sigma / n_used;
				}
				denom = M_SQRT2 / Ctrl->N.sigma;
				GMT_grd_loop (GMT, Out, row, col, ij) {
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
			else {	/* Linear transformation */
				if ((max_gradient - ave_gradient) > (ave_gradient - min_gradient))
					denom = Ctrl->N.norm / (max_gradient - ave_gradient);
				else
					denom = Ctrl->N.norm / (ave_gradient - min_gradient);
				GMT_grd_loop (GMT, Out, row, col, ij) {
					if (!GMT_is_fnan (Out->data[ij])) Out->data[ij] = (float)((Out->data[ij] - ave_gradient) * denom);
				}
				Out->header->z_max = (max_gradient - ave_gradient) * denom;
				Out->header->z_min = (min_gradient - ave_gradient) * denom;
			}
		}
	}

	GMT_set_pad (GMT, API->pad);	/* Reset to session default pad before output */

	/* Now we write out: */

	if (Ctrl->A.active) {
		if (Ctrl->N.active)
			strcpy (buffer, "Normalized directional derivative(s)");
		else
			strcpy (buffer, "Directional derivative(s)");
		sprintf (format, "\t%s\t%s\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, " Min Mean Max sigma intensities:");
		GMT_Report (API, GMT_MSG_VERBOSE, format, min_gradient, ave_gradient, max_gradient, Ctrl->N.sigma);
	}
	else {
		if (Ctrl->E.mode > 1)
			strcpy (buffer, "Lambertian radiance");
		else if (Ctrl->E.mode == 1)
			strcpy (buffer, "Peucker piecewise linear radiance");
		else
			strcpy (buffer, "Directions of grad (z) [uphill direction]");
	}

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out)) Return (API->error);
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, buffer, Out)) Return (API->error);
	if (Ctrl->G.active && GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Out) != GMT_OK) {
		Return (API->error);
	}

	if (Ctrl->S.active) {
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Slope)) Return (API->error);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, "Magnitude of grad (z)", Slope)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->S.file, Slope) != GMT_OK) {
			Return (API->error);
		}
	}

	Return (EXIT_SUCCESS);
}
