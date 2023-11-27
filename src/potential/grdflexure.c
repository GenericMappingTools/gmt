/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Author:      Paul Wessel
 * Date:        16-OCT-2015
 *
 *
 * Various isostatic calculations with user-selectable rheologies.  We support
 * 1. Elastic response
 * 2. Viscoelastic response (Maxwell)
 * 3. General Linear substance (viscoelastic response)
 * 4. Firmoviscous (elastic over viscous halfspace) response
 * 5. Firmoviscous (elastic over viscous layer over viscous halfspace) response
 * 5. Viscous (viscous halfspace) response
 * 5. Viscous (viscous layer over viscous halfspace) response
 *
 * */

#include "gmt_dev.h"
#include "longopt/grdflexure_inc.h"
#include "modeltime.h"

#define THIS_MODULE_CLASSIC_NAME	"grdflexure"
#define THIS_MODULE_MODERN_NAME	"grdflexure"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Compute flexural deformation of 3-D surfaces for various rheologies"
#define THIS_MODULE_KEYS	"<G{,GG},LD),TD("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-Vfh"

#define GMT_FFT_DIM	2	/* Dimension of FFT needed */

#define FLX_A	0	/* Airy */
#define FLX_E	1	/* Elastic */
#define FLX_VE	2	/* Viscoelastic */
#define FLX_GL	3	/* General Linear model */
#define FLX_FV1	4	/* Firmoviscous 1-layer */
#define FLX_FV2	5	/* Firmoviscous 2-layer */
#define FLX_V1	6	/* Viscous 1-layer */
#define FLX_V2	7	/* Viscous 2-layer */

#define TE_INIT	0
#define TE_END	1

struct GRDFLEXURE_CTRL {
	struct GRDFLEXURE_In {	/* Input load file, template, <list>+l or <list>.lis (i.e. with .lis extension) */
		bool active, many, list;
		char *file;
	} In;
	struct GRDFLEXURE_A {	/* -A<Nx/Ny/Nxy> In-plane forces */
		bool active;
		double Nx, Ny, Nxy;
	} A;
	struct GRDFLEXURE_C {	/* -Cy<E> or -Cp<poisson> */
		bool active[2];
		double E, nu;
	} C;
	struct GRDFLEXURE_D {	/* -D<rhom/rhol[/rhoi]/rhow>[+r<rhor] */
		bool active, approx, var_rhol, root;
		unsigned int mode;
		double rhom, rhol, rhoi, rhow, rhor;
	} D;
	struct GRDFLEXURE_E {	/* -E<te>[/<te2] */
		bool active;
		bool two;
		double te[2];
	} E;
	struct GRDFLEXURE_F {	/* -F<nu> or -F<nu_a>/<h_a>/<nu_m> */
		bool active;
		unsigned int mode;
		double nu_a, nu_m, h_a;
	} F;
	struct GRDFLEXURE_G {	/* -G<outfile> */
		bool active;
		char *file;
	} G;
	struct GRDFLEXURE_H {	/* -H<rhofile> */
		bool active, many;
		char *file;
	} H;
	struct GRDFLEXURE_L {	/* -L<outlist> */
		bool active;
		char *file;
	} L;
	struct GRDFLEXURE_M {	/* -M<maxwell_t>  */
		bool active;
		double maxwell_t;	/* Maxwell time */
		double scale;		/* scale for time */
		char unit;		/* Unit of time */
	} M;
	struct GRDFLEXURE_N {	/* -N[f|q|s<n_columns>/<n_rows>][+e|m|n][+t<width>][+w[<suffix>]][+z[p]]  */
		bool active;
		struct GMT_FFT_INFO *info;
	} N;
	struct GRDFLEXURE_Q {	/* Dump transfer functions */
		bool active;
	} Q;
	struct GRDFLEXURE_S {	/* Starved moat */
		bool active;
		double beta;	/* Fraction of moat w(x) filled in [1] */
	} S;
	struct GRDFLEXURE_T {	/* -T[l]<t0>/<t1>/<d0>|n  */
		bool active, log;
		unsigned int n_eval_times;
		struct GMT_MODELTIME *time;	/* The current sequence of times */
	} T;
	struct GRDFLEXURE_W {	/* Water depth */
		bool active;
		double water_depth;	/* Reference water depth [0] */
	} W;
	struct GRDFLEXURE_Z {	/* Moho depth */
		bool active;
		double zm;	/* Reference depth to flexed surface [0] */
	} Z;
};

struct GRDFLEXURE_RHEOLOGY {	/* Used to pass parameters in/out of functions */
	unsigned int mode;	/* Which rheological model is in effect (FLX_?) */
	double eval_time_yr;	/* Time in years of evaluation or relative time since loading */
	double load_time_yr;	/* Time in years of loading, or zero */
	double t0;		/* Time in seconds since loading */
	double D_ratio;	/* Ratio of initial to final rigidities for general linear VE model */
	double nu_ratio;	/* Ratio of asthenosphere to lower mantle viscosities */
	double nu_ratio1;	/* The inverse ratio */
	double h_a;		/* The thickness of the asthenosphere (m) */
	double ce[2];		/* One or two constants for elastic transfer functions */
	double Nx_e;		/* A constant for --"-- that is nonzero when Nx is nonzero */
	double Ny_e;		/* A constant for --"-- that is nonzero when Ny is nonzero */
	double Nxy_e;		/* A constant for --"-- that is nonzero when Nxy is nonzero */
	double cv;		/* A constant for viscous transfer functions */
	double scale;		/* Overall scale (e.g., Airy scale) */
	double dens_ratio;	/* (Ctrl->D.rhom - Ctrl->D.rhoi) / Ctrl->D.rhom */
	bool relative;		/* eval_time_yr is relative to load time [at 0] */
	bool isotropic;		/* true when no inplane forces are set (no -A) */
	double (*transfer) (double *, struct GRDFLEXURE_RHEOLOGY *);	/* pointer to function returning isostatic response for given k and R */
	double (*tr_elastic_sub) (double *, struct GRDFLEXURE_RHEOLOGY *);	/* pointer to sub-function returning elastic isostatic response for given k and R */
	void (*setup) (struct GMT_CTRL *, struct GRDFLEXURE_CTRL *, struct GRDFLEXURE_RHEOLOGY *);	/* Init function */
};

struct GRDFLEXURE_GRID {
	struct GMT_GRID *Grid;		/* Pointer to the grid, or NULL if it does not exist */
	struct GMT_MODELTIME *Time;	/* Pointer to time info for this load */
	struct GMT_FFT_WAVENUMBER *K;	/* Pointer to FFT struct, unless G is NULL */
	double rho_load;			/* Density of this load layer (may be constant or variable with time) */
};

#define	YOUNGS_MODULUS	7.0e10		/* Pascal = Nt/m**2  */
#define	NORMAL_GRAVITY	9.806199203	/* Moritz's 1980 IGF value for gravity at 45 degrees latitude (m/s) */
#define	POISSONS_RATIO	0.25

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDFLEXURE_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDFLEXURE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->C.E = YOUNGS_MODULUS;
	C->C.nu = POISSONS_RATIO;
	C->S.beta = 1.0;
	C->T.n_eval_times = 1;

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->L.file);
	gmt_M_free (GMT, C->N.info);
	gmt_M_free (GMT, C->T.time);
	gmt_M_free (GMT, C);
}

