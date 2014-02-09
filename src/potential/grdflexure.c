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
 * Various isostatic calculations with user-selectable rheologies
 *
 * */

#define THIS_MODULE_NAME	"grdflexure"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Compute flexural deformation of 3-D surfaces for various rheologies"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-Vf"
#define GMT_FFT_DIM	2	/* Dimension of FFT needed */

#define FLX_E	0	/* Elastic */
#define FLX_VE	1	/* Viscoelastic */
#define FLX_FV1	2	/* Firmoviscous 1-layer */
#define FLX_FV2	3	/* Firmoviscous 2-layer */

double smt_get_age (char *A, char *unit, double *scale);

struct GRDFLEXURE_CTRL {
	struct In {	/* Input load file or template */
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
	struct S {	/* Starved moat */
		bool active;
		double beta;	/* Fraction of moat w(x) filled in [1] */
	} S;
	struct T {	/* -T[l]<t0>[u]/<t1>[u]/<d0>[u]|n  */
		bool active, log;
		unsigned int n_times;
		double start, end, inc;	/* Time ago, so start > end */
		double time;	/* The current time in a sequence */
		double scale;	/* Scale factor from user time to year */
		char unit;	/* Either M (Myr), k (kyr), or blank (y) */
	} T;
	struct W {	/* Water depth */
		bool active;
		double water_depth;	/* Reference water depth [0] */
	} W;
	struct Z {	/* Moho depth */
		bool active;
		double zm;	/* Reference depth to flexed surface [0] */
	} Z;
};

struct RHEOLOGY {	/* Used to pass parameters in/out of functions */
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

struct FLX_GRID {
	struct GMT_GRID *Grid;		/* Pointer to the grid, or NULL if it does not exist */
	struct GMT_FFT_WAVENUMBER *K;	/* Pointer to FFT struct, unless G is NULL */
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
	C->S.beta = 1.0;
	C->T.n_times = 1;

