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
 *--------------------------------------------------------------------*/
/*
 * Author:      Joaquim Luis
 * Date:        09-OCT-1994
 * Updated:	28-Feb-1999
 *         	05-SEP-2002
 *         	13-SEP-2002
 * GMT5ed	17-MAR-2012
 *
 * */

#include "gmt_potential.h"

struct GRAVFFT_CTRL {
	GMT_LONG n_par;
	double *par;

	struct GRVF_In {
		GMT_LONG active;
		char *file[2];
	} In;
	struct GRVF_A {	/* -A */
		GMT_LONG active;
		double	te, rhol, rhom, rhow;
		double	rho_cw;		/* crust-water density contrast */
		double	rho_mc;		/* mantle-crust density contrast */
		double	rho_mw;		/* mantle-water density contrast */
	} A;
	struct GRVF_C {	/* -C<zlevel> */
		GMT_LONG active;
		int n_pt;
		double theor_inc;
	} C;
	struct GRVF_D {	/* -D[<scale>|g] */
		GMT_LONG active;
	} D;
	struct GRVF_E {	/* -E */
		GMT_LONG active;
		int n_terms;
	} E;
	struct GRVF_F {	/* -F[x_or_y]<lc>/<lp>/<hp>/<hc> or -F[x_or_y]<lo>/<hi> */
		GMT_LONG active;
		GMT_LONG mode;
		double lc, lp, hp, hc;
	} F;
	struct GRVF_G {	/* -G<outfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct GRVF_H {
		GMT_LONG active;
	} H;
	struct GRVF_I {	/* -I[<scale>|g] */
		GMT_LONG active;
		double value;
	} I;
	struct GRVF_L {	/* -L */
		GMT_LONG active;
	} L;
	struct GRVF_M {
		GMT_LONG active;
	} M;
	struct GRVF_N {	/* -N<stuff> */
		GMT_LONG active;
		GMT_LONG force_narray, suggest_narray, n_user_set;
		GMT_LONG nx2, ny2;
		double value;
	} N;
	struct GRVF_Q {
		GMT_LONG active;
	} Q;
	struct GRVF_S {	/* -S<scale> */
		GMT_LONG active;
	} S;
	struct GRVF_s {
		GMT_LONG active;
		double scale;
	} s;
	struct GRVF_T {	/* -T<te/rl/rm/rw/ri> */
		GMT_LONG active;
		double te, rhol, rhom, rhow;
		double	rho_cw;		/* crust-water density contrast */
		double	rho_mc;		/* mantle-crust density contrast */
		double	rho_mw;		/* mantle-water density contrast */
	} T;
	struct GRVF_t {	/* -t For saving real & imag FFT grids */
		GMT_LONG active;
		GMT_LONG sc_coherence, sc_admitt;
	} t;
	struct GRVF_Z {
		double	zm;			/* mean Moho depth (given by user) */
		double	zl;			/* mean depth of swell compensation (user given) */		
	} Z;
	struct GRVF_misc {	/* -T */
		int coherence;
		int give_wavelength;
		int from_below;
		int from_top;
		int rem_nothing;
		int mean_or_half_way;
		float k_or_m;
		double	z_level;	/* mean bathymetry level computed from data */
		double	z_offset;	/* constant that myght be added to grid data */
		double	rho;		/* general density contrast */
	} misc;
};

#ifndef FSIGNIF
#define FSIGNIF 24
#endif

#define GRAVITATIONAL_CONST 6.667e-11
#define	YOUNGS_MODULUS	7.0e10		/* Pascal = Nt/m**2  */
#define	NORMAL_GRAVITY	9.806199203	/* Moritz's 1980 IGF value for gravity at 45 degrees latitude */
#define	POISSONS_RATIO	0.25

GMT_LONG rem_mean = FALSE;
GMT_LONG sphericity = FALSE;

struct FFT_SUGGESTION {
	GMT_LONG nx;
	GMT_LONG ny;
	GMT_LONG worksize;	/* # single-complex elements needed in work array  */
	GMT_LONG totalbytes;	/* (8*(nx*ny + worksize))  */
	double run_time;
	double rms_rel_err;
}; /* [0] holds fastest, [1] most accurate, [2] least storage  */

struct K_XY {	/* Holds parameters needed to calculate kx, ky, kr */
	GMT_LONG nx2, ny2;
	double delta_kx, delta_ky;
};

void *New_gravfft_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRAVFFT_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct GRAVFFT_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->A.te = 0.0;
	C->s.scale = 1.0;
	C->E.n_terms = 1;
	C->misc.k_or_m = 1;
	C->misc.mean_or_half_way = 1;
	return (C);
}

void Free_gravfft_Ctrl (struct GMT_CTRL *GMT, struct GRAVFFT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->par) GMT_free (GMT, C->par);	
	if (C->In.file[0]) free (C->In.file[0]);	
	if (C->In.file[1]) free (C->In.file[1]);	
	if (C->G.file) free (C->G.file);	
	GMT_free (GMT, C);	
}

double	scale_out = 1.0;
double	earth_rad = 6371008.7714;	/* GRS-80 sphere */

void set_grid_radix_size__ (struct GMT_CTRL *GMT, struct GRAVFFT_CTRL *Ctrl, struct GMT_GRID *Gin);
void do_parker (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, float *raised, int n, double rho);
void taper_edges__ (struct GMT_CTRL *GMT, struct GMT_GRID *Grid);
void remove_level(struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRAVFFT_CTRL *Ctrl);
void do_isostasy__(struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K);
void do_admittance(struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GMT_GRID *GridB, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K);
void remove_plane__(struct GMT_CTRL *GMT, struct GMT_GRID *Grid);
void load_from_below_admit(struct GMT_CTRL *GMT, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, double *z_below);
void load_from_top_admit(struct GMT_CTRL *GMT, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, double *z_top);
void load_from_top_grid(struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, float *raised, int n);
void load_from_below_grid(struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, float *raised, int n);
void compute_only_adimtts(struct GMT_CTRL *GMT, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, double *z_top_or_bot, double delta_pt);
void write_script(void);

