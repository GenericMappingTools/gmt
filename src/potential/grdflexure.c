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


struct GMT_MODELTIME {	/* Hold info about time */
	double value;	/* Time as given by user (e.g., 1, 1k, 1M are all 1) */
	double scale;	/* Scale factor from user time to year */
	char unit;	/* Either M (Myr), k (kyr), or blank (implies y) */
	unsigned int u;	/* For labeling: Either 0 (yr), 1 (kyr), or 2 (Myr) */
};

struct GRDFLEXURE_CTRL {
	struct In {	/* Input load file, template, or =flist */
		bool active, many, list;
		char *file;
	} In;
	struct A {	/* -A<Nx/Ny/Nxy> In-plane forces */
		bool active;
		double Nx, Ny, Nxy;
	} A;
	struct C {	/* -Cy<E> or -Cp<poisson> */
		bool active[2];
		double E, nu;
	} C;
	struct D {	/* -D<rhom/rhol[/rhoi]/rhow> */
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
	struct L {	/* -L<outlist> */
		bool active;
		char *file;
	} L;
	struct M {	/* -M<maxwell_t>  */
		bool active;
		double maxwell_t;	/* Maxwell time */
		double scale;		/* scale for time */
		char unit;		/* Unit of time */
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
		unsigned int n_eval_times;
		struct GMT_MODELTIME *time;	/* The current sequence of times */
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
	double eval_time_yr;	/* Time in years of evaluation or relative time since loading */
	double load_time_yr;	/* Time in years of loading, or zero */
	double t0;		/* Time in seconds since loading */
	double nu_ratio;	/* Ratio of asthenosphere to lower mantle viscosities */
	double nu_ratio1;	/* The inverse ratio */
	double h_a;		/* The thickness of the asthenosphere (m) */
	double ce;		/* A constant for elastic transfer functions */
	double Nx_e;		/* A constant for --"-- that is nonzero when Nx is nonzero */
	double Ny_e;		/* A constant for --"-- that is nonzero when Ny is nonzero */
	double Nxy_e;		/* A constant for --"-- that is nonzero when Nxy is nonzero */
	double cv;		/* A constant for visous transfer functions */
	double scale;		/* Overall scale (e.g., Airy scale) */
	double dens_ratio;	/* (Ctrl->D.rhom - Ctrl->D.rhoi) / Ctrl->D.rhom */
	bool relative;		/* eval_time_yr is relative to load time [at 0] */
	bool isotropic;		/* true when no inplane forces are set (no -A) */
	double (*transfer) (double *, struct RHEOLOGY *);	/* pointer to function returning isostatic response for given k and R */
	double (*tr_elastic_sub) (double *, struct RHEOLOGY *);	/* pointer to sub-function returning elastic isostatic response for given k and R */
	void (*setup) (struct GMT_CTRL *, struct GRDFLEXURE_CTRL *, struct GMT_FFT_WAVENUMBER *, struct RHEOLOGY *);	/* Init function */
};

struct FLX_GRID {
	struct GMT_GRID *Grid;		/* Pointer to the grid, or NULL if it does not exist */
	struct GMT_MODELTIME *Time;	/* Pointer to time info for this load */
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
	C->T.n_eval_times = 1;

