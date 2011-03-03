/*--------------------------------------------------------------------
 * $Id: dimfilter.c,v 1.14 2011-03-03 21:02:51 guru Exp $
 *
 *	Copyright (c) 2009-2011 by P. Wessel and Seung-Sep Kim
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
/* dimfilter.c  reads a grdfile and creates filtered grd file
 *
 * user selects primary filter radius, number of sectors, and the secondary filter.
 * The Primary filter determines how we want to filter the raw data. However, instead
 * of filtering all data inside the filter radius at once, we split the filter circle
 * into several sectors and apply the filter on the data within each sector.  The
 * Secondary filter is then used to consolidate all the sector results into one output
 * value.  As an option for robust filters, we can detrend the input data prior to
 * applying the primary filter using a LS plane fit.
 *
 * Author: 	Paul Wessel with help from Caleb Fassett & Seung-Sep Kim
 * Date: 	25-MAR-2008
 * Version:	GMT 4
 *
 * For details, see Kim, S.-S., and Wessel, P. 2008, "Directional Median Filtering
 * for Regional-Residual Separation of Bathymetry, Geochem. Geophys. Geosyst.,
 * 9(Q03005), doi:10.1029/2007GC001850. 
 */
 
#define GMT_WITH_NO_PS
#include "gmt.h"

double	*weight, deg2km;