GMT_LONG GMT_gravfft_parse (struct GMTAPI_CTRL *C, struct GRAVFFT_CTRL *Ctrl, struct GMT_OPTION *options) {

	GMT_LONG n_errors = 0, pos;
	/* first 2 cols from table III of Singleton's paper on fft.... */
	int nlist[117] = {64,72,75,80,81,90,96,100,108,120,125,128,135,144,150,160,162,180,192,200,
			216,225,240,243,250,256,270,288,300,320,324,360,375,384,400,405,432,450,480,
			486,500,512,540,576,600,625,640,648,675,720,729,750,768,800,810,864,900,960,
			972,1000,1024,1080,1125,1152,1200,1215,1250,1280,1296,1350,1440,1458,1500,
			1536,1600,1620,1728,1800,1875,1920,1944,2000,2025,2048,2160,2187,2250,2304,
			2400,2430,2500,2560,2592,2700,2880,2916,3000,3072,3125,3200,3240,3375,3456,
			3600,3645,3750,3840,3888,4000,4096,4320,4374,4500,4608,4800,4860,5000};

	int j, k, n, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;
	char   ptr[GMT_BUFSIZ], t_or_b[4];

	for (opt = options; opt; opt = opt->next) {		/* Process all the options given */
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
#ifdef GMT_COMPAT
			case 'A':		/* Old pre-GMT way of setting this */
				Ctrl->T.active = TRUE;
				sscanf (opt->arg, "%lf/%lf/%lf/%lf", &Ctrl->T.te, &Ctrl->T.rhol, &Ctrl->T.rhom, &Ctrl->T.rhow);
				Ctrl->T.rho_cw = Ctrl->T.rhol - Ctrl->T.rhow;
				Ctrl->T.rho_mc = Ctrl->T.rhom - Ctrl->T.rhol;
				Ctrl->T.rho_mw = Ctrl->T.rhom - Ctrl->T.rhow;
				if (Ctrl->T.te > 1e10) { /* Given flexural rigidity, compute Te */
					Ctrl->T.te = pow ((12.0 * (1.0 - POISSONS_RATIO * POISSONS_RATIO)) * Ctrl->T.te
					     / YOUNGS_MODULUS, 1./3.);
				}
				break;
#endif
			case 'C':	/* For theoretical curves only */
				Ctrl->C.active = TRUE;
				sscanf (opt->arg, "%d/%lf/%lf/%s", &Ctrl->C.n_pt, &Ctrl->C.theor_inc, &Ctrl->misc.z_level, t_or_b);
				for (n = 0; t_or_b[n]; n++) {
					switch (t_or_b[n]) {
						case 'w':
							Ctrl->misc.give_wavelength = TRUE;
							break;
						case 'b':
							Ctrl->misc.from_below = TRUE;
							break;
						case 't':
							Ctrl->misc.from_top = TRUE;
							break;
						default:
							GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -C: [%s] is not valid, chose from [tbw]\n", &t_or_b[n]);
							n_errors++;
							break;
					}
				}
				break;
			case 'D':
				if (!opt->arg) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -D option: must give density contrast\n");
					n_errors++;
				}
				Ctrl->D.active = TRUE;
				Ctrl->misc.rho = atof (opt->arg);
				Ctrl->L.active = TRUE;
				break;
			case 'E':
				Ctrl->E.n_terms = atoi (opt->arg);
				if (Ctrl->E.n_terms > 10) {
					GMT_report (GMT, GMT_MSG_FATAL, "ERROR -E option: n_terms must be <= 10\n");
					n_errors++;
				}
				break;
			case 'H':
				Ctrl->H.active = TRUE;
				Ctrl->L.active = TRUE;
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				pos = j = 0;
				while (GMT_strtok (opt->arg, "/", &pos, ptr)) {
					switch (j) {
						case 0:
							Ctrl->In.file[1] = strdup(ptr);
							break;
						case 1:
							for (n = 0; ptr[n]; n++) {
								switch (ptr[n]) {
									case 'w':
										Ctrl->misc.give_wavelength = TRUE;
										break;
									case 'b':
										Ctrl->misc.from_below = TRUE;
										break;
									case 'c':
										Ctrl->misc.coherence = TRUE;
										break;
									case 't':
										Ctrl->misc.from_top = TRUE;
										break;
									case 'k':
										Ctrl->misc.k_or_m = 1000.;
										break;
									default:
										GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -I : [%s] is not valid, chose from [wbct]\n", &ptr[n]);
										n_errors++;
										break;
								}
							}
							break;
						default:
							break;
					}
					j++;
				}
				break;
			case 'L':
				Ctrl->L.active = TRUE;
				break;
			case 'l':
				if (opt->arg[0] == 'n' || opt->arg[0] == 'N')
					Ctrl->misc.rem_nothing = TRUE;
				else
					Ctrl->misc.mean_or_half_way = FALSE;
				break;
			case 'N':
				if (opt->arg[0] == 'f' || opt->arg[0] == 'F')
					Ctrl->N.force_narray = TRUE;
				else if (opt->arg[0] == 'q' || opt->arg[0] == 'Q')
					Ctrl->N.suggest_narray = TRUE;
				else if (opt->arg[0] == 's' || opt->arg[0] == 'S') {
					fprintf (stderr, "\t\"Good\" numbers for FFT dimensions\n");
					for (k = 0; k < 104; k++) {
						fprintf (stderr, "\t%d", nlist[k]);
						if ((k+1) % 10 == 0 || k == 103) fprintf (stderr, "\n");
					}
					return (GMT_OK);
				}
				else {
					if ((sscanf(opt->arg, "%" GMT_LL "d/%" GMT_LL "d", &Ctrl->N.nx2, &Ctrl->N.ny2)) != 2) n_errors++;
					if (Ctrl->N.nx2 <= 0 || Ctrl->N.ny2 <= 0) n_errors++;
					Ctrl->N.n_user_set = TRUE;
				}
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
#ifdef GMT_COMPAT
			case 'M':	/* Geographic data */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -M is deprecated; -fg was set instead, use this in the future.\n");
				if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
				break;
#endif
			case 'Q':
				Ctrl->Q.active = TRUE;
				Ctrl->L.active = TRUE;
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				sscanf (opt->arg, "%lf/%lf/%lf/%lf", &Ctrl->T.te, &Ctrl->T.rhol, &Ctrl->T.rhom, &Ctrl->T.rhow);
				Ctrl->T.rho_cw = Ctrl->T.rhol - Ctrl->T.rhow;
				Ctrl->T.rho_mc = Ctrl->T.rhom - Ctrl->T.rhol;
				Ctrl->T.rho_mw = Ctrl->T.rhom - Ctrl->T.rhow;
				if (Ctrl->T.te > 1e10) { /* Given flexural rigidity, compute Te */
					Ctrl->T.te = pow ((12.0 * (1.0 - POISSONS_RATIO * POISSONS_RATIO)) * Ctrl->T.te
					     / YOUNGS_MODULUS, 1./3.);
				}
				break;
			case 's':
				Ctrl->s.active = TRUE;
				Ctrl->s.scale = atof (opt->arg);
				break;
			case 't':
				Ctrl->t.active = TRUE;
				if (opt->arg[0] == 'a')
					Ctrl->t.sc_admitt = TRUE;
				else if (opt->arg[0] == 'c')
					Ctrl->t.sc_coherence = TRUE;
				break;
			case 'Z':
				sscanf (opt->arg, "%lf/%lf", &Ctrl->Z.zm, &Ctrl->Z.zl);
				break;
			case 'z':
				sscanf (opt->arg, "%lf", &Ctrl->misc.z_offset);
				break;
			default:
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->C.active) {
		n_errors += GMT_check_condition (GMT, !Ctrl->T.active, "Error: -T not set\n");
		n_errors += GMT_check_condition (GMT, Ctrl->misc.from_top && !Ctrl->Z.zm, 
									"Error: -Z not set, must give moho compensation depth\n");
		n_errors += GMT_check_condition (GMT, Ctrl->misc.from_below && !(Ctrl->Z.zm && Ctrl->Z.zl), 
									"Error: -Z not set, must give moho and swell compensation depths\n");
	}
	else {
		n_errors += GMT_check_condition (GMT, !Ctrl->In.file[0], "Syntax error: Must specify input file\n");
		n_errors += GMT_check_condition (GMT, !Ctrl->G.file && !Ctrl->I.active, 
					"Syntax error -G option: Must specify output file\n");
		n_errors += GMT_check_condition (GMT, Ctrl->Q.active && !Ctrl->T.active, "Error: -Q implies also -A\n");
		n_errors += GMT_check_condition (GMT, Ctrl->H.active && !Ctrl->T.active, "Error: -H implies also -A\n");
		n_errors += GMT_check_condition (GMT, Ctrl->S.active && !Ctrl->T.active, "Error: -S implies also -A\n");
		n_errors += GMT_check_condition (GMT, Ctrl->Q.active && !Ctrl->Z.zm, 
					"Error: for creating the flex_file I need to know it's average depth (see -Z<zm>)\n");
		n_errors += GMT_check_condition (GMT, Ctrl->H.active && !Ctrl->Z.zm, 
					"Error: for computing the Moho's effect I need to know it's average depth (see -Z<zm>)\n");
		n_errors += GMT_check_condition (GMT, !(Ctrl->D.active || Ctrl->T.active || Ctrl->S.active || 
							Ctrl->I.active || Ctrl->t.active), "Error: must set density contrast\n");
		n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->In.file[1], 
					"Error: for admittance|coherence need a gravity or geoide grid\n");
		n_errors += GMT_check_condition (GMT, Ctrl->misc.from_below && Ctrl->misc.from_top, 
					"Error: -I, choose only one model\n");
		n_errors += GMT_check_condition (GMT, Ctrl->I.active && Ctrl->misc.from_top && 
					!(Ctrl->T.rhow && Ctrl->T.rhol && Ctrl->T.rhom && Ctrl->Z.zm), 
					"Error: not all parameters needed for computing \"loading from top\" admittance were set\n");
		n_errors += GMT_check_condition (GMT, Ctrl->I.active && Ctrl->misc.from_below && 
			!(Ctrl->T.rhow && Ctrl->T.rhol && Ctrl->T.rhom && Ctrl->Z.zm && Ctrl->Z.zl), 
					"Error: not all parameters for computing \"loading from below\" admittance were set\n");
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LONG GMT_gravfft_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {

	struct GMT_CTRL *GMT = C->GMT;

		GMT_message (GMT, "gravfft %s - Compute gravitational attraction of 3-D surfaces and a little more (ATTENTION z positive up)\n\n", GMT_VERSION);
		GMT_message (GMT, "usage: gravfft <topo_grd> -C<n/wavelength/mean_depth/tbw> -D<density>\n");
		GMT_message (GMT,"       -G<out_grdfile> [-E<n_terms>] [-F] [-L[-l[n]] -I<second_file>[/<wbct>]\n");
		GMT_message (GMT,"       [-H] [-M] [-N<stuff>] [-Q] -T<te/rl/rm/rw> [-s<scale>] [-t[c|a]]\n");
		GMT_message (GMT,"       [-V] -Z<zm>[/<zl>] [-z<cte>]\n\n");

		if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

		GMT_message (GMT,"\ttopo_grd is the input grdfile with topography values\n");
		GMT_message (GMT,"\t-C n/wavelength/mean_depth/tbw Compute admittance curves based on a theoretical model.\n");
		GMT_message (GMT,"\t   Total profile length in meters = <n> * <wavelength> (unless -Kx is set).\n");
		GMT_message (GMT,"\t   --> Rest of parametrs are set within -T AND -Z options\n");
		GMT_message (GMT,"\t   Append dataflags (one or two) of tbw.\n");
		GMT_message (GMT,"\t     t writes \"elastic plate\" admittance \n");
		GMT_message (GMT,"\t     b writes \"loading from below\" admittance \n");
		GMT_message (GMT,"\t     w writes wavelength instead of wavenumber\n");
		GMT_message (GMT,"\t-D Sets density contrast across surface (used when not -T)\n");
		GMT_message (GMT,"\t-G filename for output netCDF grdfile with gravity [or geoid] values\n");
		GMT_message (GMT,"\t-H writes a grid with the Moho's gravity|geoid effect from model selcted by -A\n");
		GMT_message (GMT,"\t-I Use <second_file> and <topo_grd> to estimate admittance|coherence and write\n");
		GMT_message (GMT,"\t   it to stdout (-G ignored if set). This grid should contain gravity or geoid\n");
		GMT_message (GMT,"\t   for the same region of <topo_grd>. Default computes admittance. Output\n");
		GMT_message (GMT,"\t   contains 3 or 4 columns. Frequency (wavelength), admittance (coherence)\n");
		GMT_message (GMT,"\t   one sigma error bar and, optionaly, a theoretical admittance.\n");
		GMT_message (GMT,"\t   Append dataflags (one to three) of wbct.\n");
		GMT_message (GMT,"\t     w writes wavelength instead of wavenumber\n");
		GMT_message (GMT,"\t     c computes coherence instead of admittance\" \n");
		GMT_message (GMT,"\t     b writes a forth column with \"loading from below\" \n");
		GMT_message (GMT,"\t       theoretical admittance\n");
		GMT_message (GMT,"\t     t writes a forth column with \"elastic plate\" \n");
		GMT_message (GMT,"\t       theoretical admittance\n");
		GMT_message (GMT,"\t-Q writes out a grid with the flexural topography (with z positive up)\n");
		GMT_message (GMT,"\t   whose average depth is at the value set by the option -Z<zm>.\n");
		GMT_message (GMT,"\t-S Computes predicted gravity|geoide grid due to a subplate load.\n");
		GMT_message (GMT,"\t   produced by the current bathymetry and the theoretical admittance.\n");
		GMT_message (GMT,"\t   --> The necessary parametrs are set within -T and -Z options\n");
		GMT_message (GMT,"\t-T Computes the isostatic compensation. Input file is topo load.\n");
		GMT_message (GMT,"\t   Append elastic thickness and densities of load, mantle, and\n");
		GMT_message (GMT,"\t   water, all in SI units. Give average mantle depth via -Z\n");
		GMT_message (GMT,"\t   If the elastic thickness is > 1e10 it will be interpreted as the\n");
		GMT_message (GMT,"\t   flexural rigidity (by default it's computed from Te and Young modulus)\n");
		GMT_message (GMT,"\t-Z zm[/zl] -> Moho [and swell] average compensation depths\n");
		GMT_message (GMT, "\n\tOPTIONS:\n");
		GMT_message (GMT,"\t-E number of terms used in Parker expansion [Default = 1]\n");
		GMT_message (GMT,"\t-F compute geoid rather than gravity\n");
		GMT_message (GMT,"\t-K indicates that distances in these directions are in km [meter]\n");
		GMT_message (GMT,"\t   -Ks multiplies the bathymetry grid by -1. Used for changing z sign\n");
		GMT_message (GMT,"\t-L Leave trend alone. Do not remove least squares plane from data.\n");
		GMT_message (GMT,"\t   It applies both to bathymetry as well as <second_file> [Default removes plane].\n");
		GMT_message (GMT,"\t   Warning: both -D -H and -Q will implicitly set -L.\n");
		GMT_message (GMT,"\t-l Removes half-way from bathymetry data [Default removes mean].\n");
		GMT_message (GMT,"\t   Append n to do not remove any constant from input bathymetry data.\n");
		GMT_message (GMT,"\t-M Map units used.  Convert grid dimensions from degrees to meters.\n");
		GMT_message (GMT,"\t-N<stuff>  Choose or inquire about suitable grid dimensions for FFT.\n");
		GMT_message (GMT,"\t\t-Nf will force the FFT to use the dimensions of the data.\n");
		GMT_message (GMT,"\t\t-Nq will inQuire about more suitable dimensions.\n");
		GMT_message (GMT,"\t\t-N<nx>/<ny> will do FFT on array size <nx>/<ny>\n");
		GMT_message (GMT,"\t\t (Must be >= grdfile size). Default chooses dimensions >= data\n");
		GMT_message (GMT,"\t\t which optimize speed, accuracy of FFT.\n");
		GMT_message (GMT,"\t\t If FFT dimensions > grdfile dimensions, data are extended\n");
		GMT_message (GMT,"\t\t and tapered to zero.\n");
		GMT_message (GMT,"\t\t-Ns will print out a table with suitable dimensions and exits.\n");
		GMT_message (GMT,"\t-s multiply field by scale.Used for changing z sign or km -> meters\n");
		GMT_message (GMT,"\t-t Writes the FFT of the input grid in the form of two grids. One for\n");
		GMT_message (GMT,"\t   the real part and the other for the imaginary. The grid names are\n");
		GMT_message (GMT,"\t   built from the input grid name with _real and _imag appended. These\n");
		GMT_message (GMT,"\t   grids have the DC at the center and coordinates are the kx,ky frequencies.\n");
		GMT_message (GMT,"\t   Appending a or c will make the program write a csh script (admit_coher.sc)\n");
		GMT_message (GMT,"\t   that computes, respectively, admittance or coherence by an alternative,\n");
		GMT_message (GMT,"\t   (but much slower than the one used with -I) method based on grid\n");
		GMT_message (GMT,"\t   interpolations using grdtrack. Read comments/instructions on the script.\n");
		GMT_explain_options (GMT, "Vf.");
		GMT_message (GMT,"\t-z add a constant to the bathymetry (not to <second_file>) before doing anything else.\n");
		return (EXIT_FAILURE);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gravfft_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_gravfft (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args) {

	int i, j, k, n;
	GMT_LONG error = FALSE, stop, m;
	char	line[256], line2[256], format[64], buffer[256];
	float	*topo, *raised;
	double	delta_pt, freq;

	struct GMT_GRID *GridA = NULL, *GridB = NULL, *Out = NULL, *Out2 = NULL;
	struct K_XY K;
	struct GRAVFFT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);
	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE)
		bailout (GMT_gravfft_usage (API, GMTAPI_USAGE));		/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS)
		bailout (GMT_gravfft_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gravfft", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-Vf", "", options)) Return (API->error);
	Ctrl = New_gravfft_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gravfft_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdfft main code ----------------------------*/

	/* -------------------- Compute only a theoretical model and exit -------------------- */
	if (Ctrl->C.active) {
		double *z_top_or_bot = NULL;

		z_top_or_bot = GMT_memory (GMT, NULL, (size_t)Ctrl->C.n_pt, double);

		Ctrl->C.theor_inc *= Ctrl->misc.k_or_m;
		delta_pt = 2 * M_PI / (Ctrl->C.n_pt * Ctrl->C.theor_inc);	/* Times 2PI because frequency will be used later */
		compute_only_adimtts(GMT, Ctrl, &K, z_top_or_bot, delta_pt);
		sprintf (format, "%s%s%s\n", GMT->current.setting.format_float_out, 
				GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out);
		delta_pt /= (2.0 * M_PI) / Ctrl->misc.k_or_m;			/* Write out frequency, not wavenumber  */

		for (k = 0; k < Ctrl->C.n_pt; k++) {
			freq = (k + 1) * delta_pt;
			if (Ctrl->misc.give_wavelength) freq = 1. / freq;
			sprintf (buffer, format, freq, z_top_or_bot[k]);
			GMT_fputs (buffer, stdout);
		}

		GMT_free (GMT, z_top_or_bot);
		Return (EXIT_SUCCESS);
	}
	/* ---------------------------------------------------------------------------------- */

	if ((GridA = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, Ctrl->In.file[0], NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "Allocates memory and read data file\n");

	if (((Ctrl->T.active && !Ctrl->H.active) && Ctrl->E.n_terms > 1) || (Ctrl->S.active && Ctrl->E.n_terms > 1)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: Due to a bug, or a method limitation (I didn't figure that yet)\n"
			"with the selected options, the number of terms in Parker expansion is reset to one\n"
			"See examples in the manual if you want to compute with higher order expansion\n\n"); 
		Ctrl->E.n_terms = 1;
	}

	GMT_grd_init (GMT, GridA->header, options, TRUE);
	set_grid_radix_size__ (GMT, Ctrl, GridA);		/* This also sets the new pads */
	/* Because we taper and reflect below we DO NOT want any BCs set since that code expects 2 BC rows/cols */
	for (j = 0; j < 4; j++) GridA->header->BC[j] = GMT_BC_IS_DATA;

	/* Now read data into the real positions in the padded complex radix grid */
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA | GMT_GRID_COMPLEX_REAL, Ctrl->In.file[0], GridA) == NULL) {	/* Get subset */
		Return (API->error);
	}

	/* Check that no NaNs are present */
	stop = FALSE;
	for (j = 0; !stop && j < GridA->header->ny; j++)
		for (i = 0; !stop && i < GridA->header->nx; i++) 
			stop = GMT_is_fnan (GridA->data[GMT_IJPR(GridA->header,j,i)]);
	if (stop) {
		GMT_report (GMT, GMT_MSG_FATAL, "Input grid cannot have NaNs!\n");
		Return (EXIT_FAILURE);
	}

	(void)GMT_set_outgrid (GMT, GridA, &Out);	/* TRUE if input is a read-only array; otherwise Out just points to GridA */

	/* ------------------------------------------------------------------------------------ */
	if (Ctrl->In.file[1]) {
		if ((GridB = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, Ctrl->In.file[1], NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
		if(GridA->header->registration != GridB->header->registration) {
			GMT_report (GMT, GMT_MSG_FATAL, "Gravity and bathymetry grids have different registrations!\n");
			Return (EXIT_FAILURE);
		}
		if ((GridA->header->z_scale_factor != GridB->header->z_scale_factor) || 
				(GridA->header->z_add_offset != GridB->header->z_add_offset)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Scale/offset not compatible!\n");
			Return (EXIT_FAILURE);
		}

		if (fabs (GridA->header->inc[GMT_X] - GridB->header->inc[GMT_X]) < 1.0e-6 && 
				fabs (GridA->header->inc[GMT_Y] - GridB->header->inc[GMT_Y]) < 1.0e-6) {
			Out->header->inc[GMT_X] = GridA->header->inc[GMT_X];	/* DO I REALLY NEED THIS??? */
			Out->header->inc[GMT_Y] = GridA->header->inc[GMT_Y];
		}
		else {
			GMT_report (GMT, GMT_MSG_FATAL, "Gravity and bathymetry grid increments do not match!\n");
			Return (EXIT_FAILURE);
		}
		/* ALMOST FOR SURE THERE ARE MEMORY CLEAN UPS TO DO HERE BEFORE RETURNING */
	}
	/* ------------------------------------------------------------------------------------ */

	for (j = 0; j < GridA->header->ny; j++)
		for (i = 0; i < GridA->header->nx; i++)
			GridA->data[GMT_IJPR(GridA->header,j,i)] += (float)Ctrl->misc.z_offset;
	if (Ctrl->s.active)
		for (j = 0; j < GridA->header->ny; j++)
			for (i = 0; i < GridA->header->nx; i++)
				GridA->data[GMT_IJPR(GridA->header,j,i)] *= (float)Ctrl->s.scale;
	if (!Ctrl->misc.rem_nothing) remove_level(GMT, Out, Ctrl);	/* removes either the mean or half-way (if any) */

	if (!Ctrl->L.active) remove_plane__ (GMT, Out);
	if (!Ctrl->N.force_narray) taper_edges__ (GMT, Out);

	/* Load K_XY structure with wavenumbers and dimensions */
	K.delta_kx = 2 * M_PI / (Ctrl->N.nx2 * Out->header->inc[GMT_X]);
	K.delta_ky = 2 * M_PI / (Ctrl->N.ny2 * Out->header->inc[GMT_Y]);
	K.nx2 = Ctrl->N.nx2;	K.ny2 = Ctrl->N.ny2;

	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Give delta_kx, delta_ky units of 2pi/meters  */
		K.delta_kx /= (GMT->current.proj.DIST_M_PR_DEG * cosd (0.5 * (Out->header->wesn[YLO] + Out->header->wesn[YHI])) );
		K.delta_ky /= GMT->current.proj.DIST_M_PR_DEG;
	}

	if (Ctrl->I.active) {		/* Compute admittance or coherence from data and exit */

		GMT_report (GMT, GMT_MSG_NORMAL, "Processing gravity file %s\n", Ctrl->In.file[1]);

		GMT_grd_init (GMT, GridB->header, options, TRUE);
		set_grid_radix_size__ (GMT, Ctrl, GridB);		/* This also sets the new pads */
		/* Because we taper and reflect below we DO NOT want any BCs set since that code expects 2 BC rows/cols */
		for (j = 0; j < 4; j++) GridB->header->BC[j] = GMT_BC_IS_DATA;

		/* Now read data into the real positions in the padded complex radix grid */
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA | GMT_GRID_COMPLEX_REAL, Ctrl->In.file[1], GridB) == NULL) {	/* Get subset */
			Return (API->error);
		}

		/* Check that no NaNs are present in gravity file */
		stop = FALSE;
		for (j = 0; !stop && j < GridB->header->ny; j++)
			for (i = 0; !stop && i < GridB->header->nx; i++) 
				stop = GMT_is_fnan (GridB->data[GMT_IJPR(GridB->header,j,i)]);
		if (stop) {
			GMT_report (GMT, GMT_MSG_FATAL, "Input grid cannot have NaNs!\n");
			Return (EXIT_FAILURE);
		}

		(void)GMT_set_outgrid (GMT, GridB, &Out2);	/* TRUE if input is a read-only array; otherwise Out just points to GridB */
		if (!Ctrl->L.active) remove_plane__ (GMT, Out2);
		if (!Ctrl->N.force_narray) taper_edges__ (GMT, Out2);

		GMT_fft_2d (GMT, Out->data,  Ctrl->N.nx2, Ctrl->N.ny2, GMT_FFT_FWD, GMT_FFT_COMPLEX);
		GMT_fft_2d (GMT, Out2->data, Ctrl->N.nx2, Ctrl->N.ny2, GMT_FFT_FWD, GMT_FFT_COMPLEX);
		do_admittance(GMT, Out, Out2, Ctrl, &K);
		Return (EXIT_SUCCESS);
	}

	topo   = GMT_memory (GMT, NULL, (size_t) GridA->header->size, float);
	raised = GMT_memory (GMT, NULL, (size_t) GridA->header->size, float);

	if (Ctrl->t.active) {	/* Write the FFTed input grid as two grids; Real and Img */
		int plus_minus;
		char *infile_r = NULL, *infile_i = NULL;	/* File names for real and imaginary grids */
		strcpy (line, Ctrl->G.file);
		strcpy (line2, Ctrl->G.file);
		infile_r = strtok (line, ".");
		infile_i = strtok (line2, ".");
		strcat (infile_r, "_real.grd");		strcat (infile_i, "_imag.grd");

		for (j = 0; j < Ctrl->N.ny2; j++) {	/* Put DC at the center of the output array */
			for (i = 0; i < Ctrl->N.nx2; i++) {
				plus_minus = ((i+j) % 2 == 0) ? 1 : -1;
				Out->data[GMT_IJPR(Out->header,j,i)] *= plus_minus;
			}
		}

		GMT_fft_2d (GMT, Out->data, Ctrl->N.nx2, Ctrl->N.ny2, GMT_FFT_FWD, GMT_FFT_COMPLEX);

		/* put DC to one */
		/*datac[ij_data_0(nx_2,ny_2)] = datac[ij_data_0(nx_2,ny_2) + 1] = 1.0;*/
		/* put DC to average of its four neighbours */
		/*datac[ij_data_0(nx_2,ny_2)] = (datac[ij_data_0(nx_2+1,ny_2)] + datac[ij_data_0(nx_2-1,ny_2)]
					    + datac[ij_data_0(nx_2,ny_2+1)] + datac[ij_data_0(nx_2,ny_2-1)])/4;
		datac[ij_data_0(nx_2,ny_2)+1] = (datac[ij_data_0(nx_2+1,ny_2) + 1] + datac[ij_data_0(nx_2-1,ny_2) + 1]
					    + datac[ij_data_0(nx_2,ny_2+1) + 1] + datac[ij_data_0(nx_2,ny_2-1) + 1])/4;*/
		/* Write out frequency, not wavenumber  */
		K.delta_kx /= (2.0*M_PI);		K.delta_ky /= (2.0*M_PI);

		/*h.nx = Ctrl->N.nx2;		h.ny = Ctrl->N.ny2;
		h.x_min = -K.delta_kx * nx_2;
		h.y_min = -K.delta_ky * (ny_2 - 1);
		h.x_max =  K.delta_kx * (nx_2 - 1);
		h.y_max =  K.delta_ky * ny_2;
		h.x_inc = K.delta_kx;
		h.y_inc = K.delta_ky;
		strcpy (h.x_units, "m^(-1)");	strcpy (h.y_units, "m^(-1)");
		strcpy (h.z_units, "fft-ed z_units");
		strcpy (h.title, "Real part of fft transformed input grid");
		strcpy (h.remark, "With origin at lower left corner, kx = 0 at (nx/2 + 1) and ky = 0 at ny/2");
       		GMT_write_grd (infile_r, &h, datac, 0.0, 0.0, 0.0, 0.0, (GMT_LONG *)dummy, TRUE);*/

		//for (i = 2; i < ndatac; i+= 2)	/* move imag to real position in the array */
			//datac[i] = datac[i+1];	/* because that's what write_grd writes to file */

		/*strcpy (h.title, "Imaginary part of fft transformed input grid");
		GMT_write_grd (infile_i, &h, datac, 0.0, 0.0, 0.0, 0.0, (GMT_LONG *)dummy, TRUE);

		if (Ctrl->t.active) write_script();*/

		Return (EXIT_SUCCESS);
	}

	if (Ctrl->Q.active || Ctrl->H.active) {
		GMT_fft_2d (GMT, Out->data,  Ctrl->N.nx2, Ctrl->N.ny2, GMT_FFT_FWD, GMT_FFT_COMPLEX);
		do_isostasy__(GMT, Out, Ctrl, &K);
		GMT_fft_2d (GMT, Out->data,  Ctrl->N.nx2, Ctrl->N.ny2, GMT_FFT_INV, GMT_FFT_COMPLEX);

		scale_out *= (2.0 / Out->header->size);
		for (m = 0; m < Out->header->size; m++)
			Out->data[m] *= (float)scale_out;

		if (!Ctrl->H.active) {
			remove_level(GMT, Out, Ctrl);		/* Because average was no longer zero */
       		for (m = 0; m < Out->header->size; m+= 2)
       			Out->data[m] -= (float)Ctrl->Z.zm;
			/* The data is in the middle of the padded array */

			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA | GMT_GRID_COMPLEX_REAL, Ctrl->G.file, Out) != GMT_OK) {
				Return (API->error);
			}
			Return (EXIT_SUCCESS);
		}
		else {
			Ctrl->misc.z_level = Ctrl->Z.zm;
			Ctrl->misc.rho = Ctrl->T.rho_mc;
			scale_out = 1.0;
		}
	}

	memcpy (topo,   GridA->data, Out->header->size * sizeof (float));
	memcpy (raised, GridA->data, Out->header->size * sizeof (float));
	for (m = 0; m < Out->header->size; m++) Out->data[m] = 0.0;

	GMT_report (GMT, GMT_MSG_NORMAL, "Evatuating for term = 1");

	for (n = 1; n <= Ctrl->E.n_terms; n++) {

		if (n > 1) GMT_report (GMT, GMT_MSG_NORMAL, "-%d", n);

		if (n > 1)
			for (m = 0; m < Out->header->size; m++)
				raised[m] = (float)pow(topo[m], (double)n);

		GMT_fft_2d (GMT, raised,  Ctrl->N.nx2, Ctrl->N.ny2, GMT_FFT_FWD, GMT_FFT_COMPLEX);

		if (Ctrl->D.active || Ctrl->H.active)	/* "classical" anomaly */
			do_parker (GMT, Out, Ctrl, &K, raised, n, Ctrl->misc.rho);
		else if (Ctrl->T.active && !Ctrl->S.active)		/* Compute "loading from top" grav|geoid */
			load_from_top_grid(GMT, Out, Ctrl, &K, raised, n);
		else if (Ctrl->S.active)				/* Compute "loading from below" grav|geoid */
			load_from_below_grid(GMT, Out, Ctrl, &K, raised, n);
		else
			fprintf (stderr, "It SHOULDN'T pass here\n");
	}

	GMT_report (GMT, GMT_MSG_NORMAL, " Inverse FFT...");

	GMT_fft_2d (GMT, Out->data,  Ctrl->N.nx2, Ctrl->N.ny2, GMT_FFT_INV, GMT_FFT_COMPLEX);

	scale_out *= (2.0 / Out->header->size);
	for (m = 0; m < Out->header->size; m+= 2)
		Out->data[m] *= (float)scale_out;

	if (Ctrl->F.active) {
		strcpy (Out->header->title, "Geoid anomalies");
		strcpy (Out->header->z_units, "meter");
	}
	else {
		strcpy (Out->header->title, "Gravity anomalies");
		strcpy (Out->header->z_units, "mGal");
	}
	sprintf (Out->header->remark, "Parker expansion of order %d", Ctrl->E.n_terms);

	GMT_report (GMT, GMT_MSG_NORMAL, "write_output...");

	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA | GMT_GRID_COMPLEX_REAL, Ctrl->G.file, Out) != GMT_OK) {
		Return (API->error);
	}

	GMT_free(GMT, raised);
	GMT_free(GMT, topo);

	GMT_report (GMT, GMT_MSG_NORMAL, "done!\n");

	Return (EXIT_SUCCESS);
}