	return (C);
}

void Free_grdflexure_Ctrl (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);
	if (C->G.file) free (C->G.file);
	if (C->L.file) free (C->L.file);
	if (C->N.info) GMT_free (GMT, C->N.info);
	if (C->T.time) GMT_free (GMT, C->T.time);
	GMT_free (GMT, C);
}

double gmt_get_modeltime (char *A, char *unit, double *scale)
{	/* Convert age[k|M] to years, return unit and scale needed to convert year back to time in given unit */
	size_t k = strlen (A) - 1;
	*scale = 1.0;
	*unit = 'y';
	switch (A[k]) {
		case 'y': *scale = 1.0;    *unit = A[k]; A[k] = '\0'; break;
		case 'k': *scale = 1.0e-3; *unit = A[k]; A[k] = '\0'; break;
		case 'M': *scale = 1.0e-6; *unit = A[k]; A[k] = '\0'; break;
	}
	return (atof (A) / (*scale));
}

int compare_modeltimes (const void *time_1v, const void *time_2v)
{
	/*  Routine for qsort to sort model times array so old times (large t) will be first in list. */
	const struct GMT_MODELTIME *time_1 = time_1v, *time_2 = time_2v;
	if (time_1->value > time_2->value) return (-1);
	if (time_1->value < time_2->value) return (+1);
	return (0);
}

unsigned int gmt_modeltime_array (struct GMT_CTRL *GMT, char *arg, bool *log, struct GMT_MODELTIME **T_array)
{	/* Parse -T<tfile>, -T<t0> or -T<t0>[u]/<t1>[u]/<dt>[u][+l] and return array of times.
	 * The array times are all in years, while the unit and scale can change.  Programs that need
	 * the time in year should use T_array[k].value while programs that need the original time and
	 * unit specified by the user should use T_array[k].value * T_array[k].scale and T_array[k].unit.
	 * If a log-equidistant range is specified via +l we set *log to true, else false.  The function
	 * returns the number of times in T_array.
	 */
	char *p = NULL, s_unit;
	unsigned int n_eval_times = 0, k, u = 0;
	double s_time, s_scale;
	struct GMTAPI_CTRL *API = GMT->parent;
	struct GMT_MODELTIME *T = NULL;

	*log = false;
	if ((p = strstr (arg, "+l"))) {	/* Want logarithmic time scale */
		*log = true;
		p[0] = '\0';	/* Chop off the +l modifier */
	}
	if (!GMT_access (GMT, arg, F_OK)) {	/* A file with this name exists */
		struct GMT_TEXTSET *Tin = NULL;
		uint64_t seg, row;
		if ((Tin = GMT_Read_Data (API, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, arg, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error reading time file %s\n", arg);
			return 0;
		}
		/* Read the file successfully */
		n_eval_times = (unsigned int)Tin->n_records;
		T = GMT_memory (GMT, NULL, n_eval_times, struct GMT_MODELTIME);	/* Array with times */
		for (seg = 0, k = 0; seg < Tin->table[0]->n_segments; seg++) {	/* Read in from possibly more than one segment */
			for (row = 0; row < Tin->table[0]->segment[seg]->n_rows; row++, k++) {
				s_time = gmt_get_modeltime (Tin->table[0]->segment[seg]->record[row], &s_unit, &s_scale);
				T[k].value = s_time;
				T[k].scale = s_scale;
				T[k].unit  = s_unit;
				T[k].u = (s_unit == 'M') ? 2 : ((s_unit == 'k') ? 1 : 0);
			}
		}
		if (GMT_Destroy_Data (API, &Tin) != GMT_OK) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error destroying data set after processing\n");
			return 0;
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Sort %u model times from old to young\n", n_eval_times);
		qsort (T, n_eval_times, sizeof (struct GMT_MODELTIME), compare_modeltimes);
	}
	else {	/* Gave times directly */
		char A[GMT_LEN32] = {""}, B[GMT_LEN32] = {""}, C[GMT_LEN32] = {""}, e_unit, i_unit;
		double e_time, i_time, e_scale, i_scale;
		int n = sscanf (arg, "%[^/]/%[^/]/%s", A, B, C);
		if (!(n == 3 || n == 1)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Must give -T<tfile>, -T<t0> or -T<t0>[u]/<t1>[u]/<dt>[u][+l]\n");
			return 0;
		}
		s_time = gmt_get_modeltime (A, &s_unit, &s_scale);
		if (n == 3) {	/* Gave an equidistant range of times */
			e_time = gmt_get_modeltime (B, &e_unit, &e_scale);
			i_time = gmt_get_modeltime (C, &i_unit, &i_scale);
			if (e_time > s_time) {	/* Enforce that old time is larger */
				double_swap (s_time,  e_time);
				double_swap (s_scale, e_scale);
				char_swap   (s_unit,  e_unit);
			}
			if (*log) {	/* Equidistant spacing in log10(time).  Here we got number of output points directly, compute log10 (increment) */
				n_eval_times = urint (i_time);
				i_time = (log10 (s_time) - log10 (e_time)) / (n_eval_times - 1);	/* Convert n to log10 (i_time) */
				T = GMT_memory (GMT, NULL, n_eval_times, struct GMT_MODELTIME);	/* Array with times */
				/* Pick the finest unit used for start and end times as the user unit (and scale) */
				if (s_unit == 'M' && e_unit != 'M') s_unit = e_unit, s_scale = e_scale;
				if (s_unit == 'k' && e_unit == 'y') s_unit = e_unit, s_scale = e_scale;
				for (k = 0; k < n_eval_times; k++)
					T[k].value = pow (10.0, log10 (s_time) - k * i_time);	/* In years */
			}
			else {	/* Equidistant spacing in time */
				n_eval_times = urint ((s_time - e_time) / i_time) + 1;
				/* Use the increment unit and scale for the array */
				s_unit = i_unit;	s_scale = i_scale;
				T = GMT_memory (GMT, NULL, n_eval_times, struct GMT_MODELTIME);	/* Array with times */
				for (k = 0; k < (n_eval_times-1); k++)
					T[k].value = s_time - k * i_time;	/* In years */
				T[k].value = e_time;	/* In years */
			}
		}
		else {	/* Gave a single time */
			n_eval_times = 1;
			T = GMT_memory (GMT, NULL, n_eval_times, struct GMT_MODELTIME);	/* Array with one time */
			T[0].value = s_time;
		}
		u = (s_unit == 'M') ? 2 : ((s_unit == 'k') ? 1 : 0);
		for (k = 0; k < n_eval_times; k++) {	/* Set constant unit and scale */
			T[k].unit  = s_unit;
			T[k].scale = s_scale;
			T[k].u = u;
		}
	}
	if (*log) p[0] = '+';	/* Restore the +l modifier */
	*T_array = T;
	return (n_eval_times);
}

char *gmt_modeltime_unit (unsigned int u)
{
	static char *names[3] = {"yr", "kyr", "Myr"};
	return (names[u]);
}

void gmt_modeltime_name (struct GMT_CTRL * GMT_UNUSED(GMT), char *file, char *format, struct GMT_MODELTIME *T)
{	/* Creates a filename from the format.  If %s is included we scale and append time units */
	if (strstr (format, "%s"))	/* Want unit name */
		sprintf (file, format, T->value*T->scale, gmt_modeltime_unit (T->u));
	else if (strstr (format, "%c"))	/* Want unit letter */
		sprintf (file, format, T->value*T->scale, T->unit);
	else	/* Just use time in years */
		sprintf (file, format, T->value);
}

double transfer_elastic_sub_iso (double *k, struct RHEOLOGY *R)
{	/* Elastic transfer function (isotropic) */
	double transfer_fn = 1.0 / (R->ce * pow (k[GMT_FFT_K_IS_KR], 4.0) + 1.0);
	return (transfer_fn);
}

double transfer_elastic_sub (double *k, struct RHEOLOGY *R)
{	/* Elastic transfer function (general) */
	double transfer_fn = 1.0 / (R->ce * pow (k[GMT_FFT_K_IS_KR], 4.0) + R->Nx_e * k[GMT_FFT_K_IS_KX] * k[GMT_FFT_K_IS_KX] + R->Ny_e * k[GMT_FFT_K_IS_KY] * k[GMT_FFT_K_IS_KY] + R->Nxy_e * k[GMT_FFT_K_IS_KX] * k[GMT_FFT_K_IS_KY] + 1.0);
	return (transfer_fn);
}

double transfer_elastic (double *k, struct RHEOLOGY *R)
{	/* Elastic transfer function */
	double transfer_fn = R->scale * R->tr_elastic_sub (k, R);
	return (transfer_fn);
}

void setup_elastic (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER * GMT_UNUSED(K), struct RHEOLOGY *R) {
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
	 * and amplitudes but scale airy_ratio by A to compensate for the lower load weight. */

	rho_load = Ctrl->D.rhol;
	if (Ctrl->S.active && Ctrl->S.beta < 1.0) {	/* Treat starved infill as approximate case with different infill density */
		Ctrl->D.approx = true;
		Ctrl->D.rhoi = Ctrl->S.beta * Ctrl->D.rhoi + Ctrl->D.rhow * (1.0 - Ctrl->S.beta);
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
	R->ce = rigidity_d / ( (Ctrl->D.rhom - rho_load) * NORMAL_GRAVITY);
	if (Ctrl->A.active) {	/* Specified inplane forces */
		R->Nx_e = Ctrl->A.Nx / ( (Ctrl->D.rhom - rho_load) * NORMAL_GRAVITY);
		R->Ny_e = Ctrl->A.Ny / ( (Ctrl->D.rhom - rho_load) * NORMAL_GRAVITY);
		R->Nxy_e = 2.0 * Ctrl->A.Nxy / ( (Ctrl->D.rhom - rho_load) * NORMAL_GRAVITY);
		R->isotropic = false;
		R->tr_elastic_sub = transfer_elastic_sub;
	}
	else {
		R->isotropic = true;
		R->tr_elastic_sub = transfer_elastic_sub_iso;
	}
	R->scale = -A * (rho_load - Ctrl->D.rhow)/(Ctrl->D.rhom - rho_load);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Elastic setup: R->scale = %g R->ce = %g R->Nx_e = %g R->Ny_e = %g R->Nyx_e = %g\n",
		R->scale, R->ce, R->Nx_e, R->Ny_e, R->Nxy_e);
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
	tau = R->cv * (k * (2.0 * R->nu_ratio * CS + (1.0 - R->nu_ratio) * lambda * lambda + R->nu_ratio * S2 + C2))
		/ ((R->nu_ratio + R->nu_ratio1) * CS + (R->nu_ratio - R->nu_ratio1) * lambda + S2 + C2);
	return (tau);
}

void setup_fv2 (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct RHEOLOGY *R)
{	/* Setup function for 2-layer viscous mantle beneath elastic plate */
	setup_elastic (GMT, Ctrl, K, R);	/* Both firmoviscous setups rely on the elastic setup */
	R->t0 = (R->relative) ?  R->eval_time_yr : R->load_time_yr - R->eval_time_yr;	/* Either relative to load time or both are absolute times */
	R->t0 *= (86400*365.25);	/* Convert to seconds */
	assert (R->t0 >= 0.0);
	R->nu_ratio = Ctrl->F.nu_a / Ctrl->F.nu_m;
	assert (R->nu_ratio > 0.0);
	R->nu_ratio1 = 1.0 / R->nu_ratio;
	R->cv = (2.0 * Ctrl->F.nu_m) / (Ctrl->D.rhom * NORMAL_GRAVITY);
	R->dens_ratio = (Ctrl->D.rhom - Ctrl->D.rhoi) / Ctrl->D.rhom;
	assert (R->dens_ratio > 0.0);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "FV2 setup: R->t0 = %g R->dens_ratio = %g R->nu_ratio = %g  R->nu_ratio1 = %g R->cv = %g\n",
		R->t0, R->dens_ratio, R->nu_ratio, R->nu_ratio1, R->cv);
}

double transfer_fv2 (double *k, struct RHEOLOGY *R)
{	/* Transfer function for 2-layer viscous mantle */
	double phi_e, phi_fv2, tau;
	phi_e = R->tr_elastic_sub (k, R);
	tau = relax_time_2 (k[GMT_FFT_K_IS_KR], R);
	phi_fv2 = phi_e * (1.0 - exp (-R->t0 * R->dens_ratio / (tau * phi_e)));
	return (R->scale * phi_fv2);
}

void setup_fv (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct RHEOLOGY *R)
{	/* Setup function for 1-layer viscous mantle beneath elastic plate */
	setup_elastic (GMT, Ctrl, K, R);	/* Both firmoviscous setups rely on the elastic setup */
	R->t0 = (R->relative) ?  R->eval_time_yr : R->load_time_yr - R->eval_time_yr;	/* Either relative to load time or both are absolute times */
	R->t0 *= (86400*365.25);	/* Convert to seconds */
	assert (R->t0 >= 0.0);
	R->dens_ratio = (Ctrl->D.rhom - Ctrl->D.rhoi) / Ctrl->D.rhom;
	assert (R->dens_ratio > 0.0);
	R->cv = (2.0 * Ctrl->F.nu_a) / (Ctrl->D.rhom * NORMAL_GRAVITY);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "FV Setup: R->t0 = %g R->dens_ratio = %g R->cv = %g\n", R->t0, R->dens_ratio, R->cv);
}

double transfer_fv (double *k, struct RHEOLOGY *R)
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
	phi_e = R->tr_elastic_sub (k, R);
	tau = k[GMT_FFT_K_IS_KR] * R->cv;
	if (k[GMT_FFT_K_IS_KR] == 0.0)
		phi_fv = phi_e;
	else
		phi_fv = phi_e * (1.0 - exp (-R->t0 * R->dens_ratio / (tau * phi_e)));
	return (R->scale * phi_fv);
}

