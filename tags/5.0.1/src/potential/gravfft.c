/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Date:        09-OCT-1994
 * Updated:	28-Feb-1999
 *         	05-SEP-2002
 *         	13-SEP-2002
 *
 * */

#include "gmt.h"

#ifndef FSIGNIF
#define FSIGNIF 24
#endif

/* Macro definition ij_data(i,j) finds the array index to an element
	containing the real data(i,j) in the padded complex array */

#define ij_data(i,j) (2*(nx2*((j)+j_data_start)+(i)+i_data_start))
#define ij_data_0(i,j) (2*(nx2*((j)+0)+(i)+0))


GMT_LONG map_units = FALSE, verbose = FALSE, geoid = FALSE, rem_something = FALSE;
GMT_LONG kmx = FALSE, kmy = FALSE, kmz = FALSE, rem_mean = FALSE, kms = FALSE;
GMT_LONG force_narray = FALSE, suggest_narray = FALSE, n_user_set = FALSE;
GMT_LONG leave_trend_alone = FALSE, from_below = FALSE, from_top = FALSE;
GMT_LONG	simple_grav = FALSE, mean_or_half_way = TRUE, rem_nothing = FALSE;
GMT_LONG	sphericity = FALSE, script = FALSE, sc_coherence = FALSE, sc_admitt = FALSE;
GMT_LONG admittance = FALSE, coherence = FALSE;

struct GRD_HEADER h, ig_h;
struct FFT_SUGGESTION {
	int     nx;
	int     ny;
	int     worksize;       /* # single-complex elements needed in work array  */
	int     totalbytes;     /* (8*(nx*ny + worksize))  */
	double  run_time;
	double  rms_rel_err;
}       fft_sug[3];     /* [0] holds fastest, [1] most accurate, [2] least storage  */


GMT_LONG	narray[2];
int     dummy[4], n_terms = 1;
int     ndim = 2, ksign, iform = 1, nx2, ny2, ndatac, i_data_start, j_data_start;
float	*datac, *topo, *raised, *workc, *in_grv, *z_from_below, *z_from_top, delta_z = 0.0;
double	data_var, data2_var, delta_kx, delta_ky;
double	scale_out = 1.0;
double	z_level;		/* mean bathymetry level computed from data */
double	z_offset = 0.0;		/* constant that myght be added to grid data */
double	zm = 0.0;		/* mean Moho depth (given by user) */
double	zl = 0.0;		/* mean depth of swell compensation (user given) */
double	rho_cw = 0.0;		/* crust-water density contrast */
double	rho_mc = 0.0;		/* mantle-crust density contrast */
double	rho_mw = 0.0;		/* mantle-water density contrast */
double	rho    = 0.0;		/* general density contrast */
double	te = 0.0, rw, rl, rm;		/* elastc thickness, water, crust and mantle densities */
double	youngs_modulus = 7.0e10;	/* Pascal = Nt/m**2  */
double	poissons_ratio = 0.25;
double	G = 6.667e-11;			/* Gravitational constant */
double	earth_rad = 6371008.7714;	/* GRS-80 sphere */
double	normal_gravity = 9.806199203;	/* Moritz's 1980 IGF value for gravity at 45 degrees latitude */
double	a[3];				/* Plane fitting coefficients  */
char    *infile = NULL, *outfile = NULL, *in_grav_file = NULL;
char    *infile_r = NULL, *infile_i = NULL;	/* File names for real and imaginary grids */

int	get_prime_factors(int n, int *f);
int	get_non_symmetric_f(int *f, int n);
int	do_parker (int n, double z_level, double rho);
int	read_data(int argc, char **argv, int bat);
void	taper_edges(float *grid);
void	remove_level(void), do_isostasy(void), write_output(void);
void	do_admittance(int give_wavelength, float k_or_m);
void	remove_plane(float *grid);
void	load_from_below_admit(void);
void	load_from_top_admit(void);
void	load_from_top_grid(int n);
void	load_from_below_grid(int n);
void	compute_only_adimtts(int from_top, int n_pt, double delta_pt);
void	write_script(void);