void remove_level(struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRAVFFT_CTRL *Ctrl) {
	/* Remove the level corresponding to the mean or the half-way point.
	   This level is used as the depth parameter in the exponential term */

	GMT_LONG i, j, ji;
	double z_min, z_max, sum = 0.0, half_w = 0.0;
	float *datac = Grid->data;
	struct GRD_HEADER *h = Grid->header;	/* For shorthand */

	z_min = 1.0e100;	 z_max = -1.0e100;

	for (j = 0; j < h->ny; j++) {
		for (i = 0; i < h->nx; i++) {
			ji = GMT_IJPR(h,j,i);
			if (!GMT_is_fnan(datac[ji])) {
		  		z_min = MIN (z_min, datac[ji]);
		  		z_max = MAX (z_max, datac[ji]);
				sum += datac[ji];
			}
		}
	}

	sum /= (h->nx * h->ny);		half_w = z_min + 0.5 * (z_max - z_min);
	Ctrl->misc.z_level = (Ctrl->misc.mean_or_half_way) ? sum : half_w;

	for (j = 0; j < h->ny; j++) {
		for (i = 0; i < h->nx; i++) {
			ji = GMT_IJPR(h,j,i);
			if (!GMT_is_fnan(datac[ji]))
				datac[ji] -= (float)Ctrl->misc.z_level;
		}
	}
	GMT_report (GMT, GMT_MSG_NORMAL, "1/2-way level = %g, mean = %g : value removed --> %g\n", half_w, sum, Ctrl->misc.z_level);
	Ctrl->misc.z_level = fabs(Ctrl->misc.z_level);	/* Need absolute value for uppward continuation */
}