double gmt_get_modeltime (char *A, char *unit, double *scale) {
	/* Convert age[k|M] to years, return unit and scale needed to convert year back to time in given unit */
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

GMT_LOCAL unsigned int grdflexure_modeltime_tag (struct GMT_CTRL *GMT, struct GMT_MODELTIME *T, unsigned int n, char *format) {
	/* Determine a suitable format statement for the time tags in grdseamount and grdflexure.
	 * These use the most compact time unit and are consistent in the number of decimals */
	char unit[2] = {""};	/* Unless we have unit times there is no unit in the time tag */
	int n_dec;
	unsigned int u = 0;
	double this_inc, inc = (n == 1) ? T[0].value : DBL_MAX;
	for (unsigned int t = 0; t < (n-1); t++) {	/* Look at all intervals */
		this_inc = T[t].value - T[t+1].value;	/* Current time increment */
		if (this_inc < inc) inc = this_inc;		/* Keep the smallest time increment in years */
	}
	this_inc = (n == 1) ? inc : T[0].value - T[n-1].value;	/* Now holds duration of construction */
	if (this_inc >= 1.0e6)	/* Use Myr if duration exceeds 1 Myr  */
		inc /= 1.0e6,	unit[0] = 'M', u = 2;
	else if (this_inc >= 1.0e3)	/* Use kyr if duration exceeds 1kyr */
		inc /= 1.0e3,	unit[0] = 'k', u = 1;
	/* else we use time as is - probably dummy times */
	n_dec = MAX (0, -irint (floor (log10 (inc))));	/* For increments >= 1 we want 0 decimals */
	sprintf (format, "%%.%df", n_dec);	/* Numerical part of the format */
	if (unit[0]) strcat (format, unit);	/* Optionally append a unit */
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Format for time tags will be %s based on smallest increment %g and duration %g\n", format, inc, this_inc);
	return (u);	/* Returns 0, 1 or 2 */
}

GMT_LOCAL int grdflexure_compare_modeltimes (const void *time_1v, const void *time_2v) {
	/*  Routine for qsort to sort model times array so old times (large t) will be first in list. */
	const struct GMT_MODELTIME *time_1 = time_1v, *time_2 = time_2v;
	if (time_1->value > time_2->value) return (-1);
	if (time_1->value < time_2->value) return (+1);
	return (0);
}

unsigned int gmt_modeltime_array (struct GMT_CTRL *GMT, char *arg, bool *log, struct GMT_MODELTIME **T_array) {
	/* Parse -T<tfile>, -T<t0> or -T<t0>/<t1>/<dt>[+l] and return array of times.
	 * The array times are all in years, while the unit and scale can change.  Programs that need
	 * the time in year should use T_array[k].value while programs that need the original time and
	 * unit specified by the user should use T_array[k].value * T_array[k].scale and T_array[k].unit.
	 * If a log-equidistant range is specified via +l we set *log to true, else false.  The function
	 * returns the number of times in T_array.
	 */
	char *p = NULL, s_unit, time_fmt[GMT_LEN64] = {""};
	char unit[3] = {'\0', 'k', 'M'};
	unsigned int n_eval_times = 0, k, u = 0;
	double s_time, s_scale, scale[3] = {1.0, 1.0e-3, 1.0e-6};
	struct GMTAPI_CTRL *API = GMT->parent;
	struct GMT_MODELTIME *T = NULL;

	*log = false;
	if ((p = strstr (arg, "+l")) != NULL) {	/* Want logarithmic time scale */
		*log = true;
		p[0] = '\0';	/* Chop off the +l modifier */
	}
	if (!gmt_access (GMT, arg, F_OK)) {	/* A file with this name exists */
		struct GMT_DATASET *Tin = NULL;
		uint64_t seg, row;
		if ((Tin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, arg, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while reading time file %s\n", arg);
			return 0;
		}
		/* Read the file successfully */
		n_eval_times = (unsigned int)Tin->n_records;
		T = gmt_M_memory (GMT, NULL, n_eval_times, struct GMT_MODELTIME);	/* Array with times */
		for (seg = 0, k = 0; seg < Tin->table[0]->n_segments; seg++) {	/* Read in from possibly more than one segment */
			for (row = 0; row < Tin->table[0]->segment[seg]->n_rows; row++, k++) {
				s_time = gmt_get_modeltime (Tin->table[0]->segment[seg]->text[row], &s_unit, &s_scale);
				T[k].value = s_time;
				T[k].scale = s_scale;
				T[k].unit  = s_unit;
				T[k].u = (s_unit == 'M') ? 2 : ((s_unit == 'k') ? 1 : 0);
			}
		}
		if (GMT_Destroy_Data (API, &Tin) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while destroying data set after processing\n");
			return 0;
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Sort %u model times from old to young\n", n_eval_times);
		qsort (T, n_eval_times, sizeof (struct GMT_MODELTIME), grdflexure_compare_modeltimes);
	}
	else {	/* Gave times directly */
		char A[GMT_LEN32] = {""}, B[GMT_LEN32] = {""}, C[GMT_LEN32] = {""}, e_unit, i_unit;
		double e_time, i_time, e_scale, i_scale;
		int n = sscanf (arg, "%[^/]/%[^/]/%s", A, B, C);
		if (!(n == 3 || n == 1)) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -T: Must give -T<tfile>, -T<t0> or -T<t0>/<t1>/<dt>[+l]\n");
			return 0;
		}
		s_time = gmt_get_modeltime (A, &s_unit, &s_scale);
		if (n == 3) {	/* Gave an equidistant range of times */
			e_time = gmt_get_modeltime (B, &e_unit, &e_scale);
			i_time = gmt_get_modeltime (C, &i_unit, &i_scale);
			if (e_time > s_time) {	/* Enforce that old time is larger */
				gmt_M_double_swap (s_time,  e_time);
				gmt_M_double_swap (s_scale, e_scale);
				gmt_M_char_swap (s_unit,  e_unit);
			}
			if (*log) {	/* Equidistant spacing in log10(time).  Here we got number of output points directly, compute log10 (increment) */
				n_eval_times = urint (i_time);
				i_time = (log10 (s_time) - log10 (e_time)) / (n_eval_times - 1);	/* Convert n to log10 (i_time) */
				T = gmt_M_memory (GMT, NULL, n_eval_times, struct GMT_MODELTIME);	/* Array with times */
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
				T = gmt_M_memory (GMT, NULL, n_eval_times, struct GMT_MODELTIME);	/* Array with times */
				for (k = 0; k < (n_eval_times-1); k++)
					T[k].value = s_time - k * i_time;	/* In years */
				T[k].value = e_time;	/* In years */
			}
		}
		else {	/* Gave a single time */
			n_eval_times = 1;
			T = gmt_M_memory (GMT, NULL, n_eval_times, struct GMT_MODELTIME);	/* Array with one time */
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
	/* Now must find consistent unit for time tags since input may be a mix of kyr and Myr */
	u = grdflexure_modeltime_tag (GMT, T, n_eval_times, time_fmt);	/* Get suitable format given all increments */
	for (k = 0; k < n_eval_times; k++) {	/* Create the time tags */
		T[k].unit = unit[u];
		T[k].scale = scale[u];
		if (T[k].unit)	/* Need trailing unit */
			sprintf (T[k].tag, time_fmt, T[k].value * T[k].scale, T[k].unit);
		else
			sprintf (T[k].tag, time_fmt, T[k].value * T[k].scale);
	}
	*T_array = T;
	return (n_eval_times);
}

GMT_LOCAL char *grdflexure_modeltime_unit (unsigned int u) {
	static char *names[3] = {"yr", "kyr", "Myr"};
	return (names[u]);
}

int gmt_modeltime_validate (struct GMT_CTRL *GMT, char option, char *file) {
	/* Make sure any template names pass muster */
	unsigned int n_percent;
	if (file == NULL) return (GMT_NOERROR);	/* Not a filename */
	if (strchr (file, '%') == NULL) return (GMT_NOERROR);	/* Not a template */
	n_percent = gmt_count_char (GMT, file, '%');
	if (strstr (file, "%s") && n_percent > 1) {	/* Want tag but gave more than one format */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -%c: To use a formatted time tag, only %%s is allowed in the template\n", option);
		return (GMT_PARSE_ERROR);
	}
	else if (strstr (file, "%c")) {	/* Want unit appended */
		if (n_percent != 2) {	/* Want unit appended but did not give two formats */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -%c: To use appended time unit, the template must have a leading %% format for a floating point value and then the %%c\n", option);
			return (GMT_PARSE_ERROR);
		}
	}
	else if (n_percent != 1) {	/* Just numerical but gave more than one */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -%c: Only a single %% format for a floating point value is expected\n", option);
		return (GMT_PARSE_ERROR);
	}
	return (GMT_NOERROR);
}

void gmt_modeltime_name (struct GMT_CTRL *GMT, char *file, char *format, struct GMT_MODELTIME *T) {
	/* Creates a filename from the format.  If %s is included we scale and append time units */
	gmt_M_unused(GMT);
	if (strstr (format, "%s"))	/* Want tag */
		sprintf (file, format, T->tag);
	else if (strstr (format, "%c"))	/* Want unit letter */
		sprintf (file, format, T->value*T->scale, T->unit);
	else	/* Just use time in years */
		sprintf (file, format, T->value);
}

GMT_LOCAL double grdflexure_mean_load_density (struct GMT_CTRL *GMT, struct GMT_GRID *G, struct GMT_GRID *R) {
	/* Compute the weighted mean value of the density grid (weighted by the load heights).
	 * This constant density is then used later to scale the load heights with variable rho
	 * to the equivalent heights given this fixed density */
	uint64_t node;
	double rz_sum = 0.0, z_sum = 0.0;
	for (node = 0; node < G->header->size; node++) {
		if (gmt_M_is_fnan (G->data[node]) || gmt_M_is_fnan (R->data[node])) continue;	/* Skip useless NaNs */
		rz_sum += G->data[node] * R->data[node];
		z_sum  += G->data[node];
	}
	return ((z_sum > 0.0) ? rz_sum / z_sum : GMT->session.d_NaN);
}

GMT_LOCAL double grdflexure_transfer_elastic_sub_iso (double *k, struct GRDFLEXURE_RHEOLOGY *R) {
	/* Elastic transfer function (isotropic) */
	double grdflexure_transfer_fn = 1.0 / (R->ce[TE_INIT] * pow (k[GMT_FFT_K_IS_KR], 4.0) + 1.0);
	return (grdflexure_transfer_fn);
}

GMT_LOCAL double grdflexure_transfer_elastic_sub_iso2 (double *k, struct GRDFLEXURE_RHEOLOGY *R) {
	/* Elastic transfer function (isotropic) for end Te*/
	double grdflexure_transfer_fn = 1.0 / (R->ce[TE_END] * pow (k[GMT_FFT_K_IS_KR], 4.0) + 1.0);
	return (grdflexure_transfer_fn);
}

GMT_LOCAL double grdflexure_transfer_elastic_sub (double *k, struct GRDFLEXURE_RHEOLOGY *R) {
	/* Elastic transfer function (general) */
	double grdflexure_transfer_fn = 1.0 / (R->ce[TE_INIT] * pow (k[GMT_FFT_K_IS_KR], 4.0) + R->Nx_e * k[GMT_FFT_K_IS_KX] * k[GMT_FFT_K_IS_KX] + R->Ny_e * k[GMT_FFT_K_IS_KY] * k[GMT_FFT_K_IS_KY] + R->Nxy_e * k[GMT_FFT_K_IS_KX] * k[GMT_FFT_K_IS_KY] + 1.0);
	return (grdflexure_transfer_fn);
}

GMT_LOCAL double grdflexure_transfer_elastic (double *k, struct GRDFLEXURE_RHEOLOGY *R) {
	/* Elastic transfer function */
	double grdflexure_transfer_fn = R->scale * R->tr_elastic_sub (k, R);
	return (grdflexure_transfer_fn);
}

GMT_LOCAL double grdflexure_transfer_Airy (double *k, struct GRDFLEXURE_RHEOLOGY *R) {
	/* Airy transfer function (isotropic and independent of k) */
	double grdflexure_transfer_fn = R->scale;
	gmt_M_unused (k);
	return (grdflexure_transfer_fn);
}

GMT_LOCAL void grdflexure_setup_elastic (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GRDFLEXURE_RHEOLOGY *R) {
	/* Do the isostatic response function convolution in the Freq domain.
	   All units assumed to be in SI (that is kx, ky, modk wavenumbers in m**-1,
	   densities in kg/m**3, Te in m, etc.
	   rw, the water density, is used to set the Airy ratio and the restoring
	   force on the plate (rm - ri)*gravity if ri = rw; so use zero for topo in air (ri changed to rl).
	*/
	double  A = 1.0, rho_load, rigidity_d, rigidity_de;

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
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Starved moat with beta = %g implies an effective rho_i  = %g\n", Ctrl->S.beta, Ctrl->D.rhol);
	}
	else if (rho_load != Ctrl->D.rhoi)	/* In case rho_load was derived from a density grid */
		Ctrl->D.approx = true;

	if (Ctrl->D.approx) {	/* Do approximate calculation when both rhol and rhoi were set */
		char way = (Ctrl->D.rhoi < Ctrl->D.rhol) ? '<' : '>';
		if (Ctrl->E.te[TE_INIT] > 0.0) GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Approximate FFT-solution to flexure since rho_i (%g) %c rho_l (%g)\n", Ctrl->D.rhoi, way, Ctrl->D.rhol);
		rho_load = Ctrl->D.rhoi;
		A = sqrt ((Ctrl->D.rhom - Ctrl->D.rhoi)/(Ctrl->D.rhom - Ctrl->D.rhol));
	}
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Using effective load density rho_l = %g and Airy boost factor A = %g\n", rho_load, A);
	R->scale = -A * (rho_load - Ctrl->D.rhow)/(Ctrl->D.rhom - rho_load);
	if (gmt_M_is_zero (Ctrl->E.te[TE_INIT])) {	/* Just Airy, so rigidity etc are all zero */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Airy setup: R->scale = %g\n", R->scale);
		return;
	}

	rigidity_d = (Ctrl->C.E * Ctrl->E.te[TE_INIT] * Ctrl->E.te[TE_INIT] * Ctrl->E.te[TE_INIT]) / (12.0 * (1.0 - Ctrl->C.nu * Ctrl->C.nu));
	R->ce[TE_INIT] = rigidity_d / ( (Ctrl->D.rhom - rho_load) * NORMAL_GRAVITY);
	if (Ctrl->A.active) {	/* Specified in-plane forces */
		R->Nx_e = Ctrl->A.Nx / ( (Ctrl->D.rhom - rho_load) * NORMAL_GRAVITY);
		R->Ny_e = Ctrl->A.Ny / ( (Ctrl->D.rhom - rho_load) * NORMAL_GRAVITY);
		R->Nxy_e = 2.0 * Ctrl->A.Nxy / ( (Ctrl->D.rhom - rho_load) * NORMAL_GRAVITY);
		R->isotropic = false;
		R->tr_elastic_sub = grdflexure_transfer_elastic_sub;
	}
	else {
		R->isotropic = true;
		R->tr_elastic_sub = grdflexure_transfer_elastic_sub_iso;
	}
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Elastic setup: R->scale = %g D = %g R->ce[TE_INIT] = %g R->Nx_e = %g R->Ny_e = %g R->Nyx_e = %g\n",
		R->scale, rigidity_d, R->ce[TE_INIT], R->Nx_e, R->Ny_e, R->Nxy_e);
	if (Ctrl->E.two) {	/* Got two elastic thickness for general linear VE model */
		rigidity_de = (Ctrl->C.E * Ctrl->E.te[TE_END] * Ctrl->E.te[TE_END] * Ctrl->E.te[TE_END]) / (12.0 * (1.0 - Ctrl->C.nu * Ctrl->C.nu));
		R->ce[TE_END] = rigidity_de / ( (Ctrl->D.rhom - rho_load) * NORMAL_GRAVITY);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Elastic setup for 2nd Te: D = %g R->ce[TE_END] = %g\n", rigidity_de, R->ce[TE_END]);
		R->D_ratio = rigidity_d / rigidity_de;
	}
}

GMT_LOCAL double grdflexure_relax_time_2 (double k, struct GRDFLEXURE_RHEOLOGY *R) {
	/*  grdflexure_relax_time_2 evaluates relaxation time(k) of 2-layer viscous mantle
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
	tau = R->cv * ((R->nu_ratio + R->nu_ratio1) * CS + (R->nu_ratio - R->nu_ratio1) * lambda + S2 + C2)
		/ ((k * (2.0 * R->nu_ratio * CS + (1.0 - R->nu_ratio) * lambda * lambda + R->nu_ratio * S2 + C2)));
	if (gmt_M_is_dnan (tau)) tau = 0.0;	/* Blew up due to lambda being too large */
	return (tau);
}

GMT_LOCAL void grdflexure_setup_fv2 (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GRDFLEXURE_RHEOLOGY *R) {
	/* Setup function for 2-layer viscous mantle beneath elastic plate */
	grdflexure_setup_elastic (GMT, Ctrl, R);	/* Both firmoviscous setups rely on the elastic setup */
	R->t0 = (R->relative) ?  R->eval_time_yr : R->load_time_yr - R->eval_time_yr;	/* Either relative to load time or both are absolute times */
	R->t0 *= (86400*365.25);	/* Convert to seconds */
	assert (R->t0 >= 0.0);
	R->h_a = Ctrl->F.h_a;
	R->nu_ratio = Ctrl->F.nu_a / Ctrl->F.nu_m;
	assert (R->nu_ratio > 0.0);
	R->nu_ratio1 = 1.0 / R->nu_ratio;
	R->cv = (Ctrl->D.rhom * NORMAL_GRAVITY) / (2.0 * Ctrl->F.nu_m);
	R->dens_ratio = (Ctrl->D.rhom - Ctrl->D.rhoi) / Ctrl->D.rhom;
	assert (R->dens_ratio > 0.0);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "FV2 setup: R->t0 = %g R->dens_ratio = %g R->nu_ratio = %g  R->nu_ratio1 = %g R->cv = %g\n",
		R->t0, R->dens_ratio, R->nu_ratio, R->nu_ratio1, R->cv);
}

GMT_LOCAL double grdflexure_transfer_fv2 (double *k, struct GRDFLEXURE_RHEOLOGY *R) {
	/* Transfer function for 2-layer viscous mantle */
	double phi_e, phi_fv2, tau;
	phi_e = R->tr_elastic_sub (k, R);
	tau = grdflexure_relax_time_2 (k[GMT_FFT_K_IS_KR], R);
	phi_fv2 = phi_e * (1.0 - exp (-R->t0 * R->dens_ratio * tau / phi_e));
	return (R->scale * phi_fv2);
}

GMT_LOCAL void grdflexure_setup_fv (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GRDFLEXURE_RHEOLOGY *R) {
	/* Setup function for 1-layer viscous mantle beneath elastic plate */
	grdflexure_setup_elastic (GMT, Ctrl, R);	/* Both firmoviscous setups rely on the elastic setup */
	R->t0 = (R->relative) ?  R->eval_time_yr : R->load_time_yr - R->eval_time_yr;	/* Either relative to load time or both are absolute times */
	R->t0 *= (86400*365.25);	/* Convert to seconds */
	assert (R->t0 >= 0.0);
	R->dens_ratio = (Ctrl->D.rhom - Ctrl->D.rhoi) / Ctrl->D.rhom;
	assert (R->dens_ratio > 0.0);
	R->cv = (Ctrl->D.rhom * NORMAL_GRAVITY) / (2.0 * Ctrl->F.nu_m);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "FV Setup: R->t0 = %g R->dens_ratio = %g R->cv = %g\n", R->t0, R->dens_ratio, R->cv);
}