int main (int argc, char **argv) {

	GMT_LONG error = FALSE, stop, give_wavelength = FALSE, fft_grids = FALSE;
	GMT_LONG quick = FALSE, isostasy = FALSE, flex_file = FALSE, swell = FALSE;
	GMT_LONG moho_contrib = FALSE, theor_only = FALSE;
	/* first 2 cols from table III of Singleton's paper on fft.... */
	int nlist[117] = {64,72,75,80,81,90,96,100,108,120,125,128,135,144,150,160,162,180,192,200,
			216,225,240,243,250,256,270,288,300,320,324,360,375,384,400,405,432,450,480,
			486,500,512,540,576,600,625,640,648,675,720,729,750,768,800,810,864,900,960,
			972,1000,1024,1080,1125,1152,1200,1215,1250,1280,1296,1350,1440,1458,1500,
			1536,1600,1620,1728,1800,1875,1920,1944,2000,2025,2048,2160,2187,2250,2304,
			2400,2430,2500,2560,2592,2700,2880,2916,3000,3072,3125,3200,3240,3375,3456,
			3600,3645,3750,3840,3888,4000,4096,4320,4374,4500,4608,4800,4860,5000};

	int	i, j, k, n, n_pt;
	double	p, theor_inc, delta_pt, freq;
	float	k_or_m = 1.;
	char	*ptr, line[256], line2[256], t_or_b[4], *adm_opts, format[64], buffer[256];
	dummy[0] = dummy[1] = dummy[2] = dummy[3] = 0;

	argc = (int)GMT_begin (argc, argv);

	for (i = 1; !error && i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {

				case '\0':
					quick = TRUE;
					break;
				case 'A':
					sscanf (&argv[i][2], "%lf/%lf/%lf/%lf", &te, &rl, &rm, &rw);
					isostasy = TRUE;
					rho_cw = rl - rw;   rho_mc = rm - rl;	rho_mw = rm - rw;
					if (te > 1e10) { /* Given flexural rigidity, compute Te */
						te = pow ((12.0 * (1.0 - poissons_ratio * poissons_ratio))*te
						     / youngs_modulus, 1./3.);   
					}
					break;
				case 'C':	/* For theoretical curves only */
					theor_only = TRUE;
					sscanf (&argv[i][2], "%d/%lf/%lf/%s", &n_pt, &theor_inc, &z_level, t_or_b);
					for (n = 0; t_or_b[n]; n++) {
						switch (t_or_b[n]) {
							case 'w':
								give_wavelength = TRUE;
								break;
							case 'b':
								from_below = TRUE;
								break;
							case 't':
								from_top = TRUE;
								break;
							default:
								fprintf (stderr, "%s: Syntax error -C: [%s] is not valid, chose from [tbw]\n", GMT_program, &t_or_b[n]);
								error++;
								break;
						}
					}
					break;
				case 'D':
					if (!(&argv[i][2])) {
						fprintf (stderr, "%s: Syntax error -D option: must give density contrast\n", GMT_program);
						error++;
					}
					rho = atof (&argv[i][2]);
					simple_grav = TRUE;
					leave_trend_alone = TRUE;
					break;
				case 'E':
					n_terms = atoi (&argv[i][2]);
					if (n_terms > 10) {
						fprintf (stderr, "%s: ERROR -E option: n_terms must be <= 10\n", GMT_program);
						error++;
					}
					break;
				case 'H':
					moho_contrib = TRUE;
					leave_trend_alone = TRUE;
					break;
				case 'I':
					admittance = TRUE;
					strcpy (line, &argv[i][2]);
					ptr = strtok (line, "/");
					j = 0;
					while (ptr) {
						switch (j) {
							case 0:
								in_grav_file = ptr;
								break;
							case 1:
								adm_opts = ptr;
								for (n = 0; adm_opts[n]; n++) {
									switch (adm_opts[n]) {
										case 'w':
											give_wavelength = TRUE;
											break;
										case 'b':
											from_below = TRUE;
											break;
										case 'c':
											coherence = TRUE;
											break;
										case 't':
											from_top = TRUE;
											break;
										case 'k':
											k_or_m = 1000.;
											break;
										default:
											fprintf (stderr, "%s: Syntax error -I : [%s] is not valid, chose from [wbct]\n", GMT_program, &adm_opts[n]);
											error++;
											break;
									}
								}
								break;
							default:
								break;
						}
						ptr = strtok (CNULL, "/");
						j++;
					}
					break;
				case 'K':
					for (j = 2; argv[i][j]; j++) {
						if (argv[i][j] == 'x') kmx = TRUE;
						if (argv[i][j] == 'y') kmy = TRUE;
						if (argv[i][j] == 'z') kmz = TRUE;
						if (argv[i][j] == 's') kms = TRUE;
					}
					break;
				case 'L':
					leave_trend_alone = TRUE;
					if (argv[i][2]) rem_something = TRUE; 
					if (argv[i][2] == 'm' || argv[i][2] == 'M')
						mean_or_half_way = TRUE;
					if (argv[i][2] == 'h' || argv[i][2] == 'H')
						mean_or_half_way = FALSE;
					break;
				case 'l':
					if (argv[i][2] == 'n' || argv[i][2] == 'N')
						rem_nothing = TRUE;
					else
						mean_or_half_way = FALSE;
					break;
				case 'N':
					if (argv[i][2] == 'f' || argv[i][2] == 'F')
						force_narray = TRUE;
					else if (argv[i][2] == 'q' || argv[i][2] == 'Q')
						suggest_narray = TRUE;
					else if (argv[i][2] == 's' || argv[i][2] == 'S') {
						fprintf (stderr, "\t\"Good\" numbers for FFT dimensions\n");
						for (k = 0; k < 104; k++) {
							fprintf (stderr, "\t%d", nlist[k]);
							if ((k+1) % 10 == 0 || k == 103) fprintf (stderr, "\n");
						}
						exit (0);
					}
					else {
						if ((sscanf(&argv[i][2], "%d/%d", &nx2, &ny2)) != 2) error = TRUE;
						if (nx2 <= 0 || ny2 <= 0) error = TRUE;
						n_user_set = TRUE;
					}
					break;
				case 'F':
					geoid = TRUE;
					break;
				case 'G':
					outfile = &argv[i][2];
					break;
				case 'M':
					map_units = TRUE;
					break;
				case 'Q':
					flex_file = TRUE;
					leave_trend_alone = TRUE;
					break;
				case 'S':
					swell = TRUE;
					break;
				case 's':
					fprintf (stderr, "%s Warning -S : is currently deactivated\n", GMT_program);
					/*sphericity = TRUE;*/
					break;
				case 'T':
					fft_grids = TRUE;
					if (argv[i][2] == 'a') {
						sc_admitt = TRUE;
						script = TRUE;
					}
					else if (argv[i][2] == 'c') {
						sc_coherence = TRUE;
						script = TRUE;
					}
					break;
				case 'V':
					verbose = TRUE;
					break;
				case 'Z':
					sscanf (&argv[i][2], "%lf/%lf", &zm, &zl);
					break;
				case 'z':
					sscanf (&argv[i][2], "%lf", &z_offset);
					break;
				default:
					error = TRUE;
					GMT_default_error (argv[i][1]);
					break;
			}
		}
		else
			infile = argv[i];
	}

	if (argc == 1 || quick) {
		fprintf (stderr, "gravfft %s - Compute gravitational attraction of 3-D surfaces and a little more (ATTENTION z positive up)\n\n", GMT_VERSION);
		fprintf (stderr,"usage: gravfft <topo_grd> -A<te/rl/rm/rw> -C<n/wavelength/mean_depth/tbw> -D<density>\n");
		fprintf (stderr,"       -G<out_grdfile> [-E<n_terms>] [-F] [-K[xyzs]] [-L[m|h]] [-l[n]] -I<second_file>[/<wbct>]\n");
		fprintf (stderr,"       -H [-M] [-N<stuff>] -Q [-S] -T[c|a] [-V] -Z<zm>[/<zl>] [-z<cte>]\n\n");

		if (quick) exit (EXIT_FAILURE);

		fprintf (stderr,"\ttopo_grd is the input grdfile with topography values\n");
		fprintf (stderr,"\t-A Computes the isostatic compensation. Input file is topo load.\n");
		fprintf (stderr,"\t   Append elastic thickness and densities of load, mantle, and\n");
		fprintf (stderr,"\t   water, all in SI units. Give average mantle depth via -Z\n");
		fprintf (stderr,"\t   If the elastic thickness is > 1e10 it will be interpreted as the\n");
		fprintf (stderr,"\t   flexural rigidity (by default it's computed from Te and Young modulus)\n");
		fprintf (stderr,"\t-C n/wavelength/mean_depth/tbw Compute admittance curves based on a theoretical model.\n");
		fprintf (stderr,"\t   Total profile length in meters = <n> * <wavelength> (unless -Kx is set).\n");
		fprintf (stderr,"\t   --> Rest of parametrs are set within -A AND -Z options\n");
		fprintf (stderr,"\t   Append dataflags (one or two) of tbw.\n");
		fprintf (stderr,"\t     t writes \"elastic plate\" admittance \n");
		fprintf (stderr,"\t     b writes \"loading from below\" admittance \n");
		fprintf (stderr,"\t     w writes wavelength instead of wavenumber\n");
		fprintf (stderr,"\t-D Sets density contrast across surface (used when not -A)\n");
		fprintf (stderr,"\t-G filename for output netCDF grdfile with gravity [or geoid] values\n");
		fprintf (stderr,"\t-H writes a grid with the Moho's gravity|geoid effect from model selcted by -A\n");
		fprintf (stderr,"\t-I Use <second_file> and <topo_grd> to estimate admittance|coherence and write\n");
		fprintf (stderr,"\t   it to stdout (-G ignored if set). This grid should contain gravity or geoid\n");
		fprintf (stderr,"\t   for the same region of <topo_grd>. Default computes admittance. Output\n");
		fprintf (stderr,"\t   contains 3 or 4 columns. Frequency (wavelength), admittance (coherence)\n");
		fprintf (stderr,"\t   one sigma error bar and, optionaly, a theoretical admittance.\n");
		fprintf (stderr,"\t   Append dataflags (one to three) of wbct.\n");
		fprintf (stderr,"\t     w writes wavelength instead of wavenumber\n");
		fprintf (stderr,"\t     c computes coherence instead of admittance\" \n");
		fprintf (stderr,"\t     b writes a forth column with \"loading from below\" \n");
		fprintf (stderr,"\t       theoretical admittance\n");
		fprintf (stderr,"\t     t writes a forth column with \"elastic plate\" \n");
		fprintf (stderr,"\t       theoretical admittance\n");
		fprintf (stderr,"\t-Q writes out a grid with the flexural topography (with z positive up)\n");
		fprintf (stderr,"\t   whose average depth is at the value set by the option -Z<zm>.\n");
		fprintf (stderr,"\t-S Computes predicted gravity|geoide grid due to a subplate load.\n");
		fprintf (stderr,"\t   produced by the current bathymetry and the theoretical admittance.\n");
		fprintf (stderr,"\t   --> The necessary parametrs are set within -A and -Z options\n");
		fprintf (stderr,"\t-T Writes the FFT of the input grid in the form of two grids. One for\n");
		fprintf (stderr,"\t   the real part and the other for the imaginary. The grid names are\n");
		fprintf (stderr,"\t   built from the input grid name with _real and _imag appended. These\n");
		fprintf (stderr,"\t   grids have the DC at the center and coordinates are the kx,ky frequencies.\n");
		fprintf (stderr,"\t   Appending a or c will make the program write a csh script (admit_coher.sc)\n");
		fprintf (stderr,"\t   that computes, respectively, admittance or coherence by an alternative,\n");
		fprintf (stderr,"\t   (but much slower than the one used with -I) method based on grid\n");
		fprintf (stderr,"\t   interpolations using grdtrack. Read comments/instructions on the script.\n");
		fprintf (stderr,"\t-Z zm[/zl] -> Moho [and swell] average compensation depths\n");
		fprintf (stderr, "\n\tOPTIONS:\n");
		fprintf (stderr,"\t-E number of terms used in Parker expansion [Default = 1]\n");
		fprintf (stderr,"\t-F compute geoid rather than gravity\n");
		fprintf (stderr,"\t-K indicates that distances in these directions are in km [meter]\n");
		fprintf (stderr,"\t   -Ks multiplies the bathymetry grid by -1. Used for changing z sign\n");
		fprintf (stderr,"\t-L Leave trend alone. Do not remove least squares plane from data.\n");
		fprintf (stderr,"\t   It applies both to bathymetry as well as <second_file> [Default removes plane].\n");
		fprintf (stderr,"\t   Warning: both -D -H and -Q will implicitly set -L.\n");
		fprintf (stderr,"\t   Append m or h to just remove the mean or half-way from data and then exit\n");
		fprintf (stderr,"\t-l Removes half-way from bathymetry data [Default removes mean].\n");
		fprintf (stderr,"\t   Append n to do not remove any constant from input bathymetry data.\n");
		fprintf (stderr,"\t-M Map units used.  Convert grid dimensions from degrees to meters.\n");
		fprintf (stderr,"\t-N<stuff>  Choose or inquire about suitable grid dimensions for FFT.\n");
		fprintf (stderr,"\t\t-Nf will force the FFT to use the dimensions of the data.\n");
		fprintf (stderr,"\t\t-Nq will inQuire about more suitable dimensions.\n");
		fprintf (stderr,"\t\t-N<nx>/<ny> will do FFT on array size <nx>/<ny>\n");
		fprintf (stderr,"\t\t (Must be >= grdfile size). Default chooses dimensions >= data\n");
		fprintf (stderr,"\t\t which optimize speed, accuracy of FFT.\n");
		fprintf (stderr,"\t\t If FFT dimensions > grdfile dimensions, data are extended\n");
		fprintf (stderr,"\t\t and tapered to zero.\n");
		fprintf (stderr,"\t\t-Ns will print out a table with suitable dimensions and exits.\n");
		fprintf (stderr,"\t-s Take into account Earth Sphericity [No] (currently deactivated).\n");
		fprintf (stderr,"\t-V verbose mode\n");
		fprintf (stderr,"\t-z add a constant to the bathymetry (not to <second_file>) before doing anything else.\n");
		exit (EXIT_FAILURE);
	}

	/* -------------------- Compute only a theoretical model and exit -------------------- */

	if (theor_only) {
		if (!isostasy) {
			fprintf (stderr, "%s: Error: -A not set\n", GMT_program); 
			error++;
		}
		if (from_top && !zm) {
			fprintf (stderr, "%s: Error: -Z not used (must give moho compensation depth\n", GMT_program); 
			error++;
		}
		if (from_below && !(zm && zl)) {
			fprintf (stderr, "%s: Error: -Z not used (must give moho and swell compensation depths\n", GMT_program); 
			error++;
		}
		if (error) exit (EXIT_FAILURE);

		k_or_m = (float)((kmx || kmy) ? 1000 : 1);
		theor_inc *= k_or_m;
		delta_pt = 2 * M_PI / (n_pt * theor_inc);	/* Times 2PI because frequency will be used later */
		compute_only_adimtts((int)from_top, n_pt, delta_pt);
		sprintf (format, "%s\t%s\n", gmtdefs.d_format, gmtdefs.d_format);
		delta_pt /= (2.0 * M_PI) / k_or_m;			/* Write out frequency, not wavenumber  */
		for (k = 0; k < n_pt; k++) {
			freq = (k + 1) * delta_pt;
			if (give_wavelength) freq = 1. / freq;
			if (from_top) {
				sprintf (buffer, format, freq, z_from_top[k]);
				GMT_fputs (buffer, GMT_stdout);
			}
			else {
				sprintf (buffer, format, freq, z_from_below[k]);
				GMT_fputs (buffer, GMT_stdout);
			}
		}
		if (from_top) GMT_free (z_from_top);
		if (!from_top) GMT_free (z_from_below);
		GMT_end (argc, argv);
		return(0);
	}
	/* -----------------------------------------  ----------------------------------------- */

	if (!infile) {
		fprintf (stderr, "%s: Error: no bathymetry file given\n", GMT_program);
		error++;
	}
	if(flex_file && !isostasy) {
		fprintf (stderr, "%s: Error: -Q implies also -A\n", GMT_program); 
		error++;
	}
	if(moho_contrib && !isostasy) {
		fprintf (stderr, "%s: Error: -H implies also -A\n", GMT_program); 
		error++;
	}
	if(flex_file && !zm) {
		fprintf (stderr, "%s: Error: for creating the flex_file I need to know it's average depth (see -Z<zm>)\n", GMT_program); 
		error++;
	}
	if(moho_contrib && !zm) {
		fprintf (stderr, "%s: Error: for computing the Moho's effect I need to know it's average depth (see -Z<zm>)\n", GMT_program); 
		error++;
	}
	if (swell && !isostasy) {
		fprintf (stderr, "%s: Error: -S implies also -A\n", GMT_program); 
		error++;
	}
	if (((isostasy && !moho_contrib) && n_terms > 1) || (swell && n_terms > 1)) {
		fprintf (stderr, "%s: Warning: Due to a bug, or a method limitation (I didn't figure that out yet)\n", GMT_program);
		fprintf (stderr, "with the selected options, the number of terms in Parker expansion is reset to one\n"); 
		fprintf (stderr, "See examples in the manual if you realy want to compute with higher order expansion\n\n"); 
		n_terms = 1;
	}
	if (!(simple_grav || isostasy || swell || admittance || fft_grids)) {
		fprintf (stderr, "%s: Error: must set density contrast\n", GMT_program); 
		error++;
	}
	if (admittance && !in_grav_file) {
		fprintf (stderr, "%s: Error: for admittance|coherence need a gravity or geoide grid\n", GMT_program); 
		error++;
	}
	if (from_below && from_top) {
		fprintf (stderr, "%s: Error: -I choose only one model\n", GMT_program); 
		error++;
	}
	if (admittance && from_top && !(rw && rl && rm && zm)) {
		fprintf (stderr, "%s: Error: not all parameters needed for computing \"loading from top\" admittance were set\n", GMT_program); 
		error++;
	}
	if (admittance && from_below && !(rw && rl && rm && zm && zl)) {
		fprintf (stderr, "%s: Error: not all parameters needed for computing \"loading from below\" admittance were set\n", GMT_program); 
		error++;
	}

	if (error) exit (EXIT_FAILURE);

	if (verbose) fprintf (stderr, "%s: Allocates memory and read data file\n", GMT_program);

	if (read_data(argc, argv, TRUE) ) {	/* TRUE means setting everything for bat file */
		fprintf (stderr,"%s: Fatal memory error.  Exiting.\n", GMT_program);
		exit (EXIT_FAILURE);
	}

	if (admittance) {
		if (GMT_read_grd_info (in_grav_file, &ig_h)) {
			fprintf (stderr, "%s: Error opening file %s\n", GMT_program, in_grav_file);
			exit (EXIT_FAILURE);
		}
		if (ig_h.nx != h.nx || ig_h.ny != h.ny) {
			fprintf (stderr, "%s: Gravity file %s has improper dimensions!\n", GMT_program, in_grav_file);
			error++;
		}
		if (ig_h.x_min != h.x_min || ig_h.x_max != h.x_max || ig_h.y_min != h.y_min || ig_h.y_max != h.y_max) {
			fprintf (stderr, "%s: Gravity %s and bathymetry %s files have different limits!\n", GMT_program, in_grav_file, infile);
			error++;
		}
		if (ig_h.x_inc != h.x_inc || ig_h.y_inc != h.y_inc || ig_h.node_offset != h.node_offset) {
			fprintf (stderr, "%s: Gravity %s and bathymetry %s files have different increments or pix/grid registration!\n", GMT_program, in_grav_file, infile);
			error++;
		}
		if (verbose) fprintf (stderr, "%s: Processing gravity file %s\n", GMT_program, in_grav_file);

		if (read_data(argc, argv, FALSE) ) {
			fprintf (stderr,"%s: Fatal memory error.  Exiting.\n", GMT_program);
			exit (EXIT_FAILURE);
		}
		/* Check that no NaNs are present in gravity file */
		stop = FALSE;
		for (j = 0; !stop && j < h.ny; j++) 
		for (i = 0; !stop && i < h.nx; i++) stop = GMT_is_fnan ((double)in_grv[ij_data(i,j)]);
		if (stop) {
			fprintf (stderr, "%s: Input grid (%s) cannot have NaNs!\n", GMT_program, in_grav_file);
			exit (EXIT_FAILURE);
		}

		if (error) exit (EXIT_FAILURE);

	}

	/* Check that no NaNs are present in bathymetry file */

	stop = FALSE;
	for (j = 0; !stop && j < h.ny; j++) 
	for (i = 0; !stop && i < h.nx; i++) stop = GMT_is_fnan ((double)datac[ij_data(i,j)]);
	if (stop) {
		fprintf (stderr, "%s: Input grid (%s) cannot have NaNs!\n", GMT_program, infile);
		exit (EXIT_FAILURE);
	}

	ksign = -1;

	if (fft_grids) {	/* Write the FFTed input grid as two grids; Real and Img */
		int nx_2, ny_2, plus_minus;
		nx_2  = nx2/2;		ny_2 = ny2/2;
		strcpy (line, infile);		strcpy (line2, infile);
		infile_r = strtok (line, ".");	infile_i = strtok (line2, ".");
		strcat (infile_r, "_real.grd");		strcat (infile_i, "_imag.grd");

		for (j = 0; j < ny2; j++) {	/* Put DC at the center of the output array */
			for (i = 0; i < nx2; i++) {
				plus_minus = ((i+j) % 2 == 0) ? 1 : -1;
				datac[ij_data_0(i,j)] *= plus_minus;
			}
		}

		/*fourt_(datac, narray, &ndim, &ksign, &iform, workc);*/
		GMT_fourt (datac, narray, 2, -1, 1, workc);

		/* put DC to one */
		/*datac[ij_data_0(nx_2,ny_2)] = datac[ij_data_0(nx_2,ny_2) + 1] = 1.0;*/
		/* put DC to average of its four neighbours */
		/*datac[ij_data_0(nx_2,ny_2)] = (datac[ij_data_0(nx_2+1,ny_2)] + datac[ij_data_0(nx_2-1,ny_2)]
					    + datac[ij_data_0(nx_2,ny_2+1)] + datac[ij_data_0(nx_2,ny_2-1)])/4;
		datac[ij_data_0(nx_2,ny_2)+1] = (datac[ij_data_0(nx_2+1,ny_2) + 1] + datac[ij_data_0(nx_2-1,ny_2) + 1]
					    + datac[ij_data_0(nx_2,ny_2+1) + 1] + datac[ij_data_0(nx_2,ny_2-1) + 1])/4;*/
		/* Write out frequency, not wavenumber  */
		delta_kx /= (2.0*M_PI);		delta_ky /= (2.0*M_PI);

		h.nx = nx2;	h.ny = ny2;
		h.x_min = -delta_kx * nx_2;		h.y_min = -delta_ky * (ny_2 - 1);
		h.x_max =  delta_kx * (nx_2 - 1);	h.y_max =  delta_ky * ny_2;
		h.x_inc = delta_kx;	h.y_inc = delta_ky;
		strcpy (h.x_units, "m^(-1)");	strcpy (h.y_units, "m^(-1)");
		strcpy (h.z_units, "fft-ed z_units");
		strcpy (h.title, "Real part of fft transformed input grid");
		strcpy (h.remark, "With origin at lower left corner, kx = 0 at (nx/2 + 1) and ky = 0 at ny/2");
       		GMT_write_grd (infile_r, &h, datac, 0.0, 0.0, 0.0, 0.0, (GMT_LONG *)dummy, TRUE);

		for (i = 2; i < ndatac; i+= 2)	/* move imag to real position in the array */
			datac[i] = datac[i+1];	/* because that's what write_grd writes to file */

		strcpy (h.title, "Imaginary part of fft transformed input grid");
		GMT_write_grd (infile_i, &h, datac, 0.0, 0.0, 0.0, 0.0, (GMT_LONG *)dummy, TRUE);

		if (script) write_script();

		GMT_free(datac);
		GMT_end (argc, argv);
		return(0);

	}
	if (admittance) {	/* Compute admittance or coherence from data and exit */
		/*fourt_(datac, narray, &ndim, &ksign, &iform, workc);*/
		GMT_fourt (datac, narray, 2, -1, 1, workc);
		/*fourt_(in_grv, narray, &ndim, &ksign, &iform, workc);*/
		GMT_fourt (in_grv, narray, 2, -1, 1, workc);
		do_admittance((int)give_wavelength, k_or_m);
		GMT_end (argc, argv);
		return(0);
	}

	if (!outfile) {		/* Doing this test before was too complicated */
		fprintf (stderr, "%s: Error: no output file name given\n", GMT_program);
		exit (EXIT_FAILURE);
	}

	if(flex_file || moho_contrib) {
		/*fourt_(datac, narray, &ndim, &ksign, &iform, workc);*/
		GMT_fourt (datac, narray, 2, -1, 1, workc);
		do_isostasy();
		ksign = 1;
		/*fourt_(datac, narray, &ndim, &ksign, &iform, workc);*/
		GMT_fourt (datac, narray, 2, 1, 1, workc);
		scale_out *= (2.0 / ndatac);
		for (i = 0; i < ndatac; i+= 2) datac[i] *= (float)scale_out;
		if (!moho_contrib) {
			remove_level();		/* Because average was no longer zero */
       			for (i = 0; i < ndatac; i+= 2) datac[i] -= (float)zm;
 			GMT_write_grd (outfile, &h, datac, h.x_min, h.x_max, h.y_min, h.y_max, GMT_pad, TRUE);
			GMT_end (argc, argv);
			return(0);
		}
		else {
			ksign = -1;
			z_level = zm;
			rho = rho_mc;
			scale_out = 1.0;
		}
	}

	memcpy (topo, datac, ndatac * sizeof (float));
	memcpy (raised, datac, ndatac * sizeof (float));
	for (i = 0; i < ndatac; i++) datac[i] = 0.0;

	if (verbose) fprintf (stderr, "%s: Evatuating for term = 1", GMT_program);
	for (n = 1; n <= n_terms; n++) {

		if (verbose && n > 1) fprintf (stderr, "-%d", n);

		p = (double) n;
		if (n > 1) for (i = 0; i < ndatac; i++) raised[i] = (float)pow(topo[i], p);

		/* To avoid changing sign of negative^(even)*/
		/*if (n > 1) for (i = 0; i < ndatac; i++) raised[i] = fabs (pow (topo[i], p));*/
		/*if (n > 1 && n%2 == 0) {
			for (i = 0; i < ndatac; i++) 
				if (topo[i] < 0.) raised[i] *= -1;
		}*/

		/*fourt_ (raised, narray, &ndim, -1, &iform, workc);*/
		GMT_fourt (raised, narray, 2, -1, 1, workc);

		if (simple_grav || moho_contrib)	/* "classical" anomaly */
			do_parker (n, z_level, rho);
		else if (isostasy && !swell)		/* Compute "loading from top" grav|geoid */
			load_from_top_grid(n);
		else if (swell)				/* Compute "loading from below" grav|geoid */
			load_from_below_grid(n);
		else
			fprintf (stderr, "It SHOULDN'T pass here\n");
	}

	if (verbose) fprintf (stderr, " Inverse FFT...");

	ksign = 1;
	/*fourt_(datac, narray, &ndim, &ksign, &iform, workc);*/
	GMT_fourt (datac, narray, 2, 1, 1, workc);

	scale_out *= (2.0 / ndatac);
	for (i = 0; i < ndatac; i+= 2) datac[i] *= (float)scale_out;

	if (geoid) {
		strcpy (h.title, "Geoid anomalies");
		strcpy (h.z_units, "meter");
	}
	else {
		strcpy (h.title, "Gravity anomalies");
		strcpy (h.z_units, "mGal");
	}
	sprintf (h.remark, "Parker expansion of order %d", n_terms);

	if (verbose) fprintf (stderr, "write_output...");

	write_output();

	if (verbose) fprintf (stderr, "done!\n");

	GMT_end (argc, argv);

	return(0);
}