void setup_ve (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct RHEOLOGY *R)
{
	setup_elastic (GMT, Ctrl, K, R);	/* Both firmoviscous setups rely on the elastic setup */
	R->cv = 1.0 / (Ctrl->M.maxwell_t * (86400*365.25));	/* Convert to seconds */
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "VE Setup: R->cv = %g, t_maxwell = %g%c\n", R->cv, Ctrl->M.maxwell_t * Ctrl->M.scale, Ctrl->M.unit);
}

double transfer_ve (double *k, struct RHEOLOGY *R)
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
	tau = R->t0 * R->cv;
	phi_e = R->tr_elastic_sub (k, R);
	phi_ve = 1.0 - (1.0 - phi_e) * exp (-tau * phi_e);
	return (R->scale * phi_ve);
}

void Apply_Transfer_Function (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct RHEOLOGY *R) {
	/* Do the spectral convolution for isostatic response in the Freq domain. */
	uint64_t k;
	double  mk[3], transfer_fn;
	//FILE *fp = fopen ("Crap.txt", "w");

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Apply the Transfer Function\n");
	R->setup (GMT, Ctrl, K, R);	/* Set up parameters */

	/* Loop over complex grid and multiply with the real transfer function */
	for (k = 0; k < Grid->header->size; k += 2) {
		if (R->isotropic)	/* No in-plane forcing */
			mk[GMT_FFT_K_IS_KR] = GMT_fft_get_wave (k, K);	/* Radial wavenumber */
		else {	/* Need kx, ky, and kr */
			mk[GMT_FFT_K_IS_KX] = GMT_fft_any_wave (k, GMT_FFT_K_IS_KX, K);		/* kx wavenumber */
			mk[GMT_FFT_K_IS_KY] = GMT_fft_any_wave (k, GMT_FFT_K_IS_KY, K);		/* kx wavenumber */
			mk[GMT_FFT_K_IS_KR] = hypot (mk[GMT_FFT_K_IS_KX], mk[GMT_FFT_K_IS_KY]);	/* kr wavenumber */
		}
		transfer_fn = R->transfer (mk, R);
		Grid->data[k] *= (float)transfer_fn;
		Grid->data[k+1] *= (float)transfer_fn;
		//fprintf (fp, "%g\t%g\t%g\t%g\n", mk[0], mk[1], mk[2], transfer_fn);
	}
	//fclose (fp);
}