int main (int argc, char **argv)
{
	short int **sector = NULL;
	
	GMT_LONG nx_out, ny_out, nx_fil, ny_fil, *n_in_median, n_nan = 0;
	GMT_LONG x_half_width, y_half_width, j_origin, i_out, j_out, wsize = 0;
	GMT_LONG i_in, j_in, ii, jj, i, j, ij_in, ij_out, ij_wt, effort_level, k, s, n = 0;
	GMT_LONG distance_flag, filter_type, filter2_type, n_sectors = 1, n_sectors_2 = 0, one_or_zero = 1;
	GMT_LONG GMT_mode_selection = 0, GMT_n_multiples = 0, *n_alloc = NULL;
	
	FILE *ip;
	GMT_LONG err_cols = 0, err_l=1;
	int not_used = 0;
	double err_workarray[50], err_min, err_max, err_null_median=0.0, err_median, err_mad, err_depth, err_mean, err_sum;
   	GMT_LONG dimerr=FALSE;
	
	GMT_LONG error, new_range, new_increment, fast_way, shift = FALSE, slow, slow2, toggle = FALSE, corridor = FALSE;
#ifdef OBSOLETE	
	GMT_LONG do_scale = FALSE, trend = FALSE, first_time = TRUE;
#endif
	
	double west_new, east_new, south_new, north_new, dx_new, dy_new, offset;
	double filter_width, x_scale, y_scale, x_width, y_width, angle, z = 0.0;
	double x_out, y_out, *wt_sum, *value, last_median, this_median, last_median2 = 0.0, this_median2, xincnew2, yincnew2;
	double z_min, z_max, z2_min = 0.0, z2_max = 0.0, wx = 0.0, *c_x = NULL, *c_y = NULL, d;
	double xincold2, yincold2, y_shift = 0.0, x_fix = 0.0, y_fix = 0.0;
#ifdef DEBUG
	double x_debug[5];
	double y_debug[5];
	double z_debug[5];
#endif
	
	char *fin = CNULL, *fout = CNULL;
#ifdef OBSOLETE
	int n_bad_planes = 0, S = 0;
	double	Sx = 0.0, Sy = 0.0, Sz = 0.0, Sxx = 0.0, Syy = 0.0, Sxy = 0.0, Sxz = 0.0, Syz = 0.0;
	double denominator, scale, Sw;
	double intercept = 0.0, slope_x = 0.0, slope_y = 0.0, inv_D;
	char *fout2 = CNULL;
#endif	
	double	**work_array = NULL;
	GMT_LONG *i_origin;
	float *input, *output;
	double *x_shift = NULL;

	#ifdef OBSOLETE
	double	*work_array2;			
	float	*output2;
	short int **xx, **yy;
	#endif

	char *filter_name[5] = {
		"Boxcar",
		"Cosine Arch",
		"Gaussian",
		"Median",
		"Mode"
	};
	
	struct	GRD_HEADER h, test_h;
	void	set_weight_matrix (GMT_LONG nx_f, GMT_LONG ny_f, double y_0, double north, double south, double dx, double dy, double f_wid, GMT_LONG f_flag, GMT_LONG d_flag, double x_off, double y_off, GMT_LONG fast);


	argc = (int)GMT_begin (argc, argv);
	
	deg2km = 0.001 * 2.0 * M_PI * gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius / 360.0;
	error = new_range = new_increment = FALSE;
	fin = fout = NULL;
#ifdef OBSOLETE
fout2 = NULL;
#endif	
	filter_width = dx_new = dy_new = west_new = east_new = 0.0;
	filter_type = filter2_type = distance_flag = -1;
	
	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				/* Common parameters */
			
				case 'R':
				case 'V':
				case '\0':
					error += GMT_get_common_args (argv[i], &west_new, &east_new, &south_new, &north_new);
					break;
				
				case 'F':
					switch (argv[i][2]) {
						case 'b':
							filter_type = 0;
							break;
						case 'c':
							filter_type = 1;
							break;
						case 'g':
							filter_type = 2;
							break;
						case 'm':
							filter_type = 3;
							break;
						case 'p':
							filter_type = 4;
							break;
						default:
							error = TRUE;
							break;
					}
					filter_width = atof(&argv[i][3]);
					break;
				case 'N':	/* SCAN:  Option to set the number of sections and how to reduce the sector results to a single value */
					switch (argv[i][2]) {
						case 'l':	/* Lower bound (min) */
							filter2_type = 0;
							break;
						case 'u':	/* Upper bound (max) */
							filter2_type = 1;
							break;
						case 'a':	/* Average (mean) */
							filter2_type = 2;
							break;
						case 'm':	/* Median */
							filter2_type = 3;
							break;
						case 'p':	/* Mode */
							filter2_type = 4;
							break;
						default:
							error = TRUE;
							break;
					}
					n_sectors = atoi(&argv[i][3]);	/* Number of sections to split filter into */
					break;
				case 'G':
					fout = &argv[i][2];
					break;
#ifdef OBSOLETE					
				case 'E':
					trend = TRUE;
					break;
#endif					
				case 'C':
					corridor = TRUE;
					break;
				case 'D':
					distance_flag = atoi(&argv[i][2]);
					break;
				case 'I':
					GMT_getinc (&argv[i][2], &dx_new, &dy_new);
					new_increment = TRUE;
					break;
#ifdef OBSOLETE					
				case 'S':
					fout2 = &argv[i][2];
					do_scale = TRUE;
					break;
#endif					
				case 'T':	/* Toggle registration */
					toggle = TRUE;
					break;
				case 'Q':  /* entering to MAD error analysis mode */
					dimerr = TRUE;
					err_cols = atoi(&argv[i][2]);
					break;
				default:
					error = TRUE;
					GMT_default_error (argv[i][1]);
					break;
			}
		}
		else
			fin = argv[i];
	}
	
		
	if (argc == 1 || GMT_give_synopsis_and_exit) {
		fprintf (stderr, "%s %s - Directional filtering of 2-D grdfiles in the Space domain\n\n", GMT_program, GMT_VERSION);
		fprintf(stderr,"usage: %s input_file -D<distance_flag> -F<type><filter_width>\n", GMT_program);
		fprintf(stderr,"\t-G<output_file> -N<type><n_sectors> [-I<xinc>[m|c][/<yinc>[m|c]] ]\n");
		fprintf(stderr,"\t[-Q<cols>] [-R<west/east/south/north>] [-T] [-V]\n");
		
		if (GMT_give_synopsis_and_exit) exit (EXIT_FAILURE);
		
		fprintf(stderr,"\tDistance flag determines how grid (x,y) maps into distance units of filter width as follows:\n");
		fprintf(stderr,"\t   -D0 grid x,y same units as <filter_width>, cartesian Distances.\n");
		fprintf(stderr,"\t   -D1 grid x,y in degrees, <filter_width> in km, cartesian Distances.\n");
		fprintf(stderr,"\t   -D2 grid x,y in degrees, <filter_width> in km, x_scaled by cos(middle y), cartesian Distances.\n");
		fprintf(stderr,"\t   These first three options are faster; they allow weight matrix to be computed only once.\n");
		fprintf(stderr,"\t   Next two options are slower; weights must be recomputed for each scan line.\n");
		fprintf(stderr,"\t   -D3 grid x,y in degrees, <filter_width> in km, x_scale varies as cos(y), cartesian Distances.\n");
		fprintf(stderr,"\t   -D4 grid x,y in degrees, <filter_width> in km, spherical Distances.\n");
		fprintf(stderr,"\t-F sets the primary filter type and full (6 sigma) filter-width.  Choose between\n");
		fprintf(stderr,"\t   (b)oxcar, (c)osine arch, (g)aussian, (m)edian filters\n");
		fprintf(stderr,"\t   or p(maximum liklihood Probability estimator -- a mode estimator)\n");
		fprintf(stderr,"\t-G sets output name for filtered grdfile\n");
		fprintf(stderr,"\t-N sets the secondary filter type and the number of sectors.  Choose between\n");
		fprintf(stderr,"\t   (l)ower, (u)pper, (a)verage, (m)edian, and (p) the mode estimator)\n");
		fprintf(stderr, "\n\tOPTIONS:\n");
#ifdef OBSOLETE							
		fprintf(stderr,"\t-E Remove local planar trend from data, apply filter, then add back trend at filtered value.\n");
#endif		
		fprintf(stderr,"\t-I for new Increment of output grid; enter xinc, optionally xinc/yinc.\n");
		fprintf(stderr,"\t   Default is yinc = xinc.  Append an m [or c] to xinc or yinc to indicate minutes [or seconds];\n");
		fprintf(stderr,"\t   The new xinc and yinc should be divisible by the old ones (new lattice is subset of old).\n");
		fprintf(stderr,"\t-Q is for the error analysis mode and requires the total number of depth columns in the input file.\n");
		fprintf(stderr, "\t-R for new Range of output grid; enter <WESN> (xmin, xmax, ymin, ymax) separated by slashes.\n");
#ifdef OBSOLETE							
		fprintf(stderr,"\t-S sets output name for standard error grdfile and implies that we will compute a 2nd grid with\n");
		fprintf(stderr,"\t   a statistical measure of deviations from the average value.  For the convolution filters this\n");
		fprintf(stderr,"\t   yields the standard deviation while for the median/mode filters we use MAD\n");
#endif		
		fprintf(stderr, "\t-T Toggles between grid and pixel registration for output grid [Default is same as input registration]\n");
		GMT_explain_option ('V');
		exit (EXIT_FAILURE);
	}

	if (!dimerr) {
	
		if (!fout) {
			fprintf (stderr, "%s: GMT SYNTAX ERROR -G option:  Must specify output file\n", GMT_program);
			error++;
		}
		if (!fin) {
			fprintf (stderr, "%s: GMT SYNTAX ERROR:  Must specify input file\n", GMT_program);
			error++;
		}
		if (distance_flag < 0 || distance_flag > 4) {
			fprintf (stderr, "%s: GMT SYNTAX ERROR -D option:  Choose among 1, 2, 3, or 4\n", GMT_program);
			error++;
		}
		if (filter_type < 0 || filter_width <= 0.0) {
			fprintf (stderr, "%s: GMT SYNTAX ERROR -F option:  Correct syntax:\n", GMT_program);
			fprintf (stderr, "\t-FX<width>, with X one of bcgmp, width is filter fullwidth\n");
			error++;
		}
		if (filter2_type < 0 || n_sectors <= 0) {
			fprintf (stderr, "%s: GMT SYNTAX ERROR -N option:  Correct syntax:\n", GMT_program);
			fprintf (stderr, "\t-NX<nsectors>, with X one of luamp, nsectors is number of sectors\n");
			error++;
		}
		slow = (filter_type == 3 || filter_type == 4);		/* Will require sorting etc */
		slow2 = (filter2_type == 3 || filter2_type == 4);	/* SCAN: Will also require sorting etc */
	#ifdef OBSOLETE						
		if (trend && !slow) {
			fprintf (stderr, "%s: GMT SYNTAX ERROR -E option:  Only valid for robust filters -Fm|p.\n", GMT_program);
			error++;
		}
	#endif	
		if (error) exit (EXIT_FAILURE);

		if (project_info.region_supplied) new_range = TRUE;

		if (GMT_read_grd_info (fin, &h)) {
			fprintf (stderr, "%s: Error opening file %s\n", GMT_program, fin);
			exit (EXIT_FAILURE);
		}
		GMT_grd_init (&h, argc, argv, TRUE);	/* Update command history only */

		if (toggle) {	/* Make output grid of the opposite registration */
			one_or_zero = (h.node_offset) ? 1 : 0;
		}
		else	/* Same as input grid */
			one_or_zero = (h.node_offset) ? 0 : 1;
		
		/* Read the input grd file and close it  */
		
		input = (float *) GMT_memory (VNULL, (size_t)(h.nx * h.ny), sizeof(float), GMT_program);

		if (GMT_read_grd (fin, &h, input, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE)) {
			fprintf (stderr, "%s: Error reading file %s\n", GMT_program, fin);
			exit (EXIT_FAILURE);
		}
		
		last_median = 0.5 * (h.z_min + h.z_max);
		z_min = h.z_min;	z_max = h.z_max;

		/* Check range of output area and set i,j offsets, etc.  */

		if (!new_range) {
			west_new = h.x_min;
			east_new = h.x_max;
			south_new = h.y_min;
			north_new = h.y_max;
		}
		if (!new_increment) {
			dx_new = h.x_inc;
			dy_new = h.y_inc;
		}

		if (west_new < h.x_min) error = TRUE;
		if (east_new > h.x_max) error = TRUE;
		if (south_new < h.y_min) error = TRUE;
		if (north_new > h.y_max) error = TRUE;
		if (dx_new <= 0.0) error = TRUE;
		if (dy_new <= 0.0) error = TRUE;
		
		if (error) {
			fprintf(stderr,"%s: New WESN incompatible with old.\n", GMT_program);
			exit (EXIT_FAILURE);
		}

		/* Make sure output grid is kosher */

		test_h.x_min = west_new;	test_h.x_max = east_new;	test_h.x_inc = dx_new;
		test_h.y_min = south_new;	test_h.y_max = north_new;	test_h.y_inc = dy_new;
		GMT_grd_RI_verify (&test_h, 1);

		/* We can save time by computing a weight matrix once [or once pr scanline] only
		   if new grid spacing is multiple of old spacing */
		   
		fast_way = ((irint (dx_new / h.x_inc) == (dx_new / h.x_inc)) && (irint (dy_new / h.y_inc) == (dy_new / h.y_inc)));
		
		if (!fast_way && gmtdefs.verbose) {
			fprintf (stderr, "%s: Warning - Your output grid spacing is such that filter-weights must\n", GMT_program);
			fprintf (stderr, "be recomputed for every output node, so expect this run to be slow.  Calculations\n");
			fprintf (stderr, "can be speeded up significantly if output grid spacing is chosen to be a multiple\n");
			fprintf (stderr, "of the input grid spacing.  If the odd output grid is necessary, consider using\n");
			fprintf (stderr, "a \'fast\' grid for filtering and then resample onto your desired grid with grdsample.\n");
		}

		nx_out = one_or_zero + irint ( (east_new - west_new) / dx_new);
		ny_out = one_or_zero + irint ( (north_new - south_new) / dy_new);
		
		output = (float *) GMT_memory (VNULL, (size_t)(nx_out*ny_out), sizeof(float), GMT_program);
	#ifdef OBSOLETE						
		if (do_scale) output2 = (float *) GMT_memory (VNULL, (size_t)(nx_out*ny_out), sizeof(float), GMT_program);
	#endif	
		i_origin = (GMT_LONG *) GMT_memory (VNULL, (size_t)nx_out, sizeof(GMT_LONG), GMT_program);
		if (!fast_way) x_shift = (double *) GMT_memory (VNULL, (size_t)nx_out, sizeof(double), GMT_program);

		xincnew2 = (one_or_zero) ? 0.0 : 0.5 * dx_new;
		yincnew2 = (one_or_zero) ? 0.0 : 0.5 * dy_new;
		/* xincold2 = (h.node_offset) ? 0.0 : 0.5 * h.x_inc;
		yincold2 = (h.node_offset) ? 0.0 : 0.5 * h.y_inc; */
		xincold2 = (h.node_offset) ? 0.5 * h.x_inc : 0.0;
		yincold2 = (h.node_offset) ? 0.5 * h.y_inc : 0.0;
		offset = (h.node_offset) ? 0.0 : 0.5;
		
		if (fast_way && h.node_offset == one_or_zero) {	/* multiple grid but one is pix, other is grid */
			x_fix = 0.5 * h.x_inc;
			y_fix = 0.5 * h.y_inc;
			shift = (x_fix != 0.0 || y_fix != 0.0);
		}
		
		/* Set up weight matrix and i,j range to test  */

		x_scale = y_scale = 1.0;
		if (distance_flag > 0) {
			x_scale *= deg2km;
			y_scale *= deg2km;
		}
		if (distance_flag == 2)
			x_scale *= cos (D2R * 0.5 * (north_new + south_new) );
		else if (distance_flag > 2) {
			if ( fabs(south_new) > north_new )
				x_scale *= cos (D2R * south_new);
			else
				x_scale *= cos (D2R * north_new);
		}
		x_width = filter_width / (h.x_inc * x_scale);
		y_width = filter_width / (h.y_inc * y_scale);
		y_half_width = (GMT_LONG) (ceil(y_width) / 2.0);
		x_half_width = (GMT_LONG) (ceil(x_width) / 2.0);

		nx_fil = 2 * x_half_width + 1;
		ny_fil = 2 * y_half_width + 1;
		weight = (double *) GMT_memory (VNULL, (size_t)(nx_fil*ny_fil), sizeof(double), GMT_program);

		if (slow) {	/* SCAN: Now require several work_arrays, one for each sector */
			work_array = (double **) GMT_memory (VNULL, (size_t)(n_sectors), sizeof(double *), GMT_program);
			n_alloc = (GMT_LONG *) GMT_memory (VNULL, (size_t)(n_sectors), sizeof(GMT_LONG), GMT_program);
	#ifdef OBSOLETE							
			if (do_scale) work_array2 = (double *) GMT_memory (VNULL, (size_t)(2*nx_fil*ny_fil), sizeof(double), GMT_program);
			if (trend) {
				xx = (short int **) GMT_memory (VNULL, (size_t)(n_sectors), sizeof(short int *), GMT_program);
				yy = (short int **) GMT_memory (VNULL, (size_t)(n_sectors), sizeof(short int *), GMT_program);
			}
	#endif		
			wsize = 2*nx_fil*ny_fil/n_sectors;	/* Should be enough, watch for messages to the contrary */
			for (i = 0; i < n_sectors; i++) {
				work_array[i] = (double *) GMT_memory (VNULL, (size_t)(wsize), sizeof(double), GMT_program);
				n_alloc[i] = wsize;
	#ifdef OBSOLETE								
				if (trend) {
					xx[i] = (short int *) GMT_memory (VNULL, (size_t)(wsize), sizeof(short int), GMT_program);
					yy[i] = (short int *) GMT_memory (VNULL, (size_t)(wsize), sizeof(short int), GMT_program);
				}
	#endif			
			}
		}
		
		if (gmtdefs.verbose) {
			fprintf(stderr,"%s: Input nx,ny = (%d %d), output nx,ny = (%ld %ld), filter nx,ny = (%ld %ld)\n", GMT_program,
				h.nx, h.ny, nx_out, ny_out, nx_fil, ny_fil);
			fprintf(stderr,"%s: Filter type is %s.\n", GMT_program, filter_name[filter_type]);
		}
		
		/* Compute nearest xoutput i-indices and shifts once */
		
		for (i_out = 0; i_out < nx_out; i_out++) {
			x_out = west_new + i_out * dx_new + xincnew2;
			i_origin[i_out] = (GMT_LONG)floor(((x_out - h.x_min) / h.x_inc) + offset);
			if (!fast_way) x_shift[i_out] = x_out - (h.x_min + i_origin[i_out] * h.x_inc + xincold2);
		}
		
		/* Now we can do the filtering  */

		/* Determine how much effort to compute weights:
			1 = Compute weights once for entire grid
			2 = Compute weights once per scanline
			3 = Compute weights for every output point [slow]
		*/
		
		if (fast_way && distance_flag <= 2)
			effort_level = 1;
		else if (fast_way && distance_flag > 2)
			effort_level = 2;
		else
			effort_level = 3;
			
		if (effort_level == 1) {	/* Only need this once */
			y_out = north_new - yincnew2;
			set_weight_matrix (nx_fil, ny_fil, y_out, north_new, south_new, h.x_inc, h.y_inc, filter_width, filter_type, distance_flag, x_fix, y_fix, shift);
		}

		if (corridor) {	/* Use fixed-width diagonal corridors instead of bow-ties */
			c_x = (double *) GMT_memory (VNULL, (size_t)(n_sectors/2), sizeof(double), GMT_program);
			c_y = (double *) GMT_memory (VNULL, (size_t)(n_sectors/2), sizeof(double), GMT_program);
			n_sectors_2 = n_sectors / 2;
			for (i = 0; i < n_sectors_2; i++) {
				angle = (i + 0.5) * (M_PI/n_sectors_2);	/* Angle of central diameter of each corridor */
				sincos (angle, &c_y[i], &c_x[i]);	/* Unit vector of diameter */
			}
		}
		else {
		/* SCAN: Precalculate which sector each point belongs to */
			sector = (short int **) GMT_memory (VNULL, (size_t)(ny_fil), sizeof(short int *), GMT_program);
			for (jj = 0; jj < ny_fil; jj++) sector[jj] = (short int *) GMT_memory (VNULL, (size_t)(nx_fil), sizeof(short int), GMT_program);
			for (jj = -y_half_width; jj <= y_half_width; jj++) {	/* This double loop visits all nodes in the square centered on an output node */
				j = y_half_width + jj;
				for (ii = -x_half_width; ii <= x_half_width; ii++) {	/* (ii, jj) is local coordinates relative center (0,0) */
					i = x_half_width + ii;
					/* We are doing "bow-ties" and not wedges here */
					angle = atan2 ((double)jj, (double)ii);				/* Returns angle in -PI,+PI range */
					if (angle < 0.0) angle += M_PI;					/* Flip to complimentary sector in 0-PI range */
					sector[j][i] = (short) rint ((n_sectors * angle) / M_PI);	/* Convert to sector id 0-<n_sectors-1> */
					if (sector[j][i] == n_sectors) sector[j][i] = 0;		/* Ensure that exact PI is set to 0 */
				}
			}
		}
		n_in_median = (GMT_LONG *) GMT_memory (VNULL, (size_t)(n_sectors), sizeof(GMT_LONG), GMT_program);
		value = (double *) GMT_memory (VNULL, (size_t)(n_sectors), sizeof(double), GMT_program);
		wt_sum = (double *) GMT_memory (VNULL, (size_t)(n_sectors), sizeof(double), GMT_program);
				
		for (j_out = 0; j_out < ny_out; j_out++) {
		
			if (gmtdefs.verbose) fprintf (stderr, "%s: Processing output line %ld\r", GMT_program, j_out);
			y_out = north_new - j_out * dy_new - yincnew2;
			j_origin = (GMT_LONG)floor(((h.y_max - y_out) / h.y_inc) + offset);
			if (effort_level == 2)
				set_weight_matrix (nx_fil, ny_fil, y_out, north_new, south_new, h.x_inc, h.y_inc, filter_width, filter_type, distance_flag, x_fix, y_fix, shift);
			if (!fast_way) y_shift = y_out - (h.y_max - j_origin * h.y_inc - yincold2);
			
			for (i_out = 0; i_out < nx_out; i_out++) {
			
				if (effort_level == 3)
					set_weight_matrix (nx_fil, ny_fil, y_out, north_new, south_new, h.x_inc, h.y_inc, filter_width, filter_type, distance_flag, x_shift[i_out], y_shift, fast_way);
				memset ((void *)n_in_median, 0, (size_t)(n_sectors * sizeof (GMT_LONG)));
				memset ((void *)value, 0, (size_t)(n_sectors * sizeof (double)));
				memset ((void *)wt_sum, 0, (size_t)(n_sectors * sizeof (double)));
	#ifdef OBSOLETE			
				if (trend) S = 0, Sx = Sy = Sz = Sxx = Syy = Sxy = Sxz = Syz = 0.0;
	#endif			

				ij_out = j_out * nx_out + i_out;
				
				n = 0;
	#ifdef OBSOLETE						
				Sxx = Sw = 0.0;
	#endif			
				for (ii = -x_half_width; ii <= x_half_width; ii++) {
					i_in = i_origin[i_out] + ii;
					if ( (i_in < 0) || (i_in >= h.nx) ) continue;

					for (jj = -y_half_width; jj <= y_half_width; jj++) {
						j_in = j_origin + jj;
						if ( (j_in < 0) || (j_in >= h.ny) ) continue;
											
						ij_wt = (jj + y_half_width) * nx_fil + ii + x_half_width;
						if (weight[ij_wt] < 0.0) continue;
						
						ij_in = j_in*h.nx + i_in;
						if (GMT_is_fnan (input[ij_in])) continue;
						
						/* Get here when point is usable  */
						
						if (corridor) {	/* Point can belong to several corridors */
							for (s = 0; s < n_sectors_2; s++) {
								d = sqrt (c_y[s] * ii + c_x[s] * jj);	/* Perpendicular distance to central diameter, in nodes */
								if (d > y_half_width) continue;	/* Outside this corridor */
								if (slow) {
									if (n_in_median[s] == n_alloc[s]) {
										n_alloc[s] += wsize;
										work_array[s] = (double *) GMT_memory ((void *)work_array[s], (size_t)(n_alloc[s]), sizeof(double), GMT_program);
									}
									work_array[s][n_in_median[s]] = input[ij_in];
	#ifdef OBSOLETE												
									if (do_scale) work_array2[n++] = input[ij_in];
	#endif							
	#ifdef DEBUG
									if (n_in_median[s] < 5) x_debug[n_in_median[s]] = ii;
									if (n_in_median[s] < 5) y_debug[n_in_median[s]] = jj;
									if (n_in_median[s] < 5) z_debug[n_in_median[s]] = input[ij_in];
	#endif
	#ifdef OBSOLETE					
									if (trend) {	/* Sum up required terms to solve for slope and intercepts of planar trend */
									xx[s][n_in_median[s]] = ii;
										yy[s][n_in_median[s]] = jj;
										Sx += ii;
										Sy += jj;
										Sz += input[ij_in];
										Sxx += ii * ii;
										Syy += jj * jj;
										Sxy += ii * jj;
										Sxz += ii * input[ij_in];
										Syz += jj * input[ij_in];
										S++;
									}
		#endif							
									n_in_median[s]++;
								}
								else {
									wx = input[ij_in] * weight[ij_wt];
									value[s] += wx;
									wt_sum[s] += weight[ij_wt];
		#ifdef OBSOLETE												
									if (do_scale) {
										Sxx += wx * input[ij_in];
										Sw += weight[ij_wt];
										n++;
									}
		#endif							
								}
							}
						}
						else if (ii == 0 && jj == 0) {	/* Center point belongs to all sectors */
							if (slow) {	/* Must store copy in all work arrays */
								for (s = 0; s < n_sectors; s++) {
									if (n_in_median[s] == n_alloc[s]) {
										n_alloc[s] += wsize;
										work_array[s] = (double *) GMT_memory ((void *)work_array[s], (size_t)(n_alloc[s]), sizeof(double), GMT_program);
									}
									work_array[s][n_in_median[s]] = input[ij_in];
	#ifdef DEBUG
									if (n_in_median[s] < 5) x_debug[n_in_median[s]] = ii;
									if (n_in_median[s] < 5) y_debug[n_in_median[s]] = jj;
									if (n_in_median[s] < 5) z_debug[n_in_median[s]] = input[ij_in];
	#endif
	#ifdef OBSOLETE					
									if (trend) xx[s][n_in_median[s]] = yy[s][n_in_median[s]] = 0;	/*(0,0) at the node */
	#endif								
									n_in_median[s]++;
								}
	#ifdef OBSOLETE												
								if (do_scale) work_array2[n++] = input[ij_in];
	#endif							
							}
							else {	/* Simply add to the weighted sums */
								for (s = 0; s < n_sectors; s++) {
									wx = input[ij_in] * weight[ij_wt];
									value[s] += wx;
									wt_sum[s] += weight[ij_wt];
								}
	#ifdef OBSOLETE												
								if (do_scale) {
									Sxx += wx * input[ij_in];
									Sw += weight[ij_wt];
									n++;
								}
	#endif							
							}
	#ifdef OBSOLOTE						
							if (trend) {	/* Since r is 0, only need to update Sz and S */
								Sz += input[ij_in];
								S++;
							}
	#endif					
						}
						else {
							s = sector[jj+y_half_width][ii+x_half_width];	/* Get the sector for this node */

							if (slow) {
								if (n_in_median[s] == n_alloc[s]) {
									n_alloc[s] += wsize;
									work_array[s] = (double *) GMT_memory ((void *)work_array[s], (size_t)(n_alloc[s]), sizeof(double), GMT_program);
								}
								work_array[s][n_in_median[s]] = input[ij_in];
	#ifdef OBSOLETE												
								if (do_scale) work_array2[n++] = input[ij_in];
	#endif							
	#ifdef DEBUG
								if (n_in_median[s] < 5) x_debug[n_in_median[s]] = ii;
								if (n_in_median[s] < 5) y_debug[n_in_median[s]] = jj;
								if (n_in_median[s] < 5) z_debug[n_in_median[s]] = input[ij_in];
	#endif
	#ifdef OBSOLETE					
								if (trend) {	/* Sum up required terms to solve for slope and intercepts of planar trend */
									xx[s][n_in_median[s]] = ii;
									yy[s][n_in_median[s]] = jj;
									Sx += ii;
									Sy += jj;
									Sz += input[ij_in];
									Sxx += ii * ii;
									Syy += jj * jj;
									Sxy += ii * jj;
									Sxz += ii * input[ij_in];
									Syz += jj * input[ij_in];
									S++;
								}
	#endif							
								n_in_median[s]++;
							}
							else {
								wx = input[ij_in] * weight[ij_wt];
								value[s] += wx;
								wt_sum[s] += weight[ij_wt];
	#ifdef OBSOLETE												
								if (do_scale) {
									Sxx += wx * input[ij_in];
									Sw += weight[ij_wt];
									n++;
								}
	#endif							
							}
						}
					}
				}
				
				/* Now we have done the sectoring and we can apply the filter on each sector  */
				/* k will be the number of sectors that had enough data to do the operation */
	#ifdef OBSOLETE								
				if (trend) {	/* Must find trend coeeficients, remove from array, do the filter, then add in intercept (since r = 0) */
					denominator = S * Sxx * Syy + 2.0 * Sx * Sy * Sxy - S * Sxy * Sxy - Sx * Sx * Syy - Sy * Sy * Sxx;
					if (denominator == 0.0) {
						intercept = slope_x = slope_y = 0.0;
						n_bad_planes++;
					}
					else {
						inv_D = 1.0 / denominator;
						intercept = (S * Sxx * Syz + Sx * Sy * Sxz + Sz * Sx * Sxy - S * Sxy * Sxz - Sx * Sx * Syz - Sz * Sy * Sxx) * inv_D;
						slope_x = (S * Sxz * Syy + Sz * Sy * Sxy + Sy * Sx * Syz - S * Sxy * Syz - Sz * Sx * Syy - Sy * Sy * Sxz) * inv_D;
						slope_y = (S * Sxx * Syz + Sx * Sy * Sxz + Sz * Sx * Sxy - S * Sxy * Sxz - Sx * Sx * Syz - Sz * Sy * Sxx) * inv_D;
					}
				}
	#endif			
				
				if (slow) {	/* Take median or mode of each work array for each sector */
					if (slow2) {
						z2_min = DBL_MAX;
						z2_max = -DBL_MIN;
					}
					for (s = k = 0; s < n_sectors; s++) {
						if (n_in_median[s]) {
	#ifdef OBSOLETE											
							if (trend) {
								z_min = DBL_MAX;
								z_max = -DBL_MIN;
								for (ii = 0; ii < n_in_median[s]; ii++) {
									work_array[s][ii] -= (intercept + slope_x * xx[s][ii] + slope_y * yy[s][ii]);
									if (work_array[s][ii] < z_min) z_min = work_array[s][ii];
									if (work_array[s][ii] > z_max) z_max = work_array[s][ii];
								}
								if (first_time) last_median = 0.5 * (z_min + z_max), first_time = FALSE;
							}
	#endif						
							if (filter_type == 3) {
								GMT_median (work_array[s], n_in_median[s], z_min, z_max, last_median, &this_median);
								last_median = this_median;
							}
							else
								GMT_mode (work_array[s], n_in_median[s], n_in_median[s]/2, TRUE, GMT_mode_selection, &GMT_n_multiples, &this_median);
							value[k] = this_median;
	#ifdef OBSOLETE											
							if (trend) value[k] += intercept;	/* I.e., intercept + x * slope_x + y * slope_y, but x == y == 0 at node */
	#endif						
							if (slow2) {
								if (value[k] < z2_min) z2_min = value[k];
								if (value[k] > z2_max) z2_max = value[k];
							}
							k++;
						}
					}
				}
				else {	/* Simply divide weighted sums by the weights */
					for (s = k = 0; s < n_sectors; s++) {
						if (wt_sum[s] != 0.0) {
							value[k] = (float)(value[s] / wt_sum[s]);
							k++;
						}
					}
				}
				
				if (k == 0) {	/* No filtered values, set to NaN and move on to next output node */
					output[ij_out] = GMT_f_NaN;
	#ifdef OBSOLETE									
					if (do_scale) output2[ij_out] = GMT_f_NaN;
	#endif				
					n_nan++;
					continue;
				}

				if (slow2) {	/* Get median (or mode) of all the medians (or modes) */
					if (filter_type == 3) {
						GMT_median (value, k, z2_min, z2_max, last_median2, &this_median2);
						last_median2 = this_median2;
					}
					else
						GMT_mode (value, k, k/2, TRUE, GMT_mode_selection, &GMT_n_multiples, &this_median2);
					z = this_median2;
				}
				else {	/* Get min, max, or mean */
					switch (filter2_type) {	/* Initialize z, the final output value */
						case 0:	/* Lower bound */
							z = DBL_MAX;
							break;
						case 1:	/* Upper bound */
							z = -DBL_MAX;
							break;
						case 2:	/* Average (mean) */
							z = 0.0;
							break;
						default:
							break;
					}
					for (s = 0; s < k; s++) {	/* Apply the min, max, or mean update */
						switch (filter2_type) {
							case 0:	/* Lower bound */
								if (value[s] < z) z = value[s];
								break;
							case 1:	/* Upper bound */
								if (value[s] > z) z = value[s];
								break;
							case 2:	/* Average (mean) */
								z += value[s];
								break;
							default:
								break;
						}
					}
					if (filter2_type == 2) z /= (double)k;	/* Mean requires a final normalization */
				}
				output[ij_out] = (float)z;
	#ifdef OBSOLETE								
				if (do_scale) {	/* Now assess a measure of deviation about this value */
					if (slow) {	/* Get MAD! */
						qsort((void *)work_array2, (size_t)n, sizeof(double), GMT_comp_double_asc);
						GMT_getmad (work_array2, n, z, &scale);
					}
					else {		/* Get weighted stdev. */
						scale = sqrt ((Sxx - Sw * z * z) / (Sw * (n - 1) / n));
					}
					output2[ij_out] = (float)scale;
				}
	#endif			
			}
		}
		if (gmtdefs.verbose) fprintf (stderr, "\n");
		
		/* At last, that's it!  Output: */

		if (n_nan && gmtdefs.verbose) fprintf (stderr, "%s: Unable to estimate value at %ld nodes, set to NaN\n", GMT_program, n_nan);
	#ifdef OBSOLETE						
		if (trend && n_bad_planes && gmtdefs.verbose) fprintf (stderr, "%s: Unable to detrend data at %ld nodes\n", GMT_program, n_bad_planes);
	#endif	
		if (GMT_n_multiples > 0) fprintf (stderr, "%s: WARNING: %ld multiple modes found\n", GMT_program, GMT_n_multiples);
		
		h.nx = (int)nx_out;
		h.ny = (int)ny_out;
		h.x_min = west_new;
		h.x_max = east_new;
		h.y_min = south_new;
		h.y_max = north_new;
		h.x_inc = dx_new;
		h.y_inc = dy_new;
		h.node_offset = !one_or_zero;
		
		if (gmtdefs.verbose) fprintf (stderr, "%s: Write filtered grid\n", GMT_program);
		if (GMT_write_grd (fout, &h, output, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE)) {
			fprintf (stderr, "%s: Error writing file %s\n", GMT_program, fout);
			exit (EXIT_FAILURE);
		}
	#ifdef OBSOLETE						
		if (do_scale) {
			if (gmtdefs.verbose) fprintf (stderr, "%s: Write scale grid\n", GMT_program);
			if (GMT_write_grd (fout2, &h, output2, 0.0, 0.0, 0.0, 0.0, GMT_pad, FALSE)) {
				fprintf (stderr, "%s: Error writing file %s\n", GMT_program, fout2);
				exit (EXIT_FAILURE);
			}
		}
	#endif	
		if (gmtdefs.verbose) fprintf (stderr, "%s: Done\n", GMT_program);
		
		GMT_free ((void *) input);
		GMT_free ((void *) output);
		GMT_free ((void *) weight);
		GMT_free ((void *) i_origin);
		for (j = 0; j < ny_fil; j++) GMT_free ((void *) sector[j]);
		GMT_free ((void *) sector);
		GMT_free ((void *) value);
		GMT_free ((void *) wt_sum);
		GMT_free ((void *) n_in_median);
		if (slow) {
			for (j = 0; j < n_sectors; j++) {
				GMT_free ((void *) work_array[j]);
	#ifdef OBSOLETE								
				if (trend) {
					GMT_free ((void *) xx[j]);
					GMT_free ((void *) yy[j]);
				}
	#endif			
			}
			GMT_free ((void *) work_array);
			GMT_free ((void *) n_alloc);
	#ifdef OBSOLETE							
			if (do_scale) GMT_free ((void *) work_array2);
			if (trend) {
				GMT_free ((void *) xx);
				GMT_free ((void *) yy);
			}
	#endif		
		}
		if (!fast_way) GMT_free ((void *) x_shift);
		
	}
	else {
	   
	   /* check the crucial condition to run the program*/
	   if (!fin) {
		  fprintf (stderr, "%s: ERROR - Must specify input file\n", GMT_program);
		  exit (EXIT_FAILURE);
	   }
	   if (error) {
		  fprintf (stderr, "%s: ERROR - Must specify total # of columns in the input file\n", GMT_program);
		  exit (EXIT_FAILURE);
	   }
	   if (err_cols > 50) {
		  fprintf (stderr, "%s: ERROR - Total # of columns cannot exceed 50\n", GMT_program);
		  exit (EXIT_FAILURE);   
	   }
	   ip=fopen(fin,"r");

	   /* read depths from each column while EOF */
	   while(fscanf(ip,"%lf",&err_depth) != EOF) {
		  /* start with empty sum */
		  err_sum=0;
		  /* store data into array and find min/max */
		  err_workarray[0]=err_depth;
		  err_min=err_max=err_depth;
		  err_sum += err_depth;
		  for (i=1; i < err_cols; i++) {
			 not_used = fscanf(ip,"%lf",&err_depth);
			 err_workarray[i]=err_depth;
			 err_sum += err_depth;
		 if (err_depth < err_min) err_min=err_depth;
		 if (err_depth > err_max) err_max=err_depth;	 
		  }

		  /* calculate MEDIAN and MAD for each row */      
		  GMT_median (err_workarray, err_cols, err_min, err_max, err_null_median, &err_median);
		  err_workarray[0]=fabs(err_workarray[0]-err_median);
		  err_min=err_max=err_workarray[0];
		  for (i=1; i < err_cols; i++) {
			 err_workarray[i]=fabs(err_workarray[i]-err_median);
		 if (err_workarray[i] < err_min) err_min=err_workarray[i];
		 if (err_workarray[i] > err_max) err_max=err_workarray[i];	 
		  }      
		  GMT_median (err_workarray, err_cols, err_min, err_max, err_null_median, &err_mad);
		  err_mad=err_mad*1.482;
		  
		  /* calculate MEAN for each row */
		  err_mean = (err_cols) ? err_sum / err_cols : 0.0;
		  
		  /* print out the results */
		  printf("%f\t%f\t%f\n",err_median,err_mad,err_mean);
		  
	/* debug */
	/* fprintf (stderr,"line %i passed\n",l); */
	err_l=err_l+1;
	   }
	   /* close the input */
	   fclose(ip);
	}
	
	GMT_end (argc, argv);
		
	exit (EXIT_SUCCESS);
}