void taper_edges__ (struct GMT_CTRL *GMT, struct GMT_GRID *Grid) {
	GMT_LONG im, jm, il1, ir1, il2, ir2, jb1, jb2, jt1, jt2;
	GMT_LONG i, j, i_data_start, j_data_start;
	float *datac = Grid->data;
	double scale, cos_wt;
	struct GRD_HEADER *h = Grid->header;	/* For shorthand */

	/* Note that if nx2 = nx+1 and ny2 = ny + 1, then this routine
	 * will do nothing; thus a single row/column of zeros may be
	 * added to the bottom/right of the input array and it cannot
	 * be tapered.  But when (nx2 - nx)%2 == 1 or ditto for y,
	 * this is zero anyway.  */

	i_data_start = GMT->current.io.pad[XLO];	/* For readability */
	j_data_start = GMT->current.io.pad[YHI];
	
	/* First reflect about xmin and xmax, point symmetric about edge point */

	for (im = 1; im <= i_data_start; im++) {
		il1 = -im;	/* Outside xmin; left of edge 1  */
		ir1 = im;	/* Inside xmin; right of edge 1  */
		il2 = il1 + h->nx - 1;	/* Inside xmax; left of edge 2  */
		ir2 = ir1 + h->nx - 1;	/* Outside xmax; right of edge 2  */
		for (j = 0; j < h->ny; j++) {
			datac[GMT_IJPR(h,j,il1)] = (float)2.0*datac[GMT_IJPR(h,j,0)] - datac[GMT_IJPR(h,j,ir1)];
			datac[GMT_IJPR(h,j,ir2)] = (float)2.0*datac[GMT_IJPR(h,j,h->nx-1)] - datac[GMT_IJPR(h,j,il2)];
		}
	}

	/* Next, reflect about ymin and ymax.
	 * At the same time, since x has been reflected,
	 * we can use these vals and taper on y edges */

	scale = M_PI / (j_data_start + 1);

	for (jm = 1; jm <= j_data_start; jm++) {
		jb1 = -jm;	/* Outside ymin; bottom side of edge 1  */
		jt1 = jm;	/* Inside ymin; top side of edge 1  */
		jb2 = jb1 + h->ny - 1;	/* Inside ymax; bottom side of edge 2  */
		jt2 = jt1 + h->ny - 1;	/* Outside ymax; bottom side of edge 2  */
		cos_wt = 0.5 * (1.0 + cos(jm * scale) );
		for (i = -i_data_start; i < h->mx - i_data_start; i++) {
			datac[GMT_IJPR(h,jb1,i)] = (float)(cos_wt * (2.0*datac[GMT_IJPR(h,0,i)] - datac[GMT_IJPR(h,jt1,i)]));
			datac[GMT_IJPR(h,jt2,i)] = (float)(cos_wt * (2.0*datac[GMT_IJPR(h,h->ny-1,i)] - datac[GMT_IJPR(h,jb2,i)]));
		}
	}

	/* Now, cos taper the x edges */
	scale = M_PI / (i_data_start + 1);
	for (im = 1; im <= i_data_start; im++) {
		il1 = -im;
		ir1 = im;
		il2 = il1 + h->nx - 1;
		ir2 = ir1 + h->nx - 1;
		cos_wt = 0.5 * (1.0 + cos (im * scale));
		for (j = -j_data_start; j < h->my - j_data_start; j++) {
			datac[GMT_IJPR(h,j,il1)] *= (float)cos_wt;
			datac[GMT_IJPR(h,j,ir2)] *= (float)cos_wt;
		}
	}
	GMT_report (GMT, GMT_MSG_NORMAL, "Data reflected and tapered\n");
}

