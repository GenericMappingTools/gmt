/*--------------------------------------------------------------------
 *	$Id$
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
 * Author:      Paul Wessel
 * Date:        16-DEC-2013
 *
 *
 * Various isostatic calculations.
 *
 * */

#define THIS_MODULE_NAME	"grdflexure"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Compute flexural deformation of 3-D surfaces for various rheologies"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-Vf"
#define GMT_FFT_DIM	2	/* Dimension of FFT needed */

#define FLX_E	0
#define FLX_VE	1
#define FLX_FV1	2
#define FLX_FV2	3

struct GRDFLEXURE_CTRL {
	struct In {
		bool active, many;
		char *file;
	} In;
	struct C {	/* -Cy<E> or -Cp<poisson> */
		bool active[2];
		double E, nu;
	} C;
	struct D {	/* -D<rhom/rhol/rhoi/rhow> */
		bool active, approx;
		unsigned int mode;
		double rhom, rhol, rhoi, rhow;
	} D;
	struct E {	/* -E<te> */
		bool active;
		double te;
	} E;
	struct F {	/* -F<nu> or -F<nu_a>/<h_a>/<nu_m> */
		bool active;
		unsigned int mode;
		double nu_a, nu_m, h_a;
	} F;
	struct G {	/* -G<outfile> */
		bool active;
		char *file;
	} G;
	struct M {	/* -M<maxwell_t>  */
		bool active;
		double maxwell_t;	/* Maxwell time */
	} M;
	struct N {	/* -N[f|q|s<nx>/<ny>][+e|m|n][+t<width>][+w[<suffix>]][+z[p]]  */
		bool active;
		struct GMT_FFT_INFO *info;
	} N;
	struct T {	/* -T[l]<t0>[u]/<t1>[u]/<d0>[u]|n  */
		bool active, log;
		double start, end, inc;	/* Time ago, so start > end */
		double time;	/* The current time in a sequence */
		double scale;	/* Scale factor from user time to year */
		char unit;	/* Either M (Myr), k (kyr), or blank (y) */
	} T;
	struct W {
		bool active;
		double water_depth;	/* Reference water depth [0] */
	} W;
	struct Z {
		bool active;
		double zm;	/* Reference depth to flexed surface [0] */
	} Z;
};

struct RHEOLOGY {
	double time_yr;		/* Time in years since loading */
	double t0;		/* Time in seconds since loading */
	double nu_ratio;	/* Ratio of asthenosphere to lower mantle viscosities */
	double nu_ratio1;	/* The inverse ratio */
	double h_a;		/* The thickness of the asthenosphere (m) */
	double c;		/* A constant */
	double scale;		/* Overall scale (e.g., Airy scale) */
	double dens_ratio;	/* (Ctrl->D.rhom - Ctrl->D.rhoi) / Ctrl->D.rhom */
	double (*transfer) (double, struct RHEOLOGY *);	/* pointer to function returning isostatic response for given k and R */
	void (*setup) (struct GMT_CTRL *, struct GRDFLEXURE_CTRL *, struct GMT_FFT_WAVENUMBER *, struct RHEOLOGY *);	/* Init function */
};

#define	YOUNGS_MODULUS	7.0e10		/* Pascal = Nt/m**2  */
#define	NORMAL_GRAVITY	9.806199203	/* Moritz's 1980 IGF value for gravity at 45 degrees latitude (m/s) */
#define	POISSONS_RATIO	0.25

void *New_grdflexure_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDFLEXURE_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDFLEXURE_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.E = YOUNGS_MODULUS;
	C->C.nu = POISSONS_RATIO;

	return (C);
}

void Free_grdflexure_Ctrl (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->G.file) free (C->G.file);	
	if (C->N.info) GMT_free (GMT, C->N.info);
	GMT_free (GMT, C);	
}

double get_age (char *A, char *unit, double *scale)
{	/* Convert age[k|m] to years, return unit and scale as well */
	size_t k = strlen (A) - 1;
	*scale = 1.0;
	*unit = 0;
	switch (A[k]) {
		case 'k': *scale = 1.0e3; *unit = A[k]; A[k] = '\0'; break;
		case 'M': *scale = 1.0e6; *unit = A[k]; A[k] = '\0'; break;
	}
	return (atof (A) * (*scale));
}