void	set_weight_matrix (GMT_LONG nx_f, GMT_LONG ny_f, double y_0, double north, double south, double dx, double dy, double f_wid, GMT_LONG f_flag, GMT_LONG d_flag, double x_off, double y_off, GMT_LONG fast)
/* Last two gives offset between output node and 'origin' input node for this window (0,0 for integral grids) */
/* TRUE when input/output grids are offset by integer values in dx/dy */
   	               
{

	GMT_LONG	i, j, ij, i_half, j_half;
	double	x_scl, y_scl, f_half, r_f_half, sigma, sig_2;
	double	y1, y2, theta, x, y, r, s_y1, c_y1, s_y2, c_y2;

	/* Set Scales  */

	y_scl = (d_flag) ? deg2km : 1.0;
	if (d_flag < 2)
		x_scl = y_scl;
	else if (d_flag == 2)
		x_scl = deg2km * cos (0.5 * D2R *(north + south));
	else
		x_scl = deg2km * cos (D2R * y_0);

	/* Get radius, weight, etc.  */

	i_half = nx_f / 2;
	j_half = ny_f / 2;
	f_half = 0.5 * f_wid;
	r_f_half = 1.0 / f_half;
	sigma = f_wid / 6.0;
	sig_2 = -0.5 / (sigma * sigma);
	for (i = -i_half; i <= i_half; i++) {
		for (j = -j_half; j <= j_half; j++) {
			ij = (j + j_half) * nx_f + i + i_half;
			if (fast && i == 0)
				r = (j == 0) ? 0.0 : j * y_scl * dy;
			else if (fast && j == 0)
				r = i * x_scl * dx;
			else if (d_flag < 4) {
				x = x_scl * (i * dx - x_off);
				y = y_scl * (j * dy - y_off);
				r = hypot (x, y);
			}
			else {
				theta = D2R * (i * dx - x_off);
				y1 = D2R * (90.0 - y_0);
				y2 = D2R * (90.0 - (y_0 + (j * dy - y_off)) );
				sincos (y1, &s_y1, &c_y1);
				sincos (y2, &s_y2, &c_y2);
				r = d_acos (c_y1 * c_y2 + s_y1 * s_y2 * cos(theta)) * deg2km * R2D;
			}
			/* Now we know r in f_wid units  */
			
			if (r > f_half) {
				weight[ij] = -1.0;
				continue;
			}
			else if (f_flag == 3) {
				weight[ij] = 1.0;
				continue;
			}
			else {
				if (f_flag == 0)
					weight[ij] = 1.0;
				else if (f_flag == 1)
					weight[ij] = 1.0 + cos (M_PI * r * r_f_half);
				else
					weight[ij] = exp (r * r * sig_2);
			}
		}
	}
}