int 	read_data(int argc, char **argv, int bat) {
	/* Changed this routine to allow two calls. When called for reading the
	   bathymetry file, everything need is set here. However, when -I option
	   is used (for computing admittance) these function is used to set the
	   apropriate dimensions and prepare the grid to be FFTed. That is, to be
	   compatible with the bathymetry file. The grd_info stuff and checking
	   against the bathymetry file is done on the main program.		*/

	int     worksize, factors[32], i, j;
	double  tdummy, edummy;
	void	suggest_fft(int nx, int ny, struct FFT_SUGGESTION *fft_sug, int do_print);
	void	fourt_stats(int nx, int ny, int *f, double *r, int *s, double *t);

	if (bat) {	/* if not, header has already been read in main */
		if (GMT_read_grd_info (infile, &h)) {
			fprintf (stderr, "%s: Error opening file %s\n", GMT_program, infile);
			exit (EXIT_FAILURE);
		}
		GMT_grd_init (&h, argc, argv, TRUE);
	}

	if (rem_something) {  /*  Just remove the mean or half way and exit   */
		datac = GMT_memory (VNULL, (size_t) h.nx*h.ny, sizeof(float), GMT_program);
		if (GMT_read_grd (infile, &h, datac, 0.0, 0.0, 0.0, 0.0, (GMT_LONG *)dummy, FALSE)) {
			fprintf (stderr, "%s: Error reading file %s\n", GMT_program, infile);
			exit (EXIT_FAILURE);
		}
		remove_level();
		if (verbose && mean_or_half_way) fprintf (stderr, "Mean value removed\n");
		if (verbose && !mean_or_half_way) fprintf (stderr, "Half-way value removed\n");
		if (GMT_write_grd (outfile, &h, datac, 0.0, 0.0, 0.0, 0.0, (GMT_LONG *)dummy, FALSE)) {
			fprintf (stderr, "%s: Error writing file %s\n", GMT_program, outfile);
			exit (EXIT_FAILURE);
		}

		GMT_free (datac);

		if (verbose) fprintf (stderr, "done!\n");

		GMT_end (argc, argv);
	}

	/* Get dimensions as may be appropriate */
	if (n_user_set) {
		if (nx2 < h.nx || ny2 < h.ny) {
			fprintf(stderr,"%s: Error: You specified -Nnx/ny smaller than input grdfile.  Ignored.\n", GMT_program);
			n_user_set = FALSE;
		}
	}
	if (!(n_user_set) ) {
		if (force_narray) {
			nx2 = h.nx;
			ny2 = h.ny;
		}
		else {
			suggest_fft(h.nx, h.ny, fft_sug, (verbose || suggest_narray));
			if (fft_sug[1].totalbytes < fft_sug[0].totalbytes) {
				/* The most accurate solution needs same or less storage
				 * as the fastest solution; use the most accurate's dimensions */
				nx2 = fft_sug[1].nx;
				ny2 = fft_sug[1].ny;
			}
			else {
				/* Use the sizes of the fastest solution  */
				nx2 = fft_sug[0].nx;
				ny2 = fft_sug[0].ny;
			}
		}
	}

	/* Get here when nx2 and ny2 are set to the vals we will use.  */
	narray[0] = nx2;
	narray[1] = ny2;
	fourt_stats(nx2, ny2, factors, &edummy, &worksize, &tdummy);
	if (verbose) fprintf(stderr,"%s: Data dimension %d %d\tFFT dimension %d %d\n",
		GMT_program, h.nx, h.ny, nx2, ny2);

	/* Make an array of floats 2 * nx2 * ny2 for complex data */
	ndatac = 2 * nx2 * ny2;

	if (bat) {
		datac = GMT_memory (VNULL, (size_t) ndatac, sizeof(float), GMT_program);
		topo = GMT_memory (VNULL, (size_t) ndatac, sizeof(float), GMT_program);
		raised = GMT_memory (VNULL, (size_t) ndatac, sizeof(float), GMT_program);
		memset (datac, 0, (size_t)(ndatac*sizeof(float)));
	}
	else {		/* means the -I option was set */
		in_grv = GMT_memory (VNULL, (size_t) ndatac, sizeof(float), GMT_program);
		memset (in_grv, 0, (size_t)(ndatac*sizeof(float)));
	}
	if (worksize) {
		if (worksize < nx2) worksize = nx2;
		if (worksize < ny2) worksize = ny2;
		worksize *= 2;
		workc = GMT_memory (VNULL, (size_t)worksize, sizeof(float), GMT_program);
		memset (workc, 0, (size_t)(worksize*sizeof(float)));
	}
	else {
		workc = GMT_memory (VNULL, 4, sizeof(float), GMT_program);
		memset (workc, 0, (size_t)(4*sizeof(float)));
	}

	/* Put the data in the middle of the padded array */

	i_data_start = GMT_pad[0] = (nx2 - h.nx)/2;	/* zero if nx2 < h.nx+1  */
	j_data_start = GMT_pad[3] = (ny2 - h.ny)/2;
	GMT_pad[1] = nx2 - h.nx - GMT_pad[0];
	GMT_pad[2] = ny2 - h.ny - GMT_pad[3];

	if (bat) {
		if (GMT_read_grd (infile, &h, datac, h.x_min, h.x_max, h.y_min, h.y_max, GMT_pad, TRUE)) {
			fprintf (stderr, "%s: Error reading file %s\n", GMT_program, infile);
			exit (EXIT_FAILURE);
		}
		for (j = 0; j < h.ny; j++) for (i = 0; i < h.nx; i++) datac[ij_data(i,j)] += (float)z_offset;
		if (kmz) for (j = 0; j < h.ny; j++) for (i = 0; i < h.nx; i++) datac[ij_data(i,j)] *= 1000.;
		if (kms) for (j = 0; j < h.ny; j++) for (i = 0; i < h.nx; i++) datac[ij_data(i,j)] *= -1.;
		if (!rem_nothing) remove_level();	/* removes either the mean or half-way (if any) */
		if (!leave_trend_alone) remove_plane(datac);
		taper_edges(datac);
	}
	else {
		if (GMT_read_grd (in_grav_file, &ig_h, in_grv, ig_h.x_min, ig_h.x_max, ig_h.y_min, ig_h.y_max, GMT_pad, TRUE)) {
			fprintf (stderr, "%s: Error reading file %s\n", GMT_program, in_grav_file);
			exit (EXIT_FAILURE);
		}

		if (!leave_trend_alone) remove_plane(in_grv);
		taper_edges(in_grv);
	}

	delta_kx = 2 * M_PI / (nx2 * h.x_inc);
	delta_ky = 2 * M_PI / (ny2 * h.y_inc);
	if (map_units) {
		/* Give delta_kx, delta_ky units of 2pi/meters  */
		double tmp;
		tmp = 2.0 * M_PI * 6371008.7714 / 360.0;	/* GRS-80 sphere m/degree */
		delta_kx /= (tmp * cos(0.5 * (h.y_min + h.y_max) * D2R) );
		delta_ky /= tmp;
	}
	else {
		if (kmx) delta_kx *= 0.001;
		if (kmy) delta_ky *= 0.001;
	}
	return(0);
}