GMT_LOCAL double grdflexure_transfer_fv (double *k, struct GRDFLEXURE_RHEOLOGY *R) {
/*	Transfer function for elastic plate over viscous half-space.  Give:
 *
 *	k	- wavenumbers (1/m)
 *	rhom	- density of mantle (kg/m^3)
 *	rhoi	- density of infill material (kg/m^3)
 *	te	- elastic plate thickness (km)
 *	nu_m	- mantle viscosity (Pa s)
 *	t0	- time since loading (s)
 */
	double phi_e, phi_fv, tau;
	phi_e = R->tr_elastic_sub (k, R);
	//tau = k[GMT_FFT_K_IS_KR] * R->cv;
	tau =  R->cv / k[GMT_FFT_K_IS_KR];
	if (k[GMT_FFT_K_IS_KR] == 0.0)
		phi_fv = phi_e;
	else
		phi_fv = phi_e * (1.0 - exp (-R->t0 * R->dens_ratio * tau / phi_e));
	return (R->scale * phi_fv);
}

GMT_LOCAL void grdflexure_setup_ve (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GRDFLEXURE_RHEOLOGY *R) {
	grdflexure_setup_elastic (GMT, Ctrl, R);	/* Viscoelastic setups rely on the elastic setup */
	R->t0 = (R->relative) ?  R->eval_time_yr : R->load_time_yr - R->eval_time_yr;	/* Either relative to load time or both are absolute times */
	R->cv = 1.0 / Ctrl->M.maxwell_t;
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "VE Setup: R->cv = %g, t_maxwell = %g%c\n", R->cv, Ctrl->M.maxwell_t * Ctrl->M.scale, Ctrl->M.unit);
}

GMT_LOCAL double grdflexure_transfer_ve (double *k, struct GRDFLEXURE_RHEOLOGY *R) {
/*	Transfer function for VE plate.  Give:
 *
 *	k	- wavenumbers (1/m)
 *	rhom	- density of mantle (kg/m^3)
 *	rhoi	- density of infill material (kg/m^3)
 *	te	- elastic plate thickness (km)
 *	T	- Maxwell time (s)
 *	t0	- time since loading (s)
 */
	double phi_e, phi_ve, tau;
	tau = R->t0 * R->cv;
	phi_e = R->tr_elastic_sub (k, R);
	phi_ve = 1.0 - (1.0 - phi_e) * exp (-tau * phi_e);
	return (R->scale * phi_ve);
}

GMT_LOCAL void grdflexure_setup_gl (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GRDFLEXURE_RHEOLOGY *R) {
	grdflexure_setup_elastic (GMT, Ctrl, R);	/* Viscoelastic setups rely on the elastic setup */
	R->t0 = (R->relative) ?  R->eval_time_yr : R->load_time_yr - R->eval_time_yr;	/* Either relative to load time or both are absolute times */
	R->cv = 1.0 / Ctrl->M.maxwell_t;
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "GL Setup: R->cv = %g, t_maxwell = %g%c\n", R->cv, Ctrl->M.maxwell_t * Ctrl->M.scale, Ctrl->M.unit);
}