double kx__ (GMT_LONG k, struct K_XY *K) {
	/* Return the value of kx given k,
		where kx = 2 pi / lambda x,
		and k refers to the position
		in the datac array, datac[k].  */

	GMT_LONG ii = (k/2)%(K->nx2);
	if (ii > (K->nx2)/2) ii -= (K->nx2);
	return (ii * K->delta_kx);
}

double ky__ (GMT_LONG k, struct K_XY *K) {
	/* Return the value of ky given k,
	 * where ky = 2 pi / lambda y,
	 * and k refers to the position
	 *in the datac array, datac[k].  */

	GMT_LONG jj = (k/2)/(K->nx2);
	if (jj > (K->ny2)/2) jj -= (K->ny2);
	return (jj * K->delta_ky);
}

double modk__ (GMT_LONG k, struct K_XY *K) {
	/* Return the value of sqrt(kx*kx + ky*ky),
	 * given k, where k is array position.  */

	return (hypot (kx__ (k, K), ky__ (k, K)));
}

void do_isostasy__ (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K) {
	/* Do the isostatic response function convolution in the Freq domain.
	All units assumed to be in SI (that is kx, ky, modk wavenumbers in m**-1,
	densities in kg/m**3, Te in m, etc.
	rw, the water density, is used to set the Airy ratio and the restoring
	force on the plate (rm - ri)*gravity if ri = rw; so use zero for topo in air (ri changed to rl).  */
	int     k;
	double  airy_ratio, rigidity_d, d_over_restoring_force, mk, k2, k4, transfer_fn;
	float *datac = Grid->data;

	/*   te	 Elastic thickness, SI units (m)  */
	/*   rl	 Load density, SI units  */
	/*   rm	 Mantle density, SI units  */
	/*   rw	 Water density, SI units  */
	
	rigidity_d = (YOUNGS_MODULUS * Ctrl->T.te * Ctrl->T.te * Ctrl->T.te) / (12.0 * (1.0 - POISSONS_RATIO * POISSONS_RATIO));
	d_over_restoring_force = rigidity_d / ( (Ctrl->T.rhom - Ctrl->T.rhol) * NORMAL_GRAVITY);
	airy_ratio = -(Ctrl->T.rhol - Ctrl->T.rhow)/(Ctrl->T.rhom - Ctrl->T.rhol);
 
	if (Ctrl->T.te == 0.0) {      /* Airy isostasy; scale global variable scale_out and return */
		scale_out *= airy_ratio;
		return;
	}

	for (k = 0; k < Grid->header->size; k+= 2) {
		mk = modk__(k, K);
		k2 = mk * mk;	k4 = k2 * k2;
		transfer_fn = airy_ratio / ( (d_over_restoring_force * k4) + 1.0);	  
		datac[k] *= (float)transfer_fn;
		datac[k+1] *= (float)transfer_fn;
	}
}

void do_parker (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, float *raised, int n, double rho) {
	int i, k;
	double f, p, t, mk, v, c;
	float *datac = Grid->data;

	f = 1.0;
	for (i = 2; i <= n; i++) f *= i;	/* n! */
	p = n - 1.0;

	c = 1.0e5 * 2.0 * M_PI * GRAVITATIONAL_CONST * rho / f; /* Gives mGal */
	if (Ctrl->F.active) c /= 980619.92;

	for (k = 0; k < Grid->header->size; k+= 2) {
		mk = modk__(k, K);
		if (p == 0.0)
			t = 1.0;
		else if (p == 1.0)
			t = mk;
		else
			t = pow (mk, p);

		v = c * exp (-mk * Ctrl->misc.z_level) * t;
		if (Ctrl->F.active && mk > 0.0) v /= mk;
		datac[k] += (float) (v * raised[k]);
		datac[k+1] += (float) (v * raised[k+1]);
	}
}

GMT_LONG get_non_symmetric_f__ (GMT_LONG *f, GMT_LONG n) {
	/* Return the product of the non-symmetric factors in f[]  */
	GMT_LONG i = 0, j = 1, retval = 1;

	if (n == 1) return (f[0]);

	while (i < n) {
		while (j < n && f[j] == f[i]) j++;
		if ((j-i)%2) retval *= f[i];
		i = j;
		j = i + 1;
	}
	if (retval == 1) retval = 0;	/* There are no non-sym factors  */
	return (retval);
}