void	write_output(void) {

	/* The data are in the middle of the padded array */

	if (GMT_write_grd (outfile, &h, datac, h.x_min, h.x_max, h.y_min, h.y_max, GMT_pad, TRUE)) {
		fprintf (stderr, "%s: Error writing file %s\n", GMT_program, outfile);
		exit (EXIT_FAILURE);
	}

	GMT_free(datac);
	GMT_free(raised);
	GMT_free(topo);
}

void	remove_level(void) {
	/* Remove the level corresponding to the mean or the half-way point.
	   This level is used as the depth parameter in the exponential term */

	int i, j;
	double z_min, z_max, sum = 0.0, half_w = 0.0;

	z_min = 1.0e100;	 z_max = -1.0e100;

	for (j = 0; j < h.ny; j++) for (i = 0; i < h.nx; i++) {
		if (!GMT_is_fnan((double)datac[ij_data(i,j)])) {
		  	z_min = MIN (z_min, datac[ij_data(i,j)]);
		  	z_max = MAX (z_max, datac[ij_data(i,j)]);
			sum += datac[ij_data(i,j)];
		}
	}

	sum /= (h.nx * h.ny);		half_w = z_min + 0.5 * (z_max - z_min);
	z_level = (mean_or_half_way) ? sum : half_w;

	for (j = 0; j < h.ny; j++) for (i = 0; i < h.nx; i++) {
		if (!GMT_is_fnan((double)datac[ij_data(i,j)])) {
			datac[ij_data(i,j)] -= (float)z_level;
		}
	}
	if (verbose) fprintf (stderr, "%s: 1/2-way level = %g, mean = %g : value removed --> %g\n", GMT_program, half_w, sum, z_level);
	z_level = fabs(z_level);	/* Need absolute value for uppward continuation */
}