GMT_LOCAL double grdflexure_transfer_gl (double *k, struct GRDFLEXURE_RHEOLOGY *R) {
/*	Transfer function for general linear VE plate.  Give:
 *
 *	k	- wavenumbers (1/m)
 *	rhom	- density of mantle (kg/m^3)
 *	rhoi	- density of infill material (kg/m^3)
 *	te_init	- initial elastic plate thickness (km)
 *	te_end	- final elastic plate thickness (km)
 *	T	- Maxwell time (s)
 *	t0	- time since loading (s)
 */
	double phi_ei, phi_ef, phi_gl, tau;
	tau = R->t0 * R->cv;
	phi_ei = grdflexure_transfer_elastic_sub_iso (k, R);
	phi_ef = grdflexure_transfer_elastic_sub_iso2 (k, R);
	phi_gl = phi_ef + (phi_ei - phi_ef) * exp (-tau * R->D_ratio * phi_ei / phi_ef);
	return (R->scale * phi_gl);
}

GMT_LOCAL double grdflexure_transfer_v (double *k, struct GRDFLEXURE_RHEOLOGY *R) {
/*	Transfer function for viscous half-space.  Give:
 *
 *	k	- wavenumbers (1/m)
 *	rhom	- density of mantle (kg/m^3)
 *	rhoi	- density of infill material (kg/m^3)
 *	nu_m	- mantle viscosity (Pa s)
 *	t0	- time since loading (yr)
 */
	double phi_v, tau;
	tau =  R->cv / k[GMT_FFT_K_IS_KR];
	if (k[GMT_FFT_K_IS_KR] == 0.0)
		phi_v = 1.0;
	else
		phi_v = 1.0 - exp (-R->t0 * R->dens_ratio * tau);
	return (R->scale * phi_v);
}

GMT_LOCAL double grdflexure_transfer_v2 (double *k, struct GRDFLEXURE_RHEOLOGY *R) {
	/* Transfer function for 2-layer viscous mantle */
	double phi_v2, tau;
	tau = grdflexure_relax_time_2 (k[GMT_FFT_K_IS_KR], R);
	phi_v2 = 1.0 - exp (-R->t0 * R->dens_ratio * tau);
	return (R->scale * phi_v2);
}

GMT_LOCAL void grdflexure_apply_transfer_function (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_FFT_WAVENUMBER *K, struct GRDFLEXURE_RHEOLOGY *R) {
	/* Do the spectral convolution for isostatic response in the Freq domain. */
	uint64_t k;
	double  mk[3], grdflexure_transfer_fn;

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Apply the Transfer Function\n");
	R->setup (GMT, Ctrl, R);	/* Set up parameters */

	/* Loop over complex grid and multiply with the real transfer function */
	for (k = 0; k < Grid->header->size; k += 2) {
		if (R->isotropic)	/* No in-plane forcing */
			mk[GMT_FFT_K_IS_KR] = gmt_fft_get_wave (k, K);	/* Radial wavenumber */
		else {	/* Need kx, ky, and kr */
			mk[GMT_FFT_K_IS_KX] = gmt_fft_any_wave (k, GMT_FFT_K_IS_KX, K);		/* kx wavenumber */
			mk[GMT_FFT_K_IS_KY] = gmt_fft_any_wave (k, GMT_FFT_K_IS_KY, K);		/* kx wavenumber */
			mk[GMT_FFT_K_IS_KR] = hypot (mk[GMT_FFT_K_IS_KX], mk[GMT_FFT_K_IS_KY]);	/* kr wavenumber */
		}
		grdflexure_transfer_fn = R->transfer (mk, R);
		Grid->data[k] *= (gmt_grdfloat)grdflexure_transfer_fn;
		Grid->data[k+1] *= (gmt_grdfloat)grdflexure_transfer_fn;
	}
}