void fourt_stats__ (struct GMT_CTRL *C, GMT_LONG nx, GMT_LONG ny, GMT_LONG *f, double *r, GMT_LONG *s, double *t) {
	/* Find the proportional run time, t, and rms relative error, r,
	 * of a Fourier transform of size nx,ny.  Also gives s, the size
	 * of the workspace that will be needed by the transform.
	 * To use this routine for a 1-D transform, set ny = 1.
	 *
	 * This is all based on the comments in Norman Brenner's code
	 * FOURT, from which our C codes are translated.
	 * Brenner says:
	 * r = 3 * pow(2, -FSIGNIF) * sum{ pow(prime_factors, 1.5) }
	 * where FSIGNIF is the smallest bit in the floating point fraction.
	 *
	 * Let m = largest prime factor in the list of factors.
	 * Let p = product of all primes which appear an odd number of
	 * times in the list of prime factors.  Then the worksize needed
	 * s = max(m,p).  However, we know that if n is radix 2, then no
	 * work is required; yet this formula would say we need a worksize
	 * of at least 2.  So I will return s = 0 when max(m,p) = 2.
	 *
	 * I have two different versions of the comments in FOURT, with
	 * different formulae for t.  The simple formula says
	 *      t = n * (sum of prime factors of n).
	 * The more complicated formula gives coefficients in microsecs
	 * on a cdc3300 (ancient history, but perhaps proportional):
	 *      t = 3000 + n*(500 + 43*s2 + 68*sf + 320*nf),
	 * where s2 is the sum of all factors of 2, sf is the sum of all
	 * factors greater than 2, and nf is the number of factors != 2.
	 * We know that factors of 2 are very good to have, and indeed,
	 * Brenner's code calls different routines depending on whether
	 * the transform is of size 2 or not, so I think that the second
	 * formula is more correct, using proportions of 43:68 for 2 and
	 * non-2 factors.  So I will use the more complicated formula.
	 * However, I realize that the actual numbers are wrong for today's
	 * architectures, and the relative proportions may be wrong as well.
	 *
	 * W. H. F. Smith, 26 February 1992.
	 *  */

	GMT_LONG n_factors, i, sum2, sumnot2, nnot2;
	GMT_LONG nonsymx, nonsymy, nonsym, storage, ntotal;
	double err_scale;

	/* Find workspace needed.  First find non_symmetric factors in nx, ny  */
	n_factors = GMT_get_prime_factors (C, nx, f);
	nonsymx = get_non_symmetric_f__ (f, n_factors);
	n_factors = GMT_get_prime_factors (C, ny, f);
	nonsymy = get_non_symmetric_f__ (f, n_factors);
	nonsym = MAX (nonsymx, nonsymy);

	/* Now get factors of ntotal  */
	ntotal = GMT_get_nm (C, nx, ny);
	n_factors = GMT_get_prime_factors (C, ntotal, f);
	storage = MAX (nonsym, f[n_factors-1]);
	*s = (storage == 2) ? 0 : storage;

	/* Now find time and error estimates */

	err_scale = 0.0;
	sum2 = sumnot2 = nnot2 = 0;
	for(i = 0; i < n_factors; i++) {
		if (f[i] == 2)
			sum2 += f[i];
		else {
			sumnot2 += f[i];
			nnot2++;
		}
		err_scale += pow ((double)f[i], 1.5);
	}
	*t = 1.0e-06 * (3000.0 + ntotal * (500.0 + 43.0 * sum2 + 68.0 * sumnot2 + 320.0 * nnot2));
	*r = err_scale * 3.0 * pow (2.0, -FSIGNIF);
	
	return;
}

void suggest_fft__ (struct GMT_CTRL *GMT, GMT_LONG nx, GMT_LONG ny, struct FFT_SUGGESTION *fft_sug, GMT_LONG do_print) {
	GMT_LONG f[32], xstop, ystop;
	GMT_LONG nx_best_t, ny_best_t;
	GMT_LONG nx_best_e, ny_best_e;
	GMT_LONG nx_best_s, ny_best_s;
	GMT_LONG nxg, nyg;       /* Guessed by this routine  */
	GMT_LONG nx2, ny2, nx3, ny3, nx5, ny5;   /* For powers  */
	GMT_LONG current_space, best_space, given_space, e_space, t_space;
	double current_time, best_time, given_time, s_time, e_time;
	double current_err, best_err, given_err, s_err, t_err;

	fourt_stats__ (GMT, nx, ny, f, &given_err, &given_space, &given_time);
	given_space += nx * ny;
	given_space *= 8;
	if (do_print) GMT_report (GMT, GMT_MSG_FATAL, " Data dimension\t%ld %ld\ttime factor %.8g\trms error %.8e\tbytes %ld\n",
		nx, ny, given_time, given_err, given_space);

	best_err = s_err = t_err = given_err;
	best_time = s_time = e_time = given_time;
	best_space = t_space = e_space = given_space;
	nx_best_e = nx_best_t = nx_best_s = nx;
	ny_best_e = ny_best_t = ny_best_s = ny;

	xstop = 2 * nx;
	ystop = 2 * ny;

	for (nx2 = 2; nx2 <= xstop; nx2 *= 2) {
		for (nx3 = 1; nx3 <= xstop; nx3 *= 3) {
			for (nx5 = 1; nx5 <= xstop; nx5 *= 5) {
				nxg = nx2 * nx3 * nx5;
				if (nxg < nx || nxg > xstop) continue;

				for (ny2 = 2; ny2 <= ystop; ny2 *= 2) {
					for (ny3 = 1; ny3 <= ystop; ny3 *= 3) {
						for (ny5 = 1; ny5 <= ystop; ny5 *= 5) {
							nyg = ny2 * ny3 * ny5;
							if (nyg < ny || nyg > ystop) continue;

							fourt_stats__ (GMT, nxg, nyg, f, &current_err, &current_space, &current_time);
							current_space += nxg*nyg;
							current_space *= 8;
							if (current_err < best_err) {
								best_err = current_err;
								nx_best_e = nxg;
								ny_best_e = nyg;
								e_time = current_time;
								e_space = current_space;
							}
							if (current_time < best_time) {
								best_time = current_time;
								nx_best_t = nxg;
								ny_best_t = nyg;
								t_err = current_err;
								t_space = current_space;
							}
							if (current_space < best_space) {
								best_space = current_space;
								nx_best_s = nxg;
								ny_best_s = nyg;
								s_time = current_time;
								s_err = current_err;
							}

						}
					}
				}

			}
		}
	}

	if (do_print) {
		GMT_report (GMT, GMT_MSG_FATAL, " Highest speed\t%ld %ld\ttime factor %.8g\trms error %.8e\tbytes %ld\n",
			nx_best_t, ny_best_t, best_time, t_err, t_space);
		GMT_report (GMT, GMT_MSG_FATAL, " Most accurate\t%ld %ld\ttime factor %.8g\trms error %.8e\tbytes %ld\n",
			nx_best_e, ny_best_e, e_time, best_err, e_space);
		GMT_report (GMT, GMT_MSG_FATAL, " Least storage\t%ld %ld\ttime factor %.8g\trms error %.8e\tbytes %ld\n",
			nx_best_s, ny_best_s, s_time, s_err, best_space);
	}
	/* Fastest solution */
	fft_sug[0].nx = nx_best_t;
	fft_sug[0].ny = ny_best_t;
	fft_sug[0].worksize = (t_space/8) - (nx_best_t * ny_best_t);
	fft_sug[0].totalbytes = t_space;
	fft_sug[0].run_time = best_time;
	fft_sug[0].rms_rel_err = t_err;
	/* Most accurate solution */
	fft_sug[1].nx = nx_best_e;
	fft_sug[1].ny = ny_best_e;
	fft_sug[1].worksize = (e_space/8) - (nx_best_e * ny_best_e);
	fft_sug[1].totalbytes = e_space;
	fft_sug[1].run_time = e_time;
	fft_sug[1].rms_rel_err = best_err;
	/* Least storage solution */
	fft_sug[2].nx = nx_best_s;
	fft_sug[2].ny = ny_best_s;
	fft_sug[2].worksize = (best_space/8) - (nx_best_s * ny_best_s);
	fft_sug[2].totalbytes = best_space;
	fft_sug[2].run_time = s_time;
	fft_sug[2].rms_rel_err = s_err;

	return;
}

void set_grid_radix_size__ (struct GMT_CTRL *GMT, struct GRAVFFT_CTRL *Ctrl, struct GMT_GRID *Gin) {
	GMT_LONG k, worksize, factors[32];
	double tdummy, edummy;
	struct FFT_SUGGESTION fft_sug[3];
		
	/* Get dimensions as may be appropriate */
	if (Ctrl->N.n_user_set) {
		if (Ctrl->N.nx2 < Gin->header->nx || Ctrl->N.ny2 < Gin->header->ny) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: You specified -Nnx/ny smaller than input grid.  Ignored.\n");
			Ctrl->N.n_user_set = FALSE;
		}
	}
	if (!(Ctrl->N.n_user_set) ) {
		if (Ctrl->N.force_narray) {
			Ctrl->N.nx2 = Gin->header->nx;
			Ctrl->N.ny2 = Gin->header->ny;
			for (k = 0; k < 4; k++) Gin->header->BC[k] = GMT_BC_IS_DATA;	/* This bypasses BC pad checking later since there is no pad */
		}
		else {
			suggest_fft__ (GMT, Gin->header->nx, Gin->header->ny, fft_sug, (GMT_is_verbose (GMT, GMT_MSG_NORMAL) || Ctrl->N.suggest_narray));
			if (fft_sug[1].totalbytes < fft_sug[0].totalbytes) {
				/* The most accurate solution needs same or less storage
				 * as the fastest solution; use the most accurate's dimensions */
				Ctrl->N.nx2 = fft_sug[1].nx;
				Ctrl->N.ny2 = fft_sug[1].ny;
			}
			else {
				/* Use the sizes of the fastest solution  */
				Ctrl->N.nx2 = fft_sug[0].nx;
				Ctrl->N.ny2 = fft_sug[0].ny;
			}
		}
	}

	/* Get here when nx2 and ny2 are set to the vals we will use.  */

	fourt_stats__ (GMT, Ctrl->N.nx2, Ctrl->N.ny2, factors, &edummy, &worksize, &tdummy);
	GMT_report (GMT, GMT_MSG_NORMAL, " Data dimension %d %d\tFFT dimension %ld %ld\n",
		Gin->header->nx, Gin->header->ny, Ctrl->N.nx2, Ctrl->N.ny2);

	if (worksize) {
		if (worksize < Ctrl->N.nx2) worksize = Ctrl->N.nx2;
		if (worksize < Ctrl->N.ny2) worksize = Ctrl->N.ny2;
		worksize *= 2;
	}
	else
		worksize = 4;


	/* Put the data in the middle of the padded array */

	GMT->current.io.pad[XLO] = (Ctrl->N.nx2 - Gin->header->nx)/2;	/* zero if nx2 < Gin->header->nx+1  */
	GMT->current.io.pad[YHI] = (Ctrl->N.ny2 - Gin->header->ny)/2;
	GMT->current.io.pad[XHI] = Ctrl->N.nx2 - Gin->header->nx - GMT->current.io.pad[XLO];
	GMT->current.io.pad[YLO] = Ctrl->N.ny2 - Gin->header->ny - GMT->current.io.pad[YHI];
}