void	taper_edges (float *grid) {
	/* The original version of this function (in grdfft) was hard-wired to
	   a grid name called datac. I changed it by transmiting a pointer to 
	   grid and now it can be used with more grids. Note, however, that the
	   header parameters are allways the same, so multiple calls only work
	   for compatible grids.						JL */
	int	i, j, im, jm, il1, ir1, il2, ir2, jb1, jb2, jt1, jt2;
	double	scale, cos_wt;

	/* Note that if nx2 = h.nx+1 and ny2 = h.ny + 1, then this routine
		will do nothing; thus a single row/column of zeros may be
		added to the bottom/right of the input array and it cannot
		be tapered.  But when (nx2 - h.nx)%2 == 1 or ditto for y,
		this is zero anyway.  */


	/* First reflect about xmin and xmax, point symmetric about edge point */

	for (im = 1; im <= i_data_start; im++) {
		il1 = -im;	/* Outside xmin; left of edge 1  */
		ir1 = im;	/* Inside xmin; right of edge 1  */
		il2 = il1 + h.nx - 1;	/* Inside xmax; left of edge 2  */
		ir2 = ir1 + h.nx - 1;	/* Outside xmax; right of edge 2  */
		for (j = 0; j < h.ny; j++) {
			grid[ij_data(il1,j)] = (float)2.0*grid[ij_data(0,j)] - grid[ij_data(ir1,j)];
			grid[ij_data(ir2,j)] = (float)2.0*grid[ij_data((h.nx-1),j)] - grid[ij_data(il2,j)];
		}
	}

	/* Next, reflect about ymin and ymax.
		At the same time, since x has been reflected,
		we can use these vals and taper on y edges */

	scale = M_PI / (j_data_start + 1);

	for (jm = 1; jm <= j_data_start; jm++) {
		jb1 = -jm;	/* Outside ymin; bottom side of edge 1  */
		jt1 = jm;	/* Inside ymin; top side of edge 1  */
		jb2 = jb1 + h.ny - 1;	/* Inside ymax; bottom side of edge 2  */
		jt2 = jt1 + h.ny - 1;	/* Outside ymax; bottom side of edge 2  */
		cos_wt = 0.5 * (1.0 + cos(jm * scale) );
		for (i = -i_data_start; i < nx2 - i_data_start; i++) {
			grid[ij_data(i,jb1)] = (float)(cos_wt * (2.0*grid[ij_data(i,0)] - grid[ij_data(i,jt1)]));
			grid[ij_data(i,jt2)] = (float)(cos_wt * (2.0*grid[ij_data(i,(h.ny-1))] - grid[ij_data(i,jb2)]));
		}
	}

	/* Now, cos taper the x edges */

	scale = M_PI / (i_data_start + 1);
	for (im = 1; im <= i_data_start; im++) {
		il1 = -im;
		ir1 = im;
		il2 = il1 + h.nx - 1;
		ir2 = ir1 + h.nx - 1;
		cos_wt = 0.5 * (1.0 + cos(im * scale) );
		for (j = -j_data_start; j < ny2 - j_data_start; j++) {
			grid[ij_data(il1,j)] *= (float)cos_wt;
			grid[ij_data(ir2,j)] *= (float)cos_wt;
		}
	}
}

double  kx(int k) {
	/* Return the value of kx given k,
		where kx = 2 pi / lambda x,
		and k refers to the position
		in the datac array, datac[k].  */

	int     ii;

	ii = ((int)(k/2))%nx2;
	if (ii > nx2/2) ii -= nx2;
	return(ii * delta_kx);
}

double  ky(int k) {
	/* Return the value of ky given k,
		where ky = 2 pi / lambda y,
		and k refers to the position
		in the datac array, datac[k].  */

	int     jj;

	jj = ((int)(k/2))/nx2;
	if (jj > ny2/2) jj -= ny2;
	return(jj * delta_ky);
}

double  modk(int k) {
	/* Return the value of sqrt(kx*kx + ky*ky),
		given k, where k is array position.  */

	return (hypot (kx(k), ky(k)));
}

void do_isostasy (void) {
	/* Do the isostatic response function convolution in the Freq domain.
	All units assumed to be in SI (that is kx, ky, modk wavenumbers in m**-1,
	densities in kg/m**3, Te in m, etc.
	rw, the water density, is used to set the Airy ratio and the restoring
	force on the plate (rm - ri)*gravity if ri = rw; so use zero for topo in air (ri changed to rl).  */
	int     k;
	double  airy_ratio, rigidity_d, d_over_restoring_force, mk, k2, k4, transfer_fn;

	/*   te	 Elastic thickness, SI units (m)  */
	/*   rl	 Load density, SI units  */
	/*   rm	 Mantle density, SI units  */
	/*   rw	 Water density, SI units  */
	
	rigidity_d = (youngs_modulus * te * te * te) / (12.0 * (1.0 - poissons_ratio * poissons_ratio));
	d_over_restoring_force = rigidity_d / ( (rm - rl) * normal_gravity);
	airy_ratio = -(rl - rw)/(rm - rl);
 
	if (te == 0.0) {      /* Airy isostasy; scale global variable scale_out and return */
		scale_out *= airy_ratio;
		return;
	}
	
	for (k = 0; k < ndatac; k+= 2) {
		mk = modk(k);	k2 = mk * mk;	k4 = k2 * k2;
		transfer_fn = airy_ratio / ( (d_over_restoring_force * k4) + 1.0);	  
		datac[k] *= (float)transfer_fn;
		datac[k+1] *= (float)transfer_fn;
	}
}

int     do_parker (int n, double z_level, double rho) {
	int i, k;
	double f, p, t, mk, v, c;

	f = 1.0;
	for (i = 2; i <= n; i++) f *= i;	/* n! */
	p = (double) n - 1.0;

	c = 1.0e5 * 2.0 * M_PI * G * rho / f; /* Gives mGal */
	if (geoid) c /= 980619.92;

	for (k = 0; k < ndatac; k+= 2) {
		mk = modk(k);
		if (p == 0.0)
			t = 1.0;
		else if (p == 1.0)
			t = mk;
		else
			t = pow (mk, p);

		v = c * exp (-mk * z_level) * t;
		if (geoid && mk > 0.0) v /= mk;
		datac[k] += (float) (v * raised[k]);
		datac[k+1] += (float) (v * raised[k+1]);
	}
	return (0);
}

void	fourt_stats(int nx, int ny, int *f, double *r, int *s, double *t) {
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

	int     n_factors, i, ntotal, sum2, sumnot2, nnot2;
	int     nonsymx, nonsymy, nonsym, storage;
	double  err_scale, sig_bits = FSIGNIF;


	/* Find workspace needed.  First find non_symmetric factors in nx, ny  */
	n_factors = get_prime_factors(nx, f);
	nonsymx = get_non_symmetric_f(f, n_factors);
	n_factors = get_prime_factors(ny, f);
	nonsymy = get_non_symmetric_f(f, n_factors);
	nonsym = MAX(nonsymx, nonsymy);

	/* Now get factors of ntotal  */
	ntotal = nx * ny;
	n_factors = get_prime_factors(ntotal, f);
	storage = MAX(nonsym, f[n_factors - 1]);
	if (storage == 2)
		*s = 0;
	else
		*s = storage;

	/* Now find time and error estimates */

	err_scale = 0.0;
	sum2 = 0;
	sumnot2 = 0;
	nnot2 = 0;
	for(i = 0; i < n_factors; i++) {
		if (f[i] == 2)
			sum2 += f[i];
		else {
			sumnot2 += f[i];
			nnot2++;
		}
		err_scale += pow((double)f[i], 1.5);
	}
	*t = 1.0e-06*(3000.0 + ntotal * (500.0 + 43.0*sum2 + 68.0*sumnot2 + 320.0*nnot2));
	*r = err_scale * 3.0 * pow(2.0, -sig_bits);
	return;
}

int	get_non_symmetric_f(int *f, int n) {
	/* Return the product of the non-symmetric factors in f[]  */
	int     i = 0, j = 1, retval = 1;

	if (n == 1) return(f[0]);

	while(i < n) {
		while(j < n && f[j] == f[i]) j++;
		if ((j-i)%2) retval *= f[i];
		i = j;
		j = i + 1;
	}
	if (retval == 1) retval = 0;    /* There are no non-sym factors  */
	return(retval);
}