	return (C);
}

void Free_grdflexure_Ctrl (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->G.file) free (C->G.file);	
	if (C->N.info) GMT_free (GMT, C->N.info);
	GMT_free (GMT, C);	
}

double transfer_elastic (double k, struct RHEOLOGY *R)
{	/* Elastic transfer function */
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
	   force on the plate (rm - ri)*gravity if ri = rw; so use zero for topo in air (ri changed to rl).
	*/
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
	if (Ctrl->S.active && Ctrl->S.beta < 1.0) {	/* Treat starved infill as approximate case with different infill density */
		Ctrl->D.approx = true;
		Ctrl->D.rhoi = Ctrl->S.beta * Ctrl->D.rhoi - Ctrl->D.rhow * (1.0 - Ctrl->S.beta);
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Starved moat with beta = %g implies an effective rho_i  = %g\n", Ctrl->S.beta, Ctrl->D.rhol);
	}
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
{	/* Setup function for 2-layer viscous mantle beneath elastic plate */
	R->t0 = R->time_yr * (86400*365.25);	/* Convert to seconds */
	R->nu_ratio = Ctrl->F.nu_a / Ctrl->F.nu_m;
	R->nu_ratio1 = 1.0 / R->nu_ratio;
	R->c = (2.0 * Ctrl->F.nu_m) / (Ctrl->D.rhom * NORMAL_GRAVITY);
	R->dens_ratio = (Ctrl->D.rhom - Ctrl->D.rhoi) / Ctrl->D.rhom;
}

double transfer_fv2 (double k, struct RHEOLOGY *R)
{	/* Transfer functino for 2-layer viscous mantle */
	double phi_e, phi_fv2, tau;
	phi_e = transfer_elastic (k, R);
	tau = relax_time_2 (k, R);
	phi_fv2 = phi_e * (1.0 - exp (-R->t0 * R->dens_ratio / (tau * phi_e)));
	return (phi_fv2); 
}

void setup_fv (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct RHEOLOGY *R)
{	/* Setup function for 1-layer viscous mantle beneath elastic plate */
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

void Apply_Transfer_Function (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct RHEOLOGY *R) {
	/* Do the spectral convolution for isostatic response in the Freq domain. */
	uint64_t k;
	double  mk, transfer_fn;
	
	R->setup (GMT, Ctrl, K, R);	/* Set up parameters */

	/* Loop over complex grid and multiply with the real transfer function */
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
			case 'C':	/* Rheology constants E and nu */
				switch (opt->arg[0]) {
					case 'p': Ctrl->C.nu = atof (&opt->arg[1]); break;
					case 'y': Ctrl->C.E = atof (&opt->arg[1]); break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C option: Unrecognized modifier %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'D':	/* Set densities */
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
			case 'E':	/* Set elastic thickness */
				Ctrl->E.active = true;
				GMT_Get_Value (API, opt->arg, &Ctrl->E.te);
				if (Ctrl->E.te > 1e10) { /* Given flexural rigidity, compute Te */
					Ctrl->E.te = pow ((12.0 * (1.0 - Ctrl->C.nu * Ctrl->C.nu)) * Ctrl->E.te / Ctrl->C.E, 1.0/3.0);
				}
				break;
			case 'F':	/* Firmoviscous response */
				Ctrl->F.active = true;
				n = sscanf (opt->arg, "%lf/%[^/]/%lf", &Ctrl->F.nu_a, A, &Ctrl->F.nu_m);
				if (!(n == 3 || n == 1)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: must select -F<nu> or -F<nu_a>/<h_a>/<nu_m>\n");
					n_errors++;
				}
				if (n == 3) {
					Ctrl->F.mode = FLX_FV2;
					GMT_Get_Value (API, A, &Ctrl->F.h_a);
				}
				else
					Ctrl->F.mode = FLX_FV1;
				break;
			case 'G':	/* Output file name or template */
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'M':	/* Viscoelastic maxwell time */
				Ctrl->M.active = true;
				Ctrl->M.maxwell_t = atof (opt->arg);
				break;
			case 'N':	/* FFT parameters */
				Ctrl->N.active = true;
				Ctrl->N.info = GMT_FFT_Parse (API, 'N', GMT_FFT_DIM, opt->arg);
				if (Ctrl->N.info == NULL) n_errors++;
				break;
			case 'S':	/* Starved basin */
				Ctrl->S.active = true;
				Ctrl->S.beta = atof (opt->arg);
				break;
			case 'T':	/* Time lattice */
				Ctrl->T.active = true;
				k = (opt->arg[0] == 'l') ? 1 : 0;
				Ctrl->T.log = (k == 1);
				n = sscanf (&opt->arg[k], "%[^/]/%[^/]/%s", A, B, C);
				if (!(n == 3 || n == 1)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Must give -T<t0> or -T[l]t0[u]/t1[u]/dt[u]\n");
					n_errors++;
				}
				Ctrl->T.start = smt_get_age (A, &Ctrl->T.unit, &Ctrl->T.scale);
				if (n == 3) {
					Ctrl->T.end = smt_get_age (B, &Ctrl->T.unit, &Ctrl->T.scale);
					Ctrl->T.inc = smt_get_age (C, &Ctrl->T.unit, &Ctrl->T.scale);
					if (Ctrl->T.end > Ctrl->T.start) double_swap (Ctrl->T.start, Ctrl->T.end);	/* Enforce that old time is larger */
					Ctrl->T.n_times = (Ctrl->T.log) ? irint (Ctrl->T.inc) : lrint ((Ctrl->T.start - Ctrl->T.end) / Ctrl->T.inc) + 1;
					if (Ctrl->T.log) Ctrl->T.inc = (log10 (Ctrl->T.start) - log10 (Ctrl->T.end)) / (Ctrl->T.n_times - 1);	/* Convert n to inc_logt */
				}
				else {
					Ctrl->T.end = Ctrl->T.start;	Ctrl->T.inc = 1.0;	/* This will give one time in the series */
					Ctrl->T.n_times = 1;
				}					
				break;
			case 'W':	/* Water depth */
				Ctrl->W.active = true;
				Ctrl->W.water_depth = atof (opt->arg);
				break;
			case 'Z':	/* Moho depth */
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
	n_errors += GMT_check_condition (GMT, !Ctrl->D.active, "Syntax error -D option: Must set density values\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && (Ctrl->S.beta < 0.0 || Ctrl->S.beta > 1.0), "Syntax error -S option: beta value must be in 0-1 range\n");
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
	GMT_Message (API, GMT_TIME_NONE, "usage: grdflexure <topogrid> -D<rhom>/<rhol>/<rhoi>/<rhow> -E<te> -G<outgrid> [-C[p|y]<value] [-F<nu_a>[/<h_a>/<nu_m>]]\n");
	GMT_Message (API, GMT_TIME_NONE,"\t[-N%s] [-S<beta>] [-T[l]<t0>/<t1>/<dt>|<n>]\n\t[%s] [-W<wd>] [-Z<zm>] [-fg]\n\n", GMT_FFT_OPT, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<topogrid> is the input grdfile with topography (load) values, in meters. If -T is used,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <topogrid> may be a filename template with a floating point format (C syntax) and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a different load file name will be set and loaded for each time step.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Sets density values for mantle, load(crust), moat infill, and water in kg/m^3.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Sets elastic plate thickness in m; append k for km.  If Te > 1e10 it will be interpreted\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   as the flexural rigidity [Default computes D from Te, Young's modulus, and Poisson's ratio].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G filename for output grdfile with flexed surface.  If -T is set then <outgrid>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   must be a filename template that contains a floating point format (C syntax) and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   we use the corresponding time (in units specified in -T) to generate the file name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C use -Cy<Young> or -Cp<poisson> to change Young's modulus [%g] or Poisson's ratio [%g].\n", YOUNGS_MODULUS, POISSONS_RATIO);
	GMT_Message (API, GMT_TIME_NONE, "\t-F Sets upper mantle viscosity, and optionally its thickness and lower mantle viscosity.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Viscosity units in Pa s; thickness in meter (append k for km).\n");
	GMT_FFT_Option (API, 'N', GMT_FFT_DIM, "Choose or inquire about suitable grid dimensions for FFT, and set modifiers.");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Specify starved moat fraction in 0-1 range (1 = fully filled, 0 = no infill) [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify start, stop, and time increments for sequence of calculations [one step, no time dependency].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For a single specific time, just give <start>. unit is years; append k for kyr and M for Myr.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For a logarithmic time scale, use -Tl and specify n steps instead of time increment.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Specify water depth in m; append k for km.  Must be positive\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Specify reference depth to flexed surface in m; append k for km.  Must be positive.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Convert geographic grids to meters using a \"Flat Earth\" approximation.\n");
	GMT_Option (API, ".");
	return (EXIT_FAILURE);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdflexure_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

struct FLX_GRID *Prepare_Load (struct GMT_CTRL *GMT, struct GMT_OPTION *options, struct GRDFLEXURE_CTRL *Ctrl, char *file)
{
	struct GMT_GRID *Grid = NULL, *Orig = NULL;
	struct FLX_GRID *G = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	if (!GMT_check_filearg (GMT, '<', file, GMT_IN)) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Load file %s not found - skipped\n", file);
		return NULL;
	}
	/* Must initialize a new load grid */
	GMT_Report (API, GMT_MSG_VERBOSE, "Read load file %s\n", file);
	if ((Orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, file, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Error reading the header of file %s - file skipped\n", file);
		return NULL;
	}
	GMT_grd_init (GMT, Orig->header, options, true);	/* Update the header */
	if ((Orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY |
 		GMT_GRID_IS_COMPLEX_REAL, NULL, file, Orig)) == NULL) {	/* Get data only */
		GMT_Report (API, GMT_MSG_VERBOSE, "Error reading the data of file %s - file skipped\n", file);
		return NULL;
	}
	if (Ctrl->W.active) {	/* See if any part of the load sticks above water, and if so scale this amount as if it was submerged */
		uint64_t node, n_subaerial = 0;
		double boost = Ctrl->D.rhol / (Ctrl->D.rhol - Ctrl->D.rhow);
		for (node = 0; node < Grid->header->size; node++) {
			if (Grid->data[node] > Ctrl->W.water_depth) {
				Grid->data[node] = (float)(Ctrl->W.water_depth + (Grid->data[node] - Ctrl->W.water_depth) * boost);
				n_subaerial++;
			}
		}
		if (n_subaerial) GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " nodes were subarial so heights were scaled for the equivalent submerged case\n", n_subaerial);
	}
	/* Note: If input grid is read-only then we must duplicate it; otherwise Grid points to Orig */
	(void) GMT_set_outgrid (API->GMT, file, Orig, &Grid);
	/* From here we address the grid via Grid; we are done with using the address Orig directly. */
	G = GMT_memory (GMT, NULL, 1, struct FLX_GRID);	/* Allocate a Flex structure */
	G->K = GMT_FFT_Create (API, Grid, GMT_FFT_DIM, GMT_GRID_IS_COMPLEX_REAL, Ctrl->N.info);	/* Also detrends, if requested */
	/* Do the forward FFT */
	GMT_Report (API, GMT_MSG_VERBOSE, "forward FFT...\n");
	if (GMT_FFT (API, Grid, GMT_FFT_FWD, GMT_FFT_COMPLEX, G->K)) {
		GMT_Report (API, GMT_MSG_VERBOSE, "Error taking the FFT of %s - file skipped\n", file);
		return NULL;
	}
	G->Grid = Grid;	/* Pass grid back via the grid array */
	return (G);
}

struct RHEOLOGY *Select_Rheology (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl)
{
	unsigned int fmode = 0;
	struct RHEOLOGY *R = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	/* Select the transfer function to use */
	if (Ctrl->F.active)		/* One of two firmoviscous functions */
		fmode = Ctrl->F.mode;
	else if (Ctrl->M.active)	/* Viscoelastic */
		fmode = FLX_VE;
	else				/* Elastic */
		fmode = FLX_E;
		
	R = GMT_memory (GMT, NULL, 1, struct RHEOLOGY);	/* Allocate rheology structure */
	
	switch (fmode) {	/* Set function pointers */
		case FLX_E:
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Selected Elastic transfer function\n");
			R->setup = setup_elastic;	R->transfer = transfer_elastic;		break;
		case FLX_VE:
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Selected Viscoelastic transfer function\n");
			R->setup = setup_ve;		R->transfer = transfer_ve;		break;
		case FLX_FV1:
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Selected Firmoviscous transfer function for elastic plate over viscous half-space\n");
			R->setup = setup_fv;		R->transfer = transfer_fv;		break;
		case FLX_FV2:
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Selected Firmoviscous transfer function for elastic plate over viscous layer over viscous half-space\n");
			R->setup = setup_fv2;		R->transfer = transfer_fv2;		break;
	}
	return (R);
}

void Accumulate_Solution (struct GMT_CTRL *GMT, struct GMT_GRID *Out, struct GMT_GRID *Component)
{	/* Simply adds component grid to output grid */
	uint64_t node;
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Add in latest component\n");
	for (node = 0; node < Out->header->size; node++) Out->data[node] += Component->data[node];
}

int GMT_grdflexure (void *V_API, int mode, void *args) {
	unsigned int t, t_load;
	int error;
	bool retain_original;
	float *orig_load = NULL;
	double user_time;
	char file[GMT_LEN256] = {""};

	struct GMT_FFT_WAVENUMBER *K = NULL;
	struct RHEOLOGY *R = NULL;
	struct FLX_GRID **G = NULL;
	struct GMT_GRID *Out = NULL;
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

	/* 1. SELECT THE TRANSFER FUNCTION TO USE */
	
	R = Select_Rheology (GMT, Ctrl);
	
	/* 2. READ ALL INPUT LOAD GRIDS, DETREND, AND TAKE FFT */
	
	if (Ctrl->In.many) {	/* Must read in a new load grid, possibly one for each time increment */
		G = GMT_memory (GMT, NULL, Ctrl->T.n_times, struct FLX_GRID *);	/* Allocate grid array structure */
		for (t = 0; t < Ctrl->T.n_times; t++) {	/* For each time step there may be a load file */
			user_time = (Ctrl->T.log) ? pow (10.0, log10 (Ctrl->T.start) - t * Ctrl->T.inc) : Ctrl->T.start - t * Ctrl->T.inc;	/* In units of user's choice */
			sprintf (file, Ctrl->In.file, user_time);
			G[t] = Prepare_Load (GMT, options, Ctrl, file);
			K = G[t]->K;	/* We only need one pointer to get to wavenumbers; this just ensures we keep one */
		}
	}
	else {	/* Just read the single load grid */
		G = GMT_memory (GMT, NULL, 1, struct FLX_GRID);	/* Allocate grid array structure with one entry */
		G[0] = Prepare_Load (GMT, options, Ctrl, Ctrl->In.file);
		K = G[0]->K;	/* We only need one pointer to get to wavenumbers, since they are all the same */
	}
	
	/* 3. DETERMINE AND POSSIBLY CREATE ONE OUTPUT GRID */
	retain_original = (Ctrl->T.n_times > 1);	/* True when we will have to loop over the loads */
	if (retain_original) {	/* We may need to reuse loads for different times and will have to keep copy of unchanged H(kx,ky) */
		orig_load = GMT_memory (GMT, NULL, G[0]->Grid->header->size, float);	/* Single temporary storage to hold one original H(kx,ky) grid */
		/* We must also allocate a separate output grid */
		if ((Out = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, G[0]->Grid)) == NULL) Return (API->error);	/* Output grid of same size as input */
	}
	else	/* With a single load -> flexure operation we can just recycle the input grid for the output */
		Out = G[0]->Grid;

	/* Here, G[t] contains all the input load grids, ready to go as H(kx,ky) */
	
	for (t = 0; t < Ctrl->T.n_times; t++) {	/* For each time step (i.e., at least once) */
		
		/* 4a. SET THE CURRENT TIME VALUE (IF USED) */
		if (Ctrl->T.active) {	/* Set the current time in user units as well as years */
			user_time = (Ctrl->T.log) ? pow (10.0, log10 (Ctrl->T.start) - t * Ctrl->T.inc) : Ctrl->T.start - t * Ctrl->T.inc;	/* In units of user's choice */
			R->time_yr = user_time * Ctrl->T.scale;		/* Now in years */
			GMT_Report (API, GMT_MSG_VERBOSE, "Evaluating flexural deformation for time %g\n", user_time);
		}
		
		if (retain_original) GMT_memset (Out->data, Out->header->size, float);	/* Reset output grid to zero; not necessary when we only get here once */
		
		for (t_load = 0; t_load <= t; t_load++) {	/* For each load already emplaced at the current output time t */
			if (G[t_load] == NULL) continue;	/* Quietly skip times with no load */
			
			/* 4b. COMPUTE THE RESPONSE DUE TO THIS LOAD */
			if (retain_original) GMT_memcpy (orig_load, G[t_load]->Grid->data, G[t_load]->Grid->header->size, float);	/* Make a copy of H(kx,ky) before operations */
			Apply_Transfer_Function (GMT, G[t_load]->Grid, Ctrl, G[t_load]->K, R);	/* Multiplies H(kx,ky) by transfer function, yielding W(kx,ky) */
			if (retain_original) {	/* Must add this contribution to our total output grid */
				Accumulate_Solution (GMT, Out, G[t_load]->Grid);
				GMT_memcpy (G[t_load]->Grid->data, orig_load, G[t_load]->Grid->header->size, float);	/* Restore H(kx,ky) to what it was before operations */
			}
		}
		
		/* 4c. TAKE THE INVERSE FFT TO GET w(x,y) */
		if (GMT_FFT (API, Out, GMT_FFT_INV, GMT_FFT_COMPLEX, K))
			Return (EXIT_FAILURE);

		/* 4d. APPLY SCALING AND OFFSET */
		GMT_scale_and_offset_f (GMT, Out->data, Out->header->size, 1.0, -Ctrl->Z.zm);

		/* 4d. WRITE OUTPUT GRID */
		
		if (Ctrl->T.active) { /* Separate output grid since many time steps */
			char remark[GMT_GRID_REMARK_LEN160] = {""};
			sprintf (file, Ctrl->G.file, user_time);
			sprintf (remark, "Solution for t = %g", user_time);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, remark, Out))
				Return (API->error);
		}
		else	/* Single output grid */
			strcpy (file, Ctrl->G.file);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out))
			Return (API->error);
			
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY |
			GMT_GRID_IS_COMPLEX_REAL, NULL, file, Out) != GMT_OK) {
				Return (API->error);
		}
	}
	
	/* 5. FREE ALL GRIDS AND ARRAYS */
	for (t = 0; t < Ctrl->T.n_times; t++) {	/* Free up grid structures */
		if (G[t] == NULL) continue;	/* Quietly skip containers with no grids */
		GMT_Destroy_Data (API, &G[t]->Grid);
		GMT_free (GMT, G[t]->K->info);
		GMT_free (GMT, G[t]->K);
		GMT_free (GMT, G[t]);
	}
	GMT_free (GMT, G);
	GMT_free (GMT, R);

	GMT_Report (API, GMT_MSG_VERBOSE, "done!\n");

	Return (EXIT_SUCCESS);
}