int GMT_grdflexure_parse (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_OPTION *options) {

	unsigned int n_errors = 0, n_files = 0;
	int n;
	char A[GMT_LEN16] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {		/* Process all the options given */
		switch (opt->option) {

			case '<':	/* Input file */
				if (n_files++ > 0) break;
				if (strchr (opt->arg, '%')) {	/* File template given */
					Ctrl->In.many = true;
					Ctrl->In.file = strdup (opt->arg);
				}
				else if (opt->arg[0] == '=') {	/* List of files given */
					Ctrl->In.list = true;
					Ctrl->In.file = strdup (&opt->arg[1]);
				}
				else if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'A':	/* In-plane forces */
				Ctrl->A.active = true;
				n = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->A.Nx, &Ctrl->A.Ny, &Ctrl->A.Nxy);
				if (n != 3) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -A option: must give Nx/Ny/Nxy in-plane forces\n");
					n_errors++;
				}
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
				if (n == 3) {	/* Assume no rhoi given, shuffle args */
					Ctrl->D.rhow = Ctrl->D.rhoi;
					Ctrl->D.rhoi = Ctrl->D.rhol;
				}
				else if (Ctrl->D.rhol != Ctrl->D.rhoi)
					Ctrl->D.approx = true;
				break;
			case 'E':	/* Set elastic thickness */
				Ctrl->E.active = true;
				GMT_Get_Value (API, opt->arg, &Ctrl->E.te);
				if (Ctrl->E.te > 1e10) { /* Given flexural rigidity, compute Te from D */
					Ctrl->E.te = pow ((12.0 * (1.0 - Ctrl->C.nu * Ctrl->C.nu)) * Ctrl->E.te / Ctrl->C.E, 1.0/3.0);
				}
				break;
			case 'F':	/* Firmoviscous response selected */
				Ctrl->F.active = true;
				n = sscanf (opt->arg, "%lf/%[^/]/%lf", &Ctrl->F.nu_a, A, &Ctrl->F.nu_m);
				if (!(n == 3 || n == 1)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -F option: must select -F<nu> or -F<nu_a>/<h_a>/<nu_m>\n");
					n_errors++;
				}
				if (n == 3) {	/* 2-layer model selected */
					Ctrl->F.mode = FLX_FV2;
					GMT_Get_Value (API, A, &Ctrl->F.h_a);
				}
				else	/* 1-layer viscous model selected */
					Ctrl->F.mode = FLX_FV1;
				break;
			case 'G':	/* Output file name or template */
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'L':	/* Output file name with list of generated grids */
				Ctrl->L.active = true;
				if (opt->arg[0]) Ctrl->L.file = strdup (opt->arg);
				break;
			case 'M':	/* Viscoelastic Maxwell time [in year] */
				Ctrl->M.active = true;
				Ctrl->M.maxwell_t = gmt_get_modeltime (opt->arg, &Ctrl->M.unit, &Ctrl->M.scale);
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
				if ((Ctrl->T.n_eval_times = gmt_modeltime_array (GMT, opt->arg, &Ctrl->T.log, &Ctrl->T.time)) == 0)
					n_errors++;
				break;
			case 'W':	/* Water depth */
				Ctrl->W.active = true;
				GMT_Get_Value (API, opt->arg, &Ctrl->W.water_depth);
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
	n_errors += GMT_check_condition (GMT, !Ctrl->D.active, "Syntax error -E option: Must set elastic plate thickness regardless of rheology\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && (Ctrl->S.beta < 0.0 || Ctrl->S.beta > 1.0), "Syntax error -S option: beta value must be in 0-1 range\n");
	n_errors += GMT_check_condition (GMT, Ctrl->F.active && !Ctrl->T.active, "Syntax error -F option: Requires time information via -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && !Ctrl->T.active, "Syntax error -M option: Requires time information via -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.active && !Ctrl->T.active, "Syntax error -L option: Requires time information via -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && Ctrl->F.active, "Syntax error -M option: Cannot mix with -F\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && !strchr (Ctrl->G.file, '%'), "Syntax error -G option: Filename template must contain format specified\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->T.active && Ctrl->In.many, "Syntax error: Load template given but -T not specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int GMT_grdflexure_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdflexure <topogrid> -D<rhom>/<rhol>[/<rhoi>]/<rhow> -E<te> -G<outgrid> [-A<Nx/Ny/Nxy>] [-C[p|y]<value] [-F<nu_a>[/<h_a>/<nu_m>]]\n");
	GMT_Message (API, GMT_TIME_NONE,"\t[-L<list>] [-M<tm>] [-N%s] [-S<beta>] [-T<t0>[/<t1>/<dt>|<n>[+l]]]\n\t[%s] [-W<wd>] [-Z<zm>] [-fg]\n\n", GMT_FFT_OPT, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<topogrid> is the input grdfile with topography (load) values, in meters. If -T is used,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <topogrid> may be a filename template with a floating point format (C syntax) and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a different load file name will be set and loaded for each time step.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Time steps with no corresponding load file are allowed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, give =<flist> where <flist> contains a list of load grids and load times.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Sets density values for mantle, load(crust), optional moat infill [same as load], and water|air in kg/m^3.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Sets elastic plate thickness in m; append k for km.  If Te > 1e10 it will be interpreted\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   as the flexural rigidity [Default computes D from Te, Young's modulus, and Poisson's ratio].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G filename for output grdfile with flexed surface.  If -T is set then <outgrid>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   must be a filename template that contains a floating point format (C syntax) and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   we use the corresponding time (in units specified in -T) to generate the file name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If the floating point format is followed by %c then we scale time to unit in -T and append the unit.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Sets in-plane force components Nx, Ny and shear force Nxy [isotropic deformation].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Negative values mean compression, positive values mean extensional forces.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C use -Cy<Young> or -Cp<poisson> to change Young's modulus [%g] or Poisson's ratio [%g].\n", YOUNGS_MODULUS, POISSONS_RATIO);
	GMT_Message (API, GMT_TIME_NONE, "\t-F Sets upper mantle viscosity, and optionally its thickness and lower mantle viscosity.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Viscosity units in Pa s; thickness in meter (append k for km).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Give filename for output table with names of all grids (and model times) produced.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no filename is given then we write the list to stdout.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Set Maxwell time for visco-elastic flexure (unit is years; append k for kyr and M for Myr).\n");
	GMT_FFT_Option (API, 'N', GMT_FFT_DIM, "Choose or inquire about suitable grid dimensions for FFT, and set modifiers.");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Specify starved moat fraction in 0-1 range (1 = fully filled, 0 = no infill) [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify start, stop, and time increments for sequence of calculations [one step, no time dependency].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For a single specific time, just give <start>. unit is years; append k for kyr and M for Myr.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For a logarithmic time scale, append +l and specify n steps instead of time increment.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To read a list of times from the first column in a file instead, use -T<tfile>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note that time axis is positive back in time.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Specify water depth in m; append k for km.  Must be positive.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Subarial topography will be scaled by -D to account for density differences.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Specify reference depth to flexed surface in m; append k for km.  Must be positive.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Convert geographic grids to meters using a \"Flat Earth\" approximation.\n");
	GMT_Option (API, ".");
	return (EXIT_FAILURE);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdflexure_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

struct FLX_GRID *Prepare_Load (struct GMT_CTRL *GMT, struct GMT_OPTION *options, struct GRDFLEXURE_CTRL *Ctrl, char *file, struct GMT_MODELTIME *this_time)
{
	struct GMT_GRID *Grid = NULL, *Orig = NULL;
	struct FLX_GRID *G = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	if (this_time)
		GMT_Report (API, GMT_MSG_VERBOSE, "Prepare load file %s for time %g %s\n", file, this_time->value * this_time->scale, gmt_modeltime_unit (this_time->u));
	else
		GMT_Report (API, GMT_MSG_VERBOSE, "Prepare load file %s\n", file);

	if (!GMT_check_filearg (GMT, '<', file, GMT_IN, GMT_IS_DATASET)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Load file %s not found - skipped\n", file);
		return NULL;
	}
	/* Must initialize a new load grid */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Read load file %s\n", file);
	if ((Orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, file, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error reading the header of file %s - file skipped\n", file);
		return NULL;
	}
	GMT_grd_init (GMT, Orig->header, options, true);	/* Update the header */
	if ((Orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY |
 		GMT_GRID_IS_COMPLEX_REAL, NULL, file, Orig)) == NULL) {	/* Get data only */
		GMT_Report (API, GMT_MSG_NORMAL, "Error reading the data of file %s - file skipped\n", file);
		return NULL;
	}
	/* Note: If input grid is read-only then we must duplicate it; otherwise Grid points to Orig */
	(void) GMT_set_outgrid (API->GMT, file, Orig, &Grid);
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
	/* From here we address the grid via Grid; we are done with using the address Orig directly. */
	G = GMT_memory (GMT, NULL, 1, struct FLX_GRID);	/* Allocate a Flex structure */
	G->K = GMT_FFT_Create (API, Grid, GMT_FFT_DIM, GMT_GRID_IS_COMPLEX_REAL, Ctrl->N.info);	/* Also detrends, if requested */
	/* Do the forward FFT */
	GMT_Report (API, GMT_MSG_VERBOSE, "Forward FFT\n");
	if (GMT_FFT (API, Grid, GMT_FFT_FWD, GMT_FFT_COMPLEX, G->K)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error taking the FFT of %s - file skipped\n", file);
		return NULL;
	}
	G->Grid = Grid;	/* Pass grid back via the grid array */
	if (this_time) {	/* Deal with load time */
		G->Time = GMT_memory (GMT, NULL, 1, struct GMT_MODELTIME);	/* Allocate one Model time structure */
		GMT_memcpy (G->Time, this_time, 1, struct GMT_MODELTIME);	/* Just duplicate input time (unless NULL) */
	}
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

int compare_loads (const void *load_1v, const void *load_2v)
{
	/*  Routine for qsort to sort loads structure with old loads (large t) be first in list. */
	const struct FLX_GRID **load_1 = (const struct FLX_GRID **)load_1v, **load_2 = (const struct FLX_GRID **)load_2v;
	if ((*load_1)->Time->value > (*load_2)->Time->value) return (-1);
	if ((*load_1)->Time->value < (*load_2)->Time->value) return (+1);
	return (0);
}

int GMT_grdflexure (void *V_API, int mode, void *args) {
	unsigned int t_eval, t_load, n_load_times = 0;
	int error;
	bool retain_original;
	float *orig_load = NULL;
	char file[GMT_LEN256] = {""}, time_fmt[GMT_LEN64] = {""};

	struct GMT_FFT_WAVENUMBER *K = NULL;
	struct RHEOLOGY *R = NULL;
	struct FLX_GRID **Load = NULL, *This_Load = NULL;
	struct GMT_GRID *Out = NULL;
	struct GMT_TEXTSET *L = NULL;
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

	/*---------------------------- This is the grdflexure main code ----------------------------*/

	/* 1. SELECT THE TRANSFER FUNCTION TO USE */

	R = Select_Rheology (GMT, Ctrl);

	/* 2. READ ALL INPUT LOAD GRIDS, DETREND, AND TAKE FFT */

	if (Ctrl->In.many) {	/* Must read in load grids, possibly one for each time increment set by -T */
		n_load_times = Ctrl->T.n_eval_times;	/* This (or fewer) loads and times will be used */
		Load = GMT_memory (GMT, NULL, n_load_times, struct FLX_GRID *);	/* Allocate load array structure */
		for (t_load = 0; t_load < n_load_times; t_load++) {	/* For each time step there may be a load file */
			gmt_modeltime_name (GMT, file, Ctrl->In.file, &Ctrl->T.time[t_load]);	/* Load time equal eval time */
			Load[t_load] = Prepare_Load (GMT, options, Ctrl, file, &Ctrl->T.time[t_load]);
		}
	}
	else if (Ctrl->In.list) {	/* Must read a list of files and their load times (format: filename loadtime) */
		struct GMT_TEXTSET *Tin = NULL;
		struct GMT_MODELTIME this_time = {0.0, 0.0, 0, 0};
		uint64_t seg, row;
		double s_time, s_scale;
		char t_arg[GMT_LEN256] = {""}, s_unit;
		if ((Tin = GMT_Read_Data (API, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->In.file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error reading load file list %s\n", Ctrl->In.file);
			Return (API->error);
		}
		/* Read the file successfully */
		n_load_times = (unsigned int)Tin->n_records;
		Load = GMT_memory (GMT, NULL, n_load_times, struct FLX_GRID *);		/* Allocate load grid array structure */
		for (seg = 0, t_load = 0; seg < Tin->table[0]->n_segments; seg++) {	/* Read in from possibly more than one segment */
			for (row = 0; row < Tin->table[0]->segment[seg]->n_rows; row++, t_load++) {
				sscanf (Tin->table[0]->segment[seg]->record[row], "%s %s", file, t_arg);
				s_time = gmt_get_modeltime (t_arg, &s_unit, &s_scale);
				this_time.value = s_time;
				this_time.scale = s_scale;
				this_time.unit  = s_unit;
				this_time.u = (s_unit == 'M') ? 2 : ((s_unit == 'k') ? 1 : 0);
				Load[t_load] = Prepare_Load (GMT, options, Ctrl, file, &this_time);
			}
		}
		if (GMT_Destroy_Data (API, &Tin) != GMT_OK) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error destroying load file list after processing\n");
			Return (API->error);
		}
	}
	else {	/* Just read the single load grid */
		n_load_times = 1;
		Load = GMT_memory (GMT, NULL, n_load_times, struct FLX_GRID *);		/* Allocate grid array structure with one entry */
		Load[0] = Prepare_Load (GMT, options, Ctrl, Ctrl->In.file, NULL);	/* The single load grid (no time info) */
	}

	if (n_load_times > 1) {	/* Sort to ensure load array goes from old to young loads */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Sort %u loads from old to young\n", n_load_times);
		qsort (Load, n_load_times, sizeof (struct FLX_GRID *), compare_loads);
	}
	K = Load[0]->K;	/* We only need one pointer to get to wavenumbers as they are all the same for all grids */
	
	/* 3. DETERMINE AND POSSIBLY CREATE ONE OUTPUT GRID */

	retain_original = (n_load_times > 1 || Ctrl->T.n_eval_times > 1);	/* True when we will have to loop over the loads */
	if (retain_original) {	/* We may need to reuse loads for different times and will have to keep copy of unchanged H(kx,ky) */
		orig_load = GMT_memory (GMT, NULL, Load[0]->Grid->header->size, float);	/* Single temporary storage to hold one original H(kx,ky) grid */
		/* We must also allocate a separate output grid */
		if ((Out = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, Load[0]->Grid)) == NULL) Return (API->error);	/* Output grid of same size as input */
	}
	else	/* With a single load -> flexure operation we can just recycle the input grid for the output */
		Out = Load[0]->Grid;

	/* Here, Load[] contains all the input load grids and their laod times, ready to go as H(kx,ky) */

	if (Ctrl->L.active) {	/* Must create a textset to hold names of all output grids */
		uint64_t dim[3] = {1, 1, Ctrl->T.n_eval_times};
		unsigned int k, j;
		if ((L = GMT_Create_Data (API, GMT_IS_TEXTSET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error creating text set for file %s\n", Ctrl->L.file);
			Return (EXIT_FAILURE);
		}
		for (k = j = 0; Ctrl->G.file[k] && Ctrl->G.file[k] != '%'; k++);	/* Find first % */
		while (Ctrl->G.file[k] && !strchr ("efg", Ctrl->G.file[k])) time_fmt[j++] = Ctrl->G.file[k++];
		time_fmt[j++] = Ctrl->G.file[k];
		strcat (time_fmt, "%c");	/* Append the unit */
		GMT_Report (API, GMT_MSG_DEBUG, "Format for time will be %s\n", time_fmt);
	}

	for (t_eval = 0; t_eval < Ctrl->T.n_eval_times; t_eval++) {	/* For each time step (i.e., at least once) */

		/* 4a. SET THE CURRENT TIME VALUE (IF USED) */
		if (Ctrl->T.active) {	/* Set the current time in user units as well as years */
			R->eval_time_yr = Ctrl->T.time[t_eval].value;		/* In years */
			GMT_Report (API, GMT_MSG_VERBOSE, "Evaluating flexural deformation for time %g %s\n", Ctrl->T.time[t_eval].value * Ctrl->T.time[t_eval].scale, gmt_modeltime_unit (Ctrl->T.time[t_eval].u));
		}

		if (retain_original) GMT_memset (Out->data, Out->header->size, float);	/* Reset output grid to zero; not necessary when we only get here once */

		for (t_load = 0; t_load < n_load_times; t_load++) {	/* For each load  */
			This_Load = Load[t_load];	/* Short-hand for current load */
			if (This_Load == NULL) continue;	/* Quietly skip times with no load */
			if (Ctrl->T.active && Ctrl->T.time && This_Load->Time) {	/* Has both load time and evaluation time so can check some more */
				if (This_Load->Time->value < Ctrl->T.time[t_eval].value) continue;	/* Skip future loads */
			}
			if (Ctrl->T.active && This_Load->Time) {	/* Has time so we can report on what is happening */
				R->load_time_yr = This_Load->Time->value;	/* In years */
				R->relative = false;	/* Absolute times are given */
				if (This_Load->Time->u == Ctrl->T.time[t_eval].u) {	/* Same time units even */
					double dt = This_Load->Time->value * This_Load->Time->scale - Ctrl->T.time[t_eval].value * Ctrl->T.time[t_eval].scale;
					GMT_Report (API, GMT_MSG_VERBOSE, "  Accumulating flexural deformation for load emplaced at time %g %s [Loading time = %g %s]\n",
						This_Load->Time->value * This_Load->Time->scale, gmt_modeltime_unit (This_Load->Time->u),
						dt, gmt_modeltime_unit (This_Load->Time->u));
				}
				else {	/* Just state load time */
					GMT_Report (API, GMT_MSG_VERBOSE, "  Accumulating flexural deformation for load emplaced at time %g %s\n",
						This_Load->Time->value * This_Load->Time->scale, gmt_modeltime_unit (This_Load->Time->u));
				}
			}
			else {
				R->load_time_yr = 0.0;	/* Not given, assume R->eval_time_yr is time since loading */
				R->relative = true;	/* Relative times are given */
				GMT_Report (API, GMT_MSG_VERBOSE, "  Accumulating flexural deformation for load # %d emplaced at unspecified time\n", t_load);
			}
			/* 4b. COMPUTE THE RESPONSE DUE TO THIS LOAD */
			if (retain_original) GMT_memcpy (orig_load, This_Load->Grid->data, This_Load->Grid->header->size, float);	/* Make a copy of H(kx,ky) before operations */
			Apply_Transfer_Function (GMT, This_Load->Grid, Ctrl, This_Load->K, R);	/* Multiplies H(kx,ky) by transfer function, yielding W(kx,ky) */
			if (retain_original) {	/* Must add this contribution to our total output grid */
				Accumulate_Solution (GMT, Out, This_Load->Grid);
				GMT_memcpy (This_Load->Grid->data, orig_load, This_Load->Grid->header->size, float);	/* Restore H(kx,ky) to what it was before operations */
			}
		}

		/* 4c. TAKE THE INVERSE FFT TO GET w(x,y) */
		GMT_Report (API, GMT_MSG_VERBOSE, "Inverse FFT\n");
		if (GMT_FFT (API, Out, GMT_FFT_INV, GMT_FFT_COMPLEX, K))
			Return (EXIT_FAILURE);

		/* 4d. APPLY SCALING AND OFFSET */
		GMT_scale_and_offset_f (GMT, Out->data, Out->header->size, 1.0, -Ctrl->Z.zm);

		/* 4d. WRITE OUTPUT GRID */
		if (Ctrl->T.active) { /* Separate output grid since there are many time steps */
			char remark[GMT_GRID_REMARK_LEN160] = {""};
			gmt_modeltime_name (GMT, file, Ctrl->G.file, &Ctrl->T.time[t_eval]);
			sprintf (remark, "Solution for t = %g %s", Ctrl->T.time[t_eval].value * Ctrl->T.time[t_eval].scale,
				gmt_modeltime_unit (Ctrl->T.time[t_eval].u));
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK|GMT_COMMENT_IS_RESET, remark, Out))
				Return (API->error);
		}
		else	/* Single output grid */
			strcpy (file, Ctrl->G.file);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out))
			Return (API->error);

		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY |
			GMT_GRID_IS_COMPLEX_REAL, NULL, file, Out) != GMT_OK) {	/* This demuxes the grid before writing! */
				Return (API->error);
		}
		if (t_eval < (Ctrl->T.n_eval_times-1)) {	/* Must put the total grid back into interleave mode */
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Re-multiplexing complex grid before accumulating new increments.\n");
			GMT_grd_mux_demux (GMT, Out->header, Out->data, GMT_GRID_IS_INTERLEAVED);
		}

		if (Ctrl->L.active) {	/* Add filename and evaluation time to list */
			char record[GMT_BUFSIZ] = {""}, tmp[GMT_LEN64] = {""};
			if (Ctrl->T.active) {
				sprintf (record, "%s\t", file);
				sprintf (tmp, time_fmt, Ctrl->T.time[t_eval].value * Ctrl->T.time[t_eval].scale, Ctrl->T.time[t_eval].unit);
				strcat (record, tmp);
			}
			else
				sprintf (record, "%s", file);
			L->table[0]->segment[0]->record[t_eval] = strdup (record);
			L->table[0]->segment[0]->n_rows++;
		}
	}

	if (Ctrl->L.active && GMT_Write_Data (API, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_NONE, 0, NULL, Ctrl->L.file, L) != GMT_OK) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error writing list of grid files to %s\n", Ctrl->L.file);
		Return (API->error);
	}

	/* 5. FREE ALL GRIDS AND ARRAYS */

	for (t_load = 0; t_load < n_load_times; t_load++) {	/* Free up grid structures */
		This_Load = Load[t_load];	/* Short-hand for current load */
		if (This_Load == NULL) continue;	/* Quietly skip containers with no grids */
		GMT_Destroy_Data (API, &This_Load->Grid);
		GMT_free (GMT, This_Load->K);
		if (This_Load->Time) GMT_free (GMT, This_Load->Time);
		GMT_free (GMT, This_Load);
	}
	GMT_free (GMT, Load);
	GMT_free (GMT, R);
	if (retain_original) GMT_free (GMT, orig_load);

	GMT_Report (API, GMT_MSG_VERBOSE, "Done!\n");

	Return (EXIT_SUCCESS);
}