void	suggest_fft(int nx, int ny, struct FFT_SUGGESTION *fft_sug, int do_print) {
	int     f[64], xstop, ystop;
	int     nx_best_t, ny_best_t;
	int     nx_best_e, ny_best_e;
	int     nx_best_s, ny_best_s;
	int     nxg, nyg;       /* Guessed by this routine  */
	int     nx2, ny2, nx3, ny3, nx5, ny5;   /* For powers  */
	double  current_time, best_time, given_time, s_time, e_time;
	int     current_space, best_space, given_space, e_space, t_space;
	double  current_err, best_err, given_err, s_err, t_err;
	void	fourt_stats(int nx, int ny, int *f, double *r, int *s, double *t);


	fourt_stats(nx, ny, f, &given_err, &given_space, &given_time);
	given_space += nx*ny;
	given_space *= 8;
	if (do_print) fprintf(stderr,"%s: Data dimension\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %d\n",
		GMT_program, nx, ny, given_time, given_err, given_space);

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

			fourt_stats(nxg, nyg, f, &current_err, &current_space, &current_time);
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
		fprintf(stderr,"%s: Highest speed\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %d\n", GMT_program,
			nx_best_t, ny_best_t, best_time, t_err, t_space);
		fprintf(stderr,"%s: Most accurate\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %d\n", GMT_program,
			nx_best_e, ny_best_e, e_time, best_err, e_space);
		fprintf(stderr,"%s: Least storage\t%d %d\ttime factor %.8g\trms error %.8e\tbytes %d\n", GMT_program,
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

int     get_prime_factors(int n, int *f) {
	/* Fills the integer array f with the prime factors of n.
	 * Returns the number of locations filled in f, which is
	 * one if n is prime.
	 *
	 * f[] should have been malloc'ed to enough space before
	 * calling prime_factors().  We can be certain that f[32]
	 * is enough space, for if n fits in a long, then n < 2**32,
	 * and so it must have fewer than 32 prime factors.  I think
	 * that in general, ceil(log2((double)n)) is enough storage
	 * space for f[].
	 *
	 * Tries 2,3,5 explicitly; then alternately adds 2 or 4
	 * to the previously tried factor to obtain the next trial
	 * factor.  This is done with the variable two_four_toggle.
	 * With this method we try 7,11,13,17,19,23,25,29,31,35,...
	 * up to a maximum of sqrt(n).  This shortened list results
	 * in 1/3 fewer divisions than if we simply tried all integers
	 * between 5 and sqrt(n).  We can reduce the size of the list
	 * of trials by an additional 20% by removing the multiples
	 * of 5, which are equal to 30m +/- 5, where m >= 1.  Starting
	 * from 25, these are found by alternately adding 10 or 20.
	 * To do this, we use the variable ten_twenty_toggle.
	 *
	 * W. H. F. Smith, 26 Feb 1992, after D.E. Knuth, vol. II  */

	int     current_factor; /* The factor currently being tried  */
	int     max_factor;     /* Don't try any factors bigger than this  */
	int     n_factors = 0;  /* Returned; one if n is prime  */
	int     two_four_toggle = 0;    /* Used to add 2 or 4 to get next trial factor  */
	int     ten_twenty_toggle = 0;  /* Used to add 10 or 20 to skip_five  */
	int     skip_five = 25; /* Used to skip multiples of 5 in the list  */
	int     m;      /* Used to keep a working copy of n  */


	/* Initialize m and max_factor  */
	m = abs(n);
	if (m < 2) return(0);
	max_factor = (int)floor(sqrt((double)m));

	/* First find the 2s  */
	current_factor = 2;
	while(!(m%current_factor)) {
		m /= current_factor;
		f[n_factors] = current_factor;
		n_factors++;
	}
	if (m == 1) return(n_factors);

	/* Next find the 3s  */
	current_factor = 3;
	while(!(m%current_factor)) {
		m /= current_factor;
		f[n_factors] = current_factor;
		n_factors++;
	}
	if (m == 1) return(n_factors);

	/* Next find the 5s  */
	current_factor = 5;
	while(!(m%current_factor)) {
		m /= current_factor;
		f[n_factors] = current_factor;
		n_factors++;
	}
	if (m == 1) return(n_factors);

	/* Now try all the rest  */

	while (m > 1 && current_factor <= max_factor) {

		/* Current factor is either 2 or 4 more than previous value  */

		if (two_four_toggle) {
			current_factor += 4;
			two_four_toggle = 0;
		}
		else {
			current_factor += 2;
			two_four_toggle = 1;
		}

		/* If current factor is a multiple of 5, skip it.  But first,
			set next value of skip_five according to 10/20 toggle */

		if (current_factor == skip_five) {
			if (ten_twenty_toggle) {
				skip_five += 20;
				ten_twenty_toggle = 0;
			}
			else {
				skip_five += 10;
				ten_twenty_toggle = 1;
			}
			continue;
		}

		/* Get here when current_factor is not a multiple of 2,3 or 5 */

		while(!(m%current_factor)) {
			m /= current_factor;
			f[n_factors] = current_factor;
			n_factors++;
		}
	}

	/* Get here when all factors up to floor(sqrt(n)) have been tried.  */

	if (m > 1) {
		/* m is an additional prime factor of n  */
		f[n_factors] = m;
		n_factors++;
	}
	return (n_factors);
}

void	do_admittance(int give_wavelength, float k_or_m) {
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

	int	k, k_0 = 0, nk, ifreq, *nused;
	double	delta_k, r_delta_k, freq;
	double	*out, *err_bar, *coh, *b_pow, *g_pow, *co_spec, *quad;
	char	format[64], buffer[256];

	if (delta_kx < delta_ky) {delta_k = delta_kx;	nk = nx2/2;}
	else {delta_k = delta_ky;	nk = ny2/2;}

	/* Get an array for summing stuff */
	b_pow = GMT_memory (VNULL, (size_t)nk, sizeof(double), GMT_program);
	g_pow = GMT_memory (VNULL, (size_t)nk, sizeof(double), GMT_program);
	err_bar = GMT_memory (VNULL, (size_t)nk, sizeof(double), GMT_program);
	co_spec = GMT_memory (VNULL, (size_t)nk, sizeof(double), GMT_program);
	quad = GMT_memory (VNULL, (size_t)nk, sizeof(double), GMT_program);
	coh = GMT_memory (VNULL, (size_t)nk, sizeof(double), GMT_program);
	out = GMT_memory (VNULL, (size_t)nk, sizeof(double), GMT_program);
	nused = GMT_memory (VNULL, (size_t)nx2*ny2, sizeof(int), GMT_program);

	if (coherence)
		admittance = FALSE;

	/* Loop over it all, summing and storing, checking range for r */

	r_delta_k = 1.0 / delta_k;
	
	for (k = 2; k < ndatac; k+= 2) {
		freq = modk(k);
		ifreq = irint(fabs(freq)*r_delta_k) - 1;
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
		if (admittance) {
			out[k] = co_spec[k] / b_pow[k];
			/*err_bar[k] = out[k] * fabs (sqrt ((1.0 - coh[k]) / 2.0 * coh[k]) * out[k]) / sqrt(nused[k]); Versao do Smith*/
			err_bar[k] = out[k] * fabs (sqrt ((1.0 - coh[k]) / (2.0 * coh[k] * nused[k])) );
		}
		else if (coherence) {
			out[k] = coh[k];
			err_bar[k] = out[k] * (1.0 - coh[k]) * sqrt(2.0 / coh[k]) / sqrt(nused[k]);
		}
	}

	/* Now get here when array is summed.  */
	delta_k /= (2.0*M_PI);				/* Write out frequency, not wavenumber  */
	if (from_below) load_from_below_admit();	/* compute theoretical "load from below" admittance */
	if (from_top) load_from_top_admit();		/* compute theoretical "load from top" admittance */
	if (from_below || from_top)
		sprintf (format, "%s\t%s\t%s\t%s\n", gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format);
	else
		sprintf (format, "%s\t%s\t%s\n", gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format);

	delta_k *= k_or_m;
	for (k = k_0; k < nk; k++) {
		freq = (k + 1) * delta_k;
		if (give_wavelength) freq = 1.0/freq;
		if (from_below) {
			sprintf (buffer, format, freq, out[k], err_bar[k], z_from_below[k]);
			GMT_fputs (buffer, GMT_stdout);
		}
		else if (from_top) {
			sprintf (buffer, format, freq, out[k], err_bar[k], z_from_top[k]);
			GMT_fputs (buffer, GMT_stdout);
		}
		else {
			sprintf (buffer, format, freq, out[k], err_bar[k]);
			GMT_fputs (buffer, GMT_stdout);
		}
	}
	GMT_free (out);
	GMT_free (b_pow);
	GMT_free (g_pow);
	GMT_free (err_bar);
	GMT_free (co_spec);
	GMT_free (coh);
	GMT_free (quad);
	if (from_below) GMT_free (z_from_below);
	if (from_top) GMT_free (z_from_top);
}

void	remove_plane(float *grid)
{
	/* Remove the best-fitting plane by least squares.

	Let plane be z = a0 + a1 * x + a2 * y.  Choose the
	center of x,y coordinate system at the center of 
	the array.  This will make the Normal equations 
	matrix G'G diagonal, so solution is trivial.  Also,
	spend some multiplications on normalizing the 
	range of x,y into [-1,1], to avoid roundoff error.  */

	/* another one picked from grdfft and changed to deal with different grids */

	int	i, j, one_or_zero;
	double	x_half_length, one_on_xhl, y_half_length, one_on_yhl;
	double	sumx2, sumy2, x, y, z;

	one_or_zero = (h.node_offset) ? 0 : 1;
	x_half_length = 0.5 * (h.nx - one_or_zero);
	one_on_xhl = 1.0 / x_half_length;
	y_half_length = 0.5 * (h.ny - one_or_zero);
	one_on_yhl = 1.0 / y_half_length;

	sumx2 = sumy2 = data_var = 0.0;
	a[2] = a[1] = a[0] = 0.0;

	for (j = 0; j < h.ny; j++) {
		y = one_on_yhl * (j - y_half_length);
		for (i = 0; i < h.nx; i++) {
			x = one_on_xhl * (i - x_half_length);
			z = grid[ij_data(i,j)];
			a[0] += z;
			a[1] += z*x;
			a[2] += z*y;
			sumx2 += x*x;
			sumy2 += y*y;
		}
	}
	a[0] /= (h.nx*h.ny);
	a[1] /= sumx2;
	a[2] /= sumy2;
	for (j = 0; j < h.ny; j++) {
		y = one_on_yhl * (j - y_half_length);
		for (i = 0; i < h.nx; i++) {
			x = one_on_xhl * (i - x_half_length);
			grid[ij_data(i,j)] -= (float)(a[0] + a[1]*x + a[2]*y);
			data_var += (grid[ij_data(i,j)] * grid[ij_data(i,j)]);
		}
	}
	data_var = sqrt(data_var / (h.nx*h.ny - 1));
	/* Rescale a1,a2 into user's units, in case useful later */
	a[1] *= (2.0/(h.x_max - h.x_min));
	a[2] *= (2.0/(h.y_max - h.y_min));
	if(verbose) fprintf (stderr,"%s: Plane removed.  Mean, S.D., Dx, Dy: %.8g\t%.8g\t%.8g\t%.8g\n", GMT_program, a[0],data_var,a[1],a[2]);
}

void	load_from_below_admit(void) {

	/* Compute theoretical admittance for the "loading from below" model
	   M. McNutt & Shure (1986) in same |k| of computed data admittance	*/

	int k, nk;
	double	earth_curvature, alfa, delta_k, freq, D, twopi, t1, t2, t3;

	if (delta_kx < delta_ky) {delta_k = delta_kx;	 nk = nx2/2;}
	else {delta_k = delta_ky;	 nk = ny2/2;}

	z_from_below = GMT_memory (VNULL, (size_t) nk, sizeof(float), GMT_program);

	twopi = 2. * M_PI;
	delta_k /= twopi;	/* Use frequency, not wavenumber  */
	D = (youngs_modulus * te * te * te) / (12.0 * (1.0 - poissons_ratio * poissons_ratio));
	alfa = pow(twopi,4.) * D / (normal_gravity * rho_mc);

	for (k = 0; k < nk; k++) {
		freq = (k + 1) * delta_k;
		earth_curvature = (sphericity) ? (2 * earth_rad * freq) / (4 * M_PI * earth_rad * freq + 1) : 1.;
		t1 = earth_curvature * (twopi * G);
		if (!geoid) t1 *= 1.0e5;		/* to have it in mGals */
		if (geoid) t1 /= (normal_gravity * freq * twopi);
		t2 = rho_cw * exp(-twopi * freq * z_level) + rho_mc * exp(-twopi * freq * zm);
		t3 = -(rho_mw + rho_mc * pow(freq,4.) * alfa) * exp(-twopi * freq * zl);
		z_from_below[k] = (float) (t1 * (t2 + t3));
	}
}

void	load_from_top_admit(void) {

	/* Compute theoretical admittance for the "loading from top" model
	   M. McNutt & Shure (1986) in same |k| of computed data admittance	*/

	int k, nk;
	double	earth_curvature, alfa, delta_k, freq, D, twopi, t1, t2;

	if (delta_kx < delta_ky) {delta_k = delta_kx;	 nk = nx2/2;}
	else {delta_k = delta_ky;	 nk = ny2/2;}

	z_from_top = GMT_memory (VNULL, (size_t) nk, sizeof(float), GMT_program);

	twopi = 2. * M_PI;
	delta_k /= twopi;	/* Use frequency, not wavenumber  */
	D = (youngs_modulus * te * te * te) / (12.0 * (1.0 - poissons_ratio * poissons_ratio));
	alfa = pow(twopi,4.) * D / (normal_gravity * rho_mc);

	for (k = 0; k < nk; k++) {
		freq = (k + 1) * delta_k;
		earth_curvature = (sphericity) ? (2 * earth_rad * freq) / (4 * M_PI * earth_rad * freq + 1) : 1.;
		t1 = earth_curvature * (twopi * G);
		if (!geoid) t1 *= 1.0e5;		/* to have it in mGals */
		if (geoid) t1 /= (normal_gravity * freq * twopi);
		t2 = exp(-twopi * freq * z_level) - exp(-twopi * freq * zm) / (1 + alfa*pow(freq,4.));   
		z_from_top[k] = (float) (t1 * rho_cw * t2);
	}
}

void	compute_only_adimtts(int from_top, int n_pt, double delta_pt) {

	/* Calls the apropriate function to compute the theoretical admittance.
	   Take profit of external variables used in other program's options */

	delta_kx = delta_ky = delta_pt;
	nx2 = ny2 = n_pt * 2;

	if (from_top) load_from_top_admit();
	else load_from_below_admit();
}

void	load_from_top_grid(int n) {

	/* Computes the gravity|geoid grid due to the effect of the bathymetry using the theoretical
	admittance for the "loading from top" model --  M. McNutt & Shure (1986)  */

	int k, i;
	double	earth_curvature, alfa, D, twopi, t1, t2, f, p, t, mk;

	f = 1.0;
	for (i = 2; i <= n; i++) f *= i;	/* n! */
	p = (double) n - 1.0;

	twopi = 2. * M_PI;
	D = (youngs_modulus * te * te * te) / (12.0 * (1.0 - poissons_ratio * poissons_ratio));
	alfa = pow(twopi,4.) * D / (normal_gravity * rho_mc);
	raised[0] = 0.0;		raised[1] = 0.0;

	for (k = 0; k < ndatac; k+= 2) {
		mk = modk(k) / twopi;
		if (p == 0.0)
			t = 1.0;
		else if (p == 1.0)
			t = mk;
		else
			t = pow (mk, p);
		earth_curvature = (sphericity) ? (2 * earth_rad * mk) / (4 * M_PI * earth_rad * mk + 1) : 1.;
		t1 = earth_curvature * (twopi * G);
		if (!geoid) t1 *= 1.0e5;		/* to have it in mGals */
		if (geoid) t1 /= (normal_gravity * mk * twopi);
		t2 = exp(-twopi * mk * z_level) - exp(-twopi * mk * zm) / (1 + alfa*pow(mk,4.));
		datac[k] += (float) ((rho_cw * t1 * t2) * t / f * raised[k]);
		datac[k+1] += (float) ((rho_cw * t1 * t2) * t / f * raised[k+1]);
	}
}

void	load_from_below_grid(int n) {

	/* Computes the gravity|geoid grid due to the effect of the bathymetry using the theoretical
	admittance for the "loading from below" model --  M. McNutt & Shure (1986)  */

	int k, i;
	double	earth_curvature, alfa, D, twopi, t1, t2, t3, f, p, t, mk;

	f = 1.0;
	for (i = 2; i <= n; i++) f *= i;	/* n! */
	p = (double) n - 1.0;

	twopi = 2. * M_PI;
	D = (youngs_modulus * te * te * te) / (12.0 * (1.0 - poissons_ratio * poissons_ratio));
	alfa = pow(twopi,4.) * D / (normal_gravity * rho_mc);
	raised[0] = 0.0;		raised[1] = 0.0;

	for (k = 0; k < ndatac; k+= 2) {
		mk = modk(k) / twopi;
		if (p == 0.0)
			t = 1.0;
		else if (p == 1.0)
			t = mk;
		else
			t = pow (mk, p);
		earth_curvature = (sphericity) ? (2 * earth_rad * mk) / (4 * M_PI * earth_rad * mk + 1) : 1.;
		t1 = earth_curvature * (twopi * G);
		if (!geoid) t1 *= 1.0e5;		/* to have it in mGals */
		if (geoid) t1 /= (normal_gravity * mk * twopi);
		t2 = rho_cw * exp(-twopi * mk * z_level) + rho_mc * exp(-twopi * mk * zm);
		t3 = -(rho_mw + rho_mc * pow(mk,4.) * alfa) * exp(-twopi * mk * zl);
		datac[k] += (float) ((t1 * (t2 + t3)) * t / f * raised[k]);
		datac[k+1] += (float) ((t1 * (t2 + t3)) * t / f * raised[k+1]);
	}
}

void	write_script(void) {
	char	*sc_name = "admit_coher.sc", date[25];
	FILE	*fp;
	time_t	now;

	if ((fp = fopen(sc_name,"w")) == NULL) {
		fprintf (stderr, "%s: Could not open file %s for writing\n", GMT_program, sc_name);
		exit (-1);
	}
	strncpy(date,"\0",25);
	now = time(NULL);
	strncpy(date,ctime(&now),24);
	fprintf(fp,"#! /bin/csh -f\n");
	fprintf(fp,"#\n");
	fprintf(fp,"# Created by gravfft at: %s. Output to stdout.\n", date);
	fprintf(fp,"#\n");
	fprintf(fp,"# ATTENTION, a number of temporary files are generated by this script and may\n");
	fprintf(fp,"# overwrite others with the same name if they exist in the current directory.\n");
	fprintf(fp,"# Look at the name declarations to see that it doesn't happen.\n");
	fprintf(fp,"#\n");
	if (sc_admitt)
		fprintf(fp,"# Computes admittance from the radial average over <B*G-conj>/<B*B-conj>\n");
	else if (sc_coherence)
		fprintf(fp,"# Computes coherence from the radial average over <G*B-conj>^2/(<B*B-conj>*<G*G-conj>)\n");
	fprintf(fp,"#\n");
	fprintf(fp,"# The problem is for that you need to have four grids. Two real and two imaginary\n");
	fprintf(fp,"# At current stage I (that's the program speaking) probably only know about one time domain\n");
	fprintf(fp,"# grid (%s) and it's FFT transform (%s and %s)\n", infile, infile_r, infile_i);
	fprintf(fp,"# So I'll assume that the time domain grid is the bathymetry grid. In order to\n");
	fprintf(fp,"# proced with the computation you'll need to have also the FFTed grids of either\n");
	fprintf(fp,"# the gravity or geoid. Again I'll invent and assume that your goid/gravity grid\n");
	fprintf(fp,"# is called geoid.grd. If it is not, CHANGE the name accordingly in the next instruction.\n");
	fprintf(fp,"#\n");
	fprintf(fp,"set grd_g = geoid \t#Sets name of geoid/gravity grid WITHOUT extention.\n");
	fprintf(fp,"\t\t\t#(e.g. if the name is geoid.grd just give geoid)\n");
	fprintf(fp,"#\n");
	fprintf(fp,"# Than you'll have to run (separatly) the following command (or just uncomment it)\n");
	fprintf(fp,"#\n");
	fprintf(fp,"#\t gravfft $grd_g.grd -T [-M]\n");
	fprintf(fp,"#\n");
	fprintf(fp,"if !(-e ${grd_g}_real.grd) then\n");
	fprintf(fp,"\techo ${grd_g}_real.grd | awk '{print \"Warning: \"$1\" does not exist, so probably\"}' > \"/dev/tty\"\n");
	fprintf(fp,"\techo ${grd_g}_imag.grd | awk '{print $1\" does not exist as well. See instruction on script.\"}' > \"/dev/tty\"\n");
	fprintf(fp,"\texit\n");
	fprintf(fp,"endif\n");
	fprintf(fp,"#-----------------------------------------------------------------------------\n");
	fprintf(fp,"# Variable declarations. See that there are no name conflicts with your files.\n");
	fprintf(fp,"set dist = dist.dat\n");
	fprintf(fp,"set dist_r = dist_r.dat\n");
	if (sc_admitt) {
		fprintf(fp,"set ztmp_t = ztmp_t.dat\n");
		fprintf(fp,"set ztmp_b = ztmp_b.dat\n");
	}
	else if (sc_coherence) {
		fprintf(fp,"set ztmp_1 = ztmp_1.dat\n");
		fprintf(fp,"set ztmp_2 = ztmp_2.dat\n");
		fprintf(fp,"set ztmp_4 = ztmp_4.dat\n");
		fprintf(fp,"set ztmp_5 = ztmp_5.dat\n");
	}
	fprintf(fp,"set zz = ztmp_0.dat\n");
	fprintf(fp,"set grd1 = tmp_b_spect.grd\n");
	fprintf(fp,"set grd2 = tmp_co_spect.grd\n");
	if (sc_admitt) fprintf(fp,"set grd3 = admittance.grd\n");
	if (sc_coherence) {
		fprintf(fp,"set grd3 = coherence.grd\n");
		fprintf(fp,"set grd4 = tmp_g_spect.grd\n");
		fprintf(fp,"set grd5 = tmp_quad.grd\n");
	}
	fprintf(fp,"#-----------------------------------------------------------------------------\n");
	fprintf(fp,"#\n");
	if (sc_admitt) {
		fprintf(fp,"grdmath %s %s R2 = $grd1\n", infile_r, infile_i);
		fprintf(fp,"grdmath %s ${grd_g}_real.grd x %s ${grd_g}_imag.grd x + = $grd2\n", infile_r, infile_i);
		fprintf(fp,"#---------------------------------------------------\n");
		fprintf(fp,"# Uncomment if you want to save the admittance grid.\n");
		fprintf(fp,"# However, be awere that radial average over this grid is much\n");
		fprintf(fp,"# different than the <B*G-conj>/<B*B-conj> radial average.\n");
		fprintf(fp,"#grdmath $grd2 $grd1 DIV = $grd3\n");
		fprintf(fp,"#---------------------------------------------------\n");
	}
	else if (sc_coherence) {
		fprintf(fp,"grdmath %s %s R2 = $grd1\n", infile_r, infile_i);
		fprintf(fp,"grdmath ${grd_g}_real.grd ${grd_g}_imag.grd R2 = $grd4\n");
		fprintf(fp,"grdmath %s ${grd_g}_real.grd x %s ${grd_g}_imag.grd x + = $grd2\n", infile_r, infile_i);
		fprintf(fp,"grdmath %s ${grd_g}_real.grd x ${grd_g}_imag.grd %s x - = $grd5\n", infile_i, infile_r);
		fprintf(fp,"#----------------------------------------------------------------\n");
		fprintf(fp,"# Uncomment if you want to save the coherence grid.\n");
		fprintf(fp,"# However, that's useless because you'll find out that it's equal to one.\n");
		fprintf(fp,"# I was first surprised by that, but it follows from the definition.\n");
		fprintf(fp,"#grdmath $grd2 $grd2 x $grd5 $grd5 x + $grd1 $grd4 x DIV = $grd3\n");
		fprintf(fp,"#----------------------------------------------------------------\n");
	}
	fprintf(fp,"#\n");
	fprintf(fp,"set info = `grdinfo -C $grd1`\n");
	fprintf(fp,"set min = `echo $info[8] $info[9] | awk '{print ($1+$2)/2}'`\n");
	fprintf(fp,"set max = `echo $info[3] $info[5] | awk '{print ($1+$2)/2}'`\n");
	fprintf(fp,"set half_width = `echo $min | awk '{print $1/3}'` \t#could also divide by 2\n");
	fprintf(fp,"#\n");
	if (sc_admitt) {
		fprintf(fp,"if (-e $ztmp_t) then\n");
		fprintf(fp,"\techo $ztmp_t | awk '{print \"Warning: deleting pre-existing \"$1\" file\"}' > \"/dev/tty\"\n");
		fprintf(fp,"\t\\rm $ztmp_t\n");
		fprintf(fp,"endif\n");
		fprintf(fp,"if (-e $ztmp_b) then\n");
		fprintf(fp,"\techo $ztmp_b | awk '{print \"Warning: deleting pre-existing \"$1\" file\"}' > \"/dev/tty\"\n");
		fprintf(fp,"\t\\rm $ztmp_b\n");
		fprintf(fp,"endif\n");
	}
	else if (sc_coherence) {
		fprintf(fp,"if (-e $ztmp_1) then\n");
		fprintf(fp,"\techo $ztmp_1 | awk '{print \"Warning: deleting pre-existing \"$1\" file\"}' > \"/dev/tty\"\n");
		fprintf(fp,"\t\\rm $ztmp_1\n");
		fprintf(fp,"endif\n");
		fprintf(fp,"if (-e $ztmp_2) then\n");
		fprintf(fp,"\techo $ztmp_2 | awk '{print \"Warning: deleting pre-existing \"$1\" file\"}' > \"/dev/tty\"\n");
		fprintf(fp,"\t\\rm $ztmp_2\n");
		fprintf(fp,"endif\n");
		fprintf(fp,"if (-e $ztmp_4) then\n");
		fprintf(fp,"\techo $ztmp_4 | awk '{print \"Warning: deleting pre-existing \"$1\" file\"}' > \"/dev/tty\"\n");
		fprintf(fp,"\t\\rm $ztmp_4\n");
		fprintf(fp,"endif\n");
		fprintf(fp,"if (-e $ztmp_5) then\n");
		fprintf(fp,"\techo $ztmp_5 | awk '{print \"Warning: deleting pre-existing \"$1\" file\"}' > \"/dev/tty\"\n");
		fprintf(fp,"\t\\rm $ztmp_5\n");
		fprintf(fp,"endif\n");
	}
	fprintf(fp,"#\n");
	fprintf(fp,"# Computes the radial average.\n");
	fprintf(fp,"#\n");
	fprintf(fp,"echo $min $max | awk '{for (a = $1; a <= $2; a += $1) {print a} }' > $dist\n");
	fprintf(fp,"set nl = `wc dist.dat`\n");
	fprintf(fp,"set ii = 1\n");
	fprintf(fp,"while ($ii <= $nl[1])\n");
	fprintf(fp,"\tset raio = `tail +$ii dist.dat | head -1`\n");
	fprintf(fp,"\techo $raio $half_width | awk 'BEGIN {D2R = 0.01745329; srand()} { \\\n");
	fprintf(fp,"\tr1 = -$2;	r2 = $2; \\\n");
	fprintf(fp,"\tfor (a = 0; a < 360; a += 2){ \\\n");
	fprintf(fp,"\tx = r1 + (r2-r1)*rand() \\\n");
	fprintf(fp,"\tprint ($1+x) * cos (a*D2R), ($1+x) * sin (a*D2R)}}' > $dist_r\n");
	if (sc_admitt) {
		fprintf(fp,"\tgrdtrack $dist_r -G$grd2 > $zz; gmtmath -C2 -S $zz MEAN = | awk '{print $3}' >> $ztmp_t \n");
		fprintf(fp,"\tgrdtrack $dist_r -G$grd1 > $zz; gmtmath -C2 -S $zz MEAN = | awk '{print $3}' >> $ztmp_b \n");
		fprintf(fp,"\t@ ii++ \n");
		fprintf(fp,"end\n");
		fprintf(fp, "paste $dist $ztmp_t $ztmp_b | awk '{print 1/$1, $2/$3}'\n");
	}
	else if (sc_coherence) {
		fprintf(fp,"\tgrdtrack $dist_r -G$grd1 > $zz; gmtmath -C2 -S $zz MEAN = | awk '{print $3}' >> $ztmp_1 \n");
		fprintf(fp,"\tgrdtrack $dist_r -G$grd2 > $zz; gmtmath -C2 -S $zz MEAN = | awk '{print $3}' >> $ztmp_2 \n");
		fprintf(fp,"\tgrdtrack $dist_r -G$grd4 > $zz; gmtmath -C2 -S $zz MEAN = | awk '{print $3}' >> $ztmp_4 \n");
		fprintf(fp,"\tgrdtrack $dist_r -G$grd5 > $zz; gmtmath -C2 -S $zz MEAN = | awk '{print $3}' >> $ztmp_5 \n");
		fprintf(fp,"\t@ ii++ \n");
		fprintf(fp,"end\n");
		fprintf(fp,"paste $dist $ztmp_2 $ztmp_5 $ztmp_1 $ztmp_4 | awk '{print 1/$1, (($2*$2)+($3*$3))/($4*$5)}'\n");
	}
	fprintf(fp,"#\n");
	fprintf(fp,"# Clean temporary files\n");
	if (sc_admitt)
		fprintf(fp,"\\rm $grd1 $grd2 $dist $dist_r $ztmp_t $ztmp_b $zz\n");
	else if (sc_coherence)
		fprintf(fp,"\\rm $grd1 $grd2 $dist $dist_r $grd4 $grd5 $ztmp_1 $ztmp_2 $ztmp_4 $ztmp_5 $zz\n");
	fclose(fp);
}