double transfer_elastic (double k, struct RHEOLOGY *R)
{
	double k2, k4, transfer_fn;
	k2 = k * k;	k4 = k2 * k2;
	transfer_fn = R->scale / (R->c * k4 + 1.0);
	return (transfer_fn); 
}

void setup_elastic (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct RHEOLOGY *R) {
	/* Do the isostatic response function convolution in the Freq domain.
	All units assumed to be in SI (that is kx, ky, modk wavenumbers in m**-1,
	densities in kg/m**3, Te in m, etc.
	rw, the water density, is used to set the Airy ratio and the restoring
	force on the plate (rm - ri)*gravity if ri = rw; so use zero for topo in air (ri changed to rl).  */
	double  A = 1.0, rho_load, rigidity_d;

	/*   te	 Elastic thickness, SI units (m)  */
	/*   rl	 Load density, SI units  */
	/*   rm	 Mantle density, SI units  */
	/*   rw	 Water density, SI units  */
	/*   [ri Infill density, SI units]  */
	/* If infill density was specified then Ctrl->D.approx will be true, in which case we will do the
	 * approximate FFT solution of Wessel [2001, JGR]: Use rhoi instead of rhol to determine flexural wavelength
	 * and and amplitudes but scale airy_ratio by A to compensate for the lower load weight */
	
	rho_load = Ctrl->D.rhol;
	if (Ctrl->D.approx) {	/* Do approximate calculation when both rhol and rhoi were set */
		char way = (Ctrl->D.rhoi < Ctrl->D.rhol) ? '<' : '>';
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning: Approximate FFT-solution to flexure since rho_i (%g) %c rho_l (%g)\n", Ctrl->D.rhoi, way, Ctrl->D.rhol);
		rho_load = Ctrl->D.rhoi;
		A = sqrt ((Ctrl->D.rhom - Ctrl->D.rhoi)/(Ctrl->D.rhom - Ctrl->D.rhol));
	}
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Using effective load density rho_l = %g and Airy boost factor A = %g\n", rho_load, A);
	rigidity_d = (Ctrl->C.E * Ctrl->E.te * Ctrl->E.te * Ctrl->E.te) / (12.0 * (1.0 - Ctrl->C.nu * Ctrl->C.nu));
	R->c = rigidity_d / ( (Ctrl->D.rhom - rho_load) * NORMAL_GRAVITY);
	R->scale = -A * (rho_load - Ctrl->D.rhow)/(Ctrl->D.rhom - rho_load);
}

double relax_time_2 (double k, struct RHEOLOGY *R)
{
	/*  relax_time_2 evalues relaxation time(k) of 2-layer viscous mantle
	 *
	 *     k	= wavenumber in 1/m
	 *     R->rho_m	= Mantle density in kg/m^3
	 *     R->nu_a	= Asthenospheric viscosity in Pa s
	 *     R->h_a	= Asthenospheric thickness in m
	 *     R->nu_m	= Lower mantle viscosity in Pa s
	 */

	double lambda, S, C, CS, S2, C2, tau;
	lambda = k * R->h_a;
	S = sinh (lambda);
	C = cosh (lambda);
	CS = C * S;	S2 = S * S;	C2 = C * C;
	tau = R->c * (k * (2.0 * R->nu_ratio * CS + (1.0 - R->nu_ratio) * lambda * lambda + R->nu_ratio * S2 + C2))
		/ ((R->nu_ratio + R->nu_ratio1) * CS + (R->nu_ratio - R->nu_ratio1) * lambda + S2 + C2);
	return (tau);
}

void setup_fv2 (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct RHEOLOGY *R)
{
	R->t0 = R->time_yr * (86400*365.25);	/* Convert to seconds */
	R->nu_ratio = Ctrl->F.nu_a / Ctrl->F.nu_m;
	R->nu_ratio1 = 1.0 / R->nu_ratio;
	R->c = (2.0 * Ctrl->F.nu_m) / (Ctrl->D.rhom * NORMAL_GRAVITY);
	R->dens_ratio = (Ctrl->D.rhom - Ctrl->D.rhoi) / Ctrl->D.rhom;
}