void remove_plane__ (struct GMT_CTRL *GMT, struct GMT_GRID *Grid) {
	/* Remove the best-fitting plane by least squares.

	Let plane be z = a0 + a1 * x + a2 * y.  Choose the
	center of x,y coordinate system at the center of 
	the array.  This will make the Normal equations 
	matrix G'G diagonal, so solution is trivial.  Also,
	spend some multiplications on normalizing the 
	range of x,y into [-1,1], to avoid roundoff error.  */

	GMT_LONG i, j, ij, one_or_zero;
	double x_half_length, one_on_xhl, y_half_length, one_on_yhl;
	double sumx2, sumy2, data_var, x, y, z, a[3];
	float *datac = Grid->data;

	one_or_zero = (Grid->header->registration == GMT_PIXEL_REG) ? 0 : 1;
	x_half_length = 0.5 * (Grid->header->nx - one_or_zero);
	one_on_xhl = 1.0 / x_half_length;
	y_half_length = 0.5 * (Grid->header->ny - one_or_zero);
	one_on_yhl = 1.0 / y_half_length;

	sumx2 = sumy2 = data_var = a[2] = a[1] = a[0] = 0.0;

	for (j = 0; j < Grid->header->ny; j++) {
		y = one_on_yhl * (j - y_half_length);
		for (i = 0; i < Grid->header->nx; i++) {
			x = one_on_xhl * (i - x_half_length);
			z = datac[GMT_IJPR(Grid->header,j,i)];
			a[0] += z;
			a[1] += z*x;
			a[2] += z*y;
			sumx2 += x*x;
			sumy2 += y*y;
		}
	}
	a[0] /= (Grid->header->nx*Grid->header->ny);
	a[1] /= sumx2;
	a[2] /= sumy2;
	for (j = 0; j < Grid->header->ny; j++) {
		y = one_on_yhl * (j - y_half_length);
		for (i = 0; i < Grid->header->nx; i++) {
			ij = GMT_IJPR (Grid->header, j, i);
			x = one_on_xhl * (i - x_half_length);
			datac[ij] -= (float)(a[0] + a[1]*x + a[2]*y);
			data_var += (datac[ij] * datac[ij]);
		}
	}
	data_var = sqrt(data_var / (Grid->header->nx*Grid->header->ny - 1));
	/* Rescale a1,a2 into user's units, in case useful later */
	a[1] *= (2.0/(Grid->header->wesn[XHI] - Grid->header->wesn[XLO]));
	a[2] *= (2.0/(Grid->header->wesn[YHI] - Grid->header->wesn[YLO]));
	GMT_report (GMT, GMT_MSG_NORMAL, "Plane removed.  Mean, S.D., Dx, Dy: %.8g\t%.8g\t%.8g\t%.8g\n", a[0], data_var, a[1], a[2]);
}

void do_admittance (struct GMT_CTRL *GMT, struct GMT_GRID *GridA, struct GMT_GRID *GridB, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K) {
	/*	The following are the comments of the routine do_spectrum
	 *	of grdfft from which this code was adapted.
	 *
	 *	"This is modeled on the 1-D case, using the following ideas:
	 *	In 1-D, we ensemble average over samples of length L = 
	 *	n * dt.  This gives n/2 power spectral estimates, at
	 *	frequencies i/L, where i = 1, n/2.  If we have a total
	 *	data set of ndata, we can make nest=ndata/n independent
	 *	estimates of the power.  Our standard error is then
	 *	1/sqrt(nest).
	 *	In making 1-D estimates from 2-D data, we may choose
	 *	n and L from nx2 or ny2 and delta_kx, delta_ky as 
	 *	appropriate.  In this routine, we are giving the sum over
	 * 	all frequencies in the other dimension; that is, an
	 *	approximation of the integral."
	 */

	int		k, k_0 = 0, nk, ifreq, *nused;
	char	format[64], buffer[256];
	double	delta_k, r_delta_k, freq;
	double	*out, *err_bar, *coh, *b_pow, *g_pow, *co_spec, *quad;
	float	*datac = GridA->data;
	float	*in_grv = GridB->data;
	double	*z_from_below = NULL, *z_from_top = NULL;

	if (K->delta_kx < K->delta_ky) 
		{delta_k = K->delta_kx;	nk = (int)Ctrl->N.nx2/2;}
	else 
		{delta_k = K->delta_ky;	nk = (int)Ctrl->N.ny2/2;}

	/* Get an array for summing stuff */
	b_pow   = GMT_memory (GMT, NULL, (size_t)nk, double );
	g_pow   = GMT_memory (GMT, NULL, (size_t)nk, double);
	err_bar = GMT_memory (GMT, NULL, (size_t)nk, double);
	co_spec = GMT_memory (GMT, NULL, (size_t)nk, double);
	quad    = GMT_memory (GMT, NULL, (size_t)nk, double);
	coh     = GMT_memory (GMT, NULL, (size_t)nk, double);
	out     = GMT_memory (GMT, NULL, (size_t)nk, double);
	nused   = GMT_memory (GMT, NULL, (size_t)Ctrl->N.nx2*Ctrl->N.ny2, int);
	if (Ctrl->misc.from_below)
		z_from_below = GMT_memory (GMT, NULL, (size_t)nk, double);
	if (Ctrl->misc.from_top)
		z_from_top = GMT_memory (GMT, NULL, (size_t)nk, double);

	if (Ctrl->misc.coherence)
		Ctrl->I.active = FALSE;

	/* Loop over it all, summing and storing, checking range for r */

	r_delta_k = 1.0 / delta_k;
	
	for (k = 2; k < GridA->header->size; k+= 2) {
		freq = modk__(k, K);
		ifreq = lrint(fabs(freq) * r_delta_k) - 1;
		if (ifreq < 0) ifreq = 0;	/* Might happen when doing r average  */
		if (ifreq >= nk) continue;	/* Might happen when doing r average  */
		b_pow[ifreq] += (datac[k]*datac[k] + datac[k+1]*datac[k+1]); /* B*B-conj = bat power */
		g_pow[ifreq] += (in_grv[k]*in_grv[k] + in_grv[k+1]*in_grv[k+1]); /* G*G-conj = grav power */
		co_spec[ifreq] += (in_grv[k]*datac[k] + in_grv[k+1]*datac[k+1]); /* keep only real of G*B-conj */
		quad[ifreq] += (datac[k+1]*in_grv[k] - in_grv[k+1]*datac[k]);
		nused[ifreq]++;
	}

	for (k = k_0; k < nk; k++)	/* Coherence is allways needed for error bar computing */
			coh[k] = (co_spec[k]*co_spec[k] + quad[k]*quad[k]) / (b_pow[k]*g_pow[k]);

	for (k = k_0; k < nk; k++) {
		if (Ctrl->I.active) {
			out[k] = co_spec[k] / b_pow[k];
			/*err_bar[k] = out[k] * fabs (sqrt ((1.0 - coh[k]) / 2.0 * coh[k]) * out[k]) / sqrt(nused[k]); Versao do Smith*/
			err_bar[k] = out[k] * fabs (sqrt ((1.0 - coh[k]) / (2.0 * coh[k] * nused[k])) );
		}
		else if (Ctrl->misc.coherence) {
			out[k] = coh[k];
			err_bar[k] = out[k] * (1.0 - coh[k]) * sqrt(2.0 / coh[k]) / sqrt(nused[k]);
		}
	}

	/* Now get here when array is summed.  */
	delta_k /= (2.0*M_PI);				/* Write out frequency, not wavenumber  */
	if (Ctrl->misc.from_below) 		/* compute theoretical "load from below" admittance */
		load_from_below_admit(GMT, Ctrl, K, z_from_below);
	if (Ctrl->misc.from_top) 		/* compute theoretical "load from top" admittance */
		load_from_top_admit(GMT, Ctrl, K, z_from_top);

	if (Ctrl->misc.from_below || Ctrl->misc.from_top)
		sprintf (format, "%s%s%s%s%s%s%s\n", GMT->current.setting.format_float_out, 
			GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, 
			GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, 
			GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out);
	else
		sprintf (format, "%s%s%s%s%s\n", GMT->current.setting.format_float_out, 
			GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out, 
			GMT->current.setting.io_col_separator, GMT->current.setting.format_float_out);

	delta_k *= Ctrl->misc.k_or_m;
	for (k = k_0; k < nk; k++) {
		freq = (k + 1) * delta_k;
		if (Ctrl->misc.give_wavelength) freq = 1.0/freq;
		if (Ctrl->misc.from_below) {
			sprintf (buffer, format, freq, out[k], err_bar[k], z_from_below[k]);
			GMT_fputs (buffer, stdout);
		}
		else if (Ctrl->misc.from_top) {
			sprintf (buffer, format, freq, out[k], err_bar[k], z_from_top[k]);
			GMT_fputs (buffer, stdout);
		}
		else {
			sprintf (buffer, format, freq, out[k], err_bar[k]);
			GMT_fputs (buffer, stdout);
		}
	}
	GMT_free (GMT, out);
	GMT_free (GMT, b_pow);
	GMT_free (GMT, g_pow);
	GMT_free (GMT, err_bar);
	GMT_free (GMT, co_spec);
	GMT_free (GMT, coh);
	GMT_free (GMT, quad);
	GMT_free (GMT, nused);
	if (Ctrl->misc.from_below) GMT_free (GMT, z_from_below);
	if (Ctrl->misc.from_top) GMT_free (GMT, z_from_top);
}