static int parse (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GMT_OPTION *options) {

	unsigned int n_errors = 0, n_files = 0;
	int k, n;
	char A[GMT_LEN16] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {		/* Process all the options given */
		switch (opt->option) {

			case '<':	/* Input file(s) */
				if (n_files++ > 0) break;
				if (strchr (opt->arg, '%')) {	/* Grid file template given */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->In.many);
					n_errors += gmt_get_required_string (GMT, opt->arg, opt->option, 0, &Ctrl->In.file);
					n_errors += gmt_modeltime_validate (GMT, '<', Ctrl->In.file);
				}
				else if ((c = strstr (opt->arg, "+l"))) {	/* File with list of load files given, marked with modifier +l */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->In.list);
					c[0] = '\0';	/* Hide the modifier */
					n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file));
					c[0] = '+';	/* Restore modifier */
				}
				else if ((c = strrchr (opt->arg, '.')) && !strcmp (c, ".lis")) {	/* File with list of load files given, identified via .lis extension */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->In.list);
					n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file));
				}
				else {	/* Single grid file for a static load */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->In.active);
					n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file));
				}
				break;
			case 'A':	/* In-plane forces */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				n = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->A.Nx, &Ctrl->A.Ny, &Ctrl->A.Nxy);
				if (n != 3) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -A: must give Nx/Ny/Nxy in-plane forces\n");
					n_errors++;
				}
				break;
			case 'C':	/* Rheology constants E and nu */
				switch (opt->arg[0]) {
					case 'p':
						n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active[0]);
						n_errors += gmt_get_required_double (GMT, &opt->arg[1], opt->option, 0, &Ctrl->C.nu);
						break;
					case 'y':
						n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active[1]);
						n_errors += gmt_get_required_double (GMT, &opt->arg[1], opt->option, 0, &Ctrl->C.E);
						break;
					default:
						GMT_Report (API, GMT_MSG_ERROR, "Option -C: Unrecognized modifier %c\n", opt->arg[0]);
						n_errors++;
						break;
				}
				break;
			case 'D':	/* Set densities */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				if ((c = strstr (opt->arg, "+r"))) {	/* Got constant root [load] density */
					Ctrl->D.rhor = atof (&c[2]);
					if (Ctrl->D.rhor < 10.0) Ctrl->D.rhor *= 1000;	/* Gave units of g/cm^3 */
					Ctrl->D.root = true;
					c[0] = '\0';	/* Hide modifier */
				}
				n = sscanf (opt->arg, "%lf/%[^/]/%lf/%lf", &Ctrl->D.rhom, A, &Ctrl->D.rhoi, &Ctrl->D.rhow);
				if (!(n == 4 || n == 3)) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -D: must give 3-4 density values\n");
					n_errors++;
				}
				if (!strcmp (A, "-"))	/* Have variable load density via grids */
					Ctrl->D.var_rhol = true;
				else
					Ctrl->D.rhol = atof (A);
				if (Ctrl->D.rhom < 10.0) Ctrl->D.rhom *= 1000;	/* Gave units of g/cm^3 */
				if (Ctrl->D.rhol < 10.0) Ctrl->D.rhol *= 1000;	/* Gave units of g/cm^3 */
				if (Ctrl->D.rhoi < 10.0) Ctrl->D.rhoi *= 1000;	/* Gave units of g/cm^3 */
				if (Ctrl->D.rhow < 10.0) Ctrl->D.rhow *= 1000;	/* Gave units of g/cm^3 */
				if (n == 3) {	/* Assume no rhoi given, shuffle args */
					Ctrl->D.rhow = Ctrl->D.rhoi;
					Ctrl->D.rhoi = Ctrl->D.rhol;
				}
				else if (Ctrl->D.rhol > 0.0 && Ctrl->D.rhol != Ctrl->D.rhoi)
					Ctrl->D.approx = true;
				if (c) c[0] = '+';	/* Restore modifier */
				break;
			case 'E':	/* Set elastic thickness(es) */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				if (opt->arg[0]) {
					double val[2];
					n = GMT_Get_Values (API, opt->arg, val, 2);
					for (k = 0; k < n; k++) {
						Ctrl->E.te[k] = val[k];
						if (Ctrl->E.te[k] > 1e10) { /* Given flexural rigidity, compute Te from D */
							Ctrl->E.te[k] = pow ((12.0 * (1.0 - Ctrl->C.nu * Ctrl->C.nu)) * Ctrl->E.te[k] / Ctrl->C.E, 1.0/3.0);
						}
					}
					if (n == 2) Ctrl->E.two = true;
				}
				break;
			case 'F':	/* Firmoviscous response selected */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				n = sscanf (opt->arg, "%lf/%[^/]/%lf", &Ctrl->F.nu_a, A, &Ctrl->F.nu_m);
				if (!(n == 3 || n == 1)) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -F: must select -F<nu> or -F<nu_a>/<h_a>/<nu_m>\n");
					n_errors++;
				}
				if (n == 3) {	/* 2-layer model selected */
					Ctrl->F.mode = FLX_FV2;
					GMT_Get_Values (API, A, &Ctrl->F.h_a, 1);
				}
				else {	/* 1-layer viscous model selected */
					Ctrl->F.mode = FLX_FV1;
					Ctrl->F.nu_m = Ctrl->F.nu_a;
					Ctrl->F.nu_a = 0.0;
				}
				break;
			case 'G':	/* Output file name or template */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file));
				n_errors += gmt_modeltime_validate (GMT, 'G', Ctrl->G.file);
				break;
			case 'H':	/* Input density grid name or template */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->H.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_GRID, GMT_IN, GMT_FILE_LOCAL, &(Ctrl->H.file));
				n_errors += gmt_modeltime_validate (GMT, 'H', Ctrl->H.file);
				if (strchr (opt->arg, '%'))	/* Got a filename template */
					Ctrl->H.many = true;
				break;
			case 'L':	/* Output file name with list of generated grids */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				if (opt->arg[0]) Ctrl->L.file = strdup (opt->arg);
				break;
			case 'M':	/* Viscoelastic Maxwell time [in year] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				Ctrl->M.maxwell_t = gmt_get_modeltime (opt->arg, &Ctrl->M.unit, &Ctrl->M.scale);
				break;
			case 'N':	/* FFT parameters */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.info = GMT_FFT_Parse (API, 'N', GMT_FFT_DIM, opt->arg);
				if (Ctrl->N.info == NULL) n_errors++;
				break;
			case 'Q':	/* Dump transfer functions */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'S':	/* Starved basin */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				n_errors += gmt_get_required_double (GMT, opt->arg, opt->option, 0, &Ctrl->S.beta);
				break;
			case 'T':	/* Time lattice */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				if ((Ctrl->T.n_eval_times = gmt_modeltime_array (GMT, opt->arg, &Ctrl->T.log, &Ctrl->T.time)) == 0)
					n_errors++;
				break;
			case 'W':	/* Water depth */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				GMT_Get_Values (API, opt->arg, &Ctrl->W.water_depth, 1);
				break;
			case 'Z':	/* Moho depth */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				n_errors += gmt_get_required_double (GMT, opt->arg, opt->option, 0, &Ctrl->Z.zm);
				break;
			default:
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	if (Ctrl->N.info && Ctrl->N.active && Ctrl->N.info->info_mode == GMT_FFT_LIST) {
		return (GMT_PARSE_ERROR);	/* So that we exit the program */
	}

	n_errors += gmt_M_check_condition (GMT, !Ctrl->D.active, "Option -D: Must set density values\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->E.active, "Option -E: Must set elastic plate thickness\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && Ctrl->F.active, "Option -A: Unknown if -A will work correctly with -F\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && Ctrl->F.active, "Option -M: Cannot mix with -F\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && (Ctrl->S.beta < 0.0 || Ctrl->S.beta > 1.0),
	                                 "Option -S: beta value must be in 0-1 range\n");
	if (!Ctrl->Q.active) {	/* Unless just writing transfer function we must insist on some more tests */
		n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Must specify input file\n");
		n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file,  "Option -G: Must specify output file\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && !Ctrl->T.active, "Option -F: Requires time information via -T\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->E.two && !Ctrl->M.active, "Option -E: General linear VE requires a Maxwell time via -M\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && !Ctrl->T.active, "Option -M: Requires time information via -T\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && !Ctrl->T.active, "Option -L: Requires time information via -T\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && !strchr (Ctrl->G.file, '%'),
	                                 "Option -G: Filename template must contain format specified\n");
		n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active && Ctrl->In.many, "Load template given but -T not specified\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->H.active && !Ctrl->D.var_rhol, "Option -H: Requires rho_l = - in -D\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->H.active && Ctrl->H.many != Ctrl->In.many, "Option -H: Rho grid must be similar to <topogrid>\n");
	}

	if (Ctrl->A.active) {
		if (Ctrl->F.active)
			GMT_Report (API, GMT_MSG_WARNING, "Option -A: Unknown if -A will work correctly with -F\n");
		if (Ctrl->M.active)
			GMT_Report (API, GMT_MSG_WARNING, "Option -A: Unknown if -M will work correctly with -F\n");
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s <input> -D<rhom>/<rhol>[/<rhoi>]/<rhow>[+r<rhor>] -E[<te>[k][/<te2>[k]]] -G%s [-A<Nx/Ny/Nxy>] [-Cp|y<value>] [-F<nu_a>[/<h_a>[k]/<nu_m>]] "
		"[-H<rhogrid>] [-L<list>] [-M<tm>[k|M]] [-N%s] [-Q] [-S<beta>] [-T<t0>[/<t1>/<dt>[+l]]|<file>] [%s] [-W<wd>[k]] [-Z<zm>[k]] [-fg] [%s] [%s]\n",
		name, GMT_OUTGRID, GMT_FFT_OPT, GMT_V_OPT, GMT_ho_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n<input> gives information about the topographic load(s) in any of these formats:");
	GMT_Usage (API, 3, "%s A single static topography grid, in meters.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s A grid filename template with a floating point format (C syntax). "
		"Requires -T so a different load file name will be set and used for each time step "
		"(time steps with no corresponding load file are allowed).", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s A table <list>+l, where <list> records have load times and grids from grdseamount -M.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s A table <list>.lis, again with records from grdseamount -M.", GMT_LINE_BULLET);
	GMT_Usage (API, 1, "\n-D<rhom>/<rhol>[/<rhoi>]/<rhow>[+r<rhor>]");
	GMT_Usage (API, -2, "Set density of mantle, load(crust), optional moat infill [same as load], and water|air in kg/m^3 or g/cm^3. "
		"Set <rhol> to - if -H is used or if the input <list> contains variable density grid names.");
	GMT_Usage (API, 3, "+r Set a fixed root density. The load topography will be scaled to reflect new load density rhor.");
	GMT_Usage (API, 1, "\n-E[<te>[k][/<te2>[k]]]");
	GMT_Usage (API, -2, "Sets elastic plate thickness in m; append k for km.  If Te > 1e10 it will be interpreted "
		"as the flexural rigidity [Default computes D from Te, Young's modulus, and Poisson's ratio]. "
		"Default of 0 km may be used with -F for a pure viscous response (no plate rigidity). "
		"Select General Linear Viscoelastic model by giving initial and final elastic thicknesses (requires -M).");
	gmt_outgrid_syntax (API, 'G', "Filename for output grid with flexed surface.  If -T is set then <outgrid> "
		"must be a filename template that contains a floating point format (C syntax) and "
		"we use the corresponding time (in units specified in -T) to generate the file name. "
		"If the floating point format is followed by %%c then we scale time to unit in -T and append the unit.");

	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A<Nx/Ny/Nxy>");
	GMT_Usage (API, -2, "Set in-plane force components <Nx>, <Ny> and shear force <Nxy> in Pa*m [isotropic deformation]. "
		"Negative values mean compression, positive values mean extensional forces.");
	GMT_Usage (API, 1, "\n-Cp|y<value>");
	GMT_Usage (API, -2, "Set rheological constants given the directive (repeatable):");
	GMT_Usage (API, 3, "y: Append a value to change Young's modulus [%g].", YOUNGS_MODULUS);
	GMT_Usage (API, 3, "p: Append a value to change Poisson's ratio [%g].", POISSONS_RATIO);
	GMT_Usage (API, 1, "\n-F<nu_a>[/<h_a>[k]/<nu_m>]");
	GMT_Usage (API, -2, "Set upper mantle viscosity, and optionally its thickness and lower mantle viscosity. "
		"Viscosity units in Pa s; thickness in meter (append k for km).");
	GMT_Usage (API, 1, "\n-H<rhogrid>");
	GMT_Usage (API, -2, "Give filename for optional variable density grid. Requires rho_l = - in -D. If <topogrid> "
		"Was given as a filename template then <roghrid> must be similar.");
	GMT_Usage (API, 1, "\n-L<list>");
	GMT_Usage (API, -2, "Give filename for output table with names of all grids (and model times) produced. "
		"If no filename is given then we write the list to standard output.");
	GMT_Usage (API, 1, "\n-M<tm>[k|M]");
	GMT_Usage (API, -2, "Set Maxwell time for viscoelastic flexure (in years; append k for kyr and M for Myr).");
	GMT_FFT_Option (API, 'N', GMT_FFT_DIM, "Choose or inquire about suitable grid dimensions for FFT, and set modifiers.");
	GMT_Usage (API, 1, "\n-Q No flexure. Evaluate and write the chosen response functions Q(k[,t]) for parameters lambda = "
		"1-3000 km, Te = 1,2,5,10,20,50 km, and t = 1k,2k,5k,10k,20k,50k,100k,200k,500k,1M,2M,5M years.");
	GMT_Usage (API, 1, "\n-S<beta>");
	GMT_Usage (API, -2, "Starved moat fraction ranging from 0 (no infill) to 1 (fully filled) [1].");
	GMT_Usage (API, 1, "\n-T<t0>[/<t1>/<dt>[+l]]|<file>");
	GMT_Usage (API, -2, "Specify start, stop, and time increments for sequence of calculations [one step, no time dependency]. "
		"For a single specific time, just give <t0> (in years; append k for kyr and M for Myr).");
	GMT_Usage (API, 3, "+l For a logarithmic time scale, interpret <dt> as n_steps instead of time increment.");
	GMT_Usage (API, -2, "Alternatively, read a list of times from the first column in a file instead by appending <tfile>. "
		"Note: The time axis is positive back in time.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-W<wd>[k]");
	GMT_Usage (API, -2, "Specify water depth in m; append k for km.  Must be positive. "
		"Subaerial topography will be scaled via -D to account for density differences.");
	GMT_Usage (API, 1, "\n-Z<zm>[k]");
	GMT_Usage (API, -2, "Specify reference depth to flexed surface in m; append k for km.  Must be positive.");
	GMT_Usage (API, 1, "\n-fg Convert geographic grids to meters using a \"Flat Earth\" approximation.");
	GMT_Option (API, "h,.");
	return (GMT_MODULE_USAGE);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LOCAL struct GRDFLEXURE_GRID *grdflexure_prepare_load (struct GMT_CTRL *GMT, struct GMT_OPTION *options, struct GRDFLEXURE_CTRL *Ctrl, char *file, char *rho, struct GMT_MODELTIME *this_time) {
	uint64_t node;
	double boost = 1.0, mean_rho_l = 0.0, mean_rho_l_actual = 0.0, mean_rho_l_reported = 0.0;
	struct GMT_GRID *Grid = NULL, *Orig = NULL, *Rho = NULL;
	struct GRDFLEXURE_GRID *G = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	if (this_time)
		GMT_Report (API, GMT_MSG_INFORMATION, "Prepare load file %s for time %g %s\n", file, this_time->value * this_time->scale, grdflexure_modeltime_unit (this_time->u));
	else
		GMT_Report (API, GMT_MSG_INFORMATION, "Prepare load file %s\n", file);

	if (GMT_Get_FilePath (GMT->parent, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE|GMT_FILE_CHECK, &file))  {
		GMT_Report (API, GMT_MSG_ERROR, "Load file %s not found - skipped\n", file);
		return NULL;
	}
	/* Must initialize a new load grid */
	GMT_Report (API, GMT_MSG_INFORMATION, "Read load file %s\n", file);
	if ((Orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, file, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Failure while reading the header of file %s - file skipped\n", file);
		return NULL;
	}
	gmt_grd_init (GMT, Orig->header, options, true);	/* Update the header */
	if ((Orig = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY |
 		GMT_GRID_IS_COMPLEX_REAL, NULL, file, Orig)) == NULL) {	/* Get data only */
		GMT_Report (API, GMT_MSG_ERROR, "Failure while reading the data of file %s - file skipped\n", file);
		return NULL;
	}
	/* Note: If input grid is read-only then we must duplicate it; otherwise Grid points to Orig */
	(void) gmt_set_outgrid (API->GMT, file, false, 0, Orig, &Grid);
	HH = gmt_get_H_hidden (Grid->header);
	if (HH->has_NaNs == GMT_GRID_HAS_NANS) {	/* Must deal with load NaNs */
		uint64_t n_NaN = 0;
		/* Ensure any NaNs are set to 0 here. This can happen with data from grdseamount, for instance. We cannot have NaNs when doing FFTs */
		for (node = 0; node < Grid->header->size; node++)
			if (gmt_M_is_fnan (Grid->data[node])) Grid->data[node] = 0.0, n_NaN++;	/* Outside the load, probably */
		HH->has_NaNs = GMT_GRID_NO_NANS;	/* Since we replaced them */
		GMT_Report (API, GMT_MSG_INFORMATION, "Load grid %s had %" PRIu64 " NaNs, these were replaced with zeros\n", file, n_NaN);
	}
	if (Ctrl->D.var_rhol) {	/* Must load variable density grid, get mean load density, and scale height accordingly */
		if ((Rho = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, rho, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while reading the header of file %s - file skipped\n", rho);
			return NULL;
		}
		if (!gmt_grd_domains_match (GMT, Grid, Rho, "load and density")) {	/* Not co-registered */
			return NULL;
		}
		/* Note: Density grid are allowed to have NaNs outside the feature since we are not taking FFT of the density grid */
		if ((Rho = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY |
	 		GMT_GRID_IS_COMPLEX_REAL, NULL, rho, Rho)) == NULL) {	/* Get header and data */
			GMT_Report (API, GMT_MSG_ERROR, "Failure while reading the data of file %s - file skipped\n", rho);
			return NULL;
		}
		if (strstr (Rho->header->remark, "Mean Load Density:")) {	/* Got the remark about mean load density */
			char *c = strchr (Rho->header->remark, ':');	/* Get pointer to start of colon: and next allow for 1 space */
			mean_rho_l_reported = atof (&c[1]);	/* Get the reported mean load density */
			GMT_Report (API, GMT_MSG_INFORMATION, "Extracted mean load density of %lg from file %s\n", mean_rho_l_reported, rho);
		}
		else
			GMT_Report (API, GMT_MSG_INFORMATION, "No mean load density attribute found in %s\n", rho);
		mean_rho_l_actual = grdflexure_mean_load_density (GMT, Grid, Rho);
		if (mean_rho_l_reported > 0.0 && fabs (mean_rho_l_reported - mean_rho_l_actual) > 0.005) {	/* Allow for some slop ~1e-5 */
			GMT_Report (API, GMT_MSG_INFORMATION, "Reported and actual mean for %s differ: %lg vs %lg - the latter will be used\n",
				rho, mean_rho_l_reported, mean_rho_l_actual);
			mean_rho_l = mean_rho_l_actual;	/* We use the mean density we actually computed */
		}
		else
			mean_rho_l = mean_rho_l_reported;	/* We use the reported mean density */
		boost = 1.0 / mean_rho_l;	/* To avoid division below */
		GMT_Report (API, GMT_MSG_INFORMATION, "Convert h(x,y) and rho(x,y) to equivalent height h'(x,y) for fixed density %lg\n", mean_rho_l);
		for (node = 0; node < Rho->header->size; node++) {
			if (gmt_M_is_fnan (Rho->data[node])) continue;	/* Outside the load */
			Grid->data[node] *= (gmt_grdfloat)(Rho->data[node] * boost);
		}
		/* Now load amplitudes have been adjusted to correspond to a common load density mean_rho_l */
	}
	else
		mean_rho_l = Ctrl->D.rhol;	/* Constant load density */

	if (Ctrl->W.active) {	/* See if any part of the load sticks above water, and if so scale this amount as if it was submerged */
		uint64_t n_subaerial = 0;
		double boost = mean_rho_l / (mean_rho_l - Ctrl->D.rhow);	/* For constant density case */
		for (node = 0; node < Grid->header->size; node++) {
			if (Grid->data[node] > Ctrl->W.water_depth) {	/* Sticking above the water line */
				Grid->data[node] = (gmt_grdfloat)(Ctrl->W.water_depth + (Grid->data[node] - Ctrl->W.water_depth) * boost);
				n_subaerial++;
			}
		}
		if (n_subaerial) GMT_Report (API, GMT_MSG_WARNING, "%" PRIu64 " nodes were subaerial so heights were scaled for the equivalent submerged case\n", n_subaerial);
	}
	if (Ctrl->D.root) {	/* Need to adjust h(x,y) to reflect the same load density as the root */
		boost = mean_rho_l / Ctrl->D.rhor;	/* To avoid division below */
		GMT_Report (API, GMT_MSG_INFORMATION, "Convert h(x,y) and rho_l to equivalent height h'(x,y) for rho_l = rho_r via scale %lg\n", boost);
		for (node = 0; node < Grid->header->size; node++)
			Grid->data[node] *= boost;
		mean_rho_l = Ctrl->D.rhor;
	}
	/* Free any density grid */
	if (Ctrl->D.var_rhol && GMT_Destroy_Data (API, &Rho) != GMT_NOERROR)
		return NULL;

	/* Here, topography have possibly been adjusted to reflect the same uniform density in both the edifice and immediate
	 * root below. This load density may still differ from the infill density if that was set separately via -D */

	/* From here we address the grid via Grid; we are done with using the address Orig directly. */
	G = gmt_M_memory (GMT, NULL, 1, struct GRDFLEXURE_GRID);	/* Allocate a Flex structure */
	G->K = GMT_FFT_Create (API, Grid, GMT_FFT_DIM, GMT_GRID_IS_COMPLEX_REAL, Ctrl->N.info);	/* Also detrends, if requested */
	/* Do the forward FFT */
	GMT_Report (API, GMT_MSG_INFORMATION, "Forward FFT\n");
	if (GMT_FFT (API, Grid, GMT_FFT_FWD, GMT_FFT_COMPLEX, G->K)) {
		GMT_Report (API, GMT_MSG_ERROR, "Failure while taking the FFT of %s - file skipped\n", file);
		return NULL;
	}
	G->Grid = Grid;	/* Pass grid back via the grid array */
	if (this_time) {	/* Deal with load time */
		G->Time = gmt_M_memory (GMT, NULL, 1, struct GMT_MODELTIME);	/* Allocate one Model time structure */
		gmt_M_memcpy (G->Time, this_time, 1, struct GMT_MODELTIME);	/* Just duplicate input time (unless NULL) */
	}
	if (Ctrl->D.var_rhol || Ctrl->D.root) G->rho_load = mean_rho_l;	/* Update what the actual load density now is */
	return (G);
}

GMT_LOCAL struct GRDFLEXURE_RHEOLOGY *grdflexure_select_rheology (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl) {
	unsigned int fmode = 0;
	struct GRDFLEXURE_RHEOLOGY *R = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	/* Select the transfer function to use */
	if (Ctrl->F.active) {		/* One of two firmoviscous functions */
		fmode = Ctrl->F.mode;
		if (gmt_M_is_zero (Ctrl->E.te[TE_INIT])) fmode += 2;
	}
	else if (Ctrl->M.active)	/* Viscoelastic */
		fmode = FLX_VE;
	else if (Ctrl->E.te[TE_INIT] > 0.0)		/* Elastic */
		fmode = FLX_E;
	else
		fmode = (Ctrl->Q.active) ? FLX_E : FLX_A;	/* Airy unless -Q -E */

	R = gmt_M_memory (GMT, NULL, 1, struct GRDFLEXURE_RHEOLOGY);	/* Allocate rheology structure */

	switch (fmode) {	/* Set function pointers */
		case FLX_A:
			GMT_Report (API, GMT_MSG_INFORMATION, "Selected Airy transfer function\n");
			R->setup = grdflexure_setup_elastic;	R->transfer = grdflexure_transfer_Airy;		break;
		case FLX_E:
			GMT_Report (API, GMT_MSG_INFORMATION, "Selected Elastic transfer function\n");
			R->setup = grdflexure_setup_elastic;	R->transfer = grdflexure_transfer_elastic;		break;
		case FLX_VE:
			GMT_Report (API, GMT_MSG_INFORMATION, "Selected Viscoelastic transfer function\n");
			R->setup = grdflexure_setup_ve;		R->transfer = grdflexure_transfer_ve;		break;
		case FLX_GL:
			GMT_Report (API, GMT_MSG_INFORMATION, "Selected General Linear Viscoelastic transfer function\n");
			R->setup = grdflexure_setup_gl;		R->transfer = grdflexure_transfer_gl;		break;
		case FLX_FV1:
			GMT_Report (API, GMT_MSG_INFORMATION, "Selected Firmoviscous transfer function for elastic plate over viscous half-space\n");
			R->setup = grdflexure_setup_fv;		R->transfer = grdflexure_transfer_fv;		break;
		case FLX_FV2:
			GMT_Report (API, GMT_MSG_INFORMATION, "Selected Firmoviscous transfer function for elastic plate over viscous layer over viscous half-space\n");
			R->setup = grdflexure_setup_fv2;		R->transfer = grdflexure_transfer_fv2;		break;
		case FLX_V1:
			GMT_Report (API, GMT_MSG_INFORMATION, "Selected Viscous transfer function for viscous half-space\n");
			R->setup = grdflexure_setup_fv;		R->transfer = grdflexure_transfer_v;		break;
		case FLX_V2:
			GMT_Report (API, GMT_MSG_INFORMATION, "Selected Viscous transfer function for viscous layer over viscous half-space\n");
			R->setup = grdflexure_setup_fv2;		R->transfer = grdflexure_transfer_v2;		break;
	}
	R->mode = fmode;

	return (R);
}

GMT_LOCAL void grdflexure_accumulate_solution (struct GMT_CTRL *GMT, struct GMT_GRID *Out, struct GMT_GRID *Component) {
	/* Simply adds component grid to output grid */
	uint64_t node;
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Add in latest component\n");
	for (node = 0; node < Out->header->size; node++) Out->data[node] += Component->data[node];
}

GMT_LOCAL int grdflexure_compare_loads (const void *load_1v, const void *load_2v) {
	/*  Routine for qsort to sort loads structure with old loads (large t) be first in list. */
	const struct GRDFLEXURE_GRID **load_1 = (const struct GRDFLEXURE_GRID **)load_1v, **load_2 = (const struct GRDFLEXURE_GRID **)load_2v;
	if ((*load_1)->Time->value > (*load_2)->Time->value) return (-1);
	if ((*load_1)->Time->value < (*load_2)->Time->value) return (+1);
	return (0);
}

GMT_LOCAL int grdflexure_write_transfer_function (struct GMT_CTRL *GMT, struct GRDFLEXURE_CTRL *Ctrl, struct GRDFLEXURE_RHEOLOGY *R, struct GMT_OPTION *options) {
	/* Write a table with six segments (one each for Te = 1, 2, 5, 10, 20, 50, and 100 km).
	 * Each segment has leading columns of wavelength and wavenumber corresponding to wavelengths 1:5000 km.
	 * The next 12 columns has the chosen transfer function evaluated for times 1k, 2k, 5k, 10k, 20k, 50k, 100k, 200k, 500k, 1M, 2M, and 5M years.
	 * Each segment is written to a separate file. Obviously, if no -F or -M are given then all columns are the same since elastic */
	uint64_t k;
	int t, s, n_times, n_te;
	char file[GMT_LEN64] = {""};
	static char *FLX_response[7] = {"Airy", "Elastic", "Viscoelastic", "Firmoviscous (1 layer)", "Firmoviscous (2 layer)", "Viscous (1 layer)", "Viscous (2 layer)"};
	uint64_t dim[4] = {1, 0, 0, 0};
	double *kr, K[3], te[7] = {1.0, 2.0, 5.0, 10.0, 20.0, 50.0, 100.0};
	double times[12] = {1.0, 2.0, 5.0, 10.0, 20.0, 50.0, 100.0, 200.0, 500.0, 1000.0, 2000.0, 5000.0};	/* Times in kiloyears */
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	struct GMT_ARRAY T;

	gmt_M_memset (&T, 1, struct GMT_ARRAY);	/* Wipe clean the structure */

	R->relative = true;	/* Relative times are implicitly given */
	n_te = (R->mode > FLX_FV2) ? 1 : 7;	/* For purely viscous we don't need to loop over plate thickness */
	gmt_parse_array (GMT, 'T', "1/5000/1", &T, GMT_ARRAY_RANGE | GMT_ARRAY_UNIQUE, 0);	/* In km */
	gmt_create_array (GMT, 'T', &T, NULL, NULL);
	dim[GMT_ROW] = T.n;
	n_times = (Ctrl->F.active || Ctrl->M.active) ? 12 : 1;	/* No point repeating 12 identical results for the elastic case */
	dim[GMT_SEG] = n_te;
	dim[GMT_COL] = 2 + n_times;
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Write transfer functions\n");
	kr = gmt_M_memory (GMT, NULL, T.n, double);
	for (k = 0; k < T.n; k++) kr[k] = 2.0 * M_PI / (T.array[k] * 1000.0);	/* Radial wavenumber in 1/m */

	if ((D = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
		return GMT_RUNTIME_ERROR;
	GMT_Set_Comment (GMT->parent, GMT_IS_DATASET, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, D);
	gmt_set_tableheader (GMT, GMT_OUT, true);


	for (s = 0; s < n_te; s++) {
		S = D->table[0]->segment[s];
		Ctrl->E.te[TE_INIT] = (n_te > 1) ? te[s] * 1000 : 0.0;	/* Te in meters, zero for viscous only */
		sprintf (file, "grdflexure_transfer_function_te_%3.3d_km.txt", irint (Ctrl->E.te[TE_INIT] * 0.001));
		SH = gmt_get_DS_hidden (S);
		SH->file[GMT_OUT] = strdup (file);
		gmt_M_memcpy (S->data[0], T.array, T.n, double);
		gmt_M_memcpy (S->data[1], kr, T.n, double);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "%s transfer function for Te = %g km written to %s\n", FLX_response[R->mode], Ctrl->E.te[TE_INIT] * 0.001, SH->file[GMT_OUT]);
		for (t = 0; t < n_times; t++) {	/* For each time step (i.e., at least once) */
			R->eval_time_yr = times[t] * 1000.0;	/* In years */
			R->setup (GMT, Ctrl, R);		/* Set up parameters */
			R->scale = 1.0;	/* We want these to go 0-1 only */
			for (k = 0; k < T.n; k++) {	/* Evaluate transfer functions */
				K[GMT_FFT_K_IS_KR] = kr[k];
				S->data[t+2][k] = R->transfer (K, R);
			}
		}
	}
	gmt_free_array (GMT, &T);
	gmt_M_free (GMT, kr);
	if (GMT_Write_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_WRITE_SEGMENT, NULL, file, D) != GMT_NOERROR)
		return GMT_RUNTIME_ERROR;
	return GMT_NOERROR;
}

EXTERN_MSC int GMT_grdflexure (void *V_API, int mode, void *args) {
	unsigned int t_eval, t_load, n_load_times = 0;
	int error;
	bool retain_original;
	gmt_grdfloat *orig_load = NULL;
	double fixed_rho_load;
	char zfile[PATH_MAX] = {""}, rfile[PATH_MAX] = {""};

	struct GMT_FFT_WAVENUMBER *K = NULL;
	struct GRDFLEXURE_RHEOLOGY *R = NULL;
	struct GRDFLEXURE_GRID **Load = NULL, *This_Load = NULL;
	struct GMT_GRID *Out = NULL, *Grid_A = NULL;
	struct GMT_DATASET *L = NULL;
	struct GRDFLEXURE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdflexure main code ----------------------------*/

	/* 1. SELECT THE TRANSFER FUNCTION TO USE */

	R = grdflexure_select_rheology (GMT, Ctrl);

	if (Ctrl->Q.active) {	/* Just write transfer function and return */
		error = grdflexure_write_transfer_function (GMT, Ctrl, R, options);
		gmt_M_free (GMT, R);
		Return (error);
	}

	/* 2. READ ALL INPUT LOAD GRIDS, DETREND, AND TAKE FFT */

	if (Ctrl->In.many) {	/* Must read in load grids, possibly one for each time increment set by -T */
		char *rho = NULL;	/* Pointer to variable load density grid [or NULL] */
		n_load_times = Ctrl->T.n_eval_times;	/* This (or fewer) loads and times will be used */
		Load = gmt_M_memory (GMT, NULL, n_load_times, struct GRDFLEXURE_GRID *);	/* Allocate load array structure */
		for (t_load = 0; t_load < n_load_times; t_load++) {	/* For each time step there may be a load file */
			gmt_modeltime_name (GMT, zfile, Ctrl->In.file, &Ctrl->T.time[t_load]);	/* Load time equal eval time */
			if (Ctrl->D.var_rhol) {	/* Also form the density grid name from template */
				gmt_modeltime_name (GMT, rfile, Ctrl->H.file, &Ctrl->T.time[t_load]);
				rho = rfile;
			}
			Load[t_load] = grdflexure_prepare_load (GMT, options, Ctrl, zfile, rho, &Ctrl->T.time[t_load]);
			if (!(Ctrl->D.var_rhol || Ctrl->D.root)) Load[t_load]->rho_load = Ctrl->D.rhol;
			if (Grid_A == NULL && Load[t_load]->Grid) Grid_A = Load[t_load]->Grid;	/* First grid read */
		}
	}
	else if (Ctrl->In.list) {	/* Must read a list of files and their load times (format: filename [rhofile] loadtime) */
		struct GMT_DATASET *Tin = NULL;
		struct GMT_DATASEGMENT *S = NULL;
		struct GMT_MODELTIME this_time = {0.0, 0.0, 0, "", 0};
		int ns;
		double s_time, s_scale;
		char t_arg[GMT_LEN256] = {""}, r_arg[GMT_LEN64] = {""}, s_unit;
		if ((Tin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->In.file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while reading load file list %s\n", Ctrl->In.file);
			Return (API->error);
		}
		/* Read the file successfully */
		n_load_times = (unsigned int)Tin->n_records;
		Load = gmt_M_memory (GMT, NULL, n_load_times, struct GRDFLEXURE_GRID *);		/* Allocate load grid array structure */
		for (uint64_t seg = 0, t_load = 0; seg < Tin->table[0]->n_segments; seg++) {	/* Read in from possibly more than one segment */
			S = Tin->table[0]->segment[seg];	/* Shorthand for readability */
			for (uint64_t row = 0; row < S->n_rows; row++, t_load++) {
				if (Ctrl->D.var_rhol)
					ns = sscanf (S->text[row], "%s %s %s", zfile, r_arg, t_arg);
				else
					ns = sscanf (S->text[row], "%s %s %s", zfile, t_arg, r_arg);
				this_time.value = S->data[GMT_X][row];	/* Time in years from the numerical part of the list */
				s_time = gmt_get_modeltime (t_arg, &s_unit, &s_scale);	/* Also returns time in years - we will check */
				if (!doubleAlmostEqualZero (this_time.value, s_time)) {	/* These should be the same after accounting for units and scale */
					GMT_Report (API, GMT_MSG_ERROR, "Time mismatch between numerical and time tag for record % "PRIu64 " while reading load file list %s\n", row, Ctrl->In.file);
					Return (API->error);
				}
				this_time.scale = s_scale;	/* Get formatted time scale and unit from t_arg */
				this_time.unit  = s_unit;
				this_time.u = (s_unit == 'M') ? 2 : ((s_unit == 'k') ? 1 : 0);
				Load[t_load] = grdflexure_prepare_load (GMT, options, Ctrl, zfile, r_arg, &this_time);
				if (!(Ctrl->D.var_rhol || Ctrl->D.root)) 
					Load[t_load]->rho_load = (ns == 3) ? atof (r_arg) : Ctrl->D.rhol;
				if (Grid_A == NULL && Load[t_load]->Grid) Grid_A = Load[t_load]->Grid;	/* First grid read */
			}
		}
		if (GMT_Destroy_Data (API, &Tin) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while destroying load file list after processing\n");
			error = API->error;	goto cleanup_time;
		}
	}
	else {	/* Just read the single load grid (and optionally rho grid) */
		n_load_times = 1;
		Load = gmt_M_memory (GMT, NULL, n_load_times, struct GRDFLEXURE_GRID *);	/* Allocate grid array structure with one entry */
		Load[0] = grdflexure_prepare_load (GMT, options, Ctrl, Ctrl->In.file, Ctrl->H.file, NULL);	/* The single load [and rho] grid (no time info) */
		if (!(Ctrl->D.var_rhol || Ctrl->D.root)) Load[0]->rho_load = Ctrl->D.rhol;
	}

	if (n_load_times > 1) {	/* Sort to ensure load array goes from old to young loads */
		GMT_Report (API, GMT_MSG_INFORMATION, "Sort %u loads from old to young\n", n_load_times);
		qsort (Load, n_load_times, sizeof (struct GRDFLEXURE_GRID *), grdflexure_compare_loads);
		/* Do a check that all grids are coregistered */
		for (t_load = 0; t_load < n_load_times; t_load++) {	/* For each load  */
			if (Grid_A == NULL) continue;	/* Safety valve */
			if (Load[t_load] == NULL) continue;	/* Quietly skip times with no load */
			if (Load[t_load]->Grid == NULL) continue;	/* Quietly skip times with no load */
			if (!gmt_grd_domains_match (GMT, Grid_A, Load[t_load]->Grid, "load")) {
				error = GMT_RUNTIME_ERROR;	goto cleanup_time;
			}
		}
	}
	K = Load[0]->K;	/* We only need one pointer to get to wavenumbers as they are all the same for all grids */
	fixed_rho_load = Ctrl->D.rhol;	/* Remember the setting since we may change it temporarily below */

	/* 3. DETERMINE AND POSSIBLY CREATE ONE OUTPUT GRID */

	retain_original = (n_load_times > 1 || Ctrl->T.n_eval_times > 1);	/* True when we will have to loop over the loads */
	if (retain_original) {	/* We may need to reuse loads for different times and will have to keep copy of unchanged H(kx,ky) */
		orig_load = gmt_M_memory (GMT, NULL, Load[0]->Grid->header->size, gmt_grdfloat);	/* Single temporary storage to hold one original H(kx,ky) grid */
		/* We must also allocate a separate output grid */
		if ((Out = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, Load[0]->Grid)) == NULL) {
			error = API->error;	goto cleanup_time;
		}
	}
	else	/* With a single load -> flexure operation we can just recycle the input grid for the output */
		Out = Load[0]->Grid;

	/* Here, Load[] contains all the input load grids and their load times, ready to go as H(kx,ky) */

	if (Ctrl->L.active) {	/* Must create a dataset to hold times and names of all output grids */
		uint64_t dim[GMT_DIM_SIZE] = {1, 1, Ctrl->T.n_eval_times, 1};
		if ((L = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_WITH_STRINGS, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while creating text set for file %s\n", Ctrl->L.file);
			error = API->error;	goto cleanup_time;
		}
		L->table[0]->segment[0]->n_rows = Ctrl->T.n_eval_times;
	}

	for (t_eval = 0; t_eval < Ctrl->T.n_eval_times; t_eval++) {	/* For each time step (i.e., at least once) */

		/* 4a. SET THE CURRENT TIME VALUE (IF USED) */
		if (Ctrl->T.active) {	/* Set the current time in user units as well as years */
			R->eval_time_yr = Ctrl->T.time[t_eval].value;		/* In years */
			GMT_Report (API, GMT_MSG_INFORMATION, "Evaluating flexural deformation for time %%s\n", Ctrl->T.time[t_eval].tag);
		}

		if (retain_original) gmt_M_memset (Out->data, Out->header->size, gmt_grdfloat);	/* Reset output grid to zero; not necessary when we only get here once */

		for (t_load = 0; t_load < n_load_times; t_load++) {	/* For each load  */
			This_Load = Load[t_load];	/* Short-hand for current load */
			if (This_Load == NULL) continue;	/* Quietly skip times with no load */
			if (Ctrl->T.active && Ctrl->T.time && This_Load->Time) {	/* Has both load time and evaluation time so can check some more */
				if (This_Load->Time->value < Ctrl->T.time[t_eval].value) continue;	/* Skip loads in the future */
			}
			if (Ctrl->T.active && This_Load->Time) {	/* Has time so we can report on what is happening */
				R->load_time_yr = This_Load->Time->value;	/* In years */
				R->relative = false;	/* Absolute times are given */
				if (This_Load->Time->u == Ctrl->T.time[t_eval].u) {	/* Same time units even */
					double dt = This_Load->Time->value * This_Load->Time->scale - Ctrl->T.time[t_eval].value * Ctrl->T.time[t_eval].scale;
					GMT_Report (API, GMT_MSG_INFORMATION, "  Accumulating flexural deformation for load emplaced at time %s [Loading time = %g %s]\n",
						This_Load->Time->tag, dt, grdflexure_modeltime_unit (This_Load->Time->u));
				}
				else {	/* Just state load time */
					GMT_Report (API, GMT_MSG_INFORMATION, "  Accumulating flexural deformation for load emplaced at time %s\n", This_Load->Time->tag);
				}
			}
			else {
				R->load_time_yr = 0.0;	/* Not given, assume R->eval_time_yr is time since loading */
				R->relative = true;	/* Relative times are given */
				GMT_Report (API, GMT_MSG_INFORMATION, "  Accumulating flexural deformation for load # %d emplaced at unspecified time\n", t_load);
			}
			/* 4b. COMPUTE THE RESPONSE DUE TO THIS LOAD */
			Ctrl->D.rhol = This_Load->rho_load;	/* Since it may be variable and grdflexure_apply_transfer_function uses Ctrl->D.rhol to set things up */
			if (retain_original) gmt_M_memcpy (orig_load, This_Load->Grid->data, This_Load->Grid->header->size, gmt_grdfloat);	/* Make a copy of H(kx,ky) before operations */
			grdflexure_apply_transfer_function (GMT, This_Load->Grid, Ctrl, This_Load->K, R);	/* Multiplies H(kx,ky) by transfer function, yielding W(kx,ky) */
			if (retain_original) {	/* Must add this contribution to our total output grid */
				grdflexure_accumulate_solution (GMT, Out, This_Load->Grid);
				gmt_M_memcpy (This_Load->Grid->data, orig_load, This_Load->Grid->header->size, gmt_grdfloat);	/* Restore H(kx,ky) to what it was before operations */
			}
			Ctrl->D.rhol = fixed_rho_load;	/* Reset */
		}

		/* 4c. TAKE THE INVERSE FFT TO GET w(x,y) */
		GMT_Report (API, GMT_MSG_INFORMATION, "Inverse FFT\n");
		/* Passing the no_demux flag since we are doing it manually given the loops and reuse of Out */
		if (GMT_FFT (API, Out, GMT_FFT_INV, GMT_FFT_COMPLEX|GMT_FFT_NO_DEMUX, K)) {
			error = GMT_RUNTIME_ERROR;	goto cleanup_time;
		}

		/* 4d. APPLY SCALING AND OFFSET */
		gmt_scale_and_offset_f (GMT, Out->data, Out->header->size, 1.0, -Ctrl->Z.zm);

		/* 4e. WRITE OUTPUT GRID */
		if (Ctrl->T.active) { /* Separate and unique output grid names from time since there are many time steps */
			char remark[GMT_GRID_REMARK_LEN160] = {""};
			gmt_modeltime_name (GMT, zfile, Ctrl->G.file, &Ctrl->T.time[t_eval]);
			sprintf (remark, "Solution for t = %s", Ctrl->T.time[t_eval].tag);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK|GMT_COMMENT_IS_RESET, remark, Out)) {
				error = API->error;	goto cleanup_time;
			}
		}
		else	/* Single output grid (no -T set) */
			strcpy (zfile, Ctrl->G.file);
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Out)) {
			error = API->error;	goto cleanup_time;
		}

		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA | GMT_GRID_IS_COMPLEX_REAL,
			NULL, zfile, Out) != GMT_NOERROR) {
			error = API->error;	goto cleanup_time;
		}
		if (t_eval < (Ctrl->T.n_eval_times-1)) {	/* Must put the total grid back into interleave mode */
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Re-multiplexing complex grid before accumulating new increments.\n");
			gmt_grd_mux_demux (GMT, Out->header, Out->data, GMT_GRID_IS_INTERLEAVED);
		}

		if (Ctrl->L.active) {	/* Add filename and evaluation time to list dataset */
			/* Output will be: <numerical-time> <flexgridname> <timetag> */
			char record[GMT_BUFSIZ] = {""};
			sprintf (record, "%s\t", zfile);	/* Flexure output grid for this time */
			/* Add formatted time tag after file name */
			strcat (record, Ctrl->T.time[t_eval].tag);
			L->table[0]->segment[0]->data[GMT_X][t_eval] = Ctrl->T.time[t_eval].value;	/* Output time in years */
			L->table[0]->segment[0]->text[t_eval] = strdup (record);
		}
	}

	error = GMT_NOERROR;
	if (Ctrl->L.active) {	/* Write out list of times and flexure grids */
		char *header = "Time(yr)\tFlexgrid\tTimetag";
		if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, L)) Return (API->error);
		if (Ctrl->L.active && GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_NORMAL, NULL, Ctrl->L.file, L) != GMT_NOERROR) {
			if (Ctrl->L.file)
				GMT_Report (API, GMT_MSG_ERROR, "Failure while writing list of output times and flexure grid file names to %s\n", Ctrl->L.file);
			else
				GMT_Report (API, GMT_MSG_ERROR, "Failure while writing list of output times and flexure grid file names to standard output\n");
			error = API->error;
		}
	}

	/* 5. FREE ALL GRIDS AND ARRAYS */

cleanup_time:

	for (t_load = 0; t_load < n_load_times; t_load++) {	/* Free up grid structures */
		This_Load = Load[t_load];			/* Short-hand for current load */
		if (This_Load == NULL) continue;		/* Quietly skip containers with no grids */
		GMT_Destroy_Data (API, &This_Load->Grid);	/* Free up grid used */
		GMT_FFT_Destroy (API, &This_Load->K);		/* Free up wavenumber vectors and info structure created by GMT_FFT_Create */
		gmt_M_free (GMT, This_Load->Time);	/* Free time array, if used */
		gmt_M_free (GMT, This_Load);			/* Free load structure */
	}
	gmt_M_free (GMT, Load);
	gmt_M_free (GMT, R);
	if (retain_original) gmt_M_free (GMT, orig_load);

	Return (error);
}