double transfer_fv2 (double k, struct RHEOLOGY *R)
{
	double phi_e, phi_fv2, tau;
	phi_e = transfer_elastic (k, R);
	tau = relax_time_2 (k, R);
	phi_fv2 = phi_e * (1.0 - exp (-R->t0 * R->dens_ratio / (tau * phi_e)));
	return (phi_fv2); 
}

void setup_fv (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct RHEOLOGY *R)
{
	R->t0 = R->time_yr * (86400*365.25);	/* Convert to seconds */
	R->dens_ratio = (Ctrl->D.rhom - Ctrl->D.rhoi) / Ctrl->D.rhom;
	R->c = (2.0 * Ctrl->F.nu_m) / (Ctrl->D.rhom * NORMAL_GRAVITY);
}

double transfer_fv (double k, struct RHEOLOGY *R)
{
/*	Firmoviscous response function for elastic plate over
 *	viscous half-space.  Give:
 *
 *	k	- wavenumbers (1/m)
 *	rhom	- density of mantle (kg/m^3)
 *	rhoi	- density of infill material (kg/m^3)
 *	te	- elastic plate thickness (km)
 *	nu_m	- mantle viscosity (Pa s)
 *	t0	- time since loading (yr)
 */
	double phi_e, phi_fv, tau;
	phi_e = transfer_elastic (k, R);
	tau = k * R->c;
	phi_fv = phi_e * (1.0 - exp (-R->t0 * R->dens_ratio / (tau * phi_e)));
	return (phi_fv);
}

void setup_ve (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct RHEOLOGY *R)
{
	R->c = 1.0 / (Ctrl->M.maxwell_t * (86400*365.25));	/* Convert to seconds */
}

double transfer_ve (double k, struct RHEOLOGY *R)
{
/*	Viscoelastic response function for VE plate.  Give:
 *
 *	k	- wavenumbers (1/m)
 *	rhom	- density of mantle (kg/m^3)
 *	rhoi	- density of infill material (kg/m^3)
 *	te	- elastic plate thickness (km)
 *	T	- Maxwell time (Myt)
 *	t0	- time since loading (yr)
 */
	double phi_e, phi_ve, tau;
	tau = R->t0 * R->c;
	phi_e = transfer_elastic (k, R);
	phi_ve = 1.0 - (1.0 - phi_e) * exp (-tau * phi_e);
	return (phi_ve);
}

void spectral_solution (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct RHEOLOGY *R) {
	/* Do the spectral convolution for isostatic response in the Freq domain. */
	uint64_t k;
	double  mk, transfer_fn;
	
	R->setup (GMT, Ctrl, K, R);	/* Set up parameters */

	for (k = 0; k < Grid->header->size; k += 2) {
		mk = GMT_fft_get_wave (k, K);
		transfer_fn = R->transfer (mk, R);
		Grid->data[k] *= (float)transfer_fn;
		Grid->data[k+1] *= (float)transfer_fn;
	}
}