void compute_only_adimtts(struct GMT_CTRL *GMT, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, double *z_top_or_bot, double delta_pt) {

	/* Calls the apropriate function to compute the theoretical admittance.
	   Take profit of external variables used in other program's options */

	K->delta_kx = K->delta_ky = delta_pt;
	Ctrl->N.nx2 = Ctrl->N.ny2 = Ctrl->C.n_pt * 2;

	if (Ctrl->misc.from_top)
		load_from_top_admit(GMT, Ctrl, K, z_top_or_bot);
	else
		load_from_below_admit(GMT, Ctrl, K, z_top_or_bot);
}

void load_from_below_admit(struct GMT_CTRL *GMT, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, double *z_from_below) {

	/* Compute theoretical admittance for the "loading from below" model
	   M. McNutt & Shure (1986) in same |k| of computed data admittance

	   The z_from_below is a vector that must have been previously allocated 
	   with a size of "nk" like that variable is computed here.	*/

	int k, nk;
	double	earth_curvature, alfa, delta_k, freq, D, twopi, t1, t2, t3;

	if (K->delta_kx < K->delta_ky)
		{delta_k = K->delta_kx;	 nk = (int)Ctrl->N.nx2/2;}
	else 
		{delta_k = K->delta_ky;	 nk = (int)Ctrl->N.ny2/2;}

	twopi = 2. * M_PI;
	delta_k /= twopi;	/* Use frequency, not wavenumber  */
	D = (YOUNGS_MODULUS * Ctrl->T.te * Ctrl->T.te * Ctrl->T.te) / (12.0 * (1.0 - POISSONS_RATIO * POISSONS_RATIO));
	alfa = pow(twopi, 4.) * D / (NORMAL_GRAVITY * Ctrl->T.rho_mc);

	for (k = 0; k < nk; k++) {
		freq = (k + 1) * delta_k;
		earth_curvature = (sphericity) ? (2 * earth_rad * freq) / (4 * M_PI * earth_rad * freq + 1) : 1.;
		t1 = earth_curvature * (twopi * GRAVITATIONAL_CONST);
		if (!Ctrl->F.active) t1 *= 1.0e5;		/* to have it in mGals */
		if (Ctrl->F.active) t1 /= (NORMAL_GRAVITY * freq * twopi);
		t2 = Ctrl->T.rho_cw * exp(-twopi * freq * Ctrl->misc.z_level) + 
			Ctrl->T.rho_mc * exp(-twopi * freq * Ctrl->Z.zm);
		t3 = -(Ctrl->T.rho_mw + Ctrl->T.rho_mc * pow(freq,4.) * alfa) * exp(-twopi * freq * Ctrl->Z.zl);
		z_from_below[k] = t1 * (t2 + t3);
	}
}

void load_from_top_admit(struct GMT_CTRL *GMT, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, double *z_from_top) {

	/* Compute theoretical admittance for the "loading from top" model
	   M. McNutt & Shure (1986) in same |k| of computed data admittance
	   
	   The z_from_top is a vector that must have been previously allocated 
	   with a size of "nk" like that variable is computed here.	*/

	int k, nk;
	double	earth_curvature, alfa, delta_k, freq, D, twopi, t1, t2;

	if (K->delta_kx < K->delta_ky) 
		{delta_k = K->delta_kx;	 nk = (int)Ctrl->N.nx2/2;}
	else 
		{delta_k = K->delta_ky;	 nk = (int)Ctrl->N.ny2/2;}

	twopi = 2. * M_PI;
	delta_k /= twopi;	/* Use frequency, not wavenumber  */
	D = (YOUNGS_MODULUS * Ctrl->T.te * Ctrl->T.te * Ctrl->T.te) / (12.0 * (1.0 - POISSONS_RATIO * POISSONS_RATIO));
	alfa = pow(twopi,4.) * D / (NORMAL_GRAVITY * Ctrl->T.rho_mc);

	for (k = 0; k < nk; k++) {
		freq = (k + 1) * delta_k;
		earth_curvature = (sphericity) ? (2 * earth_rad * freq) / (4 * M_PI * earth_rad * freq + 1) : 1.;
		t1 = earth_curvature * (twopi * GRAVITATIONAL_CONST);
		if (!Ctrl->F.active) t1 *= 1.0e5;		/* to have it in mGals */
		if (Ctrl->F.active) t1 /= (NORMAL_GRAVITY * freq * twopi);
		t2 = exp(-twopi * freq * Ctrl->misc.z_level) - exp(-twopi * freq * Ctrl->Z.zm) / (1 + alfa*pow(freq,4.));   
		z_from_top[k] = t1 * Ctrl->T.rho_cw * t2;
	}
}

void load_from_top_grid(struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, float *raised, int n) {

	/* Computes the gravity|geoid grid due to the effect of the bathymetry using the theoretical
	admittance for the "loading from top" model --  M. McNutt & Shure (1986)  */

	GMT_LONG k, i;
	double	earth_curvature, alfa, D, twopi, t1, t2, f, p, t, mk;
	float *datac = Grid->data;

	f = 1.0;
	for (i = 2; i <= n; i++)
		f *= i;	/* n! */
	p = n - 1.0;

	twopi = 2 * M_PI;
	D = (YOUNGS_MODULUS * Ctrl->T.te * Ctrl->T.te * Ctrl->T.te) / (12.0 * (1.0 - POISSONS_RATIO * POISSONS_RATIO));
	alfa = pow(twopi,4.) * D / (NORMAL_GRAVITY * Ctrl->T.rho_mc);
	raised[0] = 0.0;		raised[1] = 0.0;

	for (k = 0; k < Grid->header->size; k+= 2) {
		mk = modk__(k, K) / twopi;
		if (p == 0.0)
			t = 1.0;
		else if (p == 1.0)
			t = mk;
		else
			t = pow (mk, p);
		earth_curvature = (sphericity) ? (2 * earth_rad * mk) / (4 * M_PI * earth_rad * mk + 1) : 1.;
		t1 = earth_curvature * (twopi * GRAVITATIONAL_CONST);
		if (!Ctrl->F.active) t1 *= 1.0e5;		/* to have it in mGals */
		if (Ctrl->F.active) t1 /= (NORMAL_GRAVITY * mk * twopi);
		t2 = exp(-twopi * mk * Ctrl->misc.z_level) - exp(-twopi * mk * Ctrl->Z.zm) / (1 + alfa*pow(mk,4.));
		datac[k] += (float) ((Ctrl->T.rho_cw * t1 * t2) * t / f * raised[k]);
		datac[k+1] += (float) ((Ctrl->T.rho_cw * t1 * t2) * t / f * raised[k+1]);
	}
}

void load_from_below_grid(struct GMT_CTRL *GMT, struct GMT_GRID *Grid, struct GRAVFFT_CTRL *Ctrl, struct K_XY *K, float *raised, int n) {

	/* Computes the gravity|geoid grid due to the effect of the bathymetry using the theoretical
	admittance for the "loading from below" model --  M. McNutt & Shure (1986)  */

	GMT_LONG k, i;
	double	earth_curvature, alfa, D, twopi, t1, t2, t3, f, p, t, mk;
	float *datac = Grid->data;

	f = 1.0;
	for (i = 2; i <= n; i++)
		f *= i;	/* n! */
	p = n - 1.0;

	twopi = 2. * M_PI;
	D = (YOUNGS_MODULUS * Ctrl->T.te * Ctrl->T.te * Ctrl->T.te) / (12.0 * (1.0 - POISSONS_RATIO * POISSONS_RATIO));
	alfa = pow(twopi,4.) * D / (NORMAL_GRAVITY * Ctrl->T.rho_mc);
	raised[0] = 0.0;		raised[1] = 0.0;

	for (k = 0; k < Grid->header->size; k+= 2) {
		mk = modk__(k, K) / twopi;
		if (p == 0.0)
			t = 1.0;
		else if (p == 1.0)
			t = mk;
		else
			t = pow (mk, p);
		earth_curvature = (sphericity) ? (2 * earth_rad * mk) / (4 * M_PI * earth_rad * mk + 1) : 1.;
		t1 = earth_curvature * (twopi * GRAVITATIONAL_CONST);
		if (!Ctrl->F.active) t1 *= 1.0e5;		/* to have it in mGals */
		if (Ctrl->F.active) t1 /= (NORMAL_GRAVITY * mk * twopi);
		t2 = Ctrl->T.rho_cw * exp(-twopi * mk * Ctrl->misc.z_level) + 
			 Ctrl->T.rho_mc * exp(-twopi * mk * Ctrl->Z.zm);
		t3 = -(Ctrl->T.rho_mw + Ctrl->T.rho_mc * pow(mk,4.) * alfa) * exp(-twopi * mk * Ctrl->Z.zl);
		datac[k] += (float) ((t1 * (t2 + t3)) * t / f * raised[k]);
		datac[k+1] += (float) ((t1 * (t2 + t3)) * t / f * raised[k+1]);
	}
}

void write_script(void) {

}