int GMT_grdflexure_parse (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_OPTION *options) {

	unsigned int k, n_errors = 0, n_files = 0;
	int n;
	char A[GMT_LEN16] = {""}, B[GMT_LEN16] = {""}, C[GMT_LEN16] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {		/* Process all the options given */
		switch (opt->option) {

			case '<':	/* Input file */
				if (n_files++ > 0) break;
				if (strchr (opt->arg, '%')) {
					Ctrl->In.many = true;
					Ctrl->In.file = strdup (opt->arg);
				}
				else if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'C':
				switch (opt->arg[0]) {
					case 'p': Ctrl->C.nu = atof (&opt->arg[1]); break;
					case 'y': Ctrl->C.E = atof (&opt->arg[1]); break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Unrecognized modifier %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'D':
				Ctrl->D.active = true;
				n = sscanf (opt->arg, "%lf/%lf/%lf/%lf", &Ctrl->D.rhom, &Ctrl->D.rhol, &Ctrl->D.rhoi, &Ctrl->D.rhow);
				if (!(n == 4 || n == 3)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -D option: must give 3-4 density values\n");
					n_errors++;
				}
				if (n == 3) {
					Ctrl->D.rhow = Ctrl->D.rhoi;
					Ctrl->D.rhoi = Ctrl->D.rhol;
				}
				else if (Ctrl->D.rhol != Ctrl->D.rhoi)
					Ctrl->D.approx = true;
				break;
			case 'E':
				Ctrl->E.active = true;
				GMT_Get_Value (API, opt->arg, &Ctrl->E.te);
				if (Ctrl->E.te > 1e10) { /* Given flexural rigidity, compute Te */
					Ctrl->E.te = pow ((12.0 * (1.0 - Ctrl->C.nu * Ctrl->C.nu)) * Ctrl->E.te / Ctrl->C.E, 1.0/3.0);
				}
				break;
			case 'F':
				Ctrl->F.active = true;
				n = sscanf (opt->arg, "%lf/%[^/]/%lf", &Ctrl->F.nu_a, A, &Ctrl->F.nu_m);
				if (!(n == 3 || n == 1)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: must -F<nu> or -F<nu_a>/<h_a>/<nu_m>\n");
					n_errors++;
				}
				if (n == 3) {
					Ctrl->F.mode = FLX_FV2;
					GMT_Get_Value (API, A, &Ctrl->F.h_a);
				}
				else
					Ctrl->F.mode = FLX_FV1;
				break;
			case 'G':
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'M':
				Ctrl->M.active = true;
				Ctrl->M.maxwell_t = atof (opt->arg);
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.info = GMT_FFT_Parse (API, 'N', GMT_FFT_DIM, opt->arg);
				if (Ctrl->N.info == NULL) n_errors++;
				break;
			case 'T':
				Ctrl->T.active = true;
				k = (opt->arg[0] == 'l') ? 1 : 0;
				Ctrl->T.log = (k == 1);
				n = sscanf (&opt->arg[k], "%[^/]/%[^/]/%s", A, B, C);
				if (!(n == 3 || n == 1)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Must give -T<t0> or -T[l]t0[u]/t1[u]/dt[u]\n");
					n_errors++;
				}
				Ctrl->T.start = get_age (A, &Ctrl->T.unit, &Ctrl->T.scale);
				if (n == 3) {
					Ctrl->T.end = get_age (B, &Ctrl->T.unit, &Ctrl->T.scale);
					Ctrl->T.inc = get_age (C, &Ctrl->T.unit, &Ctrl->T.scale);
				}
				else {
					Ctrl->T.end = Ctrl->T.start;	Ctrl->T.inc = 1.0;	/* This will give one time in the series */	
				}					
				break;
			case 'W':
				Ctrl->W.active = true;
				Ctrl->W.water_depth = atof (opt->arg);
				break;
			case 'Z':
				Ctrl->Z.active = true;
				Ctrl->Z.zm = atof (opt->arg);
				break;
			default:
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->N.active && Ctrl->N.info->info_mode == GMT_FFT_LIST) {
		return (GMT_PARSE_ERROR);	/* So that we exit the program */
	}
	
	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file,  "Syntax error -G option: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->D.active, "Syntax error -D option: must set density values\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.active && !Ctrl->T.active, "Syntax error -F option: Requires time information via -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && !Ctrl->T.active, "Syntax error -M option: Requires time information via -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.active && !Ctrl->E.active, "Syntax error -F option: Requires elastic thickness via -E\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && !Ctrl->E.active, "Syntax error -M option: Requires elastic thickness via -E\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->T.end > Ctrl->T.start, "Syntax error -T option: Start time must be larger (older) than end time\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && !strchr (Ctrl->G.file, '%'), "Syntax error -G option: Filename template must contain format specified\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->T.active && Ctrl->In.many, "Syntax error: Load template given but -T not specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int GMT_grdflexure_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdflexure <topogrd> -D<rhom>/<rhol>/<rhoi>/<rhow> -E<te> -G<outgrid> [-C[p|y]<value] [-F<nu_a>[/<h_a>/<nu_m>]]\n");
	GMT_Message (API, GMT_TIME_NONE,"\t[-N%s] [-T[l]<t0>/<t1>/<dt>|<n>]\n\t[%s] [-W<wd>] [-Z<zm>] [-fg]\n\n", GMT_FFT_OPT, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE,"\t<topogrd> is the input grdfile with topography (load) values, in meters. If -T is used,\n");
	GMT_Message (API, GMT_TIME_NONE,"\t   <topogrd> may be a filename template with a floating point format (C syntax) and\n");
	GMT_Message (API, GMT_TIME_NONE,"\t   a different load file name will be set and loaded for each time step.\n");
	GMT_Message (API, GMT_TIME_NONE,"\t-D Sets density values for mantle, load(crust), moat infill, and water in kg/m^3.\n");
	GMT_Message (API, GMT_TIME_NONE,"\t-E Sets elastic plate thickness in m; append k for km.  If Te > 1e10 it will be interpreted\n");
	GMT_Message (API, GMT_TIME_NONE,"\t   as the flexural rigidity [Default computes D from Te, Young's modulus, and Poisson's ratio].\n");
	GMT_Message (API, GMT_TIME_NONE,"\t-G filename for output grdfile with flexed surface.  If -T is set then <outgrid>\n");
	GMT_Message (API, GMT_TIME_NONE,"\t   must be a filename template that contains a floating point format (C syntax) and\n");
	GMT_Message (API, GMT_TIME_NONE,"\t   we use the corresponding time (in units specified in -T) to generate the file name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE,"\t-C use -Cy<Young> or -Cp<poisson> to change Young's modulus [%s] or Poisson's ratio [%s].\n");
	GMT_Message (API, GMT_TIME_NONE,"\t-F Sets upper mantle viscosity, and optionally its thickness and lower mantle viscosity.\n");
	GMT_Message (API, GMT_TIME_NONE,"\t   Viscosity units in Pa s; thickness in meter (append k for km).\n");
	GMT_FFT_Option (API, 'N', GMT_FFT_DIM, "Choose or inquire about suitable grid dimensions for FFT, and set modifiers.");
	GMT_Message (API, GMT_TIME_NONE,"\t-T Specify start, stop, and time increments for sequence of calculations [one step, no time dependency].\n");
	GMT_Message (API, GMT_TIME_NONE,"\t   For a single specific time, just give <start>. unit is years; append k for kyr and M for Myr.\n");
	GMT_Message (API, GMT_TIME_NONE,"\t   For a logarithmic time scale, use -Tl and specify n steps instead of time increment.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE,"\t-W Specify water depth in m; append k for km.\n");
	GMT_Message (API, GMT_TIME_NONE,"\t-Z Specify reference depth to flexed surface in m; append k for km.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Convert geographic grids to meters using a \"Flat Earth\" approximation.\n");
	GMT_Option (API, ".");
	return (EXIT_FAILURE);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdflexure_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdflexure (void *V_API, int mode, void *args) {
	unsigned int fmode, n_times, t;
	int error;
	bool init_load;
	float *orig_load = NULL;
	double user_time;
	char file[GMT_LEN256] = {""};

	struct GMT_GRID *Grid = NULL, *Orig = NULL;
	struct GMT_FFT_WAVENUMBER *K = NULL;
	struct RHEOLOGY *R = NULL;
	struct GRDFLEXURE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdflexure_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE)
		bailout (GMT_grdflexure_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS)
		bailout (GMT_grdflexure_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdflexure_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdflexure_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdfft main code ----------------------------*/

	/* Set the transfer function to use */
	if (Ctrl->F.active)		/* One of two firmoviscous functions */
		fmode = Ctrl->F.mode;
	else if (Ctrl->M.active)	/* Viscoelastic */
		fmode = FLX_VE;
	else				/* Elastic */
		fmode = FLX_E;
		
	R = GMT_memory (GMT, NULL, 1, struct RHEOLOGY);	/* Allocate rheology structure */
	
	switch (fmode) {	/* Set function pointers */
		case FLX_E:
			R->setup = setup_elastic;	R->transfer = transfer_elastic;		break;
		case FLX_VE:
			R->setup = setup_ve;		R->transfer = transfer_ve;		break;
		case FLX_FV1:
			R->setup = setup_fv;		R->transfer = transfer_fv;		break;
		case FLX_FV2:
			R->setup = setup_fv2;		R->transfer = transfer_fv2;		break;
	}
	
	/* 0. DETERMINE THE NUMBER OF TIME STEPS */
	
	n_times = (Ctrl->T.active) ? lrint ((Ctrl->T.start - Ctrl->T.end) / Ctrl->T.inc) + 1 : 1;
	
	for (t = 0; t < n_times; t++) {	/* For each time step (or just once) */
		
		/* 1. SET THE CURRENT TIME VALUE (IF USED) */
		if (Ctrl->T.active) {	/* Set the current time in user units as well as years */
			user_time = Ctrl->T.start = t * Ctrl->T.inc;	/* In units of user's choice */
			R->time_yr = user_time * Ctrl->T.scale;		/* Now in years */
			GMT_Report (API, GMT_MSG_VERBOSE, "Evaluating solution for time %g\n", user_time);
		}
		
		/* 2. DETERMINE INPUT FILE NAME */
		if (Ctrl->In.many) {	/* Must read in a new load grid for each time increment */
			sprintf (file, Ctrl->In.file, user_time);
			if (!GMT_check_filearg (GMT, '<', file, GMT_IN)) {
				GMT_Report (API, GMT_MSG_VERBOSE, "No load file found for time %g - skipping this time\n", user_time);
				continue;
			}
			init_load = true;
		}
		else if (t == 0) {	/* First time with a constant grid */
			strcpy (file, Ctrl->In.file);
			init_load = true;
		}
		else	/* Later times with the same constant grid */
			init_load = false;
		
		/* 3. OBTAIN NEXT (OR POSSIBLY ONLY) LOAD */
		if (init_load) {	/* Must initialize a new load grid */
			GMT_Report (API, GMT_MSG_VERBOSE, "Read load file %s\n", file);
			if ((Orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, file, NULL)) == NULL)
				Return (API->error);
			GMT_grd_init (GMT, Orig->header, options, true);	/* Update the header */
			if ((Orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY |
		 		GMT_GRID_IS_COMPLEX_REAL, NULL, file, Orig)) == NULL)	/* Get data only */
				Return (API->error);
			/* Note: If input grid is read-only then we must duplicate it; otherwise Grid points to Orig */
			(void) GMT_set_outgrid (GMT, file, Orig, &Grid);
			/* From here we address the grid via Grid ; we are done with using the address Orig directly. */
			K = GMT_FFT_Create (API, Grid, GMT_FFT_DIM, GMT_GRID_IS_COMPLEX_REAL, Ctrl->N.info);	/* Also detrends, if requested */
			/* 4. DO THE FORWARD FFT */
			GMT_Report (API, GMT_MSG_VERBOSE, "forward FFT...\n");
			if (GMT_FFT (API, Grid, GMT_FFT_FWD, GMT_FFT_COMPLEX, K)) {
				Return (EXIT_FAILURE);
			}
			if (n_times > 1 && !Ctrl->In.many) {	/* First time with a constant grid that is needed again; make a copy */
				orig_load = GMT_memory (GMT, NULL, Grid->header->size, float);
				GMT_memcpy (orig_load, Grid->data, Grid->header->size, float);
			}
		}
		else if (t > 0) {	/* Restore the constant FFT'ed grid */
			GMT_memcpy (Grid->data, orig_load, Grid->header->size, float);
		}

		/* 5. COMPUTE THE NEW SOLUTION */
		spectral_solution (GMT, Grid, Ctrl, K, R);
		
		/* 6. DO THE INVERSE FFT */
		if (GMT_FFT (API, Grid, GMT_FFT_INV, GMT_FFT_COMPLEX, K))
			Return (EXIT_FAILURE);

		/* 7. APPLY SCALING */
		GMT_scale_and_offset_f (GMT, Grid->data, Grid->header->size, 1.0, -Ctrl->Z.zm);

		/* 8. WRITE OUTPUT GRID */
		
		/* Get ready for output */
		if (Ctrl->T.active)
			sprintf (file, Ctrl->G.file, user_time);
		else
			strcpy (file, Ctrl->G.file);
		/* The data are in the middle of the padded array; only the interior (original dimensions) will be written to file */
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid))
			Return (API->error);
			
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY |
			GMT_GRID_IS_COMPLEX_REAL, NULL, file, Grid) != GMT_OK) {
				Return (API->error);
		}

		/* 9. FREE GRID IF NEW ONE IS NEEDED */
		if (Ctrl->In.many) {
			GMT_Destroy_Data (API, &Grid);
			GMT_free (GMT, K);
		}
	}
	
	if (K) GMT_free (GMT, K);
	GMT_free (GMT, R);

	GMT_Report (API, GMT_MSG_VERBOSE, "done!\n");

	Return (EXIT_SUCCESS);
}
